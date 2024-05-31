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

#include "adapter/android/entrance/java/jni/java_event_callback.h"

#include "base/log/log.h"

namespace OHOS::Ace::Platform {

JavaEventCallback::JavaEventCallback(jobject object)
    : env_(JniEnvironment::GetInstance().GetJniEnv()),
      object_(JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), object))
{}

bool JavaEventCallback::Initialize()
{
    if (!object_ || !env_) {
        LOGE("JavaEventCallback: the object or env is null");
        return false;
    }
    const jclass clazz = env_->GetObjectClass(object_.get());
    if (clazz == nullptr) {
        LOGE("JavaEventCallback: fail to get event class");
        return false;
    }
    onEventMethodId_ = env_->GetMethodID(clazz, "onEvent", "(ILjava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    onFinishMethodId_ = env_->GetMethodID(clazz, "onFinish", "()V");
    onStatusBarBgColorMethodId_ = env_->GetMethodID(clazz, "onStatusBarBgColorChanged", "(I)V");
    env_->DeleteLocalRef(clazz);
    if ((onEventMethodId_ == nullptr) || (onFinishMethodId_ == nullptr) || (onStatusBarBgColorMethodId_ == nullptr)) {
        LOGE("JavaEventCallback: fail to get method");
        return false;
    }
    return true;
}

bool JavaEventCallback::OnEvent(
    int32_t pageId, const std::string& eventId, const std::string& param, std::string& result) const
{
    const auto& env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("JavaEventCallback: env is null");
        return false;
    }
    bool hasException = false;
    jstring jEventId = env->NewStringUTF(eventId.c_str());
    if (jEventId == nullptr) {
        return false;
    }
    jstring jParam = env->NewStringUTF(param.c_str());
    if (jParam == nullptr) {
        env->DeleteLocalRef(jEventId);
        return false;
    }
    jstring jResult =
        static_cast<jstring>(env->CallObjectMethod(object_.get(), onEventMethodId_, pageId, jEventId, jParam));
    if (env->ExceptionCheck()) {
        LOGE("JavaEventCallback: onEvent has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        hasException = true;
    }
    if (!hasException && jResult) {
        auto resultStr = env->GetStringUTFChars(jResult, nullptr);
        if (resultStr != nullptr) {
            result = resultStr;
            env->ReleaseStringUTFChars(jResult, resultStr);
        }
    }
    env->DeleteLocalRef(jEventId);
    env->DeleteLocalRef(jParam);
    env->DeleteLocalRef(jResult);
    return !hasException;
}

void JavaEventCallback::OnFinish() const
{
    const auto& env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("JavaEventCallback: env is null");
        return;
    }
    env->CallVoidMethod(object_.get(), onFinishMethodId_);
    if (env->ExceptionCheck()) {
        LOGE("JavaEventCallback: OnFinish has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void JavaEventCallback::OnStatusBarBgColorChanged(uint32_t color)
{
    const auto& env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("JavaEventCallback: env is null");
        return;
    }
    env->CallVoidMethod(object_.get(), onStatusBarBgColorMethodId_, static_cast<int32_t>(color));
    if (env->ExceptionCheck()) {
        LOGE("JavaEventCallback: OnStatusBarBgColorChanged has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

} // namespace OHOS::Ace::Platform
