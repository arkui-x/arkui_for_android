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

#include "adapter/android/osal/resource_convertor.h"

namespace {
constexpr double DENSITY_120 = 120.0;
constexpr double DENSITY_160 = 160.0;
constexpr double DENSITY_240 = 240.0;
constexpr double DENSITY_320 = 320.0;
constexpr double DENSITY_480 = 480.0;
constexpr double DENSITY_640 = 640.0;
constexpr double DPI_BASE = 160.0;
}

namespace OHOS::Ace {
Global::Resource::DeviceType ConvertDeviceTypeToGlobal(DeviceType type)
{
    switch (type) {
        case DeviceType::PHONE:
            return Global::Resource::DeviceType::DEVICE_PHONE;
        case DeviceType::TV:
            return Global::Resource::DeviceType::DEVICE_TV;
        case DeviceType::WATCH:
            return Global::Resource::DeviceType::DEVICE_WEARABLE;
        case DeviceType::CAR:
            return Global::Resource::DeviceType::DEVICE_CAR;
        case DeviceType::TABLET:
            return Global::Resource::DeviceType::DEVICE_TABLET;
        default:
            return Global::Resource::DeviceType::DEVICE_NOT_SET;
    }
}

Global::Resource::Direction ConvertDirectionToGlobal(DeviceOrientation orientation)
{
    switch (orientation) {
        case DeviceOrientation::PORTRAIT:
            return Global::Resource::Direction::DIRECTION_VERTICAL;
        case DeviceOrientation::LANDSCAPE:
            return Global::Resource::Direction::DIRECTION_HORIZONTAL;
        default:
            return Global::Resource::Direction::DIRECTION_NOT_SET;
    }
}

Global::Resource::ScreenDensity ConvertDensityToGlobal(double density)
{
    static const std::vector<std::pair<double, Global::Resource::ScreenDensity>> resolutions = {
        { 0.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET },
        { DENSITY_120, Global::Resource::ScreenDensity::SCREEN_DENSITY_SDPI },
        { DENSITY_160, Global::Resource::ScreenDensity::SCREEN_DENSITY_MDPI },
        { DENSITY_240, Global::Resource::ScreenDensity::SCREEN_DENSITY_LDPI },
        { DENSITY_320, Global::Resource::ScreenDensity::SCREEN_DENSITY_XLDPI },
        { DENSITY_480, Global::Resource::ScreenDensity::SCREEN_DENSITY_XXLDPI },
        { DENSITY_640, Global::Resource::ScreenDensity::SCREEN_DENSITY_XXXLDPI },
    };
    double deviceDpi = density * DPI_BASE;
    auto resolution = Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET;
    for (const auto& [dpi, value] : resolutions) {
        resolution = value;
        if (LessOrEqual(deviceDpi, dpi)) {
            break;
        }
    }
    return resolution;
}

Global::Resource::ColorMode ConvertColorModeToGlobal(ColorMode colorMode)
{
    switch (colorMode) {
        case ColorMode::DARK:
            return Global::Resource::ColorMode::DARK;
        case ColorMode::LIGHT:
            return Global::Resource::ColorMode::LIGHT;
        default:
            return Global::Resource::ColorMode::COLOR_MODE_NOT_SET;
    }
}

Global::Resource::InputDevice ConvertInputDevice(bool deviceAccess)
{
    return deviceAccess ? Global::Resource::InputDevice::INPUTDEVICE_POINTINGDEVICE :
        Global::Resource::InputDevice::INPUTDEVICE_NOT_SET;
}

std::shared_ptr<Global::Resource::ResConfig> ConvertConfigToGlobal(const ResourceConfiguration& config)
{
    std::shared_ptr<Global::Resource::ResConfig> newResCfg(Global::Resource::CreateResConfig());
    newResCfg->SetLocaleInfo(AceApplicationInfo::GetInstance().GetLanguage().c_str(),
        AceApplicationInfo::GetInstance().GetScript().c_str(),
        AceApplicationInfo::GetInstance().GetCountryOrRegion().c_str());
    newResCfg->SetDeviceType(ConvertDeviceTypeToGlobal(config.GetDeviceType()));
    newResCfg->SetDirection(ConvertDirectionToGlobal(config.GetOrientation()));
    newResCfg->SetScreenDensity(config.GetDensity());
    newResCfg->SetColorMode(ConvertColorModeToGlobal(config.GetColorMode()));
    newResCfg->SetInputDevice(ConvertInputDevice(config.GetDeviceAccess()));
    return newResCfg;
}

DeviceType ConvertDeviceTypeToAce(Global::Resource::DeviceType type)
{
    switch (type) {
        case Global::Resource::DeviceType::DEVICE_PHONE:
            return DeviceType::PHONE;
        case Global::Resource::DeviceType::DEVICE_TV:
            return DeviceType::TV;
        case Global::Resource::DeviceType::DEVICE_WEARABLE:
            return DeviceType::WATCH;
        case Global::Resource::DeviceType::DEVICE_CAR:
            return DeviceType::CAR;
        case Global::Resource::DeviceType::DEVICE_TABLET:
            return DeviceType::TABLET;
        default:
            return DeviceType::UNKNOWN;
    }
}

DeviceOrientation ConvertDirectionToAce(Global::Resource::Direction orientation)
{
    switch (orientation) {
        case Global::Resource::Direction::DIRECTION_VERTICAL:
            return DeviceOrientation::PORTRAIT;
        case Global::Resource::Direction::DIRECTION_HORIZONTAL:
            return DeviceOrientation::LANDSCAPE;
        default:
            return DeviceOrientation::ORIENTATION_UNDEFINED;
    }
}

double ConvertDensityToAce(Global::Resource::ScreenDensity density)
{
    switch (density) {
        case Global::Resource::ScreenDensity::SCREEN_DENSITY_SDPI:
            return DENSITY_120 / DPI_BASE;
        case Global::Resource::ScreenDensity::SCREEN_DENSITY_MDPI:
            return DENSITY_160 / DPI_BASE;
        case Global::Resource::ScreenDensity::SCREEN_DENSITY_LDPI:
            return DENSITY_240 / DPI_BASE;
        case Global::Resource::ScreenDensity::SCREEN_DENSITY_XLDPI:
            return DENSITY_320 / DPI_BASE;
        case Global::Resource::ScreenDensity::SCREEN_DENSITY_XXLDPI:
            return DENSITY_480 / DPI_BASE;
        case Global::Resource::ScreenDensity::SCREEN_DENSITY_XXXLDPI:
            return DENSITY_640 / DPI_BASE;
        default:
            return 0.0;
    }
}
} // namespace OHOS::Ace
