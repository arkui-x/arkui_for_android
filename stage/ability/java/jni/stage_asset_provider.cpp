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

#include "stage_asset_provider.h"

#include <string>

#include "base/log/log.h"
#include "base/utils/string_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
namespace {
const std::string MODULE_JSON_NAME = "module.json";
const std::string ABC_EXTENSION_NAME = ".abc";
const std::string ABILITY_STAGE_ABC_NAME = "AbilityStage.abc";
const std::string MODULE_STAGE_ABC_NAME = "modules.abc";
const std::string TEMP_DIR = "/temp";
const std::string FILES_DIR = "/files";
const std::string PREFERENCE_DIR = "/preference";
const std::string DATABASE_DIR = "/database";
const std::string ASSETS_DIR = "/assets";
const std::string RESOURCES_INDEX_NAME = "resources.index";
const std::string SYSTEM_RES_INDEX_NAME = "systemres";
const std::string SEPARATOR = "/";
} // namespace
std::shared_ptr<StageAssetProvider> StageAssetProvider::instance_ = nullptr;
std::mutex StageAssetProvider::mutex_;
StageAssetProvider::StageAssetProvider() : assetManager_(nullptr, nullptr) {}

StageAssetProvider::~StageAssetProvider() {}

std::shared_ptr<StageAssetProvider> StageAssetProvider::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<StageAssetProvider>();
        }
    }

    return instance_;
}

void StageAssetProvider::SetAppPath(const std::string& appPath)
{
    LOGI("StageAssetProvider::SetAppPath : %{public}s", appPath.c_str());
    appPath_ = appPath;
}

void StageAssetProvider::SetAssetsFileRelativePaths(const std::string& path)
{
    std::lock_guard<std::mutex> lock(allFilePathMutex_);
    Ace::StringUtils::StringSplitter(path, ';', allFilePath_);
    for (auto str : allFilePath_) {
        LOGI("SetAssetsFileRelativePaths::str : %{public}s", str.c_str());
    }
}

void StageAssetProvider::SetAssetManager(JNIEnv* env, jobject assetManager)
{
    assetManager_ = Ace::Platform::JniEnvironment::MakeJavaGlobalRef(
        Ace::Platform::JniEnvironment::GetInstance().GetJniEnv(), assetManager);
}

jobject StageAssetProvider::GetAssetManager()
{
    return assetManager_.get();
}

std::list<std::vector<uint8_t>> StageAssetProvider::GetModuleJsonBufferList()
{
    LOGI("Get module json buffer list");
    std::vector<std::string> modulePath;
    {
        std::lock_guard<std::mutex> lock(allFilePathMutex_);
        for (auto& path : allFilePath_) {
            if (path.find(MODULE_JSON_NAME) != std::string::npos) {
                modulePath.emplace_back(path);
            }
        }
    }

    std::list<std::vector<uint8_t>> bufferList;
    for (auto& path : modulePath) {
        auto lastPos = path.find_last_of('/');
        std::string fileName = path.substr(lastPos + 1, path.size());
        std::string filePath = path.substr(0, lastPos);

        auto assetProvider = CreateAndFindAssetProvider(filePath);
        LOGI("filename : %{public}s", fileName.c_str());
        auto mapping = assetProvider->GetAsMapping(fileName);
        if (mapping == nullptr) {
            LOGE("mapping is nullptr");
            continue;
        }
        auto moduleMap = mapping->GetMapping();
        if (moduleMap == nullptr) {
            LOGE("moduleMap is nullptr");
            continue;
        }
        std::vector<uint8_t> moduleBuffer;
        moduleBuffer.assign(&moduleMap[0], &moduleMap[mapping->GetSize()]);
        bufferList.emplace_back(moduleBuffer);
    }

    return bufferList;
}

std::vector<uint8_t> StageAssetProvider::GetModuleBuffer(
    const std::string& moduleName, std::string& modulePath, bool esmodule)
{
    std::string fullAbilityName;
    if (esmodule) {
        fullAbilityName = MODULE_STAGE_ABC_NAME;
    } else {
        fullAbilityName = ABILITY_STAGE_ABC_NAME;
    }
    std::vector<std::string> abcPath;
    {
        std::lock_guard<std::mutex> lock(allFilePathMutex_);
        for (auto& path : allFilePath_) {
            if (path.find(fullAbilityName) != std::string::npos) {
                abcPath.emplace_back(path);
            }
        }
    }

    std::vector<uint8_t> buffer;
    std::string moduleNameMark = SEPARATOR + moduleName + SEPARATOR;
    for (auto& path : abcPath) {
        if (path.find(moduleNameMark) != std::string::npos) {
            modulePath = path;
            LOGI("modulePath : %{public}s", modulePath.c_str());
            auto lastPos = path.find_last_of('/');
            std::string fileName = path.substr(lastPos + 1, path.size());
            std::string filePath = path.substr(0, lastPos);
            auto assetProvider = CreateAndFindAssetProvider(filePath);
            auto mapping = assetProvider->GetAsMapping(fileName);
            if (mapping == nullptr) {
                LOGE("mapping is nullptr");
                continue;
            }
            auto moduleMap = mapping->GetMapping();
            if (moduleMap == nullptr) {
                LOGE("moduleMap is nullptr");
                continue;
            }
            buffer.assign(&moduleMap[0], &moduleMap[mapping->GetSize()]);
            break;
        }
    }
    return buffer;
}

