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

#include "base/log/ace_trace.h"

#include <android/trace.h>

#include "base/utils/macros.h"

namespace OHOS::Ace {

void AceCountTrace(const char *key, int32_t count){}

bool AceTraceEnabled()
{
    return ATrace_isEnabled();
}

void AceTraceBegin(const char* name)
{
    if (name != nullptr) {
        ATrace_beginSection(name);
    } else {
        ATrace_beginSection("");
    }
}

void AceTraceEnd()
{
    ATrace_endSection();
}

void AceAsyncTraceBegin(int32_t taskId, const char* name, bool isAnimationTrace) {}

void AceAsyncTraceEnd(int32_t taskId, const char* name, bool isAnimationTrace) {}

void AceAsyncTraceBeginCommercial(int32_t taskId, const char* name, bool isAnimationTrace) {}
void AceAsyncTraceEndCommercial(int32_t taskId, const char* name, bool isAnimationTrace) {}

void AceTraceBeginCommercial(const char* name) {}
void AceTraceEndCommercial() {}

} // namespace OHOS::Ace
