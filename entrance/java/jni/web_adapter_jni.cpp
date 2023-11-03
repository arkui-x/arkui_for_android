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

#include "web_adapter_jni.h"

#include <unistd.h>
#include <sstream>
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

#include "jni_environment.h"
#include "core/components_ng/pattern/web/cross_platform/web_object_event.h"

namespace OHOS::Ace::Platform {
class WebObject : public AceType {
public:
    DECLARE_ACE_TYPE(WebObject, AceType)

public:
    WebObject(void* object) :
        object_(Platform::JniEnvironment::MakeJavaGlobalRef(
            Platform::JniEnvironment::GetInstance().GetJniEnv(), *(jobject *)object)) {}
    Platform::JniEnvironment::JavaGlobalRef& Get()
    {
        return object_;
    }
private:
    Platform::JniEnvironment::JavaGlobalRef object_;
};

using namespace JNI_TOOL;
namespace JNI_TOOL {
std::string GetStringFromJNI(const jobject& obj, std::string funcName)
{
    LOGI("jni call %{public}s", funcName.c_str());
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return "";
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return "";
    }

    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()Ljava/lang/String;");
    if (method == nullptr) {
        LOGE("fail to get method id: %{public}s", funcName.c_str());
        return "";
    }
    jstring jsReturnString = (jstring)env->CallObjectMethod(obj, method);
    const char *cReturnString = env->GetStringUTFChars(jsReturnString, JNI_FALSE);
    std::string returnString(cReturnString);
    env->ReleaseStringUTFChars(jsReturnString, cReturnString);
    return returnString;
}

int GetIntFromJNI(const jobject& obj, std::string funcName)
{
    LOGI("jni call %{public}s", funcName.c_str());
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return -1;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return -1;
    }
    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()I");
    if (method == nullptr) {
        LOGE("fail to get method id: %{public}s", funcName.c_str());
        return -1;
    }
    int retValue = env->CallIntMethod(obj, method);
    return retValue;
}

long GetLongFromJNI(const jobject& obj, std::string funcName)
{
    LOGI("jni call %{public}s", funcName.c_str());
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return 0;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return 0;
    }
    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()J");
    if (method == nullptr) {
        LOGE("fail to get method id: %{public}s", funcName.c_str());
        return 0;
    }
    jlong returnValue = env->CallLongMethod(obj, method);
    return (long)returnValue;
}

float GetFloatFromJNI(const jobject& obj, std::string funcName)
{
    LOGI("jni call %{public}s", funcName.c_str());
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return 0;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return 0;
    }
    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()F");
    if (method == nullptr) {
        LOGE("fail to get method id: %{public}s", funcName.c_str());
        return 0;
    }
    int retValue = env->CallFloatMethod(obj, method);
    return retValue;
}

bool GetBoolFromJNI(const jobject& obj, std::string funcName)
{
    LOGI("jni call %{public}s", funcName.c_str());
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return false;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return false;
    }
    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()Z");
    if (method == nullptr) {
        LOGE("fail to get method id: %{public}s", funcName.c_str());
        return false;
    }
    bool returnValue = env->CallBooleanMethod(obj, method);
    return returnValue;
}

