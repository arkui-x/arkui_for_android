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

#include "adapter/android/entrance/java/jni/download_manager_jni.h"

#include "base/log/log.h"

namespace OHOS::Ace::Platform {

namespace {

const char DOWNLOAD_MANAGER_CLASS_NAME[] = "ohos/ace/adapter/DownloadManager";
const char METHOD_DOWNLOAD[] = "download";
const char SIGNATURE_DOWNLOAD[] = "(Ljava/lang/String;)[B";

}

Platform::JniEnvironment::JavaGlobalRef DownloadManagerJni::downloadManagerClass_ = { nullptr, nullptr };
jmethodID DownloadManagerJni::downloadMethod_ = nullptr;

bool DownloadManagerJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    if (!env) {
        LOGE("JNIEnv is null when registering DownloadManager jni!");
        return false;
    }

    jclass myClass = env->FindClass(DOWNLOAD_MANAGER_CLASS_NAME);
    if (myClass == nullptr) {
        LOGE("Failed to find DownloadManager class: %{public}s", DOWNLOAD_MANAGER_CLASS_NAME);
        return false;
    }

    downloadMethod_ = env->GetStaticMethodID(myClass, METHOD_DOWNLOAD, SIGNATURE_DOWNLOAD);
    if (downloadMethod_ == nullptr) {
        env->DeleteLocalRef(myClass);
        LOGE("Failed to find method: %{public}s", METHOD_DOWNLOAD);
        return false;
    }
    downloadManagerClass_ = Platform::JniEnvironment::MakeJavaGlobalRef(env, static_cast<jobject>(myClass));

    env->DeleteLocalRef(myClass);
    return true;
}

bool DownloadManagerJni::Download(const std::string& url, std::vector<uint8_t>& dataOut)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("Download: env is null, url: %{private}s", url.c_str());
        return false;
    }
    if (downloadManagerClass_ == nullptr || downloadMethod_ == nullptr) {
        LOGE("downloadManagerClass or downloadMethod is null. Stop downloading data, url: %{private}s", url.c_str());
        return false;
    }
    jstring jUrl = env->NewStringUTF(url.c_str());
    if (jUrl == nullptr) {
        return false;
    }

    jobject jRetObj =
        env->CallStaticObjectMethod(static_cast<jclass>(downloadManagerClass_.get()), downloadMethod_, jUrl);
    if (env->ExceptionCheck()) {
        LOGE("Download: has exception, url: %{private}s", url.c_str());
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    if (!jRetObj) {
        LOGE("Download: get null object, url: %{private}s", url.c_str());
        return false;
    }
    jbyteArray jData = static_cast<jbyteArray>(jRetObj);
    jsize numBytes = env->GetArrayLength(jData);
    if (numBytes == 0) {
        LOGW("Download: the returned data length is 0, url: %{private}s", url.c_str());
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

} // namespace OHOS::Ace::Platform
