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

#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "base/log/log.h"
#include "base/utils/string_utils.h"
#include "native_module_manager.h"

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
const std::string ARKUI_X_ASSETS_DIR = "/arkui-x";
const std::string PREFERENCE_DIR = "/preference";
const std::string DATABASE_DIR = "/database";
const std::string ASSETS_DIR = "/assets";
const std::string RESOURCES_INDEX_NAME = "resources.index";
const std::string SYSTEM_RES_INDEX_NAME = "systemres";
const std::string SEPARATOR = "/";
const std::string RESOURCES_DIR_NAME = "resources";
const std::string ARCH_ARM64 = "/arm64-v8a";
const std::string ARCH_ARM = "/armeabi-v7a";
const std::string ARCH_X86 = "/x86_64";
const std::string SO_SUFFIX = ".so";
const std::string EXTERN_LIBS_DIR = "/libs";
} // namespace
std::shared_ptr<StageAssetProvider> StageAssetProvider::instance_ = nullptr;
std::mutex StageAssetProvider::mutex_;
StageAssetProvider::StageAssetProvider() : assetManager_(nullptr, nullptr), architecture_(ARCH_ARM64) {}

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
        auto moduleMap = mapping->GetAsset();
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

    std::vector<uint8_t> buffer;
    auto dynamicLoadFlag = true;
    std::string moduleNameMark = SEPARATOR + moduleName + SEPARATOR;

    std::vector<std::string> abcPath;
    {
        std::lock_guard<std::mutex> lock(allFilePathMutex_);
        for (auto& path : allFilePath_) {
            if (path.find(fullAbilityName) != std::string::npos) {
                abcPath.emplace_back(path);
            }
            if (path.find(moduleNameMark) != std::string::npos) {
                dynamicLoadFlag = false;
            }
        }
    }

    if (dynamicLoadFlag) {
        auto path = GetAppDataModuleDir() + SEPARATOR + moduleName;
        std::vector<std::string> fileFullPaths;
        GetAppDataModuleAssetList(path, fileFullPaths, false);
        for (auto& path : fileFullPaths) {
            if (path.find(moduleNameMark) != std::string::npos && path.find(fullAbilityName) != std::string::npos) {
                modulePath = path;
                buffer = GetBufferByAppDataPath(path);
                break;
            }
        }

        // copy dynamic resource files
        auto resourceDescDir = resourcesFilePrefixPath_ + SEPARATOR + moduleName;
        if (ExistDir(path) && !ExistDir(resourceDescDir)) {
            MakeDir(resourceDescDir);
            CopyDir(path, resourceDescDir);
        }
        auto downloadPath = GetAppDataModuleDir() + SEPARATOR + SYSTEM_RES_INDEX_NAME;
        auto systemresDescDir = resourcesFilePrefixPath_ + SEPARATOR + SYSTEM_RES_INDEX_NAME;
        if (ExistDir(downloadPath) && !ExistDir(systemresDescDir)) {
            MakeDir(systemresDescDir);
            CopyDir(downloadPath, systemresDescDir);
        }
    } else {
        for (auto& path : abcPath) {
            if (path.find(moduleNameMark) != std::string::npos) {
                modulePath = path;
                auto lastPos = path.find_last_of('/');
                std::string fileName = path.substr(lastPos + 1, (int)(path.size()));
                std::string filePath = path.substr(0, lastPos);
                auto assetProvider = CreateAndFindAssetProvider(filePath);
                auto mapping = assetProvider->GetAsMapping(fileName);
                if (mapping == nullptr) {
                    LOGE("mapping is nullptr");
                    continue;
                }
                auto moduleMap = mapping->GetAsset();
                if (moduleMap == nullptr) {
                    LOGE("moduleMap is nullptr");
                    continue;
                }
                buffer.assign(&moduleMap[0], &moduleMap[mapping->GetSize()]);
                break;
            }
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

    std::vector<uint8_t> buffer;
    auto dynamicLoadFlag = true;
    std::string moduleNameMark = SEPARATOR + moduleName + SEPARATOR;
    std::vector<std::string> abcPath;
    {
        std::lock_guard<std::mutex> lock(allFilePathMutex_);
        for (auto& path : allFilePath_) {
            if (path.find(fullAbilityName) != std::string::npos) {
                abcPath.emplace_back(path);
            }
            if (path.find(moduleNameMark) != std::string::npos) {
                dynamicLoadFlag = false;
            }
        }
    }

    if (dynamicLoadFlag) {
        auto path = GetAppDataModuleDir() + SEPARATOR + moduleName;
        std::vector<std::string> fileFullPaths;
        GetAppDataModuleAssetList(path, fileFullPaths, false);
        for (auto& path : fileFullPaths) {
            if (path.find(moduleNameMark) != std::string::npos && path.find(fullAbilityName) != std::string::npos) {
                modulePath = path;
                buffer = GetBufferByAppDataPath(path);
                break;
            }
        }
    } else {
        for (auto& path : abcPath) {
            if (path.find(moduleNameMark) != std::string::npos) {
                modulePath = path;
                auto lastPos = path.find_last_of('/');
                std::string fileName = path.substr(lastPos + 1, path.size());
                std::string filePath = path.substr(0, lastPos);
                auto assetProvider = CreateAndFindAssetProvider(filePath);
                auto mapping = assetProvider->GetAsMapping(fileName);
                if (mapping == nullptr) {
                    LOGE("mapping is nullptr");
                    continue;
                }
                auto moduleMap = mapping->GetAsset();
                if (moduleMap == nullptr) {
                    LOGE("moduleMap is nullptr");
                    continue;
                }
                buffer.assign(&moduleMap[0], &moduleMap[mapping->GetSize()]);
                break;
            }
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
    auto moduleMap = mapping->GetAsset();
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

    auto assetProvider = Ace::AceType::MakeRefPtr<AssetProvider>(std::make_unique<Ace::PackAssetProvider>(
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
    arkuiXAssetsDir_ = filesRootDir + ARKUI_X_ASSETS_DIR;
    size_t lastSlashPos = appLibDir_.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        if (appLibDir_.substr(lastSlashPos) == "/arm64") {
            appDataLibDir_ = filesRootDir + ARKUI_X_ASSETS_DIR + EXTERN_LIBS_DIR + ARCH_ARM64;
            architecture_ = ARCH_ARM64;
        } else if (appLibDir_.substr(lastSlashPos) == "/arm") {
            appDataLibDir_ = filesRootDir + ARKUI_X_ASSETS_DIR + EXTERN_LIBS_DIR + ARCH_ARM;
            architecture_ = ARCH_ARM;
        } else {
            appDataLibDir_ = filesRootDir + ARKUI_X_ASSETS_DIR + EXTERN_LIBS_DIR + ARCH_X86;
            architecture_ = ARCH_X86;
        }
    }
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

std::string StageAssetProvider::GetAppDataModuleDir() const
{
    return arkuiXAssetsDir_;
}

std::string StageAssetProvider::GetAppDataLibDir() const
{
    return appDataLibDir_;
}

std::vector<std::string> StageAssetProvider::GetAllFilePath()
{
    return allFilePath_;
}

bool StageAssetProvider::GetAppDataModuleAssetList(
    const std::string& path, std::vector<std::string>& fileFullPaths, bool onlyChild)
{
    DIR* pDir = nullptr;
    struct dirent* ptr = nullptr;
    if (!(pDir = opendir(path.c_str()))) {
        return false;
    }
    while ((ptr = readdir(pDir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
            if (onlyChild) {
                std::string file = path + SEPARATOR + ptr->d_name;
                fileFullPaths.emplace_back(file);
            } else if (ptr->d_type == DT_DIR && !GetAppDataModuleAssetList(path + "/" + ptr->d_name, fileFullPaths, false)) {
                break;
            } else if (ptr->d_type == DT_REG) {
                std::string file = path + SEPARATOR + ptr->d_name;
                fileFullPaths.emplace_back(file);
            }
        }
    }
    closedir(pDir);
    return true;
}

std::vector<uint8_t> StageAssetProvider::GetBufferByAppDataPath(const std::string& fileFullPath)
{
    std::vector<uint8_t> buffer;
    std::lock_guard<std::mutex> lock(allFilePathMutex_);
    std::FILE* fp = std::fopen(fileFullPath.c_str(), "r");
    if (fp == nullptr) {
        return buffer;
    }

    if (std::fseek(fp, 0, SEEK_END) != 0) {
        LOGE("seek file tail error");
        std::fclose(fp);
        return buffer;
    }

    int64_t size = std::ftell(fp);
    if (size == -1L || size == 0L || size > FOO_MAX_LEN) {
        LOGE("tell file error");
        std::fclose(fp);
        return buffer;
    }
    uint8_t* dataArray = new (std::nothrow) uint8_t[size];
    if (dataArray == nullptr) {
        LOGE("new uint8_t array failed");
        std::fclose(fp);
        return buffer;
    }

    rewind(fp);
    std::unique_ptr<uint8_t[]> data(dataArray);
    size_t result = std::fread(data.get(), 1, size, fp);
    if (result != (size_t)size) {
        LOGE("read file failed");
        std::fclose(fp);
        return buffer;
    }

    std::fclose(fp);
    auto mapping = std::make_unique<FileAssetMapping>(std::move(data), size);
    if (mapping == nullptr) {
        LOGE("mapping is nullptr");
        return buffer;
    }
    auto moduleMap = mapping->GetAsset();
    if (moduleMap == nullptr) {
        LOGE("moduleMap is nullptr");
        return buffer;
    }
    buffer.assign(&moduleMap[0], &moduleMap[mapping->GetSize()]);
    return buffer;
}

bool StageAssetProvider::CopyFile(std::string sourceFile, std::string newFile)
{
    std::ifstream in;
    std::ofstream out;
    in.open(sourceFile.c_str(), std::ios::binary);
    if (in.fail()) {
        LOGE("open sourcefile error!");
        in.close();
        out.close();
        return false;
    }
    out.open(newFile.c_str(), std::ios::binary);
    if (out.fail()) {
        LOGE("new file error!");
        out.close();
        in.close();
        return false;
    }
    out << in.rdbuf();
    out.close();
    in.close();
    return true;
}

bool StageAssetProvider::ExistDir(std::string target)
{
    DIR* dir = nullptr;
    if (!(dir = opendir(target.c_str()))) {
        closedir(dir);
        return false;
    } else {
        closedir(dir);
        return true;
    }
}

bool StageAssetProvider::MakeDir(std::string target)
{
    if (access(target.c_str(), 0) != 0) {
        mkdir(target.c_str(), S_IRWXU | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        return true;
    } else {
        LOGE("make dir error!");
        return false;
    }
}

bool StageAssetProvider::CopyDir(std::string source, const std::string& target)
{
    DIR* dir = nullptr;
    struct stat st;
    struct dirent* dp = nullptr;

    if (stat(source.c_str(), &st) < 0 || !S_ISDIR(st.st_mode)) {
        LOGE("source is null!");
        return false;
    }

    if (!(dir = opendir(source.c_str()))) {
        LOGE("source dir open error!");
        return false;
    }

    while ((dp = readdir(dir)) != nullptr) {
        if ((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, ".", 2))) {
            continue;
        }
        if (!strncmp(dp->d_name, "ets", 3) || !strncmp(dp->d_name, "module.json", 12)) {
            continue;
        }
        std::string newpath = source + SEPARATOR + dp->d_name;
        stat(newpath.c_str(), &st);
        std::string newtarget = target + SEPARATOR + dp->d_name;

        if (S_ISDIR(st.st_mode)) {
            MakeDir(newtarget);
            CopyDir(newpath, newtarget);
        } else {
            CopyFile(newpath, newtarget);
        }
    }
    closedir(dir);
    return true;
}

void StageAssetProvider::CopyHspResourcePath(const std::string& moduleName)
{
    auto sourcePath = GetAppDataModuleDir() + SEPARATOR + moduleName;
    if (!ExistDir(sourcePath)) {
        LOGW("hsp does not exist, stop copying");
        return;
    }

    auto resourcePath = sourcePath + SEPARATOR + RESOURCES_INDEX_NAME;
    auto targetRootDir = resourcesFilePrefixPath_ + SEPARATOR + moduleName;
    if (ExistDir(targetRootDir)) {
        LOGW("dir exists, dir: %{public}s", targetRootDir.c_str());
        return;
    }

    MakeDir(targetRootDir);
    auto targetPath = targetRootDir + SEPARATOR + RESOURCES_INDEX_NAME;
    CopyFile(resourcePath, targetPath);

    auto targetDir = targetRootDir + SEPARATOR + RESOURCES_DIR_NAME;
    MakeDir(targetDir);
    auto resourceDir = sourcePath + SEPARATOR + RESOURCES_DIR_NAME;
    CopyDir(resourceDir, targetDir);
}

bool StageAssetProvider::MakeMultipleDir(const std::string& path)
{
    if (ExistDir(path)) {
        return true;
    }
    auto lastPos = path.find_last_of(SEPARATOR);
    std::string filePath = path.substr(0, lastPos);
    return MakeMultipleDir(filePath) ? MakeDir(path) : false;
}

bool StageAssetProvider::CopyBufferToFile(std::vector<uint8_t>& buffer, const std::string& newFile)
{
    std::ofstream out;
    out.open(newFile.c_str(), std::ios::binary);
    if (out.fail()) {
        LOGE("new file error!");
        out.close();
        return false;
    }
    out.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
    out.close();
    return true;
}

void StageAssetProvider::CopyNativeLibToAppDataModuleDir(const std::string& bundleName)
{
    std::vector<std::string> libPaths;
    for (auto& path : allFilePath_) {
        if (path.find(architecture_) != std::string::npos && path.find(SO_SUFFIX) != std::string::npos) {
            libPaths.emplace_back(path);
        }
    }

    for (auto& path : libPaths) {
        auto lastPos = path.find_last_of(SEPARATOR);
        std::string fileName = path.substr(lastPos + 1, path.size());
        std::string filePath = path.substr(0, lastPos);

        auto assetProvider = CreateAndFindAssetProvider(filePath);
        auto mapping = assetProvider->GetAsMapping(fileName);
        if (mapping == nullptr) {
            LOGE("mapping is nullptr");
            continue;
        }
        auto moduleMap = mapping->GetAsset();
        if (moduleMap == nullptr) {
            LOGE("moduleMap is nullptr");
            continue;
        }
        std::vector<uint8_t> buffer;
        buffer.assign(&moduleMap[0], &moduleMap[mapping->GetSize()]);

        auto beginPos = filePath.find(ARKUI_X_ASSETS_DIR);
        auto endPath = filePath.substr(beginPos + ARKUI_X_ASSETS_DIR.length() + 1, filePath.size() - 1);
        auto endPos = endPath.find_first_of(SEPARATOR);
        auto moduleName = endPath.substr(0, endPos);

        auto newLibDir = GetAppDataModuleDir() + SEPARATOR + moduleName + EXTERN_LIBS_DIR + architecture_;
        if (!MakeMultipleDir(newLibDir)) {
            LOGE("make multilevel dir failed");
            continue;
        }

        auto newFile = newLibDir + SEPARATOR + fileName;
        if (!CopyBufferToFile(buffer, newFile)) {
            LOGE("copy file failed");
            continue;
        }
    }
}

void StageAssetProvider::SetNativeLibPaths(
    const std::string& bundleName, const std::vector<std::string>& moduleNames)
{
    std::map<std::string, std::vector<std::string>> nativeLibPaths;
    for (const auto& moduleName : moduleNames) {
        auto path = GetAppDataModuleDir() + SEPARATOR + moduleName;
        if (!ExistDir(path)) {
            continue;
        }
        std::vector<std::string> fileFullPaths;
        GetAppDataModuleAssetList(path, fileFullPaths, false);
        for (auto& path : fileFullPaths) {
            if (path.find(architecture_) != std::string::npos && path.find(SO_SUFFIX) != std::string::npos) {
                auto lastPos = path.find_last_of(SEPARATOR);
                std::string filePath = path.substr(0, lastPos);
                auto key = bundleName + SEPARATOR + moduleName;
                if (nativeLibPaths.find(key) == nativeLibPaths.end()) {
                    std::vector<std::string> tempPaths { filePath };
                    nativeLibPaths[key] = tempPaths;
                }
            }
        }
    }

    for (auto& nativeLibPath : nativeLibPaths) {
        NativeModuleManager::GetInstance()->SetAppLibPath(nativeLibPath.first, nativeLibPath.second);
    }
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS