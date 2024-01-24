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

#include "adapter/android/entrance/java/jni/msdp/interaction_impl.h"
#include "core/common/interaction/interaction_data.h"

namespace OHOS::Ace {
InteractionInterface* InteractionInterface::GetInstance()
{
    static InteractionImpl instance;
    return &instance;
}

int32_t InteractionImpl::UpdateShadowPic(const OHOS::Ace::ShadowInfoCore& shadowInfo)
{
    return -1;
}

int32_t InteractionImpl::SetDragWindowVisible(bool visible)
{
    return -1;
}

int32_t InteractionImpl::StartDrag(const DragDataCore& dragData,
    std::function<void(const OHOS::Ace::DragNotifyMsg&)> callback)
{
    return -1;
}

int32_t InteractionImpl::UpdateDragStyle(OHOS::Ace::DragCursorStyleCore style)
{
    return -1;
}

int32_t InteractionImpl::UpdatePreviewStyle(const OHOS::Ace::PreviewStyle& previewStyle)
{
    return -1;
}

int32_t InteractionImpl::UpdatePreviewStyleWithAnimation(const OHOS::Ace::PreviewStyle& previewStyle,
    const OHOS::Ace::PreviewAnimation& animation)
{
    return -1;
}

int32_t InteractionImpl::StopDrag(DragDropRet result)
{
    return -1;
}

int32_t InteractionImpl::GetUdKey(std::string& udKey)
{
    return -1;
}

int32_t InteractionImpl::GetShadowOffset(ShadowOffsetData& shadowOffsetData)
{
    return -1;
}

int32_t InteractionImpl::GetDragSummary(std::map<std::string, int64_t>& summary)
{
    return -1;
}

int32_t InteractionImpl::GetDragExtraInfo(std::string& extraInfo)
{
    return -1;
}

int32_t InteractionImpl::EnterTextEditorArea(bool enable)
{
    return -1;
}

int32_t InteractionImpl::GetDragState(DragState& dragState) const
{
    return -1;
}

int32_t InteractionImpl::AddPrivilege()
{
    return -1;
}

} // namespace OHOS::Ace