/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_DMSERVER_DISPLAY_INFO_H
#define FOUNDATION_DMSERVER_DISPLAY_INFO_H

#include <cstdint>
#include <parcel.h>

#include "base/utils/macros.h"
#include "dm_common.h"
#include "wm_common.h"
#include "wm_single_instance.h"
#include "nocopyable.h"

namespace OHOS::Rosen {

constexpr int ROTATION_0 = 0;
constexpr int ROTATION_90 = 1;
constexpr int ROTATION_180 = 2;
constexpr int ROTATION_270 = 3;

class ACE_EXPORT DisplayInfo : public virtual RefBase {
public:
    DisplayInfo();
    ~DisplayInfo();
    DISALLOW_COPY_AND_MOVE(DisplayInfo);

    DisplayId GetDisplayId() const;
    int32_t GetWidth() const;
    int32_t GetHeight() const;
    Orientation GetOrientation() const;
    DisplayOrientation GetDisplayOrientation() const;
    float GetRefreshRate() const;
    float GetDensityPixels() const;
    float GetScaledDensity() const;
    int GetDensityDpi() const;
    float GetXDpi() const;
    float GetYDpi() const;

    void SetRefreshRate(float refreshRate);
    void SetDisplayId(DisplayId displayId);
    void SetWidth(int32_t width);
    void SetHeight(int32_t height);
    void SetOrientation(Orientation orientation);
    void SetDisplayOrientation(DisplayOrientation displayOrientation);
    void SetDensityPixels(float displayDensity);
    void SetScaledDensity(float scaledDensity);
    void SetDensityDpi(int dpi);
    void SetXDpi(float xDpi);
    void SetYDpi(float yDpi);

private:
    DisplayId id_ { DISPLAY_ID_INVALID };
    int32_t width_ { 0 };
    int32_t height_ { 0 };
    float refreshRate_;
    Orientation orientation_ { Orientation::UNSPECIFIED };
    DisplayOrientation displayOrientation_ { DisplayOrientation::UNKNOWN };
    float displayDensity_ { 0.0 };
    float scaledDensity_ { 0.0 };
    int densityDpi_ { 0 };
    float xDpi_ { 0.0 };
    float yDpi_ { 0.0 };
};
} // namespace OHOS::Rosen
#endif // FOUNDATION_DMSERVER_DISPLAY_INFO_H