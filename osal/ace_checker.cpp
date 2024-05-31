/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "base/log/ace_checker.h"

namespace OHOS::Ace {
int32_t AceChecker::pageNodes_ = 0;
int32_t AceChecker::pageDepth_ = 0;
int32_t AceChecker::nodeChildren_ = 0;
int32_t AceChecker::functionTimeout_ = 0;
int32_t AceChecker::vsyncTimeout_ = 0;
int32_t AceChecker::nodeTimeout_ = 0;
int32_t AceChecker::foreachItems_ = 0;
int32_t AceChecker::flexLayouts_ = 0;

bool AceChecker::IsPerformanceCheckEnabled()
{
    return false;
}

void AceChecker::NotifyCaution(const std::string& tag) {}

void AceChecker::InitPerformanceParameters() {}
} // namespace OHOS::Ace
