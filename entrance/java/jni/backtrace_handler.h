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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_BACKTRACE_HANDLER_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_BACKTRACE_HANDLER_H

#include <cstdint>

namespace OHOS::Ace::Platform {

// Installs async-signal-safe crash handlers (SIGSEGV/SIGABRT/SIGBUS/SIGILL/SIGFPE)
// that dump a frame-pointer based native backtrace. Idempotent; safe to call more
// than once. Designed to be invoked once from JNI_OnLoad of libarkui_android.so.
//
// The whole dump path is async-signal-safe: no malloc, no stdio, no locks, no
// HILOG. It reads /proc/self/maps through open()/read() and walks the FP chain
// (x29) with hand-rolled integer formatting, emitting via write(2) and
// __android_log_write. Any pre-existing handler (the runtime / signal chain /
// crash reporter) is saved and chained to, so a system crash report is still
// produced.
//
// The FP chain is only walked on aarch64; on other ABIs only the signal is
// reported. Stack frames that omit the frame pointer (JIT/JS, some third-party
// libs) terminate the chain, so the dump covers the native C++ caller frames
// around the crash rather than the full stack.
//
// Each printed frame is "<module>+0x<vaddr>", where vaddr = runtime_pc - the
// module's load base. That value is consumable by addr2line / llvm-symbolizer
// directly (point them at the matching unstripped .so), with no manual base
// correction needed.
// Multi-threading: the re-entrancy guard is process-wide, so if several threads
// crash concurrently only the first one dumps a backtrace; the others forward
// straight to the previous handler. This avoids racing the process-global maps
// model.
void InitBacktraceHandler();

// Dumps the frame-pointer chain starting at the given frame pointer, using the
// same async-signal-safe code path as the crash handler. Exposed so the walk can
// be triggered from a normal context (e.g. diagnostics or a forced-crash test).
// No-op when fp == 0 or on non-aarch64 ABIs.
void DumpBacktraceFromFp(uintptr_t fp);

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_BACKTRACE_HANDLER_H
