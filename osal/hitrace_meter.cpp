/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hitrace_meter.h"

#include <android/api-level.h>
#include <android/trace.h>
#include <atomic>
#include <mutex>
#include <string>

#include "hilog/log.h"
#include "securec.h"
#include "ui/base/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

ACE_WEAK_SYM void ATrace_beginAsyncSection(const char* name, int32_t taskId);
ACE_WEAK_SYM void ATrace_endAsyncSection(const char* name, int32_t taskId);
ACE_WEAK_SYM void ATrace_setCounter(const char *counterName, int64_t counterValue);

#ifdef __cplusplus
}
#endif

namespace {
std::atomic<int32_t> g_androidApiLevel(-1);
std::once_flag g_onceFlag;
constexpr int ANDROID_API_29 = 29; // ATrace section APIs are supported starting from Android API level 29
constexpr int VAR_NAME_MAX_SIZE = 400;

inline std::string FormatTraceName(const std::string& name)
{
    return std::string("H:") + name;
}

inline std::string FormatTraceName(const char* name)
{
    std::string ret("H:");
    if (name != nullptr && strlen(name) > 0) {
        ret.append(name);
    }
    return ret;
}

inline int32_t GetAndroidApiLevel()
{
    std::call_once(g_onceFlag, []() {
        g_androidApiLevel = android_get_device_api_level();
    });
    return g_androidApiLevel.load();
}

inline bool IsTraceLevelValid(HiTraceOutputLevel level)
{
    return (level >= HITRACE_LEVEL_DEBUG) && (level <= HITRACE_LEVEL_MAX);
}

inline bool IsTraceDisabled(uint64_t tag)
{
    return !IsTagEnabled(tag);
}
}; // namespace

ACE_FORCE_EXPORT void UpdateTraceLabel() {}

ACE_FORCE_EXPORT void SetTraceDisabled(bool disable) {}

ACE_FORCE_EXPORT void StartTrace(uint64_t tag, const std::string& name, float limit)
{
    if (!IsTraceDisabled(tag)) {
        ATrace_beginSection(FormatTraceName(name).c_str());
    }
}

ACE_FORCE_EXPORT void StartTraceEx(HiTraceOutputLevel level, uint64_t tag, const char* name, const char* customArgs)
{
    if (IsTraceLevelValid(level) && !IsTraceDisabled(tag)) {
        ATrace_beginSection(FormatTraceName(name).c_str());
    }
}

ACE_FORCE_EXPORT void StartTraceDebug(bool isDebug, uint64_t tag, const std::string& name, float limit) {}

ACE_FORCE_EXPORT void StartTraceArgs(uint64_t tag, const char* fmt, ...) {}

ACE_FORCE_EXPORT void StartTraceArgsDebug(bool isDebug, uint64_t tag, const char* fmt, ...) {}

ACE_FORCE_EXPORT void StartTraceWrapper(uint64_t tag, const char* name) {}

ACE_FORCE_EXPORT void FinishTrace(uint64_t tag)
{
    if (!IsTraceDisabled(tag)) {
        ATrace_endSection();
    }
}

ACE_FORCE_EXPORT void FinishTraceEx(HiTraceOutputLevel level, uint64_t tag)
{
    if (IsTraceLevelValid(level) && !IsTraceDisabled(tag)) {
        ATrace_endSection();
    }
}

ACE_FORCE_EXPORT void FinishTraceDebug(bool isDebug, uint64_t tag) {}

ACE_FORCE_EXPORT void StartAsyncTrace(uint64_t tag, const std::string& name, int32_t taskId, float limit)
{
    if (!IsTraceDisabled(tag) && (GetAndroidApiLevel() >= ANDROID_API_29)) {
        ATrace_beginAsyncSection(FormatTraceName(name).c_str(), taskId);
    }
}

ACE_FORCE_EXPORT void StartAsyncTraceEx(HiTraceOutputLevel level, uint64_t tag, const char* name, int32_t taskId,
    const char* customCategory, const char* customArgs)
{
    if (IsTraceLevelValid(level) && !IsTraceDisabled(tag) && (GetAndroidApiLevel() >= ANDROID_API_29)) {
        ATrace_beginAsyncSection(FormatTraceName(name).c_str(), taskId);
    }
}

ACE_FORCE_EXPORT void StartAsyncTraceDebug(bool isDebug, uint64_t tag, const std::string& name, int32_t taskId,
    float limit) {}

ACE_FORCE_EXPORT void StartAsyncTraceArgs(uint64_t tag, int32_t taskId, const char* fmt, ...) {}

