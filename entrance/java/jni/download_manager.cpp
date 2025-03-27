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

#include "base/network/download_manager.h"

#include <memory>
#include <mutex>
#include <optional>

#include "adapter/android/entrance/java/jni/download_manager_jni.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace {
std::unique_ptr<DownloadManager> DownloadManager::instance_ = nullptr;
std::mutex DownloadManager::mutex_;
class DownloadManagerImpl final : public DownloadManager {
public:
    bool Download(const std::string& url, std::vector<uint8_t>& dataOut) override
    {
        return Platform::DownloadManagerJni::Download(url, dataOut);
    }

    bool Download(const std::string& url, const std::shared_ptr<DownloadResult>& result) override
    {
        return false;
    }

    bool DownloadAsync(
        DownloadCallback&& downloadCallback, const std::string& url, int32_t instanceId, int32_t nodeId) override
    {
        return false;
    }

    bool DownloadSync(
        DownloadCallback&& downloadCallback, const std::string& url, int32_t instanceId, int32_t nodeId) override
    {
        return false;
    }

    bool DownloadAsyncWithPreload(
        DownloadCallback&& downloadCallback, const std::string& url, int32_t instanceId) override
    {
        return false;
    }

    bool DownloadSyncWithPreload(
        DownloadCallback&& downloadCallback, const std::string& url, int32_t instanceId) override
    {
        return false;
    }

    bool fetchCachedResult(const std::string& url, std::string& result) override
    {
        return false;
    }

    bool RemoveDownloadTask(const std::string& url, int32_t nodeId, bool isCancel = true) override
    {
        return false;
    }

    bool RemoveDownloadTaskWithPreload(const std::string& url, bool isCancel = true) override
    {
        return false;
    }

    bool IsContains(const std::string& url) override
    {
        return false;
    }

    DownloadManagerImpl() = default;

    ~DownloadManagerImpl() = default;
};

DownloadManager* DownloadManager::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
            instance_.reset(new DownloadManagerImpl());
        }
    }
    return instance_.get();
}

} // namespace OHOS::Ace
