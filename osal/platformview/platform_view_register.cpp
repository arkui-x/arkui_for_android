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

#include "adapter/android/osal/platformview/platform_view_register.h"

#include "adapter/android/osal/platformview/platform_view_plugin.h"
#include "core/common/platformview/platform_view_proxy.h"

namespace OHOS::Ace::Platform {

bool PlatformViewRegister::Register()
{
    PlatformViewProxy::GetInstance().SetDelegate(std::make_unique<PlatformViewPlugin>());
    return true;
}

} // namespace OHOS::Ace::Platform
