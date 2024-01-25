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


#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INTERACTION_MANAGER_IMPL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_INTERACTION_MANAGER_IMPL_H

#include "core/common/interaction/interaction_interface.h"

namespace OHOS::Ace {
class InteractionImpl : public InteractionInterface {
DECLARE_ACE_TYPE(InteractionImpl, InteractionInterface);

public:
    int32_t UpdateShadowPic(const ShadowInfoCore& shadowInfo) override;

    int32_t SetDragWindowVisible(bool visible) override;

    int32_t StartDrag(const DragDataCore& dragData,
        std::function<void(const OHOS::Ace::DragNotifyMsg&)> callback) override;

    int32_t UpdateDragStyle(DragCursorStyleCore style) override;

    int32_t UpdatePreviewStyle(const PreviewStyle& previewStyle) override;

    int32_t UpdatePreviewStyleWithAnimation(const PreviewStyle& previewStyle,
        const PreviewAnimation& animation) override;

    int32_t StopDrag(DragDropRet result) override;

    int32_t GetUdKey(std::string& udKey) override;

    int32_t GetShadowOffset(ShadowOffsetData& shadowOffsetData) override;

    int32_t GetDragState(DragState& dragState) const override;

    int32_t GetDragSummary(std::map<std::string, int64_t>& summary) override;

    int32_t GetDragExtraInfo(std::string& extraInfo) override;

    int32_t EnterTextEditorArea(bool enable) override;

    int32_t AddPrivilege() override;
};

} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_ACE_ENGINE_ADAPTER_OHOS_CAPABILITY_INTERACTION_IMPL_H