std::vector<uint8_t> StageAssetProvider::GetModuleAbilityBuffer(
    const std::string& moduleName, const std::string& abilityName, std::string& modulePath, bool esmodule)
{
    LOGI("Get Module Ability Buffer");
    std::string fullAbilityName;
    if (esmodule) {
        fullAbilityName = "modules";
    } else {
        fullAbilityName = abilityName;
    }
    fullAbilityName.append(ABC_EXTENSION_NAME);
    std::vector<std::string> abcPath;
    {
        std::lock_guard<std::mutex> lock(allFilePathMutex_);
        for (auto& path : allFilePath_) {
            if (path.find(fullAbilityName) != std::string::npos) {
                abcPath.emplace_back(path);
            }
        }
    }
    std::vector<uint8_t> buffer;
    std::string moduleNameMark = SEPARATOR + moduleName + SEPARATOR;
    for (auto& path : abcPath) {
        if (path.find(moduleNameMark) != std::string::npos) {
            modulePath = path;
            LOGI("modulePath : %{public}s", modulePath.c_str());
            auto lastPos = path.find_last_of('/');
            std::string fileName = path.substr(lastPos + 1, path.size());
            std::string filePath = path.substr(0, lastPos);
            auto assetProvider = CreateAndFindAssetProvider(filePath);
            auto mapping = assetProvider->GetAsMapping(fileName);
            if (mapping == nullptr) {
                LOGE("mapping is nullptr");
                continue;
            }
            auto moduleMap = mapping->GetMapping();
            if (moduleMap == nullptr) {
                LOGE("moduleMap is nullptr");
                continue;
            }
            buffer.assign(&moduleMap[0], &moduleMap[mapping->GetSize()]);
            break;
        }
    }
    return buffer;
}

std::vector<uint8_t> StageAssetProvider::GetAbcPathBuffer(const std::string& abcPath)
{
    LOGI("Get Module Ability Buffer");
    std::string findPath;
    {
        std::lock_guard<std::mutex> lock(allFilePathMutex_);
        for (auto& path : allFilePath_) {
            if (path.find(abcPath) != std::string::npos) {
                findPath = path;
            }
        }
    }
    std::vector<uint8_t> buffer;
    auto lastPos = findPath.find_last_of('/');
    std::string fileName = findPath.substr(lastPos + 1, findPath.size());
    std::string filePath = findPath.substr(0, lastPos);
    auto assetProvider = CreateAndFindAssetProvider(filePath);
    auto mapping = assetProvider->GetAsMapping(fileName);
    if (mapping == nullptr) {
        LOGE("mapping is nullptr");
        return buffer;
    }
    auto moduleMap = mapping->GetMapping();
    if (moduleMap == nullptr) {
        LOGE("moduleMap is nullptr");
        return buffer;
    }
    buffer.assign(&moduleMap[0], &moduleMap[mapping->GetSize()]);
    return buffer;
}

Ace::RefPtr<AssetProvider> StageAssetProvider::CreateAndFindAssetProvider(const std::string& path)
{
    LOGI("Create and find asset provider, path: %{public}s", path.c_str());
    std::lock_guard<std::mutex> lock(assetProvidersMutex_);
    auto finder = assetProviders_.find(path);
    if (finder != assetProviders_.end()) {
        return finder->second;
    }

    auto assetProvider = Ace::AceType::MakeRefPtr<AssetProvider>(std::make_unique<flutter::APKAssetProvider>(
        Ace::Platform::JniEnvironment::GetInstance().GetJniEnv().get(), assetManager_.get(), path));
    assetProviders_.emplace(path, assetProvider);
    return assetProvider;
}

void StageAssetProvider::SetCacheDir(const std::string& cacheDir)
{
    LOGI("SetCacheDir: %{public}s", cacheDir.c_str());
    cacheDir_ = cacheDir;
}

void StageAssetProvider::SetFileDir(const std::string& filesRootDir)
{
    LOGI("SetFileDir: %{public}s", filesRootDir.c_str());
    tempDir_ = filesRootDir + TEMP_DIR;
    filesDir_ = filesRootDir + FILES_DIR;
    preferenceDir_ = filesRootDir + PREFERENCE_DIR;
    databaseDir_ = filesRootDir + DATABASE_DIR;
}

void StageAssetProvider::SetAppLibDir(const std::string& libDir)
{
    appLibDir_ = libDir;
}

std::string StageAssetProvider::GetAppLibDir() const
{
    return appLibDir_;
}

std::string StageAssetProvider::GetBundleCodeDir() const
{
    return appPath_ + ASSETS_DIR;
}

std::string StageAssetProvider::GetCacheDir() const
{
    return cacheDir_;
}

std::string StageAssetProvider::GetTempDir() const
{
    return tempDir_;
}

std::string StageAssetProvider::GetFilesDir() const
{
    return filesDir_;
}

std::string StageAssetProvider::GetDatabaseDir() const
{
    return databaseDir_;
}

std::string StageAssetProvider::GetPreferencesDir() const
{
    return preferenceDir_;
}

void StageAssetProvider::GetResIndexPath(
    const std::string& moduleName, std::string& appResIndexPath, std::string& sysResIndexPath)
{
    appResIndexPath = resourcesFilePrefixPath_ + SEPARATOR + moduleName + SEPARATOR + RESOURCES_INDEX_NAME;
    sysResIndexPath = resourcesFilePrefixPath_ + SEPARATOR + SYSTEM_RES_INDEX_NAME + SEPARATOR + RESOURCES_INDEX_NAME;
}

void StageAssetProvider::SetResourcesFilePrefixPath(const std::string& resourcesFilePrefixPath)
{
    resourcesFilePrefixPath_ = resourcesFilePrefixPath;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS