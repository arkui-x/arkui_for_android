/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

//Device type, same w/ java in AceView
constexpr int32_t ORIENTATION_PORTRAIT = 1;
constexpr int32_t ORIENTATION_LANDSCAPE = 2;

} // namespace

// currently it is unused, use ATrace_isEnabled
bool SystemProperties::traceEnabled_ = false;
bool SystemProperties::isRound_ = false;
int32_t SystemProperties::deviceWidth_ = 0;
int32_t SystemProperties::deviceHeight_ = 0;
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
ScreenShape SystemProperties::screenShape_ {ScreenShape::NOT_ROUND };
LongScreenType SystemProperties::LongScreen_ { LongScreenType::NOT_LONG };
bool SystemProperties::rosenBackendEnabled_ = false;
bool SystemProperties::accessibilityEnabled_ = false;

void SystemProperties::InitDeviceType(DeviceType type) {
    // treat all other device type as phone
    if (type == DeviceType::TV)
        deviceType_ = type;    
}

DeviceType SystemProperties::GetDeviceType() {
    return deviceType_;
}
void SystemProperties::InitDeviceInfo(int32_t deviceWidth, int32_t deviceHeight, int32_t orientation,
                                      double resolution, bool isRound) {
    // SetDeviceOrientation should be earlier than deviceWidth/Height's initialization
    SetDeviceOrientation(orientation);

    isRound_ = isRound;
    resolution_ = resolution;
    deviceWidth_ = deviceWidth;
    deviceHeight_ = deviceHeight;
    if (isRound_) 
        screenShape_ = ScreenShape::ROUND;
    else
        screenShape_ = ScreenShape::NOT_ROUND;
}


void SystemProperties::SetDeviceOrientation(int32_t orientation) {
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

void SystemProperties::InitDeviceTypeBySystemProperty() {
    // empty.  Android doesn't use this function
}

float SystemProperties::GetFontWeightScale() {
    // To Be Done
    return 1.0f;
}

void SystemProperties::InitMccMnc(int32_t mcc, int32_t mnc) {
    mcc_ = mcc;
    mnc_ = mnc;
}

bool SystemProperties::IsScoringEnabled(const std::string& name) {
    return false;
}

bool SystemProperties::GetDebugEnabled() {
    return false;
}



} // namespace OHOS::Ace