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

#include "display_manager_android.h"

namespace OHOS::Ace {

DisplayManager& DisplayManager::GetInstance()
{
    static DisplayManagerAndroid instance;
    return instance;
}

bool DisplayManagerAndroid::ConvertScreenIdToRsScreenId(uint64_t screenId, uint64_t& rsScreenId)
{
    rsScreenId = screenId;
    return true;
}

} // namespace OHOS::Ace
