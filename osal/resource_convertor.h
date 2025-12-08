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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_RESOURCE_CONVERTOR_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_RESOURCE_CONVERTOR_H

#include "resource_manager.h"
#include "base/utils/utils.h"
#include "base/utils/resource_configuration.h"
#include "core/common/ace_application_info.h"

namespace OHOS::Ace {
Global::Resource::DeviceType ConvertDeviceTypeToGlobal(DeviceType type);
Global::Resource::Direction ConvertDirectionToGlobal(DeviceOrientation orientation);
Global::Resource::ScreenDensity ConvertDensityToGlobal(double density);
std::shared_ptr<Global::Resource::ResConfig> ConvertConfigToGlobal(const ResourceConfiguration& config);

DeviceType ConvertDeviceTypeToAce(Global::Resource::DeviceType type);
DeviceOrientation ConvertDirectionToAce(Global::Resource::Direction orientation);
double ConvertDensityToAce(Global::Resource::ScreenDensity density);
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_RESOURCE_CONVERTOR_H
