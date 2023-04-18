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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_APK_ASSET_PROVIDER_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_APK_ASSET_PROVIDER_H

#include <memory>
#include <string>

#include "flutter/shell/platform/android/apk_asset_provider.h"

#include "base/resource/asset_manager.h"
#include "base/utils/macros.h"
#include "core/common/flutter/flutter_asset_manager.h"

namespace OHOS::Ace {

class ACE_EXPORT ApkAssetProvider : public FlutterAssetProvider {
    DECLARE_ACE_TYPE(ApkAssetProvider, FlutterAssetProvider);

public:
    explicit ApkAssetProvider(std::unique_ptr<flutter::APKAssetProvider> provider, const std::string basePath = "")
        : assetProvider_(std::move(provider)), basePath_(basePath)
    {}
    ~ApkAssetProvider() override = default;

    std::unique_ptr<fml::Mapping> GetAsMapping(const std::string& assetName) const override
    {
        if (!assetProvider_) {
            return nullptr;
        }
        return assetProvider_->GetAsMapping(assetName);
    }

    bool IsValid() const override
    {
        if (!assetProvider_) {
            return false;
        }
        return assetProvider_->IsValid();
    }

    std::string GetAssetPath(const std::string& assetName, bool isAddHapPath) override
    {
        return basePath_;
    }

    void GetAssetList(const std::string& path, std::vector<std::string>& assetList) override
    {
        // TODO: need implement it
    }

private:
    std::string basePath_;
    std::unique_ptr<flutter::AssetResolver> assetProvider_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_APK_ASSET_PROVIDER_H