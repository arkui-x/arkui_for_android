/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

namespace OHOS::Ace {

constexpr Dimension SHEET_DEVICE_WIDTH_BREAKPOINT = 600.0_vp;

RefPtr<DisplayInfo> DisplayInfoUtils::GetDisplayInfo(int32_t displayId)
{
    if (displayInfo_) {
        displayInfo_->SetWidth(SystemProperties::GetDeviceHeight());
        displayInfo_->SetHeight(SystemProperties::GetDeviceHeight());
        return displayInfo_;
    }
    return AceType::MakeRefPtr<DisplayInfo>();
}

void DisplayInfoUtils::InitIsFoldable() {}

bool DisplayInfoUtils::GetIsFoldable()
{
    hasInitIsFoldable_ = true;
    return false;
}

FoldStatus DisplayInfoUtils::GetCurrentFoldStatus()
{
    auto context = NG::PipelineContext::GetCurrentContextSafely();
    CHECK_NULL_RETURN(context, FoldStatus::UNKNOWN);
    if (context->GetRootWidth() > SHEET_DEVICE_WIDTH_BREAKPOINT.ConvertToPx() &&
        context->GetRootHeight() > SHEET_DEVICE_WIDTH_BREAKPOINT.ConvertToPx()) {
        return FoldStatus::EXPAND;
    }
    return FoldStatus::UNKNOWN;
}

std::vector<Rect> DisplayInfoUtils::GetCurrentFoldCreaseRegion()
{
    hasInitFoldCreaseRegion_ = true;
    return {};
}
} // namespace OHOS::Ace::DisplayInfoUtils
