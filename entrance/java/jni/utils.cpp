/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "adapter/android/entrance/java/jni/utils.h"

#include "foundation/appframework/window_manager/interfaces/innerkits/wm/wm_common.h"

namespace OHOS::Ace::Platform {

NG::SafeAreaInsets ConvertAvoidArea(const OHOS::Rosen::AvoidArea& avoidArea)
{
    auto clampPos = [](int32_t val) -> uint32_t { return static_cast<uint32_t>(val < 0 ? 0 : val); };
    return NG::SafeAreaInsets({ clampPos(avoidArea.leftRect_.posX_),
                                  clampPos(avoidArea.leftRect_.posX_) + clampPos(avoidArea.leftRect_.width_) },
        { clampPos(avoidArea.topRect_.posY_),
            clampPos(avoidArea.topRect_.posY_) + clampPos(avoidArea.topRect_.height_) },
        { clampPos(avoidArea.rightRect_.posX_),
            clampPos(avoidArea.rightRect_.posX_) + clampPos(avoidArea.rightRect_.width_) },
        { clampPos(avoidArea.bottomRect_.posY_),
            clampPos(avoidArea.bottomRect_.posY_) + clampPos(avoidArea.bottomRect_.height_) });
}

} // namespace OHOS::Ace::Platform
