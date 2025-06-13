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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_IMAGE_PERF_ANDROID_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_IMAGE_PERF_ANDROID_H

#include "base/image/image_perf.h"

namespace OHOS::Ace {

class ImagePerfPreview : public ImagePerf {
public:
    void StartRecordImageLoadStat(int64_t id) override;
    void EndRecordImageLoadStat(int64_t id, const std::string& imageType, std::pair<int, int> size, int state) override;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_IMAGE_PERF_ANDROID_H
