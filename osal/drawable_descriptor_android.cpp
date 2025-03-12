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

#include "adapter/android/osal/drawable_descriptor_android.h"

namespace OHOS::Ace {
RefPtr<DrawableDescriptor> DrawableDescriptor::CreateDrawable(void* sptrAddr)
{
    return nullptr;
}

int32_t DrawableDescriptorAndroid::GetDrawableSrcType()
{
    return -1;
}

void DrawableDescriptorAndroid::RegisterRedrawCallback(RedrawCallback&& callback) {}

void DrawableDescriptorAndroid::Draw(RSCanvas& canvas, const NG::ImagePaintConfig& config) {}
} // namespace OHOS::Ace
