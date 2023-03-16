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

#include "adapter/android/capability/java/jni/grantresult/grant_result_manager.h"

namespace OHOS::Ace::Platform {
GrantResult GrantResultManager::grantResult_ = nullptr;
void GrantResultManager::JSRegisterGrantResult(GrantResult callback)
{
    grantResult_ = callback;
}

void GrantResultManager::OnRequestPremissionCallback(
    const std::vector<std::string>& permissions, const std::vector<int>& grantResults)
{
    if (grantResult_) {
        grantResult_(permissions, grantResults);
        grantResult_ = nullptr;
    }
}
} // namespace OHOS::Ace::Platform