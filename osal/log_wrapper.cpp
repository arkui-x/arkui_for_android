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

int32_t LogWrapper::GetId()
{
#ifdef ACE_INSTANCE_LOG
    return Container::CurrentId();
#else
    return 0;
#endif
}

} // namespace OHOS::Ace