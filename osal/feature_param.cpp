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

#include "base/utils/feature_param.h"

namespace {
constexpr int32_t DEFAULT_RESPONSE_DELAY = 50000000; // default max response delay is 50ms.
} // namespace

namespace OHOS::Ace {

bool FeatureParam::IsSyncLoadEnabled()
{
    return false;
}

uint32_t FeatureParam::GetSyncloadResponseDeadline()
{
    return DEFAULT_RESPONSE_DELAY;
}

bool FeatureParam::IsUINodeGcEnabled()
{
    return false;
}
} // OHOS::Ace