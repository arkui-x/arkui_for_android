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

#include "core/common/modal_ui_extension.h"

namespace OHOS::Ace {

RefPtr<NG::FrameNode> ModalUIExtension::Create(const AAFwk::Want& want, const ModalUIExtensionCallbacks& callbacks,
    bool isAsyncModalBinding, bool isModal)
{
    return nullptr;
}

int32_t ModalUIExtension::GetSessionId(const RefPtr<NG::FrameNode>& uiExtNode)
{
    return 0;
}

void ModalUIExtension::SetBindModalCallback(const RefPtr<NG::FrameNode>& uiExtNode,
    std::function<void()>&& bindModalCallback)
{
}
} // namespace OHOS::Ace
