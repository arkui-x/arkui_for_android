/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "core/common/ai/image_analyzer_adapter_impl.h"

namespace OHOS::Ace {
void ImageAnalyzerAdapterImpl::SetImageAnalyzerConfig(void* config, bool isOptions) {};

void* ImageAnalyzerAdapterImpl::GetImageAnalyzerConfig()
{
    return nullptr;
}

void* ImageAnalyzerAdapterImpl::ConvertPixmapNapi(const RefPtr<PixelMap>& pixelMap)
{
    return nullptr;
}

ImageAnalyzerAdapter* CreateImageAnalyzerAdapter()
{
    return nullptr;
}

} // namespace OHOS::Ace