/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <sstream>
#include "base/log/log_wrapper.h"
#include "base/utils/utils.h"
#include "adapter/android/osal/pixel_map_android.h"

namespace OHOS::Ace {

PixelFormat PixelMapAndroid::PixelFormatConverter(Media::PixelFormat pixelFormat)
{
    switch (pixelFormat) {
        case Media::PixelFormat::RGB_565:
            return PixelFormat::RGB_565;
        case Media::PixelFormat::RGBA_8888:
            return PixelFormat::RGBA_8888;
        case Media::PixelFormat::BGRA_8888:
            return PixelFormat::BGRA_8888;
        case Media::PixelFormat::ALPHA_8:
            return PixelFormat::ALPHA_8;
        case Media::PixelFormat::RGBA_F16:
            return PixelFormat::RGBA_F16;
        case Media::PixelFormat::UNKNOWN:
            return PixelFormat::UNKNOWN;
        case Media::PixelFormat::ARGB_8888:
            return PixelFormat::ARGB_8888;
        case Media::PixelFormat::RGB_888:
            return PixelFormat::RGB_888;
        case Media::PixelFormat::NV21:
            return PixelFormat::NV21;
        case Media::PixelFormat::NV12:
            return PixelFormat::NV12;
        case Media::PixelFormat::CMYK:
            return PixelFormat::CMYK;
        default:
            return PixelFormat::UNKNOWN;
    }
}

AlphaType PixelMapAndroid::AlphaTypeConverter(Media::AlphaType alphaType)
{
    switch (alphaType) {
        case Media::AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN:
            return AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN;
        case Media::AlphaType::IMAGE_ALPHA_TYPE_OPAQUE:
            return AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
        case Media::AlphaType::IMAGE_ALPHA_TYPE_PREMUL:
            return AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
        case Media::AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL:
            return AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
        default:
            return AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN;
    }
}

int32_t PixelMapAndroid::GetWidth() const
{
    CHECK_NULL_RETURN(pixmap_, 0);
    return pixmap_->GetWidth();
}

int32_t PixelMapAndroid::GetHeight() const
{
    CHECK_NULL_RETURN(pixmap_, 0);
    return pixmap_->GetHeight();
}

const uint8_t* PixelMapAndroid::GetPixels() const
{
    CHECK_NULL_RETURN(pixmap_, nullptr);
    return pixmap_->GetPixels();
}

PixelFormat PixelMapAndroid::GetPixelFormat() const
{
    CHECK_NULL_RETURN(pixmap_, PixelFormat::UNKNOWN);
    return PixelFormatConverter(pixmap_->GetPixelFormat());
}

AlphaType PixelMapAndroid::GetAlphaType() const
{
    CHECK_NULL_RETURN(pixmap_, AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN);
    return AlphaTypeConverter(pixmap_->GetAlphaType());
}

int32_t PixelMapAndroid::GetRowBytes() const
{
    CHECK_NULL_RETURN(pixmap_, 0);
    return pixmap_->GetRowBytes();
}

int32_t PixelMapAndroid::GetByteCount() const
{
    CHECK_NULL_RETURN(pixmap_, 0);
    return pixmap_->GetByteCount();
}

void* PixelMapAndroid::GetPixelManager() const
{
    Media::InitializationOptions opts;
    CHECK_NULL_RETURN(pixmap_, nullptr);
    auto newPixelMap = Media::PixelMap::Create(*pixmap_, opts);
    return reinterpret_cast<void*>(new Media::PixelMapManager(newPixelMap.release()));
}

void* PixelMapAndroid::GetRawPixelMapPtr() const
{
    CHECK_NULL_RETURN(pixmap_, nullptr);
    return pixmap_.get();
}

std::string PixelMapAndroid::GetId()
{
    // using pixmap addr
    CHECK_NULL_RETURN(pixmap_, "nullptr");
    std::stringstream strm;
    strm << pixmap_.get();
    return strm.str();
}

std::string PixelMapAndroid::GetModifyId()
{
    return std::string();
}

std::shared_ptr<Media::PixelMap> PixelMapAndroid::GetPixelMapSharedPtr()
{
    return pixmap_;
}

RefPtr<PixelMap> PixelMap::CreatePixelMap(void* rawPtr)
{
    std::shared_ptr<Media::PixelMap>* pixmapPtr = reinterpret_cast<std::shared_ptr<Media::PixelMap>*>(rawPtr);
    if (pixmapPtr == nullptr || *pixmapPtr == nullptr) {
        LOGW("pixmap pointer is nullptr when CreatePixelMap.");
        return nullptr;
    }
    return AceType::MakeRefPtr<PixelMapAndroid>(*pixmapPtr);
}

RefPtr<PixelMap> PixelMap::Create(std::unique_ptr<Media::PixelMap>&& pixmap)
{
    return AceType::MakeRefPtr<PixelMapAndroid>(std::move(pixmap));
}

RefPtr<PixelMap> PixelMap::GetFromDrawable(void* ptr)
{
    return nullptr;
}

RefPtr<PixelMap> PixelMap::CreatePixelMapFromDataAbility(void* ptr)
{
    return nullptr;
}

RefPtr<PixelMap> PixelMap::ConvertSkImageToPixmap(
    const uint32_t* colors, uint32_t colorLength, int32_t width, int32_t height)
{
    return nullptr;
}

void* PixelMapAndroid::GetWritablePixels() const
{
    return pixmap_->GetWritablePixels();
}
} // namespace OHOS::Ace