/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "base/log/log_wrapper.h"

#include <android/log.h>

#ifdef _GNU_SOURCE
#include <dlfcn.h>
#endif
#include <mutex>

#ifdef ACE_INSTANCE_LOG
#include "core/common/container.h"
#endif

namespace OHOS::Ace {
namespace {

constexpr int LOG_LEVEL[] = { ANDROID_LOG_DEBUG, ANDROID_LOG_INFO, ANDROID_LOG_WARN, ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL };

const char* LOG_TAGS[] = {
    "Ace",
    "JSApp",
};

#ifdef ACE_INSTANCE_LOG
constexpr const char* INSTANCE_ID_GEN_REASONS[] = {
    "scope",
    "active",
    "default",
    "singleton",
    "foreground",
    "undefined",
};
#endif

} // namespace

// Initialize the static member object
LogLevel LogWrapper::level_ = LogLevel::DEBUG;

char LogWrapper::GetSeparatorCharacter()
{
    return '/';
}

void LogWrapper::PrintLog(LogDomain domain, LogLevel level, AceLogTag tag, const char* fmt, va_list args)
{
    std::string newFmt(fmt);
    StripFormatString("{public}", newFmt);
    StripFormatString("{private}", newFmt);
    __android_log_vprint(
        LOG_LEVEL[static_cast<int>(level)], LOG_TAGS[static_cast<uint32_t>(domain)], newFmt.c_str(), args);
}

#ifdef ACE_INSTANCE_LOG
int32_t LogWrapper::GetId()
{
    return Container::CurrentId();
}

const std::string LogWrapper::GetIdWithReason()
{
    int32_t currentId = ContainerScope::CurrentId();
    std::pair<int32_t, InstanceIdGenReason> idWithReason = ContainerScope::CurrentIdWithReason();
    return std::to_string(currentId) + ":" + std::to_string(idWithReason.first) + ":" +
           INSTANCE_ID_GEN_REASONS[static_cast<uint32_t>(idWithReason.second)];
}
#endif

bool LogBacktrace(size_t maxFrameNums)
{
    static const char* (*pfnGetTrace)(size_t, size_t);
#ifdef _GNU_SOURCE
    if (!pfnGetTrace) {
        pfnGetTrace = (decltype(pfnGetTrace))dlsym(RTLD_DEFAULT, "GetTrace");
    }
#endif
    if (!pfnGetTrace) {
        return false;
    }

    static std::mutex mtx;
    std::lock_guard lock(mtx);
    size_t skipFrameNum = 2;
    LOGI("Backtrace: skipFrameNum=%{public}zu maxFrameNums=%{public}zu\n%{public}s",
        skipFrameNum, maxFrameNums, pfnGetTrace(skipFrameNum, maxFrameNums));
    return true;
}

} // namespace OHOS::Ace