/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_DISPLAY_MANAGER_ANDROID_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_DISPLAY_MANAGER_ANDROID_H

#include "base/display_manager/display_manager.h"

namespace OHOS::Ace {

class DisplayManagerAndroid : public DisplayManager {
    DECLARE_ACE_TYPE(DisplayManagerAndroid, DisplayManager);

public:
    bool ConvertScreenIdToRsScreenId(uint64_t screenId, uint64_t& rsScreenId) override;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_DISPLAY_MANAGER_ANDROID_H
