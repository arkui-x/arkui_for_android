/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>

#include "base/thread/thread_priority.h"

namespace OHOS::Ace {
namespace {
constexpr int32_t BACKGROUND_THREAD_PRIORITY = 15;
} // namespace

void ThreadPriority::SetThreadPriority(OHOS::Ace::TaskExecutor::TaskType taskType)
{
    if (taskType == OHOS::Ace::TaskExecutor::TaskType::BACKGROUND) {
        SetBackGroundThreadPriority();
    }
}

void ThreadPriority::SetBackGroundThreadPriority()
{
    if (setpriority(PRIO_PROCESS, gettid(), BACKGROUND_THREAD_PRIORITY) < 0) {
        LOGW("Failed to set thread priority using setpriority: %s", strerror(errno));
    }
}
} // namespace OHOS::Ace
