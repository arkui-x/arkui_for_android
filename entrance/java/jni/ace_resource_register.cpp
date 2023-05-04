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

#include "adapter/android/entrance/java/jni/ace_resource_register.h"

#include <cstdint>
#include <functional>
#include <sstream>

#include "base/log/log.h"
#include "base/utils/utils.h"
#include "core/common/container_scope.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::Platform {

AceResourceRegister::AceResourceRegister(jobject object, int32_t instanceId)
    : object_(JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), object)),
      instanceId_(instanceId)
{}

bool AceResourceRegister::Initialize(JNIEnv* env)
{
    if (env == nullptr || object_ == nullptr) {
        LOGE("AceResourceRegister: env or object is invalid");
        return false;
    }

    const jclass clazz = env->GetObjectClass(object_.get());
    if (clazz == nullptr) {
        LOGE("AceResourceRegister: fail to get evnet clazz");
        return false;
    }

    registerResourceMethod_ = env->GetMethodID(clazz, "createResource", "(Ljava/lang/String;Ljava/lang/String;)J");
    releaseResourceMethod_ = env->GetMethodID(clazz, "releaseResource", "(Ljava/lang/String;)Z");
    onCallMethod_ = env->GetMethodID(clazz, "onCallMethod", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");

    env->DeleteLocalRef(clazz);
    if (registerResourceMethod_ == nullptr || releaseResourceMethod_ == nullptr || onCallMethod_ == nullptr) {
        LOGE("AceResourceRegister: fail to get method id");
        return false;
    }

    static const JNINativeMethod methods[] = {
        {
            .name = "nativeOnEvent",
            .signature = "(JLjava/lang/String;Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&OnCallEvent),
        },
    };

    const jclass registerClass = env->FindClass("ohos/ace/adapter/AceResourceRegister");
    if (registerClass == nullptr) {
        LOGE("AceResourceRegister: fail to find java class");
        return false;
    }

    bool ret = env->RegisterNatives(registerClass, methods, ArraySize(methods)) == 0;
    env->DeleteLocalRef(registerClass);
    return ret;
}

void AceResourceRegister::OnCallEvent(JNIEnv* env, jclass clazz, jlong resRegisterPtr, jstring id, jstring param)
{
    auto resRegister = JavaLongToPointer<AceResourceRegister>(resRegisterPtr);
    if (!resRegister) {
        LOGE("AceResourceRegister OnCallEvent: resRegister is null");
        return;
    }
    if (!resRegister->IsRunOnPlatfromThread()) {
        LOGE("AceResourceRegister OnCallEvent: not run on platform thread");
        return;
    }
    if (!env) {
        LOGE("AceResourceRegister OnCallEvent: env is null");
        return;
    }
    std::string eventId;
    std::string eventParam;
    auto idStr = env->GetStringUTFChars(id, nullptr);
    if (idStr != nullptr) {
        eventId = idStr;
        env->ReleaseStringUTFChars(id, idStr);
    }
    auto paramStr = env->GetStringUTFChars(param, nullptr);
    if (paramStr != nullptr) {
        eventParam = paramStr;
        env->ReleaseStringUTFChars(param, paramStr);
    }
    ContainerScope scope(resRegister->instanceId_);
    resRegister->OnEvent(eventId, eventParam);
}

bool AceResourceRegister::OnMethodCall(const std::string& method, const std::string& param, std::string& result)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("AceResourceRegister OnMethodCall: env is null");
        return false;
    }

    bool hasException = false;
    jstring jMethod = env->NewStringUTF(method.c_str());
    if (jMethod == nullptr) {
        return false;
    }
    jstring jParam = env->NewStringUTF(param.c_str());
    if (jParam == nullptr) {
        env->DeleteLocalRef(jMethod);
        return false;
    }
    jstring jResult = static_cast<jstring>(env->CallObjectMethod(object_.get(), onCallMethod_, jMethod, jParam));
    if (env->ExceptionCheck()) {
        LOGE("AceResourceRegister OnMethodCall: has exception");
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
    env->DeleteLocalRef(jMethod);
    env->DeleteLocalRef(jParam);
    env->DeleteLocalRef(jResult);
    return !hasException;
}

int64_t AceResourceRegister::CreateResource(const std::string& resourceType, const std::string& param)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("AceResourceRegister CreateResource: env is null");
        return false;
    }

    LOGI("CreateResource type:%{public}s param:%{public}s", resourceType.c_str(), param.c_str());

    bool hasException = false;
    int64_t resourceId = -1;
    jstring jResourceType = env->NewStringUTF(resourceType.c_str());
    if (jResourceType == nullptr) {
        return false;
    }
    jstring jParam = env->NewStringUTF(param.c_str());
    if (jParam == nullptr) {
        env->DeleteLocalRef(jResourceType);
        return false;
    }
    jlong jResult = env->CallLongMethod(object_.get(), registerResourceMethod_, jResourceType, jParam);

    if (env->ExceptionCheck()) {
        LOGE("AceResourceRegister CreateResource: has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        hasException = true;
    }
    if (!hasException) {
        resourceId = static_cast<int64_t>(jResult);
    }
    env->DeleteLocalRef(jResourceType);
    env->DeleteLocalRef(jParam);
    LOGI("CreateResource resourceId:%{public}ld", resourceId);

    return resourceId;
}

bool AceResourceRegister::ReleaseResource(const std::string& resourceHash)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("AceResourceRegister ReleaseResource: env is null");
        return false;
    }

    bool hasException = false;
    bool result = false;

    jstring jResrouceHash = env->NewStringUTF(resourceHash.c_str());
    if (jResrouceHash == nullptr) {
        return false;
    }

    jboolean jResult = env->CallBooleanMethod(object_.get(), releaseResourceMethod_, jResrouceHash);
    if (env->ExceptionCheck()) {
        LOGE("AceResourceRegister ReleaseResource: has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        hasException = true;
    }
    if (!hasException && jResult) {
        result = static_cast<bool>(jResult);
    }

    env->DeleteLocalRef(jResrouceHash);

    return !hasException;
}

bool AceResourceRegister::IsRunOnPlatfromThread()
{
    auto context = GetPipelineContext().Upgrade();
    if (!context) {
        return false;
    }
    auto taskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::PLATFORM);
    return taskExecutor.IsRunOnCurrentThread();
}

} // namespace OHOS::Ace::Platform