ACE_FORCE_EXPORT void StartAsyncTraceArgsDebug(bool isDebug, uint64_t tag, int32_t taskId, const char* fmt, ...) {}

ACE_FORCE_EXPORT void StartAsyncTraceWrapper(uint64_t tag, const char* name, int32_t taskId) {}

ACE_FORCE_EXPORT void StartTraceChain(uint64_t tag, const struct HiTraceIdStruct* hiTraceId, const char* name) {}

ACE_FORCE_EXPORT void FinishAsyncTrace(uint64_t tag, const std::string& name, int32_t taskId)
{
    if (!IsTraceDisabled(tag) && (GetAndroidApiLevel() >= ANDROID_API_29)) {
        ATrace_endAsyncSection(FormatTraceName(name).c_str(), taskId);
    }
}

ACE_FORCE_EXPORT void FinishAsyncTraceEx(HiTraceOutputLevel level, uint64_t tag, const char* name, int32_t taskId)
{
    if (IsTraceLevelValid(level) && !IsTraceDisabled(tag) && (GetAndroidApiLevel() >= ANDROID_API_29)) {
        ATrace_endAsyncSection(FormatTraceName(name).c_str(), taskId);
    }
}

ACE_FORCE_EXPORT void FinishAsyncTraceDebug(bool isDebug, uint64_t tag, const std::string& name, int32_t taskId) {}

ACE_FORCE_EXPORT void FinishAsyncTraceArgs(uint64_t tag, int32_t taskId, const char* fmt, ...) {}

ACE_FORCE_EXPORT void FinishAsyncTraceArgsDebug(bool isDebug, uint64_t tag, int32_t taskId, const char* fmt, ...) {}

ACE_FORCE_EXPORT void FinishAsyncTraceWrapper(uint64_t tag, const char* name, int32_t taskId) {}

ACE_FORCE_EXPORT void MiddleTrace(uint64_t tag, const std::string& beforeValue, const std::string& afterValue) {}

ACE_FORCE_EXPORT void MiddleTraceDebug(bool isDebug, uint64_t tag, const std::string& beforeValue,
    const std::string& afterValue) {}

ACE_FORCE_EXPORT void CountTrace(uint64_t tag, const std::string& name, int64_t count)
{
    if (!IsTraceDisabled(tag) && (GetAndroidApiLevel() >= ANDROID_API_29)) {
        ATrace_setCounter(FormatTraceName(name).c_str(), count);
    }
}

ACE_FORCE_EXPORT void CountTraceEx(HiTraceOutputLevel level, uint64_t tag, const char* name, int64_t count)
{
    if (IsTraceLevelValid(level) && !IsTraceDisabled(tag) && (GetAndroidApiLevel() >= ANDROID_API_29)) {
        ATrace_setCounter(FormatTraceName(name).c_str(), count);
    }
}

ACE_FORCE_EXPORT void CountTraceDebug(bool isDebug, uint64_t tag, const std::string& name, int64_t count) {}

ACE_FORCE_EXPORT void CountTraceWrapper(uint64_t tag, const char* name, int64_t count) {}

ACE_FORCE_EXPORT bool IsTagEnabled(uint64_t tag)
{
    return (tag > 0) && ATrace_isEnabled();
}

ACE_FORCE_EXPORT void ParseTagBits(const uint64_t tag, std::string& bitStrs) {}

ACE_FORCE_EXPORT int StartCaptureAppTrace(TraceFlag flag, uint64_t tags, uint64_t limitSize, std::string& fileName)
{
    return 0;
}

ACE_FORCE_EXPORT int StopCaptureAppTrace(void)
{
    return 0;
}

ACE_FORCE_EXPORT HitracePerfScoped::HitracePerfScoped(bool isDebug, uint64_t tag, const std::string& name)
    : mTag_(tag), mName_(name) {}

ACE_FORCE_EXPORT HitracePerfScoped::~HitracePerfScoped() {}

ACE_FORCE_EXPORT HitraceMeterFmtScoped::HitraceMeterFmtScoped(uint64_t tag, const char* fmt, ...) : mTag(tag)
{
    if (IsTraceDisabled(tag)) {
        return;
    }

    char name[VAR_NAME_MAX_SIZE] = { 0 };
    va_list args;
    va_start(args, fmt);
    int res = vsnprintf_s(name, sizeof(name), sizeof(name) - 1, fmt, args);
    va_end(args);
    if (res < 0) {
        HILOG_ERROR(LOG_CORE, "vsnprintf_s failed: %{public}d, name: %{public}s", errno, fmt);
        return;
    }

    ATrace_beginSection(FormatTraceName(name).c_str());
}
