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

#ifndef FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_GRANT_RESULT_MANAGER_H
#define FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_GRANT_RESULT_MANAGER_H

#include <functional>
#include <string>
#include <vector>

#include "base/utils/macros.h"

namespace OHOS::Ace::Platform {
using GrantResult =
    std::function<void(const std::vector<std::string>& permissions, const std::vector<int>& grantResults)>;
class ACE_EXPORT GrantResultManager final {
public:
    GrantResultManager() = default;
    ~GrantResultManager() = default;

    static void JSRegisterGrantResult(GrantResult callback);
    static void OnRequestPremissionCallback(
        const std::vector<std::string>& permissions, const std::vector<int>& grantResults);

private:
    static GrantResult grantResult_;
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_GRANT_RESULT_MANAGER_H