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

#include "adapter/android/osal/file_asset_provider.h"

#include <dirent.h>
#include <limits>
#include <sys/types.h>

#include "base/log/ace_trace.h"
#include "base/log/log.h"

namespace OHOS::Ace {

FileAssetProvider::~FileAssetProvider() {}

bool FileAssetProvider::Initialize(const std::string& packagePath, const std::vector<std::string>& assetBasePaths)
{
    ACE_SCOPED_TRACE("Initialize");
    if (assetBasePaths.empty()) {
        LOGE("the assetBasePath is empty");
        return false;
    }
    assetBasePaths_ = assetBasePaths;
    packagePath_ = packagePath;
    return true;
}

bool FileAssetProvider::IsValid() const
{
    return true;
}

class FileAssetMapping : public AssetMapping {
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

std::unique_ptr<AssetMapping> FileAssetProvider::GetAsMapping(const std::string& assetName) const
{
    ACE_SCOPED_TRACE("GetAsMapping");
    LOGD("assert name is: %{public}s", assetName.c_str());
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& basePath : assetBasePaths_) {
        std::string fileName = packagePath_ + basePath + "/" + assetName;
        std::FILE* fp = std::fopen(fileName.c_str(), "r");
        if (fp == nullptr) {
            continue;
        }

        if (std::fseek(fp, 0, SEEK_END) != 0) {
            LOGE("seek file tail error");
            std::fclose(fp);
            continue;
        }

        long size = std::ftell(fp);
        uint8_t* dataArray = new (std::nothrow) uint8_t[size];
        if (dataArray == nullptr) {
            LOGE("new uint8_t array failed");
            std::fclose(fp);
            continue;
        }

        rewind(fp);
        std::unique_ptr<uint8_t[]> data(dataArray);
        size_t result = std::fread(data.get(), 1, size, fp);
        if (result != (size_t)size) {
            LOGE("read file failed");
            std::fclose(fp);
            continue;
        }

        fclose(fp);
        return std::make_unique<FileAssetMapping>(std::move(data), size);
    }
    return nullptr;
}

std::string FileAssetProvider::GetAssetPath(const std::string& assetName, bool)
{
    for (const auto& basePath : assetBasePaths_) {
        std::string assetBasePath = packagePath_ + basePath;
        std::string fileName = assetBasePath + assetName;
        std::FILE* fp = std::fopen(fileName.c_str(), "r");
        if (fp == nullptr) {
            continue;
        }
        std::fclose(fp);
        return assetBasePath;
    }
    LOGE("Cannot find base path of %{public}s", assetName.c_str());
    return "";
}

void FileAssetProvider::GetAssetList(const std::string& path, std::vector<std::string>& assetList)
{
    for (const auto& basePath : assetBasePaths_) {
        DIR* dp = nullptr;
        auto openDir = packagePath_ + basePath + path;
        if (nullptr == (dp = opendir(openDir.c_str()))) {
            continue;
        }
        struct dirent* dptr = nullptr;
        while ((dptr = readdir(dp)) != nullptr) {
            if (strcmp(dptr->d_name, ".") != 0 && strcmp(dptr->d_name, "..") != 0) {
                std::string file = "./";
                file += dptr->d_name;
                assetList.emplace_back(file);
            }
        }
        closedir(dp);
    }
}

} // namespace OHOS::Ace
