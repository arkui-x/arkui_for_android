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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_ASSET_PROVIDER_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_ASSET_PROVIDER_H

#include <list>
#include <map>
#include <mutex>

#include "jni.h"
#include "jni_environment.h"
#include "third_party/flutter/engine/flutter/shell/platform/android/apk_asset_provider.h"

#include "base/utils/macros.h"
#include "core/common/flutter/flutter_asset_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {

class ACE_EXPORT AssetProvider : public Ace::FlutterAssetProvider {
    DECLARE_ACE_TYPE(AssetProvider, Ace::FlutterAssetProvider);

public:
    explicit AssetProvider(std::unique_ptr<flutter::APKAssetProvider> provider) : assetProvider_(std::move(provider)) {}
    ~AssetProvider() override = default;

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
        return "";
    }

    void GetAssetList(const std::string& path, std::vector<std::string>& assetList) override {}

private:
    std::unique_ptr<flutter::AssetResolver> assetProvider_;
};

class StageAssetProvider {
public:
    StageAssetProvider();
    ~StageAssetProvider();

    static std::shared_ptr<StageAssetProvider> GetInstance();
    void SetAppPath(const std::string& appPath);
    void SetAssetsFileRelativePaths(const std::string& appPath);
    void SetAssetManager(JNIEnv* env, jobject assetManager);
    jobject GetAssetManager();
    std::list<std::vector<uint8_t>> GetModuleJsonBufferList();
    std::vector<uint8_t> GetModuleBuffer(const std::string& moduleName, std::string& modulePath);
    std::vector<uint8_t> GetModuleAbilityBuffer(
        const std::string& moduleName, const std::string& abilityName, std::string& modulePath);
    Ace::RefPtr<AssetProvider> CreateAndFindAssetProvider(const std::string& path);
    void SetCacheDir(const std::string& filesRootDir);
    void SetFileDir(const std::string& filesRootDir);
    std::string GetBundleCodeDir() const;
    std::string GetCacheDir() const;
    std::string GetTempDir() const;
    std::string GetFilesDir() const;
    std::string GetDatabaseDir() const;
    std::string GetPreferencesDir() const;

private:
    std::string appPath_ { "" };
    std::vector<std::string> allFilePath_;
    std::mutex allFilePathMutex_;
    std::map<std::string, Ace::RefPtr<AssetProvider>> assetProviders_;
    std::mutex assetProvidersMutex_;
    JNIEnv* env_;
    Ace::Platform::JniEnvironment::JavaGlobalRef assetManager_;
    std::string cacheDir_ { "" };
    std::string tempDir_ { "" };
    std::string filesDir_ { "" };
    std::string databaseDir_ { "" };
    std::string preferenceDir_ { "" };
    static std::shared_ptr<StageAssetProvider> instance_;
    static std::mutex mutex_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_ASSET_PROVIDER_H