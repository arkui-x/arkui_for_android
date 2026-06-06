/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_PICTURE_ANDROID_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_PICTURE_ANDROID_H

#include "base/image/picture.h"

namespace OHOS::Ace {
class PictureAndroid : public Picture {
    DECLARE_ACE_TYPE(PictureAndroid, Picture);

public:
    PictureAndroid() = default;
    ~PictureAndroid() override = default;

    RefPtr<PixelMap> GetMainPixel() override
    {
        return nullptr;
    }

    RefPtr<PixelMap> GetAuxPicturePixelMap(AuxiliaryPictureType type) override
    {
        return nullptr;
    }

    RefPtr<PixelMap> GetHdrComposedPixelMap(PixelFormat format) override
    {
        return nullptr;
    }
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_PICTURE_ANDROID_H
