/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/utils/singleton.h"
#include "base/utils/utils.h"

namespace OHOS::Ace {

namespace {

const char DOWNLOAD_MANAGER_CLASS_NAME[] = "ohos/ace/adapter/DownloadManager";
const char METHOD_DOWNLOAD[] = "download";
const char SIGNATURE_DOWNLOAD[] = "(Ljava/lang/String;)[B";

class DownloadManagerImpl final : public DownloadManager, public Singleton<DownloadManagerImpl> {
    DECLARE_SINGLETON(DownloadManagerImpl);
    ACE_DISALLOW_MOVE(DownloadManagerImpl);

public:
    bool Download(const std::string& url, std::vector<uint8_t>& dataOut) override
    {
        auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
        if (!env) {
            LOGE("Download: env is null");
            return false;
        }

        if (!Initizlize()) {
            return false;
        }

        bool hasException = false;
        jstring jUrl = env->NewStringUTF(url.c_str());
        if (jUrl == nullptr) {
            return false;
        }

        jobject jRetObj = env->CallStaticObjectMethod(static_cast<jclass>(downloadManagerClass_.get()), downloadMethod_, jUrl);
        if (env->ExceptionCheck()) {
            LOGE("Download: has exception");
            env->ExceptionDescribe();
            env->ExceptionClear();
            hasException = true;
        }
        if (!hasException && jRetObj) {
            jbyteArray jData = static_cast<jbyteArray>(jRetObj);
            jsize numBytes = env->GetArrayLength(jData);
            if (numBytes == 0) {
                LOGW("Download: the returned data length is 0");
                env->DeleteLocalRef(jUrl);
                env->DeleteLocalRef(jRetObj);
                return false;
            }
            uint8_t* data = reinterpret_cast<uint8_t*>(env->GetByteArrayElements(jData, NULL));
            std::copy(data, data + numBytes, std::back_inserter(dataOut));
            env->ReleaseByteArrayElements(jData, reinterpret_cast<jbyte*>(data), JNI_ABORT);
            env->DeleteLocalRef(jUrl);
            env->DeleteLocalRef(jRetObj);
            return true;
        }
        return false;
    }

private:
    bool Initizlize()
    {
        if (initialized_) {
            return true;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (initialized_) {
            return true;
        }

        auto jniEnv = Platform::JniEnvironment::GetInstance().GetJniEnv();
        if (!jniEnv) {
            return false;
        }
        jclass clazz = jniEnv->FindClass(DOWNLOAD_MANAGER_CLASS_NAME);
        if (clazz == nullptr) {
            LOGE("DownloadManagerImpl: Failed to find class");
            return false;
        }

        downloadMethod_ = jniEnv->GetStaticMethodID(clazz, METHOD_DOWNLOAD, SIGNATURE_DOWNLOAD);
        if (downloadMethod_ == nullptr) {
            jniEnv->DeleteLocalRef(clazz);
            LOGE("DownloadManagerImpl: Failed to find method");
            return false;
        }

        downloadManagerClass_ = Platform::JniEnvironment::MakeJavaGlobalRef(
            jniEnv, static_cast<jobject>(clazz));
        jniEnv->DeleteLocalRef(clazz);
        initialized_ = true;
        return true;
    }

    std::mutex mutex_;
    bool initialized_ = false;
    Platform::JniEnvironment::JavaGlobalRef downloadManagerClass_ { nullptr, nullptr };
    jmethodID downloadMethod_ = nullptr;
};

DownloadManagerImpl::DownloadManagerImpl() = default;

DownloadManagerImpl::~DownloadManagerImpl() = default;

}

DownloadManager& DownloadManager::GetInstance()
{
    return Singleton<DownloadManagerImpl>::GetInstance();
}

} // namespace OHOS::Ace
