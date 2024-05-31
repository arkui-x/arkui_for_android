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

#include "adapter/android/entrance/java/jni/pack_asset_provider.h"
#include "base/resource/asset_manager.h"
#include "base/utils/macros.h"
#include "core/common/asset_manager_impl.h"

namespace OHOS::Ace {
class ACE_EXPORT ApkAssetProvider : public AssetProviderImpl {
    DECLARE_ACE_TYPE(ApkAssetProvider, AssetProviderImpl);

public:
    explicit ApkAssetProvider(std::unique_ptr<PackAssetProvider> provider, const std::string basePath = "")
        : basePath_(basePath), assetProvider_(std::move(provider))
    {
        LOGD("Create apkassetprovider base path: %{public}s", basePath.c_str());
    }
    ~ApkAssetProvider() override = default;

    void SetAssetManager(AAssetManager *assetManager)
    {
        LOGD("SetAssetManager %{public}p", assetManager);
        assetManager_ = assetManager;
    }

    std::unique_ptr<AssetMapping> GetAsMapping(const std::string& assetName) const override
    {
        if (!assetProvider_) {
            return nullptr;
        }

        std::string name;
        if (assetName.find("./") == 0) {
            name = assetName.substr(2); // 2 for jumping ahead of ./
        } else {
            name = assetName;
        }

        return assetProvider_->GetAsMapping(name);
    }

    bool IsValid() const override
    {
        if (!assetProvider_) {
            return false;
        }
        return assetProvider_->IsValid();
    }

    void SetAppPath(const std::string &appPath)
    {
        appPath_ = appPath;
    }

    std::string GetAssetPath(const std::string& assetName, bool isAddHapPath) override
    {
        LOGD("GetAssetPath %{public}s", assetName.c_str());
        if (assetManager_ == nullptr) {
            LOGE("Fail to get assetmanager");
            return "";
        }

        size_t pos = assetName.find_last_of('/');
        std::string dirPath;
        std::string fileName;
        if (pos == std::string::npos) {
            dirPath = basePath_;
            fileName = assetName;
        } else {
            dirPath = basePath_ + "/" + assetName.substr(0, pos);
            fileName = assetName.substr(pos + 1);
        }

        AAssetDir *dir = AAssetManager_openDir(assetManager_, dirPath.c_str());
        if (dir == nullptr) {
            LOGE("Fail to open asset dir: %{public}s", dirPath.c_str());
            return "";
        }
        const char *entry = nullptr;
        while ((entry = AAssetDir_getNextFileName(dir)) != nullptr) {
            if (fileName == entry) {
                AAssetDir_close(dir);
                return (isAddHapPath ? (appPath_ + "/" + basePath_) : basePath_) + "/";
            }
        }
        AAssetDir_close(dir);
        return "";
    }

    void GetAssetList(const std::string& path, std::vector<std::string>& assetList) override
    {
        LOGD("GetAssetList %{public}s", path.c_str());
        if (assetManager_ == nullptr) {
            LOGE("Fail to get assetmanager");
            return;
        }

        std::string dirPath;
        if (path.empty() || path == "." || path == "./") {
            dirPath = basePath_;
        } else {
            if (path.find("./") == 0) {
                dirPath = basePath_ + "/" + path.substr(2); // 2 for jump ahead of ./
            } else {
                dirPath = basePath_ + "/" + path;
            }
        }

        AAssetDir *dir = AAssetManager_openDir(assetManager_, dirPath.c_str());
        if (dir == nullptr) {
            LOGE("Fail to open asset dir: %{public}s", dirPath.c_str());
            return;
        }
        const char *entry = nullptr;
        while ((entry = AAssetDir_getNextFileName(dir)) != nullptr) {
            std::string file = "./";
            file += entry;
            assetList.emplace_back(file);
        }
        AAssetDir_close(dir);
    }

private:
    std::string basePath_;
    std::unique_ptr<PackAssetProvider> assetProvider_;
    std::string appPath_;
    AAssetManager *assetManager_ = nullptr;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_APK_ASSET_PROVIDER_H
