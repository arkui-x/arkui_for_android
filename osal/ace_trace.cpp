/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 
#include <android/trace.h>

#include "base/log/ace_trace.h"
#include "base/utils/macros.h"

namespace OHOS::Ace {

bool AceTraceEnabled() {
    return ATrace_isEnabled();
}

bool AceTraceBegin(const char* name) {
    if (name != nullptr) {
        ATrace_beginSection(name);
    } else {
        ATrace_beginSection("");
    }
}

void AceTraceEnd() {
    ATrace_endSection();
}

} // namespace OHOS::Ace