std::map<std::string, std::string> GetStringMapFromJNI(const jobject& obj, std::string funcName)
{
    LOGI("jni call %{public}s", funcName.c_str());
    std::map<std::string, std::string> returnMap;
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return returnMap;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return returnMap;
    }

    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()Ljava/util/Map;");
    if (method == nullptr) {
        LOGE("fail to get method id: %{public}s", funcName.c_str());
        return returnMap;
    }
    jobject jRequestHeaderMap = (jobject)env->CallObjectMethod(obj, method);

    jclass hashmapClass = env->FindClass("java/util/HashMap");
    jmethodID entrySetMID = env->GetMethodID(hashmapClass, "entrySet", "()Ljava/util/Set;");

    jobject setObj = env->CallObjectMethod(jRequestHeaderMap, entrySetMID);

    jclass setClass = env->FindClass("java/util/Set");
    jmethodID iteratorMID = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");

    jobject iteratorObj = env->CallObjectMethod(setObj, iteratorMID);

    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jmethodID hasNextMID = env->GetMethodID(iteratorClass, "hasNext", "()Z");

    jmethodID nextMID = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");

    jclass entryClass = env->FindClass("java/util/Map$Entry");
    jmethodID getKeyMID = env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
    jmethodID getValueMID = env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");
    while (env->CallBooleanMethod(iteratorObj, hasNextMID)) {
        jobject entryObj = env->CallObjectMethod(iteratorObj, nextMID);
        jstring jsHeaderKey = (jstring) env->CallObjectMethod(entryObj, getKeyMID);
        if (jsHeaderKey == NULL)
            continue;
        const char *cHeaderKey = env->GetStringUTFChars(jsHeaderKey, JNI_FALSE);
        std::string headerKey(cHeaderKey);

        jstring jsHeaderValue = (jstring) env->CallObjectMethod(entryObj, getValueMID);
        if (jsHeaderValue == NULL)
            continue;
        const char *cHeaderValue = env->GetStringUTFChars(jsHeaderValue, JNI_FALSE);
        std::string headerValue(cHeaderValue);
        returnMap.insert({headerKey, headerValue});

        env->ReleaseStringUTFChars(jsHeaderKey, cHeaderKey);
        env->ReleaseStringUTFChars(jsHeaderValue, cHeaderValue);
    }
    return returnMap;
}
}

class JniWebResourceRequestObject final : public WebResourceRequestObject {
public:
    std::map<std::string, std::string> GetRequestHeader(void *object);
    std::string GetRequestUrl(void *object);
    std::string GetMethod(void *object);
    bool IsRequestGesture(void *object);
    bool IsMainFrame(void *object);
    bool IsRedirect(void *object);
};

std::map<std::string, std::string> JniWebResourceRequestObject::GetRequestHeader(void *object)
{
    return GetStringMapFromJNI(*(jobject *)object, "getRequestHeader");
}

std::string JniWebResourceRequestObject::GetRequestUrl(void *object)
{
    return GetStringFromJNI(*(jobject *)object, "getRequestUrl");
}

std::string JniWebResourceRequestObject::GetMethod(void *object)
{
    return GetStringFromJNI(*(jobject *)object, "getMethod");
}

bool JniWebResourceRequestObject::IsRequestGesture(void *object)
{
    return GetBoolFromJNI(*(jobject *)object, "isRequestGesture");
}

bool JniWebResourceRequestObject::IsMainFrame(void *object)
{
    return GetBoolFromJNI(*(jobject *)object, "isMainFrame");
}

bool JniWebResourceRequestObject::IsRedirect(void *object)
{
    return GetBoolFromJNI(*(jobject *)object, "isRedirect");
}

class JniWebResourceErrorObject final : public WebResourceErrorObject {
public:
    std::string GetErrorInfo(void *object);
    int GetErrorCode(void *object);
};

std::string JniWebResourceErrorObject::GetErrorInfo(void *object)
{
    return GetStringFromJNI(*(jobject *)object, "getErrorInfo");
}

int JniWebResourceErrorObject::GetErrorCode(void *object)
{
    return GetIntFromJNI(*(jobject *)object, "getErrorCode");
}

class JniWebResourceResponseObject final : public WebResourceResponseObject {
public:
    std::map<std::string, std::string> GetResponseHeader(void* object);
    std::string GetResponseData(void* object);
    std::string GetEncoding(void* object);
    std::string GetMimeType(void* object);
    std::string GetReason(void* object);
    int GetStatusCode(void* object);
};

std::map<std::string, std::string> JniWebResourceResponseObject::GetResponseHeader(void *object)
{
    return GetStringMapFromJNI(*(jobject *)object, "getResponseHeader");
}

std::string JniWebResourceResponseObject::GetResponseData(void *object)
{
    return GetStringFromJNI(*(jobject *)object, "getResponseData");
}

std::string JniWebResourceResponseObject::GetEncoding(void *object)
{
    return GetStringFromJNI(*(jobject *)object, "getResponseEncoding");
}

std::string JniWebResourceResponseObject::GetMimeType(void *object)
{
    return GetStringFromJNI(*(jobject *)object, "getResponseMimeType");
}

std::string JniWebResourceResponseObject::GetReason(void *object)
{
    return GetStringFromJNI(*(jobject *)object, "getReason");
}

