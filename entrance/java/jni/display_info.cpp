/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

namespace {
DisplayOrientation ConvertToDisplayOrientation(int orentation)
{
    switch (orentation) {
        case ROTATION_0:
            return DisplayOrientation::PORTRAIT;
        case ROTATION_90:
            return DisplayOrientation::LANDSCAPE;
        case ROTATION_180:
            return DisplayOrientation::PORTRAIT_INVERTED;
        case ROTATION_270:
            return DisplayOrientation::LANDSCAPE_INVERTED;
        default:
            return DisplayOrientation::PORTRAIT;
    }
}

Rotation ConvertToRotation(uint32_t rotation)
{
    switch (rotation) {
        case ROTATION_0:
            return Rotation::ROTATION_0;
        case ROTATION_90:
            return Rotation::ROTATION_270;
        case ROTATION_180:
            return Rotation::ROTATION_180;
        case ROTATION_270:
            return Rotation::ROTATION_90;
        default:
            return Rotation::ROTATION_0;
    }
}
} // namespace

DisplayInfo::DisplayInfo()
{
    int displayId = DisplayInfoJni::getDisplayId();
    int orentation = DisplayInfoJni::getOrentation();
    uint32_t rotation = DisplayInfoJni::GetRotation();
    int32_t width = DisplayInfoJni::getDisplayWidth();
    int32_t height = DisplayInfoJni::getDisplayHeight();
    float refreshRate = DisplayInfoJni::getRefreshRate();
    float densityPixels = DisplayInfoJni::getDensityPixels();
    float scaledDensity = DisplayInfoJni::getScaledDensity();
    int dpi = DisplayInfoJni::getDensityDpi();
    float xDpi = DisplayInfoJni::getXDpi();
    float yDpi = DisplayInfoJni::getYDpi();

    LOGD("DisplayInfo:: displayId=%d orentation=%d with=%d height=%d refreshRate=%.3f xDpi=%.3f yDpi=%.3f",
        displayId, orentation, width, height, refreshRate, xDpi, yDpi);

    SetDisplayId(displayId);
    SetWidth(width);
    SetHeight(height);
    SetRefreshRate(refreshRate);
    SetDensityPixels(densityPixels);
    SetScaledDensity(scaledDensity);
    SetDensityDpi(dpi);
    SetXDpi(xDpi);
    SetYDpi(yDpi);
    SetDisplayOrientation(ConvertToDisplayOrientation(orentation));
    SetRotation(ConvertToRotation(rotation));
}

DisplayInfo::DisplayInfo(DisplayId displayId)
{
    int queryDisplayId = displayId == DISPLAY_ID_INVALID ? -1 : static_cast<int>(displayId);
    int displayIdInt = DisplayInfoJni::GetDisplayId(queryDisplayId);
    int orentation = DisplayInfoJni::GetOrentation(queryDisplayId);
    uint32_t rotation = DisplayInfoJni::GetRotation(queryDisplayId);
    int32_t width = DisplayInfoJni::GetDisplayWidth(queryDisplayId);
    int32_t height = DisplayInfoJni::GetDisplayHeight(queryDisplayId);
    float refreshRate = DisplayInfoJni::GetRefreshRate(queryDisplayId);
    float densityPixels = DisplayInfoJni::GetDensityPixels(queryDisplayId);
    float scaledDensity = DisplayInfoJni::GetScaledDensity(queryDisplayId);
    int dpi = DisplayInfoJni::GetDensityDpi(queryDisplayId);
    float xDpi = DisplayInfoJni::GetXDpi(queryDisplayId);
    float yDpi = DisplayInfoJni::GetYDpi(queryDisplayId);

    LOGD("DisplayInfo:: displayId=%d orentation=%d with=%d height=%d refreshRate=%.3f xDpi=%.3f yDpi=%.3f",
        displayIdInt, orentation, width, height, refreshRate, xDpi, yDpi);

    SetDisplayId(displayIdInt);
    SetWidth(width);
    SetHeight(height);
    SetRefreshRate(refreshRate);
    SetDensityPixels(densityPixels);
    SetScaledDensity(scaledDensity);
    SetDensityDpi(dpi);
    SetXDpi(xDpi);
    SetYDpi(yDpi);
    SetDisplayOrientation(ConvertToDisplayOrientation(orentation));
    SetRotation(ConvertToRotation(rotation));
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

float DisplayInfo::GetXDpi() const
{
    return xDpi_;
}

float DisplayInfo::GetYDpi() const
{
    return yDpi_;
}

Rotation DisplayInfo::GetRotation() const
{
    return rotation_;
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

void DisplayInfo::SetXDpi(float xDpi)
{
    xDpi_ = xDpi;
}

void DisplayInfo::SetYDpi(float yDpi)
{
    yDpi_ = yDpi;
}

void DisplayInfo::SetRotation(Rotation rotation)
{
    rotation_ = rotation;
}
} // namespace OHOS::Rosen