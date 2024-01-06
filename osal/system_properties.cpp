/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "base/utils/system_properties.h"

#include "base/log/log.h"

namespace OHOS::Ace {
namespace {

// Device type, same as w/ java in AceView
constexpr int32_t ORIENTATION_PORTRAIT = 1;
constexpr int32_t ORIENTATION_LANDSCAPE = 2;
constexpr char UNDEFINED_PARAM[] = "undefined parameter";

} // namespace

// It is not used currently. Use ATrace_isEnabled instead.
bool SystemProperties::traceEnabled_ = false;
bool SystemProperties::isRound_ = false;
bool SystemProperties::isDeviceAccess_ = false;
int32_t SystemProperties::deviceWidth_ = 0;
int32_t SystemProperties::deviceHeight_ = 0;
int32_t SystemProperties::devicePhysicalWidth_ = 0;
int32_t SystemProperties::devicePhysicalHeight_ = 0;
double SystemProperties::resolution_ = 1.0;
DeviceType SystemProperties::deviceType_ { DeviceType::PHONE };
DeviceOrientation SystemProperties::orientation_ { DeviceOrientation::PORTRAIT };
std::string SystemProperties::brand_ = INVALID_PARAM;
std::string SystemProperties::manufacturer_ = INVALID_PARAM;
std::string SystemProperties::model_ = INVALID_PARAM;
std::string SystemProperties::product_ = INVALID_PARAM;
std::string SystemProperties::apiVersion_ = INVALID_PARAM;
std::string SystemProperties::releaseType_ = INVALID_PARAM;
std::string SystemProperties::paramDeviceType_ = INVALID_PARAM;
int32_t SystemProperties::mcc_ = MCC_UNDEFINED;
int32_t SystemProperties::mnc_ = MNC_UNDEFINED;
ColorMode SystemProperties::colorMode_ { ColorMode::LIGHT };
ScreenShape SystemProperties::screenShape_ { ScreenShape::NOT_ROUND };
LongScreenType SystemProperties::LongScreen_ { LongScreenType::NOT_LONG };
bool SystemProperties::unZipHap_ = true;
bool SystemProperties::svgTraceEnable_ = false;
bool SystemProperties::rosenBackendEnabled_ = true;
bool SystemProperties::downloadByNetworkEnabled_ = false;
bool SystemProperties::isHookModeEnabled_ = false;
bool SystemProperties::accessibilityEnabled_ = false;
bool SystemProperties::windowAnimationEnabled_ = false;
bool SystemProperties::debugBoundaryEnabled_ = false;
bool SystemProperties::extSurfaceEnabled_ = true;
uint32_t SystemProperties::dumpFrameCount_ = 0;
bool SystemProperties::layoutTraceEnable_ = false;

void SystemProperties::InitDeviceType(DeviceType type)
{
    deviceType_ = type;
}

DeviceType SystemProperties::GetDeviceType()
{
    return deviceType_;
}
void SystemProperties::InitDeviceInfo(
    int32_t deviceWidth, int32_t deviceHeight, int32_t orientation, double resolution, bool isRound)
{
    // SetDeviceOrientation should be earlier than deviceWidth/Height's initialization
    SetDeviceOrientation(orientation);

    isRound_ = isRound;
    resolution_ = resolution;
    deviceWidth_ = deviceWidth;
    deviceHeight_ = deviceHeight;
    if (isRound_) {
        screenShape_ = ScreenShape::ROUND;
    } else {
        screenShape_ = ScreenShape::NOT_ROUND;
    }
}

void SystemProperties::SetDeviceOrientation(int32_t orientation)
{
    if (orientation == ORIENTATION_PORTRAIT && orientation_ != DeviceOrientation::PORTRAIT) {
        std::swap(deviceWidth_, deviceHeight_);
        orientation_ = DeviceOrientation::PORTRAIT;
    } else if (orientation == ORIENTATION_LANDSCAPE && orientation_ != DeviceOrientation::LANDSCAPE) {
        std::swap(deviceWidth_, deviceHeight_);
        orientation_ = DeviceOrientation::LANDSCAPE;
    } else {
        LOGW("SetDeviceOrientation, undefined orientation or current orientation is same as the orientation to set");
    }
}

void SystemProperties::InitDeviceTypeBySystemProperty() {}

float SystemProperties::GetFontWeightScale()
{
    return 1.0f;
}

void SystemProperties::InitMccMnc(int32_t mcc, int32_t mnc)
{
    mcc_ = mcc;
    mnc_ = mnc;
}

bool SystemProperties::IsScoringEnabled(const std::string& name)
{
    return false;
}

bool SystemProperties::GetDebugEnabled()
{
#ifdef ACE_DEBUG_LOG
    return true;
#else
    return false;
#endif
}

bool SystemProperties::IsSyscapExist(const char* cap)
{
    return false;
}

std::string SystemProperties::GetLanguage()
{
    return UNDEFINED_PARAM;
}

std::string SystemProperties::GetRegion()
{
    return UNDEFINED_PARAM;
}

std::string SystemProperties::GetPartialUpdatePkg()
{
    return "";
}

int32_t SystemProperties::GetSvgMode()
{
    // 1 for using svgdom of ArkUI, 0 for using SkiaSvgDom
#ifdef NG_BUILD
    return 0;
#else
    return 1;
#endif
}

bool SystemProperties::GetIsUseMemoryMonitor()
{
    return false;
}

bool SystemProperties::IsFormAnimationLimited()
{
    return false;
}

bool SystemProperties::GetImageFrameworkEnabled()
{
    return false;
}

bool SystemProperties::GetResourceDecoupling()
{
    return false;
}

int32_t SystemProperties::GetJankFrameThreshold()
{
    return 0;
}

bool SystemProperties::GetTitleStyleEnabled()
{
    return false;
}

std::string SystemProperties::GetCustomTitleFilePath()
{
    return UNDEFINED_PARAM;
}

bool SystemProperties::Is24HourClock()
{
    return false;
}
} // namespace OHOS::Ace
