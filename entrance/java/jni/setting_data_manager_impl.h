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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_SETTING_DATA_MANAGER_IMPL_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_SETTING_DATA_MANAGER_IMPL_H

#include "core/common/setting_data_manager.h"

namespace OHOS::Ace {

class SettingDataManagerImpl : public SettingDataManager {
public:
    SettingDataManagerImpl() = default;
    ~SettingDataManagerImpl() override = default;
    int32_t Initialize() override;

    bool IsInitialized() const override;

    int32_t GetCurrentUserId() override;

    int32_t RegisterObserver(
        const std::string& key, const DataUpdateFunc& updateFunc, int32_t userId = INVALID_USER_ID) override;

    int32_t UnregisterObserver(const std::string& key, int32_t userId = INVALID_USER_ID) override;

    int32_t GetStringValue(
        const std::string& key, std::string& value, int32_t userId = INVALID_USER_ID) const override;

    int32_t GetInt32ValueStrictly(
        const std::string& key, int32_t& value, int32_t userId = INVALID_USER_ID) const override;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_SETTING_DATA_MANAGER_IMPL_H
