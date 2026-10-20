/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <dlfcn.h>

#include "base/log/log_wrapper.h"
#include "base/network/download_manager.h"
#include "core/image/image_file_cache.h"
#include "cache_download_client.h"
#include "cache_error.h"
#include "cache_types.h"

#define ACE_FORCE_EXPORT __attribute__((visibility("default")))

namespace OHOS::Ace {
DownloadManager* CreateAndroidDownloadManager();
std::unique_ptr<DownloadManager> DownloadManager::instance_ = nullptr;
std::mutex DownloadManager::mutex_;

namespace {
constexpr int32_t MAXIMUM_WAITING_PERIOD = 2800;
} // namespace

class ACE_FORCE_EXPORT DownloadManagerImpl : public DownloadManager {
public:
    explicit DownloadManagerImpl(const std::string& path)
    {
        Plugin::CacheDownload::CacheDownloadClient::GetInstance().SetFileCachePath(path);
    }
    ~DownloadManagerImpl() {}

    bool Download(const std::string& url, std::vector<uint8_t>& dataOut) override
    {
        return false;
    }

    bool Download(const std::string& url, const std::shared_ptr<DownloadResult>& downloadResult) override
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
        auto callback = std::make_shared<OHOS::Plugin::CacheDownload::CacheDownloadCallback>();
        callback->onSuccess = [this, successCallback = downloadCallback.successCallback, instanceId,
                                  url](const std::string&) {
            auto cached = OHOS::Plugin::CacheDownload::CacheDownloadClient::GetInstance().FetchBytes(url);
            std::string data(cached.begin(), cached.end());
            successCallback(std::move(data), true, instanceId);
            RemoveDownloadTaskWithPreload(url, false);
        };
        callback->onError = [this, failCallback = downloadCallback.failCallback, instanceId,
                                url](const std::string&,
                                const OHOS::Plugin::CacheDownload::CacheDownloadError& error) {
            std::string errorMsg = "Http task of url " + url + " failed, code " +
                                   std::to_string(error.code) + ", msg: " + error.message;
            failCallback(errorMsg,
                { ImageErrorCode::GET_IMAGE_ASYNC_HTTP_FAILED, "async http task of uri failed." },
                true, instanceId);
            RemoveDownloadTaskWithPreload(url, false);
        };
        OHOS::Plugin::CacheDownload::CacheDownloadClient::GetInstance().Download(url, callback);
        return true;
    }

    bool fetchCachedResult(const std::string& url, std::string& result) override
    {
        auto bytes = OHOS::Plugin::CacheDownload::CacheDownloadClient::GetInstance().FetchBytes(url);
        if (!bytes.empty()) {
            result.assign(bytes.begin(), bytes.end());
            return true;
        }
        return false;
    }

    void RemoveUrlCache(const std::string& url) override
    {
        OHOS::Plugin::CacheDownload::CacheDownloadClient::GetInstance().Remove(url);
    }

    bool DownloadSyncWithPreload(
        DownloadCallback&& downloadCallback, const std::string& url, int32_t instanceId) override
    {
        auto condition = std::make_shared<DownloadCondition>();
        auto callback = std::make_shared<OHOS::Plugin::CacheDownload::CacheDownloadCallback>();
        callback->onSuccess = [this, condition, url](const std::string&) {
            auto cached = OHOS::Plugin::CacheDownload::CacheDownloadClient::GetInstance().FetchBytes(url);
            {
                std::unique_lock<std::mutex> lock(condition->downloadMutex);
                condition->downloadSuccess = true;
                condition->dataOut.assign(cached.begin(), cached.end());
            }
            condition->cv.notify_all();
            RemoveDownloadTaskWithPreload(url, false);
        };
        callback->onError = [this, condition, url](
                                const std::string&,
                                const OHOS::Plugin::CacheDownload::CacheDownloadError& error) {
            {
                std::unique_lock<std::mutex> lock(condition->downloadMutex);
                std::string errorMsg = "Http task of url " + url + " failed, code " +
                                       std::to_string(error.code) + ", msg: " + error.message;
                condition->errorMsg = errorMsg;
                condition->errorInfo = { ImageErrorCode::GET_IMAGE_SYNC_HTTP_FAILED,
                    "sync http task of uri failed." };
                condition->downloadSuccess = false;
            }
            condition->cv.notify_all();
            RemoveDownloadTaskWithPreload(url, false);
        };
        OHOS::Plugin::CacheDownload::CacheDownloadClient::GetInstance().Download(url, callback);
        return HandleDownloadResult(true, std::move(downloadCallback), condition, instanceId, url);
    }

    bool RemoveDownloadTask(const std::string& url, int32_t nodeId, bool isCancel = true) override
    {
        return false;
    }

    bool RemoveDownloadTaskWithPreload(const std::string& url, bool isCancel = true) override
    {
        // Same semantics as the OHOS implementation (download_manager_impl.cpp): this
        // only cleans up the download task. CacheDownloadClient::Remove(url) must NOT
        // be called here - it evicts the RAM entry and deletes the cache file, so every
        // finished download would be removed right after being written and cold starts
        // would re-download the same images forever.
        if (isCancel) {
            OHOS::Plugin::CacheDownload::CacheDownloadClient::GetInstance().Cancel(url);
        }
        return true;
    }

    void* WrapDownloadInfoToNapiValue(void* env, const ImageErrorInfo& errorInfo) override
    {
        return nullptr;
    }

private:
    bool HandleDownloadResult(bool result, DownloadCallback&& downloadCallback,
        const std::shared_ptr<DownloadCondition>& downloadCondition, int32_t instanceId, const std::string& url)
    {
        if (!result) {
            return result;
        }
        {
            std::unique_lock<std::mutex> downloadLock(downloadCondition->downloadMutex);
            // condition_variable is waiting for any of the success, cancel or failed to respond in sync mode
            downloadCondition->cv.wait_for(
                downloadLock, std::chrono::milliseconds(MAXIMUM_WAITING_PERIOD), [downloadCondition] {
                    return downloadCondition ? downloadCondition->downloadSuccess.has_value() : false;
                });
        }
        if (downloadCondition->downloadSuccess.value_or(false)) {
            downloadCallback.successCallback(std::move(downloadCondition->dataOut), false, instanceId);
        } else {
            downloadCallback.failCallback(downloadCondition->errorMsg, downloadCondition->errorInfo, false, instanceId);
        }
        return true;
    }

    bool HandleDownloadResult(bool result, const std::shared_ptr<DownloadCondition>& downloadCondition,
        const std::shared_ptr<DownloadResult>& downloadResult)
    {
        if (!result) {
            return result;
        }
        {
            std::unique_lock<std::mutex> downloadLock(downloadCondition->downloadMutex);
            // condition_variable is waiting for any of the success, cancel or failed to respond in sync mode
            downloadCondition->cv.wait_for(
                downloadLock, std::chrono::milliseconds(MAXIMUM_WAITING_PERIOD), [downloadCondition, downloadResult] {
                    return downloadCondition ? downloadResult->downloadSuccess.has_value() : false;
                });
        }
        return true;
    }

    bool IsContains(const std::string& url) override
    {
        return false;
    }
};

DownloadManager* DownloadManager::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
            auto path = ImageFileCache::GetInstance().GetImageCacheFilePath();
            DownloadManager* manager = new DownloadManagerImpl(path);
            if (manager != nullptr) {
                instance_.reset(manager);
            } else {
                instance_.reset(CreateAndroidDownloadManager());
            }
        }
    }
    return instance_.get();
}

} // namespace OHOS::Ace