int JniWebResourceResponseObject::GetStatusCode(void *object)
{
    return GetIntFromJNI(*(jobject *)object, "getResponseCode");
}

class JniWebScrollObjectWrapper final : public WebScrollObject {
public:
    float GetX(void* object);
    float GetY(void* object);
};

float JniWebScrollObjectWrapper::GetX(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "getX");
}

float JniWebScrollObjectWrapper::GetY(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "getY");
}

class JniWebScaleChangeObjectWrapper final : public WebScaleChangeObject {
public:
    float GetNewScale(void* object);
    float GetOldScale(void* object);
};

float JniWebScaleChangeObjectWrapper::GetNewScale(void* object)
{
    return GetFloatFromJNI(*(jobject *)object, "getNewScale");
}

float JniWebScaleChangeObjectWrapper::GetOldScale(void* object)
{
    return GetFloatFromJNI(*(jobject *)object, "getOldScale");
}

class JniWebConsoleMessageObject final : public WebConsoleMessageObject {
public:
    std::string GetMessage(void *object);
    int GetMessageLevel(void *object);
    std::string GetSourceId(void *object);
    int GetLineNumber(void *object);
};

std::string JniWebConsoleMessageObject::GetMessage(void *object)
{
    return GetStringFromJNI(*(jobject *)object, "getMessage");
}

int JniWebConsoleMessageObject::GetMessageLevel(void *object)
{
    return GetIntFromJNI(*(jobject *)object, "getMessageLevel");
}

std::string JniWebConsoleMessageObject::GetSourceId(void *object)
{
    return GetStringFromJNI(*(jobject *)object, "getSourceId");
}

int JniWebConsoleMessageObject::GetLineNumber(void *object)
{
    return GetIntFromJNI(*(jobject *)object, "getLineNumber");
}

bool WebAdapterJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    auto JWebResourceRequestObject = Referenced::MakeRefPtr<JniWebResourceRequestObject>();
    WebObjectEventManager::GetInstance().SetResourceRequestObject(JWebResourceRequestObject);
    auto JWebResourceErrorObject = Referenced::MakeRefPtr<JniWebResourceErrorObject>();
    WebObjectEventManager::GetInstance().SetResourceErrorObject(JWebResourceErrorObject);
    auto JWebOnScrollObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebScrollObjectWrapper>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetScrollObject(JWebOnScrollObject);
    auto JWebResourceResponseObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebResourceResponseObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetResourceResponseObject(JWebResourceResponseObject);
    auto JWebConsoleMessageObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebConsoleMessageObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetConsoleMessageObject(JWebConsoleMessageObject);
    auto JWebScaleChangeObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebScaleChangeObjectWrapper>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetScaleChangeObject(JWebScaleChangeObject);

    static const JNINativeMethod methods[] = {
        {
            .name = "nativeOnObjectEvent",
            .signature =
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/Object;)V",
            .fnPtr = reinterpret_cast<void*>(&NativeOnObjectEvent),
        },
        {
            .name = "nativeOnObjectEventWithBoolReturn",
            .signature =
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/Object;)Z",
            .fnPtr = reinterpret_cast<void*>(&NativeOnObjectEventWithBoolReturn),
        },
    };

    if (!env) {
        LOGE("JNIEnv is null when register FormAdapterJni jni!");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/ace/adapter/capability/web/AceWebBase");
    if (clazz == nullptr) {
        LOGE("Find AceWebBase class failed!");
        return false;
    }

    auto res = env->RegisterNatives(clazz, methods, ArraySize(methods)) == 0;
    env->DeleteLocalRef(clazz);
    return res;
}

void WebAdapterJni::NativeOnObjectEvent(JNIEnv* env, jobject clazz, jstring id, jstring param, jobject object)
{
    if (!env) {
        LOGW("env is null");
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
    WebObjectEventManager::GetInstance().OnObjectEvent(eventId, eventParam, (void *)&object);
}

bool WebAdapterJni::NativeOnObjectEventWithBoolReturn(JNIEnv* env, jobject clazz, jstring id, jstring param, jobject object)
{
    if (!env) {
        LOGW("env is null");
        return false;
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
    return WebObjectEventManager::GetInstance().OnObjectEventWithBoolReturn(eventId, eventParam, (void *)&object);
}
} // namespace OHOS::Ace
