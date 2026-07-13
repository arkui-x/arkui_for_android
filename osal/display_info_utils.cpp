/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/common/display_info_utils.h"

#include "core/common/display_info.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "display_info_jni.h"
#include "display_info.h"
#include "display_manager_agent.h"
#include "display.h"
#include "display_manager.h"

namespace OHOS::Ace {

constexpr int32_t RECT_X_OFFSET = 0;
constexpr int32_t RECT_Y_OFFSET = 1;
constexpr int32_t RECT_WIDTH_OFFSET = 2;
constexpr int32_t RECT_HEIGHT_OFFSET = 3;
constexpr int32_t RECT_PARAM_COUNT = 4;

RefPtr<DisplayInfo> DisplayInfoUtils::GetDisplayInfo(int32_t displayId)
{
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplaySync();
    CHECK_NULL_RETURN(display, displayInfo_);
    auto dmDisplayInfo = display->GetDisplayInfo();
    CHECK_NULL_RETURN(dmDisplayInfo, displayInfo_);
    displayInfo_->SetWidth(dmDisplayInfo->GetWidth());
    displayInfo_->SetHeight(dmDisplayInfo->GetHeight());
    displayInfo_->SetDisplayId(dmDisplayInfo->GetDisplayId());
    auto dmRotation = dmDisplayInfo->GetRotation();
    displayInfo_->SetRotation(static_cast<Rotation>(static_cast<uint32_t>(dmRotation)));
    displayInfo_->SetDisplaySourceMode(DisplaySourceMode::NONE);
    GetIsFoldable();
    GetCurrentFoldStatus();
    GetCurrentFoldCreaseRegion();
    return displayInfo_;
}

void DisplayInfoUtils::InitIsFoldable()
{
    auto agent = Rosen::DisplayManagerAgent::GetInstance();
    if (agent) {
        displayInfo_->SetIsFoldable(agent->IsFoldable());
    }
}

bool DisplayInfoUtils::GetIsFoldable()
{
    if (hasInitIsFoldable_) {
        return displayInfo_->GetIsFoldable();
    }
    auto agent = Rosen::DisplayManagerAgent::GetInstance();
    bool isFoldable = agent ? agent->IsFoldable() : false;
    displayInfo_->SetIsFoldable(isFoldable);
    hasInitIsFoldable_ = true;
    return isFoldable;
}

FoldStatus DisplayInfoUtils::GetCurrentFoldStatus()
{
    auto agent = Rosen::DisplayManagerAgent::GetInstance();
    if (agent) {
        auto dmFoldStatus = static_cast<Rosen::FoldStatus>(agent->GetFoldStatus());
        displayInfo_->SetFoldStatus(static_cast<FoldStatus>(static_cast<uint32_t>(dmFoldStatus)));
    }
    return displayInfo_->GetFoldStatus();
}

void DisplayInfoUtils::UpdateDisplaySourceMode(const sptr<Rosen::Display>& display)
{
    displayInfo_->SetDisplaySourceMode(DisplaySourceMode::NONE);
}

DisplaySourceMode DisplayInfoUtils::GetDisplaySourceMode()
{
    return displayInfo_->GetDisplaySourceMode();
}

std::vector<Rect> DisplayInfoUtils::GetCurrentFoldCreaseRegion()
{
    if (hasInitFoldCreaseRegion_) {
        return displayInfo_->GetCurrentFoldCreaseRegion();
    }
    std::vector<Rect> rects;
    auto agent = Rosen::DisplayManagerAgent::GetInstance();
    if (!agent) {
        return rects;
    }
    auto creaseRegionData = agent->GetLiveCreaseRegion();
    if (creaseRegionData.empty()) {
        return rects;
    }

    int32_t rectCount = static_cast<int32_t>(creaseRegionData.size()) / RECT_PARAM_COUNT;
    for (int32_t i = 0; i < rectCount; i++) {
        Rect rect;
        const int32_t baseIdx = i * RECT_PARAM_COUNT;
        rect.SetRect(
            static_cast<double>(creaseRegionData[baseIdx + RECT_X_OFFSET]),
            static_cast<double>(creaseRegionData[baseIdx + RECT_Y_OFFSET]),
            static_cast<double>(creaseRegionData[baseIdx + RECT_WIDTH_OFFSET]),
            static_cast<double>(creaseRegionData[baseIdx + RECT_HEIGHT_OFFSET]));
        rects.insert(rects.end(), rect);
    }

    displayInfo_->SetCurrentFoldCreaseRegion(rects);
    hasInitFoldCreaseRegion_ = true;
    return rects;
}
} // namespace OHOS::Ace::DisplayInfoUtils
