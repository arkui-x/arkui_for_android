/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "backtrace_handler.h"

#include <android/log.h>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <fcntl.h>
#include <ucontext.h>
#include <unistd.h>

#include <atomic>

// Engine log. Only used from the normal (non-signal) context, e.g. inside
// InitBacktraceHandler(). NEVER call LOG* from within the signal handler:
// HILOG routes through IPC/locks/malloc and is not async-signal-safe.
#include "base/log/log.h"

namespace OHOS::Ace::Platform {
namespace {

constexpr const char* LOG_TAG = "ArkUI-x";

// ---------------------------------------------------------------------------
// Tunables
// ---------------------------------------------------------------------------
constexpr uint32_t MAX_STACK_DEPTH = 64;      // hard cap on walked frames
constexpr uintptr_t INSTRUCTION_LENGTH = 4;   // aarch64 fixed-width instruction
constexpr int HEX_SHIFT = 4;                  // bits per hex digit (AppendHex/ParseHex)
constexpr int DECIMAL_BASE = 10;              // base for decimal formatting (AppendDec/ParseHex)
constexpr size_t FRAME_POINTER_SLOTS = 2;     // [fp]=x29, [fp+8]=x30: one frame spans 2 pointers
// An ArkUI-X process loads dozens of .so plus JIT/scudo regions and per-thread
// stacks, so /proc/self/maps reaches several thousand entries. The original 768
// (and even 2048) truncated the stack/so mappings and lost frames / module
// names; 4096 covers the measured ~3900 entries with headroom.
constexpr size_t MAX_MAP_RECORDS = 4096;
// 1MB; the original 256KB cannot hold the full maps text such a process emits.
constexpr size_t MAPS_FILE_BUF_SIZE = 1024 * 1024;
constexpr size_t DUMP_BUF_SIZE = 8192;
constexpr size_t FLUSH_THRESHOLD = 256;       // flush when fewer than this many bytes remain
constexpr size_t ALT_STACK_SIZE = 64 * 1024;
constexpr size_t LOG_LINE_SIZE = 512;         // per-line buffer used when emitting to the system log

#if defined(__aarch64__)
constexpr int REG_FP = 29; // x29 is the frame pointer on AArch64
#endif

// ---------------------------------------------------------------------------
// /proc/self/maps model (plain data, no heap, safe to touch in a handler)
// ---------------------------------------------------------------------------
struct MapRecord {
    uintptr_t start;
    uintptr_t end;
    uintptr_t offset;
    bool executable;        // permission has 'x' (i.e. may hold return addresses)
    const char* name;       // points into g_mapsFileBuf; full length, never truncated
};

// File-scope statics: kept off the (possibly tiny alternate) signal stack and
// reused across crashes. Reuse is serialized by g_inHandler below.
MapRecord g_maps[MAX_MAP_RECORDS];
size_t g_mapCount = 0;
char g_mapsFileBuf[MAPS_FILE_BUF_SIZE];

// ---------------------------------------------------------------------------
// Output buffer + async-signal-safe append helpers (zero allocation)
// ---------------------------------------------------------------------------
struct DumpBuffer {
    char data[DUMP_BUF_SIZE];
    size_t len;
};

void AppendCh(DumpBuffer& buf, char ch)
{
    if (buf.len < DUMP_BUF_SIZE - 1) {
        buf.data[buf.len++] = ch;
    }
}

void AppendLit(DumpBuffer& buf, const char* text)
{
    while (*text != '\0') {
        AppendCh(buf, *text++);
    }
}

void AppendStr(DumpBuffer& buf, const char* text)
{
    if (text == nullptr) {
        return;
    }
    while (*text != '\0') {
        AppendCh(buf, *text++);
    }
}

void AppendHex(DumpBuffer& buf, uintptr_t value)
{
    static const char* digits = "0123456789abcdef";
    char tmp[2 * sizeof(uintptr_t)];
    int i = 0;
    if (value == 0) {
        AppendCh(buf, '0');
        return;
    }
    while (value != 0) {
        tmp[i++] = digits[value & 0xf];
        value >>= HEX_SHIFT;
    }
    while (i > 0) {
        AppendCh(buf, tmp[--i]);
    }
}

void AppendHex0x(DumpBuffer& buf, uintptr_t value)
{
    AppendLit(buf, "0x");
    AppendHex(buf, value);
}

void AppendDec(DumpBuffer& buf, uint32_t value)
{
    char tmp[10];
    int i = 0;
    if (value == 0) {
        AppendCh(buf, '0');
        return;
    }
    while (value != 0) {
        tmp[i++] = '0' + (value % DECIMAL_BASE);
        value /= DECIMAL_BASE;
    }
    while (i > 0) {
        AppendCh(buf, tmp[--i]);
    }
}

// Signed-decimal variant for values that may legitimately be negative on Linux,
// e.g. siginfo_t::si_code (SI_TKILL == -6 for the SIGABRT raised by
// abort()/raise()/pthread_kill()). AppendDec only formats the unsigned magnitude.
void AppendDecSigned(DumpBuffer& buf, int32_t value)
{
    uint32_t magnitude;
    if (value < 0) {
        AppendCh(buf, '-');
        // Negate without UB: -(value + 1) + 1 is safe even for INT32_MIN.
        magnitude = static_cast<uint32_t>(-(value + 1)) + 1U;
    } else {
        magnitude = static_cast<uint32_t>(value);
    }
    AppendDec(buf, magnitude);
}

// Writes the whole buffer to stderr (async-signal-safe) and additionally routes
// each line to the system log via __android_log_write, the lightweight (no
// varargs formatting) logging call. Resets buf.len to 0 afterwards, so deep
// backtraces can batch-flush without overflowing the fixed-size buffer.
void FlushDump(DumpBuffer& buf)
{
    ssize_t written = 0;
    while (written < static_cast<ssize_t>(buf.len)) {
        ssize_t bytes = write(STDERR_FILENO, buf.data + written, buf.len - written);
        if (bytes > 0) {
            written += bytes;
        } else if (bytes < 0 && errno == EINTR) {
            continue;
        } else {
            break; // give up silently; we are crashing anyway
        }
    }

#ifdef __ANDROID__
    // Emit line-by-line so the system log shows one readable entry per frame.
    size_t lineStart = 0;
    char line[LOG_LINE_SIZE];
    for (size_t i = 0; i <= buf.len; ++i) {
        if (i == buf.len || buf.data[i] == '\n') {
            size_t lineLen = i - lineStart;
            size_t cap = sizeof(line) - 1;
            size_t copyLen = lineLen < cap ? lineLen : cap;
            for (size_t k = 0; k < copyLen; ++k) {
                line[k] = buf.data[lineStart + k];
            }
            line[copyLen] = '\0';
            if (copyLen > 0) {
                __android_log_write(ANDROID_LOG_FATAL, LOG_TAG, line);
            }
            lineStart = i + 1;
        }
    }
#endif
    buf.len = 0; // allow batch flushing from DumpBacktrace
}

// ---------------------------------------------------------------------------
// /proc/self/maps parsing (async-signal-safe: open/read/close only)
// ---------------------------------------------------------------------------
bool ParseHex(const char*& cursor, const char* end, uintptr_t& out)
{
    uintptr_t value = 0;
    bool found = false;
    while (cursor < end) {
        char ch = *cursor;
        int digit;
        if (ch >= '0' && ch <= '9') {
            digit = ch - '0';
        } else if (ch >= 'a' && ch <= 'f') {
            digit = DECIMAL_BASE + (ch - 'a');
        } else if (ch >= 'A' && ch <= 'F') {
            digit = DECIMAL_BASE + (ch - 'A');
        } else {
            break;
        }
        value = (value << HEX_SHIFT) | static_cast<uintptr_t>(digit);
        found = true;
        ++cursor;
    }
    out = value;
    return found;
}

const char* SkipSpaces(const char* cursor, const char* end)
{
    while (cursor < end && (*cursor == ' ' || *cursor == '\t')) {
        ++cursor;
    }
    return cursor;
}

// Advances past one whitespace-delimited token, returning its end pointer.
const char* SkipToken(const char* cursor, const char* end)
{
    while (cursor < end && *cursor != ' ' && *cursor != '\t' && *cursor != '\n') {
        ++cursor;
    }
    return cursor;
}

// Parses one maps line in [line, end). Returns false for malformed lines.
// `line` is mutable so the caller's buffer can be null-terminated in place.
bool ParseMapLine(char* line, const char* end, MapRecord& rec)
{
    const char* cursor = line;

    uintptr_t start = 0;
    if (!ParseHex(cursor, end, start)) {
        return false;
    }
    if (cursor < end && *cursor == '-') {
        ++cursor;
    }

    uintptr_t mapEnd = 0;
    if (!ParseHex(cursor, end, mapEnd)) {
        return false;
    }
    if (mapEnd <= start) {
        return false;
    }

    cursor = SkipSpaces(cursor, end);

    // Permissions token, e.g. "r-xp".
    const char* perms = cursor;
    cursor = SkipToken(cursor, end);
    bool isExec = false;
    for (const char* ch = perms; ch < cursor; ++ch) {
        if (*ch == 'x') {
            isExec = true;
            break;
        }
    }
    cursor = SkipSpaces(cursor, end);

    uintptr_t offset = 0;
    ParseHex(cursor, end, offset); // file offset (may be absent for some entries)

    // Skip dev token and inode token.
    cursor = SkipSpaces(cursor, end);
    cursor = SkipToken(cursor, end);
    cursor = SkipSpaces(cursor, end);
    cursor = SkipToken(cursor, end);
    cursor = SkipSpaces(cursor, end);

    // Remainder of the line is the pathname. Point directly into g_mapsFileBuf
    // (no fixed-size copy, so the path is never truncated) and null-terminate it
    // in place by overwriting the trailing '\n' (the buffer is ours/mutable).
    rec.name = cursor;
    while (cursor < end && *cursor != '\n') {
        ++cursor;
    }
    line[cursor - line] = '\0'; // null-terminate the name in place (buffer is mutable)

    rec.start = start;
    rec.end = mapEnd;
    rec.offset = offset;
    rec.executable = isExec;
    return true;
}

void LoadMaps()
{
    g_mapCount = 0;
    int fd = open("/proc/self/maps", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return;
    }

    size_t total = 0;
    // Leave one byte so ParseMapLine can always null-terminate a name in place.
    while (total < MAPS_FILE_BUF_SIZE - 1) {
        ssize_t bytes = read(fd, g_mapsFileBuf + total, MAPS_FILE_BUF_SIZE - 1 - total);
        if (bytes > 0) {
            total += static_cast<size_t>(bytes);
        } else if (bytes == 0) {
            break;
        } else if (errno == EINTR) {
            continue;
        } else {
            break;
        }
    }
    close(fd);

    char* cursor = g_mapsFileBuf;
    char* fileEnd = g_mapsFileBuf + total;
    while (cursor < fileEnd && g_mapCount < MAX_MAP_RECORDS) {
        char* lineEnd = cursor;
        while (lineEnd < fileEnd && *lineEnd != '\n') {
            ++lineEnd;
        }
        if (ParseMapLine(cursor, lineEnd, g_maps[g_mapCount])) {
            ++g_mapCount;
        }
        cursor = (lineEnd < fileEnd) ? lineEnd + 1 : fileEnd;
    }
}

const MapRecord* FindRecord(uintptr_t addr)
{
    for (size_t i = 0; i < g_mapCount; ++i) {
        const MapRecord& rec = g_maps[i];
        if (addr >= rec.start && addr < rec.end) {
            return &rec;
        }
    }
    return nullptr;
}

bool NamesMatch(const char* lhs, const char* rhs)
{
    if (lhs == nullptr || rhs == nullptr) {
        return false;
    }
    while (*lhs != '\0' && *rhs != '\0') {
        if (*lhs != *rhs) {
            return false;
        }
        ++lhs;
        ++rhs;
    }
    return *lhs == *rhs; // equal only if both ended together
}

// Returns the load base (runtime address of vaddr 0) of the shared object that
// owns `mod`: the mapping with file offset 0 and the SAME name. We match by
// name (MapRecord.name points into the raw maps text, so it is never truncated)
// because an address-only heuristic fails: /proc/self/maps also contains huge
// anonymous regions (scudo reserves, heap) whose offset is 0, and those would
// be picked instead of the library's own base.
uintptr_t FindLoadBase(const MapRecord* mod)
{
    if (mod == nullptr || mod->name[0] == '\0') {
        return 0;
    }
    uintptr_t best = 0;
    for (size_t i = 0; i < g_mapCount; ++i) {
        const MapRecord& rec = g_maps[i];
        if (rec.offset == 0 && rec.start <= mod->start && rec.start > best && NamesMatch(rec.name, mod->name)) {
            best = rec.start;
        }
    }
    return best;
}

// ---------------------------------------------------------------------------
// Frame-pointer chain walk + dump
// ---------------------------------------------------------------------------
void DumpSignalHeader(DumpBuffer& buf, int sig, siginfo_t* info)
{
    AppendLit(buf, "\n================ ArkUI-x native crash ================\n");
    AppendLit(buf, "Fatal signal ");
    AppendDec(buf, static_cast<uint32_t>(sig));
    AppendLit(buf, " (");
    switch (sig) {
        case SIGSEGV: AppendLit(buf, "SIGSEGV"); break;
        case SIGABRT: AppendLit(buf, "SIGABRT"); break;
        case SIGBUS:  AppendLit(buf, "SIGBUS"); break;
        case SIGILL:  AppendLit(buf, "SIGILL"); break;
        case SIGFPE:  AppendLit(buf, "SIGFPE"); break;
        default:      AppendLit(buf, "?"); break;
    }
    AppendLit(buf, "), si_code ");
    AppendDecSigned(buf, info ? info->si_code : 0);
    if (info != nullptr && info->si_addr != nullptr) {
        AppendLit(buf, ", fault addr ");
        AppendHex0x(buf, reinterpret_cast<uintptr_t>(info->si_addr));
    }
    AppendCh(buf, '\n');
}

void DumpBacktrace(DumpBuffer& buf, uintptr_t fp)
{
    AppendLit(buf, "Backtrace (frame-pointer chain):\n");

#if defined(__aarch64__)
    if (fp == 0) {
        AppendLit(buf, "  fp == 0, cannot walk the chain\n");
        return;
    }

    // The mapping holding the initial fp is (one of) the stack mapping(s); use it
    // to bound the walk so a corrupted chain cannot run away. If unknown, fall
    // back to the monotonic-up check plus the depth cap.
    const MapRecord* stack = FindRecord(fp);
    const uintptr_t stackLo = stack ? stack->start : 0;
    const uintptr_t stackHi = stack ? stack->end : 0;

    for (uint32_t idx = 0; idx < MAX_STACK_DEPTH; ++idx) {
        // The whole frame [fp, fp + 2*sizeof(uintptr_t)) must lie inside the
        // stack mapping before we dereference it. Checking only `fp < stackHi`
        // is not enough: the two 8-byte reads at fp/fp+8 would still run past
        // the mapping edge when fp sits in the last 16 bytes and hit a guard
        // page, re-faulting inside the handler. When fp does not fall in any
        // known stack mapping (nullptr) there is no safe bound to rely on, so
        // stop the walk rather than dereference blindly.
        if (stack != nullptr) {
            if (fp < stackLo || fp + FRAME_POINTER_SLOTS * sizeof(uintptr_t) > stackHi) {
                break;
            }
        } else {
            // fp did not land in any parsed mapping (e.g. the stack mapping was
            // truncated by MAX_MAP_RECORDS). Rather than give up the whole
            // backtrace, do a minimal sanity check on the AArch64 16-byte frame
            // alignment and attempt the read. A second fault here is still caught
            // by the g_inHandler re-entrancy guard.
            if (fp == 0 || (fp & 0xf) != 0) {
                break;
            }
        }

        // Frame layout produced by the AArch64 prologue (stp x29, x30):
        //   [fp + 0] = saved x29 (link to the previous frame)
        //   [fp + 8] = saved x30 (return address / LR)
        uintptr_t lr = *reinterpret_cast<uintptr_t*>(fp + 8);
        uintptr_t next = *reinterpret_cast<uintptr_t*>(fp);
        uintptr_t callSite = (lr != 0) ? lr - INSTRUCTION_LENGTH : 0;

        // A deep native stack can hold more frames than DUMP_BUF_SIZE; flush
        // and reuse the buffer before appending this frame so frames are never
        // silently dropped. FlushDump is async-signal-safe and resets buf.len.
        if (buf.len > DUMP_BUF_SIZE - FLUSH_THRESHOLD) {
            FlushDump(buf);
        }

        AppendLit(buf, "  #");
        AppendDec(buf, idx);
        AppendLit(buf, " pc=");
        AppendHex0x(buf, callSite);

        const MapRecord* mod = FindRecord(callSite);
        if (mod != nullptr) {
            AppendLit(buf, " ");
            AppendStr(buf, mod->name);
            AppendLit(buf, " +0x");
            // Print the vaddr = callSite - load base, which addr2line /
            // llvm-symbolizer consume directly (no manual +base correction).
            // Fall back to the mapping-relative offset if the load base is unknown.
            uintptr_t loadBase = FindLoadBase(mod);
            uintptr_t relOffset = (loadBase != 0) ? (callSite - loadBase) : (callSite - mod->start);
            AppendHex(buf, relOffset);
        }
        AppendCh(buf, '\n');

        // The saved fp must point upward (stack grows down); otherwise the
        // chain has ended or is corrupted. The full [next, next+16) range is
        // re-checked against the stack mapping at the top of the next iteration.
        if (next <= fp) {
            break;
        }
        fp = next;
    }
#else
    (void)fp; // FP-chain walk is aarch64-only; nothing to traverse on this ABI.
    AppendLit(buf, "  FP-chain walk is only implemented on aarch64 (arch skipped)\n");
#endif
}

// ---------------------------------------------------------------------------
// Signal handler
// ---------------------------------------------------------------------------
std::atomic<bool> g_installed { false };
// Process-wide (NOT thread_local) on purpose. While one thread is dumping
// (reading /proc/self/maps, walking the FP chain, writing the system log), any other
// thread that crashes skips its own dump and just forwards to the previous
// handler. This is a deliberate trade-off: the maps model (g_maps /
// g_mapsFileBuf) is process-global, so letting two threads dump concurrently
// would race and corrupt it (and risk a second fault). Consequence: in a
// multi-threaded crash, only the first crashing thread emits a backtrace.
std::atomic<bool> g_inHandler { false };
struct sigaction g_savedHandlers[NSIG];

// Hands control back to whatever handler (if any) was installed before us, and
// ensures a SIG_DFL disposition still terminates with a system crash report.
void ForwardToPrevious(int sig, siginfo_t* info, void* context)
{
    const struct sigaction& prev = g_savedHandlers[sig];
    if (prev.sa_flags & SA_SIGINFO) {
        if (prev.sa_sigaction != nullptr) {
            prev.sa_sigaction(sig, info, context);
            return;
        }
    } else if (prev.sa_handler != SIG_DFL && prev.sa_handler != SIG_IGN && prev.sa_handler != SIG_ERR) {
        prev.sa_handler(sig);
        return;
    }

    // SIG_DFL / SIG_IGN / nothing usable: restore the prior disposition and
    // re-raise so the default action (crash report/coredump) runs.
    sigaction(sig, &prev, nullptr);
    if (prev.sa_handler != SIG_IGN) {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, sig);
        sigprocmask(SIG_UNBLOCK, &mask, nullptr);
        raise(sig);
    }
}

void CrashHandler(int sig, siginfo_t* info, void* context)
{
    // Guard against re-entrancy (a fault while dumping). If we are already inside
    // the handler, skip the dump and just forward.
    bool expected = false;
    if (!g_inHandler.compare_exchange_strong(expected, true)) {
        ForwardToPrevious(sig, info, context);
        return;
    }

    uintptr_t fp = 0;
#if defined(__aarch64__)
    if (context != nullptr) {
        auto* uc = static_cast<ucontext_t*>(context);
        fp = uc->uc_mcontext.regs[REG_FP]; // saved x29 of the interrupted frame
    }
#endif

    DumpBuffer dump {};
    DumpSignalHeader(dump, sig, info);
    LoadMaps();
    DumpBacktrace(dump, fp);
    AppendLit(dump, "======================================================\n");
    FlushDump(dump);

    g_inHandler.store(false);

    ForwardToPrevious(sig, info, context);
}

// ---------------------------------------------------------------------------
// Installation (runs in normal context)
// ---------------------------------------------------------------------------
void InstallAltStack()
{
    // Static so the buffer outlives the call. sigaltstack is per-thread on the
    // system libc, so this protects the thread that runs JNI_OnLoad; other
    // threads fall back to their own stack (stack-overflow on them may not be
    // caught, ordinary SEGV/ABRT still are).
    static char altStack[ALT_STACK_SIZE];
    stack_t sigStack {};
    sigStack.ss_sp = altStack;
    sigStack.ss_size = sizeof(altStack);
    sigStack.ss_flags = 0;
    sigaltstack(&sigStack, nullptr);
}

} // namespace

void InitBacktraceHandler()
{
    bool expected = false;
    if (!g_installed.compare_exchange_strong(expected, true)) {
        return; // already installed
    }

    InstallAltStack();

    struct sigaction sa {};
    sa.sa_sigaction = CrashHandler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK; // SA_ONSTACK -> run on the alternate stack
    sigemptyset(&sa.sa_mask);

    const int handledSignals[] = { SIGSEGV, SIGABRT, SIGBUS, SIGILL, SIGFPE };
    for (int sig : handledSignals) {
        struct sigaction old {};
        if (sigaction(sig, &sa, &old) == 0) {
            g_savedHandlers[sig] = old;
        }
    }

    LOGI("ArkUI-x backtrace handler installed.");
}

void DumpBacktraceFromFp(uintptr_t fp)
{
    bool expected = false;
    if (!g_inHandler.compare_exchange_strong(expected, true)) {
        return; // a crash dump is in progress on this thread
    }

    DumpBuffer dump {};
    LoadMaps();
    DumpBacktrace(dump, fp);
    AppendLit(dump, "(end of manual backtrace)\n");
    FlushDump(dump);

    g_inHandler.store(false);
}

} // namespace OHOS::Ace::Platform
