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

#include "display_info.h"
#include "display_info_jni.h"

#include <new>
#include <parcel.h>
#include "base/log/log.h"
#include "base/utils/utils.h"

using namespace OHOS::Ace;
using namespace OHOS::Ace::Platform;

namespace OHOS::Rosen {

DisplayInfo::DisplayInfo()
{
    int displayId = DisplayInfoJni::getDisplayId();
    int orentation = DisplayInfoJni::getOrentation();
    int32_t width = DisplayInfoJni::getDisplayWidth();
    int32_t height = DisplayInfoJni::getDisplayHeight();
    float refreshRate = DisplayInfoJni::getRefreshRate();
    float densityPixels = DisplayInfoJni::getDensityPixels();
    float scaledDensity = DisplayInfoJni::getScaledDensity();
    int dpi = DisplayInfoJni::getDensityDpi();

    LOGI("DisplayInfo:: displayId=%d orentation=%d with=%d height=%d refreshRate=%.3f", displayId, orentation, width, height, refreshRate);

    SetDisplayId(displayId);
    SetWidth(width);
    SetHeight(height);
    SetRefreshRate(refreshRate);
    SetDensityPixels(densityPixels);
    SetScaledDensity(scaledDensity);
    SetDensityDpi(dpi);

    DisplayOrientation arkOrientation = DisplayOrientation::PORTRAIT;
    switch( orentation ) {
        case ROTATION_0: {
            arkOrientation = DisplayOrientation::PORTRAIT;
            break;
        }
        case ROTATION_90: {
            arkOrientation = DisplayOrientation::LANDSCAPE;
            break;
        }
        case ROTATION_180: {
            arkOrientation = DisplayOrientation::PORTRAIT_INVERTED;
            break;
        }
        case ROTATION_270: {
            arkOrientation = DisplayOrientation::LANDSCAPE_INVERTED;
            break;
        }
        default:
            break;
    }
    SetDisplayOrientation(arkOrientation);
}

DisplayInfo::~DisplayInfo()
{}

DisplayId DisplayInfo::GetDisplayId() const
{
    return id_;
}
int32_t DisplayInfo::GetWidth() const
{
    return width_;
}
int32_t DisplayInfo::GetHeight() const
{
    return height_;
}
Orientation DisplayInfo::GetOrientation() const
{
    return orientation_;
}
DisplayOrientation DisplayInfo::GetDisplayOrientation() const
{
    return displayOrientation_;
}

float DisplayInfo::GetDensityPixels() const
{
    return displayDensity_;
}

int DisplayInfo::GetDensityDpi() const
{
    return densityDpi_;
}

float DisplayInfo::GetScaledDensity() const
{
    return scaledDensity_;
}

float DisplayInfo::GetRefreshRate() const
{
    return refreshRate_;
}

void DisplayInfo::SetDisplayId(DisplayId displayId)
{
    id_ = displayId;
}
void DisplayInfo::SetWidth(int32_t width)
{
    width_ = width;
}
void DisplayInfo::SetHeight(int32_t height)
{
    height_ = height;
}
void DisplayInfo::SetOrientation(Orientation orientation)
{
    orientation_ = orientation;
}
void DisplayInfo::SetDisplayOrientation(DisplayOrientation displayOrientation)
{
    displayOrientation_ = displayOrientation;
}

void DisplayInfo::SetRefreshRate(float refreshRate)
{
    refreshRate_ = refreshRate;
}

void DisplayInfo::SetDensityPixels(float displayDensity)
{
    displayDensity_ = displayDensity;
}

void DisplayInfo::SetScaledDensity(float scaledDensity)
{
    scaledDensity_ = scaledDensity;
}

void DisplayInfo::SetDensityDpi(int dpi)
{
    densityDpi_ = dpi;
}

} // namespace OHOS::Rosen