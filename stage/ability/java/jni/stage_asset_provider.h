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
#include <string>
#include "jni.h"
#include "jni_environment.h"

#include "base/utils/macros.h"
#include "core/common/asset_manager_impl.h"
#include "pack_asset_provider.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class ACE_EXPORT AssetProvider : public Ace::AssetProviderImpl {
    DECLARE_ACE_TYPE(AssetProvider, Ace::AssetProviderImpl);

public:
    explicit AssetProvider(std::unique_ptr<Ace::PackAssetProvider> provider) : assetProvider_(std::move(provider)) {}
    ~AssetProvider() override = default;

    std::unique_ptr<Ace::AssetMapping> GetAsMapping(const std::string& assetName) const override
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
    std::unique_ptr<Ace::PackAssetProvider> assetProvider_;
};

constexpr int64_t FOO_MAX_LEN = 20 * 1024 * 1024;
class FileAssetMapping : public Ace::AssetMapping {
public:
    FileAssetMapping(std::unique_ptr<uint8_t[]> data, size_t size) : data_(std::move(data)), size_(size) {}
    ~FileAssetMapping() override {}

    size_t GetSize() const override
    {
        return size_;
    }

    const uint8_t* GetAsset() const override
    {
        return data_.get();
    }

private:
    std::unique_ptr<uint8_t[]> data_;
    size_t size_ = 0;
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
    std::vector<uint8_t> GetModuleBuffer(const std::string& moduleName, std::string& modulePath, bool esmodule);
    std::vector<uint8_t> GetModuleAbilityBuffer(
        const std::string& moduleName, const std::string& abilityName, std::string& modulePath, bool esmodule);
    std::vector<uint8_t> GetAbcPathBuffer(const std::string& abcPath);
    Ace::RefPtr<AssetProvider> CreateAndFindAssetProvider(const std::string& path);
    void SetCacheDir(const std::string& filesRootDir);
    void SetFileDir(const std::string& filesRootDir);
    void SetAppLibDir(const std::string& libDir);
    std::string GetAppLibDir() const;
    std::string GetBundleCodeDir() const;
    std::string GetCacheDir() const;
    std::string GetTempDir() const;
    std::string GetFilesDir() const;
    std::string GetDatabaseDir() const;
    std::string GetPreferencesDir() const;
    void GetResIndexPath(const std::string& moduleName, std::string& appResIndexPath, std::string& sysResIndexPath);
    void SetResourcesFilePrefixPath(const std::string& resourcesFilePrefixPath);

    std::string GetAppDataModuleDir() const;
    std::string GetAppDataLibDir() const;
    bool GetAppDataModuleAssetList(const std::string& path, std::vector<std::string>& fileFullPaths, bool onlyChild);
    std::vector<std::string> GetAllFilePath();
    std::vector<uint8_t> GetBufferByAppDataPath(const std::string& fileFullPath);
    bool CopyFile(std::string sourceFile, std::string newFile);
    bool ExistDir(std::string target);
    bool MakeDir(std::string target);
    bool CopyDir(std::string source, const std::string& target);
    void CopyHspResourcePath(const std::string& moduleName);
    void CopyNativeLibToAppDataModuleDir(const std::string& bundleName);
    void SetNativeLibPaths(const std::string& bundleName, const std::vector<std::string>& moduleNames);

private:
    bool MakeMultipleDir(const std::string& path);
    bool CopyBufferToFile(std::vector<uint8_t>& buffer, const std::string& newFile);
    std::string appPath_;
    std::vector<std::string> allFilePath_;
    std::mutex allFilePathMutex_;
    std::map<std::string, Ace::RefPtr<AssetProvider>> assetProviders_;
    std::mutex assetProvidersMutex_;
    Ace::Platform::JniEnvironment::JavaGlobalRef assetManager_;
    std::string cacheDir_;
    std::string tempDir_;
    std::string filesDir_;
    std::string databaseDir_;
    std::string appLibDir_;
    std::string appDataLibDir_;
    std::string arkuiXAssetsDir_;
    std::string preferenceDir_;
    std::string resourcesFilePrefixPath_;
    std::string architecture_;
    static std::shared_ptr<StageAssetProvider> instance_;
    static std::mutex mutex_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_ASSET_PROVIDER_H