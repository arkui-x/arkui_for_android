/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_IMAGE_SOURCE_ANDROID_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_IMAGE_SOURCE_ANDROID_H

#include <memory>

#include "image_source.h"

#include "base/image/image_source.h"

namespace OHOS::Ace {
class ImageSourceAndroid : public ImageSource {
    DECLARE_ACE_TYPE(ImageSourceAndroid, ImageSource)
public:
    explicit ImageSourceAndroid(std::unique_ptr<Media::ImageSource>&& source) : imageSource_(std::move(source)) {}

    std::string GetProperty(const std::string& key) override;
    RefPtr<PixelMap> CreatePixelMap(const Size& size) override;
    RefPtr<PixelMap> CreatePixelMap(uint32_t index, const Size& size) override;
    Size GetImageSize() override;

private:
    std::unique_ptr<Media::ImageSource> imageSource_;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_IMAGE_SOURCE_ANDROID_H
