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

#include "adapter/preview/entrance/setting_data_manager_impl.h"

namespace OHOS::Ace {

SettingDataManager& SettingDataManager::GetInstance()
{
    static SettingDataManagerImpl instance;
    return instance;
}

int32_t SettingDataManagerImpl::Initialize()
{
    return -1;
}

bool SettingDataManagerImpl::IsInitialized() const
{
    return false;
}

int32_t SettingDataManagerImpl::GetCurrentUserId()
{
    return -1;
}

int32_t SettingDataManagerImpl::RegisterObserver(
    const std::string& key, const DataUpdateFunc& updateFunc, const int32_t userId)
{
    return -1;
}

int32_t SettingDataManagerImpl::UnregisterObserver(const std::string& key, const int32_t userId)
{
    return -1;
}

int32_t SettingDataManagerImpl::GetStringValue(const std::string& key, std::string& value, const int32_t userId) const
{
    return -1;
}

int32_t SettingDataManagerImpl::GetInt32ValueStrictly(
    const std::string& key, int32_t& value, const int32_t userId) const
{
    return -1;
}
} // namespace OHOS::Ace
