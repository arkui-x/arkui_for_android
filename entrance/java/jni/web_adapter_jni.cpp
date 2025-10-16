/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <unordered_map>
#include <sstream>
#include <mutex>
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

#include "jni_environment.h"
#include "core/components_ng/pattern/web/web_object_event.h"

namespace OHOS::Ace::Platform {
static constexpr const int RESPONSE_STATUS_CODE_RANGE_ONE_MIN = 100;
static constexpr const int RESPONSE_STATUS_CODE_RANGE_ONE_MAX = 299;
static constexpr const int RESPONSE_STATUS_CODE_RANGE_TWO_MIN = 400;
static constexpr const int RESPONSE_STATUS_CODE_RANGE_TWO_MAX = 599;
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
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return 0.0f;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return 0.0f;
    }
    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()F");
    if (method == nullptr) {
        LOGE("fail to get method id: %{public}s", funcName.c_str());
        return 0.0f;
    }
    float retValue = env->CallFloatMethod(obj, method);
    return retValue;
}

bool GetBoolFromJNI(const jobject& obj, std::string funcName)
{
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
        jstring jsHeaderKey = (jstring)env->CallObjectMethod(entryObj, getKeyMID);
        if (jsHeaderKey == NULL)
            continue;
        const char* cHeaderKey = env->GetStringUTFChars(jsHeaderKey, JNI_FALSE);
        std::string headerKey(cHeaderKey);

        jstring jsHeaderValue = (jstring)env->CallObjectMethod(entryObj, getValueMID);
        if (jsHeaderValue == NULL)
            continue;
        const char* cHeaderValue = env->GetStringUTFChars(jsHeaderValue, JNI_FALSE);
        std::string headerValue(cHeaderValue);
        returnMap.insert({ headerKey, headerValue });

        env->ReleaseStringUTFChars(jsHeaderKey, cHeaderKey);
        env->ReleaseStringUTFChars(jsHeaderValue, cHeaderValue);
    }
    return returnMap;
}

std::vector<std::string> GetStringVectorFromJNI(const jobject& obj, std::string funcName)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return std::vector<std::string>();
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return std::vector<std::string>();
    }

    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()[Ljava/lang/String;");
    if (method == nullptr) {
        LOGE("fail to get method id: %{public}s", funcName.c_str());
        return std::vector<std::string>();
    }
    std::vector<std::string> strVector;
    jstring jstr = nullptr;
    const char *str = nullptr;
    jobjectArray jsReturnObjectArray = (jobjectArray)env->CallObjectMethod(obj, method);
    jsize arrayLength = env->GetArrayLength(jsReturnObjectArray);
    for (int index = 0; index < arrayLength; index++) {
        jstr = static_cast<jstring>(env->GetObjectArrayElement(jsReturnObjectArray, index));
        str = env->GetStringUTFChars(jstr, JNI_FALSE);
        strVector.push_back(str);
    }
    return strVector;
}

void CallVoidMethodFromJNI(const jobject& obj, std::string funcName)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return;
    }
    jmethodID method = env->GetMethodID(clazz, funcName.c_str(), "()V");
    if (method == nullptr) {
        LOGE("fail to get method id: %{public}s", funcName.c_str());
        return;
    }
    env->CallVoidMethod(obj, method);
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
    float GetContentWidth(void* object);
    float GetContentHeight(void* object);
    float GetFrameWidth(void* object);
    float GetFrameHeight(void* object);
};

float JniWebScrollObjectWrapper::GetX(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "getX");
}

float JniWebScrollObjectWrapper::GetY(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "getY");
}

float JniWebScrollObjectWrapper::GetContentWidth(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "getContentWidth");
}

float JniWebScrollObjectWrapper::GetContentHeight(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "getContentHeight");
}

float JniWebScrollObjectWrapper::GetFrameWidth(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "getWidth");
}

float JniWebScrollObjectWrapper::GetFrameHeight(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "getHeight");
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

class JniWebFileChooserObject final : public WebFileChooserObject {
public:
    JniWebFileChooserObject() = default;
    ~JniWebFileChooserObject() = default;

    std::string GetTitle(void* object);
    int GetMode(void* object);
    std::vector<std::string> GetAcceptType(void* object);
    bool IsCapture(void* object);
    int AddObject(void* object);
    void DelObject(int index);
    Platform::JniEnvironment::JavaGlobalRef& GetGeolocationJobject(int index);
    void HandleFileList(void* object, std::vector<std::string>& result, int index);

private:
    int index_;
    std::unordered_map<int, WebObject*> objectMap_;
    std::mutex mutex_;
};

std::string JniWebFileChooserObject::GetTitle(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getTitle");
}

int JniWebFileChooserObject::GetMode(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "getMode");
}

std::vector<std::string> JniWebFileChooserObject::GetAcceptType(void* object)
{
    return GetStringVectorFromJNI(*(jobject *)object, "getAcceptType");
}

bool JniWebFileChooserObject::IsCapture(void* object)
{
    return GetBoolFromJNI(*(jobject *)object, "isCapture");
}

int JniWebFileChooserObject::AddObject(void* object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    objectMap_[++index_] = new WebObject(object);
    return index_;
}

void JniWebFileChooserObject::DelObject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    if (iter != objectMap_.end()) {
        auto obj = iter->second;
        delete obj;
        objectMap_.erase(index);
    }
}

Platform::JniEnvironment::JavaGlobalRef& JniWebFileChooserObject::GetGeolocationJobject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    auto obj = iter->second;
    return obj->Get();
}

void JniWebFileChooserObject::HandleFileList(void* object, std::vector<std::string>& result, int index)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return;
    }
    auto obj = GetGeolocationJobject(index).get();
    if (!obj) {
        LOGE("obj is nullptr");
        return;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return;
    }
    if (result.empty()) {
        LOGE("result is nullptr");
        return;
    }
    jclass jStringCls = env->FindClass("java/lang/String");
    if (!jStringCls) {
        LOGE("jStringCls is nullptr");
        return;
    }
    jobjectArray objectArray = env->NewObjectArray(result.size(), jStringCls, nullptr);
    for (size_t index = 0; index < result.size(); ++index) {
        jstring jstr = env->NewStringUTF(result[index].c_str());
        env->SetObjectArrayElement(objectArray, index, jstr);
    }
    jmethodID method = env->GetMethodID(clazz, "handleFileList", "([Ljava/lang/String;)V");
    if (method == nullptr) {
        LOGE("method is nullptr");
        return;
    }
    env->CallVoidMethod(obj, method, objectArray);
}

class JniWebGeolocationObject final : public WebGeolocationObject {
public:
    JniWebGeolocationObject() = default;
    ~JniWebGeolocationObject() = default;

    std::string GetOrigin(void* object);
    int AddObject(void* object);
    void DelObject(int index);
    Platform::JniEnvironment::JavaGlobalRef& GetGeolocationJobject(int index);
    void Invoke(int index, const std::string& origin, const bool& allow, const bool& retain);

private:
    int index_;
    std::unordered_map<int, WebObject*> objectMap_;
    std::mutex mutex_;
};

std::string JniWebGeolocationObject::GetOrigin(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getOrigin");
}

int JniWebGeolocationObject::AddObject(void* object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    objectMap_[++index_] = new WebObject(object);
    return index_;
}

void JniWebGeolocationObject::DelObject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    if (iter != objectMap_.end()) {
        auto obj = iter->second;
        delete obj;
        objectMap_.erase(index);
    }
}

Platform::JniEnvironment::JavaGlobalRef& JniWebGeolocationObject::GetGeolocationJobject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    auto obj = iter->second;
    return obj->Get();
}

void JniWebGeolocationObject::Invoke(int index, const std::string& origin, const bool& allow, const bool& retain)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return;
    }
    auto obj = GetGeolocationJobject(index).get();
    if (!obj) {
        LOGE("obj is nullptr");
        return;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return;
    }
    jmethodID method = env->GetMethodID(clazz, "invoke", "(Ljava/lang/String;ZZ)V");
    if (method == nullptr) {
        LOGE("method is nullptr");
        return;
    }
    jstring jorigin = env->NewStringUTF(origin.c_str());
    jboolean jallow = allow ? JNI_TRUE : JNI_FALSE;
    jboolean jretain = retain ? JNI_TRUE : JNI_FALSE;
    env->CallVoidMethod(obj, method, jorigin, jallow, jretain);
}

class JniWebRefreshAccessedHistoryObject final : public WebRefreshAccessedHistoryObject {
public:
    std::string GetUrl(void* object);
    bool GetIsRefreshed(void* object);
};

std::string JniWebRefreshAccessedHistoryObject::GetUrl(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getUrl");
}

bool JniWebRefreshAccessedHistoryObject::GetIsRefreshed(void* object)
{
    return GetBoolFromJNI(*(jobject *)object, "getIsRefreshed");
}

class JniWebFullScreenEnterObject final : public WebFullScreenEnterObject {
public:
 	JniWebFullScreenEnterObject() = default;
    ~JniWebFullScreenEnterObject() = default;
     int GetWidths(void* object);
     int GetHeights(void* object);
     int AddObject(void* object);
     void DelObject(int index);
     Platform::JniEnvironment::JavaGlobalRef& GetGeolocationJobject(int index);
     void ExitFullScreen(void* object, int index);
private:
    int index_;
    std::unordered_map<int, WebObject*> objectMap_;
    std::mutex mutex_;
};

int JniWebFullScreenEnterObject::GetWidths(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "widths");
}

int JniWebFullScreenEnterObject::GetHeights(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "heights");
}

int JniWebFullScreenEnterObject::AddObject(void* object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    objectMap_[++index_] = new WebObject(object);
    return index_;
}

void JniWebFullScreenEnterObject::DelObject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    if (iter != objectMap_.end()) {
        auto obj = iter->second;
        delete obj;
        objectMap_.erase(index);
    }
}

Platform::JniEnvironment::JavaGlobalRef& JniWebFullScreenEnterObject::GetGeolocationJobject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    auto obj = iter->second;
    return obj->Get();
}

void JniWebFullScreenEnterObject::ExitFullScreen(void* object, int index)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return;
    }
    auto obj = GetGeolocationJobject(index).get();
    if (!obj) {
        LOGE("obj is nullptr");
        return;
    }
    CallVoidMethodFromJNI(obj, "exitFullScreen");
}

class JniWebFullScreenExitObject final : public WebFullScreenExitObject {
public:
 	  JniWebFullScreenExitObject() = default;
    ~JniWebFullScreenExitObject() = default;
};

class JniWebDownloadResponseObject final : public WebDownloadResponseObject {
public:
    std::string GetUrl(void* object);
    std::string GetMimetype(void* object);
    long GetContentLength(void* object);
    std::string GetContentDisposition(void *object);
    std::string GetUserAgent(void *object);
};

std::string JniWebDownloadResponseObject::GetUrl(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getUrl");
}

std::string JniWebDownloadResponseObject::GetMimetype(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getMimetype");
}

long JniWebDownloadResponseObject::GetContentLength(void* object)
{
    return GetLongFromJNI(*(jobject *)object, "getContentLength");
}

std::string JniWebDownloadResponseObject::GetContentDisposition(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getContentDisposition");
}

std::string JniWebDownloadResponseObject::GetUserAgent(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getUserAgent");
}

class JniWebHttpAuthRequestObject final : public WebHttpAuthRequestObject {
public:
    JniWebHttpAuthRequestObject() = default;
    ~JniWebHttpAuthRequestObject() = default;

    std::string GetHost(void* object);
    std::string GetRealm(void* object);
    int AddObject(void* object);
    void DelObject(int index);
    Platform::JniEnvironment::JavaGlobalRef& GetGeolocationJobject(int index);
    bool Confirm(void* object, std::string& userName, std::string& pwd, int index);
    bool IsHttpAuthInfoSaved(void* object, int index);
    void Cancel(void* object, int index);

private:
    int index_;
    std::unordered_map<int, WebObject*> objectMap_;
    std::mutex mutex_;
};

std::string JniWebHttpAuthRequestObject::GetHost(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getHost");
}

std::string JniWebHttpAuthRequestObject::GetRealm(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getRealm");
}

int JniWebHttpAuthRequestObject::AddObject(void* object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    objectMap_[++index_] = new WebObject(object);
    return index_;
}

void JniWebHttpAuthRequestObject::DelObject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    if (iter != objectMap_.end()) {
        auto obj = iter->second;
        delete obj;
        objectMap_.erase(index);
    }
}

Platform::JniEnvironment::JavaGlobalRef& JniWebHttpAuthRequestObject::GetGeolocationJobject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    auto obj = iter->second;
    return obj->Get();
}

bool JniWebHttpAuthRequestObject::Confirm(void* object, std::string& userName, std::string& pwd, int index)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return false;
    }
    auto obj = GetGeolocationJobject(index).get();
    if (!obj) {
        LOGE("obj is nullptr");
        return false;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return false;
    }
    jmethodID method = env->GetMethodID(clazz, "proceed", "(Ljava/lang/String;Ljava/lang/String;)Z");
    if (method == nullptr) {
        LOGE("method is nullptr");
        return false;
    }
    jstring juserName = env->NewStringUTF(userName.c_str());
    jstring jpwd = env->NewStringUTF(pwd.c_str());
    bool returnValue = env->CallBooleanMethod(obj, method, juserName, jpwd);
    return returnValue;
}

bool JniWebHttpAuthRequestObject::IsHttpAuthInfoSaved(void* object, int index)
{
    auto obj = GetGeolocationJobject(index).get();
    if (!obj) {
        LOGE("obj is nullptr");
        return false;
    }
    return GetBoolFromJNI(obj, "useHttpAuthUsernamePassword");
}

void JniWebHttpAuthRequestObject::Cancel(void* object, int index)
{
    auto obj = GetGeolocationJobject(index).get();
    if (!obj) {
        LOGE("obj is nullptr");
        return;
    }
    CallVoidMethodFromJNI(obj, "cancel");
}

class JniWebPermissionRequestObject final : public WebPermissionRequestObject {
public:
    JniWebPermissionRequestObject() = default;
    ~JniWebPermissionRequestObject() = default;

    int AddObject(void* object);
    void DelObject(int index);
    Platform::JniEnvironment::JavaGlobalRef& GetGeolocationJobject(int index);
    std::string GetOrigin(void* object);
    int GetResourcesId(void* object);
    void Grant(void* object, const int resourcesId, int index);
    void Deny(void* object, int index);

private:
    int index_;
    std::unordered_map<int, WebObject*> objectMap_;
    std::mutex mutex_;
};

int JniWebPermissionRequestObject::AddObject(void* object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    objectMap_[++index_] = new WebObject(object);
    return index_;
}

void JniWebPermissionRequestObject::DelObject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    if (iter != objectMap_.end()) {
        auto obj = iter->second;
        delete obj;
        objectMap_.erase(index);
    }
}

Platform::JniEnvironment::JavaGlobalRef& JniWebPermissionRequestObject::GetGeolocationJobject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    auto obj = iter->second;
    return obj->Get();
}

std::string JniWebPermissionRequestObject::GetOrigin(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getOrigin");
}

int JniWebPermissionRequestObject::GetResourcesId(void* object)
{
    return GetIntFromJNI(*(jobject *)object, "getResources");
}

void JniWebPermissionRequestObject::Grant(void* object, const int resourcesId, int index)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return;
    }
    auto obj = GetGeolocationJobject(index).get();
    if (!obj) {
        LOGE("obj is nullptr");
        return;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return;
    }
    jmethodID method = env->GetMethodID(clazz, "grant", "(I)V");
    if (method == nullptr) {
        LOGE("method is nullptr");
        return;
    }
    env->CallVoidMethod(obj, method, resourcesId);
}

void JniWebPermissionRequestObject::Deny(void* object, int index)
{
    auto obj = GetGeolocationJobject(index).get();
    if (!obj) {
        LOGE("obj is nullptr");
        return;
    }
    CallVoidMethodFromJNI(obj, "deny");
}

class JniWebCommonDialogObject final : public WebCommonDialogObject {
public:
    JniWebCommonDialogObject() = default;
    ~JniWebCommonDialogObject() = default;

    std::string GetUrl(void* object);
    std::string GetMessage(void* object);
    std::string GetValue(void* object);
    int AddObject(void* object);
    void DelObject(int index);
    Platform::JniEnvironment::JavaGlobalRef& GetGeolocationJobject(int index);
    void Confirm(void* object, const std::string& promptResult, int index);
    void Confirm(void* object, int index);
    void Cancel(void* object, int index);

private:
    int index_;
    std::unordered_map<int, WebObject*> objectMap_;
    std::mutex mutex_;
};

std::string JniWebCommonDialogObject::GetUrl(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getUrl");
}

std::string JniWebCommonDialogObject::GetMessage(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getMessage");
}

std::string JniWebCommonDialogObject::GetValue(void* object)
{
    return GetStringFromJNI(*(jobject *)object, "getDefaultValue");
}

int JniWebCommonDialogObject::AddObject(void* object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    objectMap_[++index_] = new WebObject(object);
    return index_;
}

void JniWebCommonDialogObject::DelObject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    if (iter != objectMap_.end()) {
        auto obj = iter->second;
        delete obj;
        objectMap_.erase(index);
    }
}

Platform::JniEnvironment::JavaGlobalRef& JniWebCommonDialogObject::GetGeolocationJobject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    auto obj = iter->second;
    return obj->Get();
}

void JniWebCommonDialogObject::Confirm(void* object, const std::string& promptResult, int index)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGE("jni env not ready, env is null");
        return;
    }
    auto obj = GetGeolocationJobject(index).get();
    if (!obj) {
        LOGE("obj is nullptr");
        return;
    }
    const jclass clazz = env->GetObjectClass(obj);
    if (clazz == nullptr) {
        LOGE("clazz is nullptr");
        return;
    }
    jmethodID method = env->GetMethodID(clazz, "confirm", "(Ljava/lang/String;)V");
    if (method == nullptr) {
        LOGE("method is nullptr");
        return;
    }
    jstring jpromptResult = env->NewStringUTF(promptResult.c_str());
    env->CallVoidMethod(obj, method, jpromptResult);
}

void JniWebCommonDialogObject::Confirm(void* object, int index)
{
    auto obj = GetGeolocationJobject(index).get();
    if (!obj) {
        LOGE("obj is nullptr");
        return;
    }
    CallVoidMethodFromJNI(obj, "confirm");
}

void JniWebCommonDialogObject::Cancel(void* object, int index)
{
    auto obj = GetGeolocationJobject(index).get();
    if (!obj) {
        LOGE("obj is nullptr");
        return;
    }
    CallVoidMethodFromJNI(obj, "cancel");
}

class JniWebOnSslErrorEventReceiveEventObject : public WebOnSslErrorEventReceiveEventObject {
public:
    JniWebOnSslErrorEventReceiveEventObject() = default;
    ~JniWebOnSslErrorEventReceiveEventObject() = default;

    int32_t GetError(void* object) override;
    std::vector<std::string> GetCertChainData(void* object) override;
    int AddObject(void* object) override;
    void DelObject(int index) override;
    Platform::JniEnvironment::JavaGlobalRef& GetSslErrorObject(int index);
    void Confirm(void* object, int index) override;
    void Cancel(void* object, int index) override;

private:
    int index_ = 0;
    std::unordered_map<int, WebObject*> objectMap_;
    std::mutex mutex_;
    std::vector<std::string> certChainDataCache_;
};

int32_t JniWebOnSslErrorEventReceiveEventObject::GetError(void* object)
{
    return GetIntFromJNI(*(jobject*)object, "getError");
}

std::vector<std::string> JniWebOnSslErrorEventReceiveEventObject::GetCertChainData(void* object)
{
    certChainDataCache_ = GetStringVectorFromJNI(*(jobject*)object, "getCertChainDataArray");
    return certChainDataCache_;
}

int JniWebOnSslErrorEventReceiveEventObject::AddObject(void* object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    objectMap_[++index_] = new (std::nothrow) WebObject(object);
    return index_;
}

void JniWebOnSslErrorEventReceiveEventObject::DelObject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    if (iter != objectMap_.end()) {
        auto obj = iter->second;
        if (obj != nullptr) {
            delete obj;
        }
        objectMap_.erase(index);
    }
}

Platform::JniEnvironment::JavaGlobalRef& JniWebOnSslErrorEventReceiveEventObject::GetSslErrorObject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    if (iter == objectMap_.end() || iter->second == nullptr) {
        static Platform::JniEnvironment::JavaGlobalRef emptyRef(nullptr, Platform::JniEnvironment::DeleteJavaGlobalRef);
        return emptyRef;
    }
    auto obj = iter->second;
    return obj->Get();
}

void JniWebOnSslErrorEventReceiveEventObject::Confirm(void* object, int index)
{
    auto& objRef = GetSslErrorObject(index);
    if (!objRef) {
        LOGE("Confirm: GetSslErrorObject returned empty ref");
        return;
    }
    auto obj = objRef.get();
    if (!obj) {
        LOGE("Confirm: obj is nullptr");
        return;
    }
    CallVoidMethodFromJNI(obj, "proceed");
}

void JniWebOnSslErrorEventReceiveEventObject::Cancel(void* object, int index)
{
    auto& objRef = GetSslErrorObject(index);
    if (!objRef) {
        LOGE("Cancel: GetSslErrorObject returned empty ref");
        return;
    }
    auto obj = objRef.get();
    if (!obj) {
        LOGE("Cancel: obj is nullptr");
        return;
    }
    CallVoidMethodFromJNI(obj, "cancel");
}

class JniWebSslErrorEventObject final : public WebSslErrorEventObject {
public:
    JniWebSslErrorEventObject() = default;
    ~JniWebSslErrorEventObject() = default;

    int AddObject(void* object) override;
    void DelObject(int index) override;
    int32_t GetError(void* object) override;
    std::string GetUrl(void* object) override;
    std::string GetOriginalUrl(void* object) override;
    std::string GetReferrer(void* object) override;
    bool IsFatalError(void* object) override;
    bool IsMainFrame(void* object) override;
    std::vector<std::string> GetCertificateChain(void* object);
    void Confirm(void* object, int index) override;
    void Cancel(void* object, bool abortLoading, int index) override;
    Platform::JniEnvironment::JavaGlobalRef& GetAllSslErrorObject(int index);

private:
    int index_ = 0;
    std::unordered_map<int, WebObject*> objectMap_;
    std::mutex mutex_;
    std::vector<std::string> certChainDataCache_;
};

int JniWebSslErrorEventObject::AddObject(void* object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    objectMap_[++index_] = new (std::nothrow) WebObject(object);
    return index_;
}

void JniWebSslErrorEventObject::DelObject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    if (iter != objectMap_.end()) {
        auto obj = iter->second;
        if (obj != nullptr) {
            delete obj;
        }
        objectMap_.erase(index);
    }
}

Platform::JniEnvironment::JavaGlobalRef& JniWebSslErrorEventObject::GetAllSslErrorObject(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = objectMap_.find(index);
    if (iter == objectMap_.end() || iter->second == nullptr) {
        static Platform::JniEnvironment::JavaGlobalRef emptyRef(nullptr, Platform::JniEnvironment::DeleteJavaGlobalRef);
        return emptyRef;
    }
    auto obj = iter->second;
    return obj->Get();
}

int32_t JniWebSslErrorEventObject::GetError(void* object)
{
    return GetIntFromJNI(*(jobject*)object, "getError");
}

std::string JniWebSslErrorEventObject::GetUrl(void* object)
{
    return GetStringFromJNI(*(jobject*)object, "getUrl");
}

std::string JniWebSslErrorEventObject::GetOriginalUrl(void* object)
{
    return GetStringFromJNI(*(jobject*)object, "getOriginalUrl");
}

std::string JniWebSslErrorEventObject::GetReferrer(void* object)
{
    return GetStringFromJNI(*(jobject*)object, "getReferrer");
}

bool JniWebSslErrorEventObject::IsFatalError(void* object)
{
    return GetBoolFromJNI(*(jobject*)object, "getIsFatalError");
}

bool JniWebSslErrorEventObject::IsMainFrame(void* object)
{
    return GetBoolFromJNI(*(jobject*)object, "getIsMainFrame");
}

std::vector<std::string> JniWebSslErrorEventObject::GetCertificateChain(void* object)
{
    certChainDataCache_ = GetStringVectorFromJNI(*(jobject*)object, "getCertChainDataArray");
    return certChainDataCache_;
}

void JniWebSslErrorEventObject::Confirm(void* object, int index)
{
    auto& objRef = GetAllSslErrorObject(index);
    if (!objRef) {
        LOGE("Confirm: GetAllSslErrorObject returned empty ref");
        return;
    }
    auto obj = objRef.get();
    if (!obj) {
        LOGE("obj is nullptr");
        return;
    }
    CallVoidMethodFromJNI(obj, "confirm");
}

void JniWebSslErrorEventObject::Cancel(void* object, bool abortLoading, int index)
{
    auto& objRef = GetAllSslErrorObject(index);
    if (!objRef) {
        LOGE("Cancel: GetAllSslErrorObject returned empty ref");
        return;
    }
    auto obj = objRef.get();
    if (!obj) {
        LOGE("Cancel: obj is nullptr");
        return;
    }
    CallVoidMethodFromJNI(obj, "cancel");
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
    auto JWebFileChooserObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebFileChooserObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetFileChooserObject(JWebFileChooserObject);
    auto JWebGeolocationObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebGeolocationObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetGeolocationObject(JWebGeolocationObject);
    auto JWebDownloadResponseObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebDownloadResponseObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetDownloadResponseObject(JWebDownloadResponseObject);
    auto JWebHttpAuthRequestObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebHttpAuthRequestObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetHttpAuthRequestObject(JWebHttpAuthRequestObject);
    auto JWebPermissionRequestObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebPermissionRequestObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetPermissionRequestObject(JWebPermissionRequestObject);
    auto JWebCommonDialogObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebCommonDialogObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetCommonDialogObject(JWebCommonDialogObject);
    auto JWebRefreshAccessedHistoryObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebRefreshAccessedHistoryObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetRefreshAccessedHistoryObject(JWebRefreshAccessedHistoryObject);
    auto JWebFullScreenEnterObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebFullScreenEnterObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetFullScreenEnterObject(JWebFullScreenEnterObject);
    auto JWebFullScreenExitObject = OHOS::Ace::Referenced::MakeRefPtr<JniWebFullScreenExitObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetFullScreenExitObject(JWebFullScreenExitObject);
    auto JWebOnSslErrorEventReceiveEventObject = Referenced::MakeRefPtr<JniWebOnSslErrorEventReceiveEventObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetOnSslErrorEventReceiveEventObject(
        JWebOnSslErrorEventReceiveEventObject);
    auto JWebSslErrorEventObject = Referenced::MakeRefPtr<JniWebSslErrorEventObject>();
    OHOS::Ace::WebObjectEventManager::GetInstance().SetSslErrorEventObject(JWebSslErrorEventObject);
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
        {
            .name = "nativeOnObjectEventWithObjectReturn",
            .signature =
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/Object;)Ljava/lang/Object;",
            .fnPtr = reinterpret_cast<void*>(&NativeOnObjectEventWithObjectReturn),
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

bool WebAdapterJni::NativeOnObjectEventWithBoolReturn(
    JNIEnv* env, jobject clazz, jstring id, jstring param, jobject object)
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
    return WebObjectEventManager::GetInstance().OnObjectEventWithBoolReturn(eventId, eventParam, (void*)&object);
}

jobject WebAdapterJni::ConvertMapToJavaMap(JNIEnv* env, const std::map<std::string, std::string>& headersMap)
{
    CHECK_NULL_RETURN(env, nullptr);
    jclass hashMapClass = env->FindClass("java/util/HashMap");
    CHECK_NULL_RETURN(hashMapClass, nullptr);
    jmethodID hashMapConstructor = env->GetMethodID(hashMapClass, "<init>", "()V");
    jmethodID hashMapPut =
        env->GetMethodID(hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    if (hashMapConstructor == nullptr || hashMapPut == nullptr) {
        env->DeleteLocalRef(hashMapClass);
        return nullptr;
    }
    jobject hashMap = env->NewObject(hashMapClass, hashMapConstructor);
    for (const auto& entry : headersMap) {
        jstring key = env->NewStringUTF(entry.first.c_str());
        jstring value = env->NewStringUTF(entry.second.c_str());
        env->CallObjectMethod(hashMap, hashMapPut, key, value);
        env->DeleteLocalRef(key);
        env->DeleteLocalRef(value);
    }
    env->DeleteLocalRef(hashMapClass);
    return hashMap;
}

bool WebAdapterJni::CheckResponseParam(const RefPtr<OHOS::Ace::WebResponse>& webResponse)
{
    CHECK_NULL_RETURN(webResponse, false);
    if ((webResponse->GetStatusCode() < RESPONSE_STATUS_CODE_RANGE_ONE_MIN ||
         webResponse->GetStatusCode() > RESPONSE_STATUS_CODE_RANGE_ONE_MAX) &&
        (webResponse->GetStatusCode() < RESPONSE_STATUS_CODE_RANGE_TWO_MIN ||
         webResponse->GetStatusCode() > RESPONSE_STATUS_CODE_RANGE_TWO_MAX)) {
        LOGE("WebAdapterJni::CheckResponseParam response statusCode out of range");
        return false;
    }
    if (webResponse->GetReason().empty()) {
        LOGE("WebAdapterJni::CheckResponseParam response statusReason is empty");
        return false;
    }
    return true;
}

jobject WebAdapterJni::ConvertWebResponseToJObject(JNIEnv* env, const RefPtr<OHOS::Ace::WebResponse>& webResponse)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(webResponse, nullptr);
    if (!CheckResponseParam(webResponse)) {
        return nullptr;
    }
    jclass webResourceResponseClass = env->FindClass("android/webkit/WebResourceResponse");
    CHECK_NULL_RETURN(webResourceResponseClass, nullptr);
    jmethodID constructor = env->GetMethodID(webResourceResponseClass, "<init>",
        "(Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/util/Map;Ljava/io/InputStream;)V");
    if (constructor == nullptr) {
        LOGE("ConvertWebResponseToJObject WebResourceResponse constructor failed");
        env->DeleteLocalRef(webResourceResponseClass);
        return nullptr;
    }
    jclass byteArrayInputStreamClass = env->FindClass("java/io/ByteArrayInputStream");
    if (byteArrayInputStreamClass == nullptr) {
        LOGE("ConvertWebResponseToJObject FindClass java/io/ByteArrayInputStream failed");
        env->DeleteLocalRef(webResourceResponseClass);
        return nullptr;
    }
    jmethodID byteArrayInputStreamConstructor = env->GetMethodID(byteArrayInputStreamClass, "<init>", "([B)V");
    if (byteArrayInputStreamConstructor == nullptr) {
        LOGE("ConvertWebResponseToJObject java/io/ByteArrayInputStream init failed");
        env->DeleteLocalRef(byteArrayInputStreamClass);
        env->DeleteLocalRef(webResourceResponseClass);
        return nullptr;
    }
    jbyteArray byteArray = env->NewByteArray(webResponse->GetData().size());
    env->SetByteArrayRegion(
        byteArray, 0, webResponse->GetData().size(), reinterpret_cast<const jbyte*>(webResponse->GetData().c_str()));
    jstring mimeType = env->NewStringUTF(webResponse->GetMimeType().c_str());
    jstring encoding = env->NewStringUTF(webResponse->GetEncoding().c_str());
    jint statusCode = webResponse->GetStatusCode();
    jstring reasonPhrase = env->NewStringUTF(webResponse->GetReason().c_str());
    std::map<std::string, std::string> headersMap = webResponse->GetHeaders();
    jobject headers = ConvertMapToJavaMap(env, headersMap);
    jobject inputStream = env->NewObject(byteArrayInputStreamClass, byteArrayInputStreamConstructor, byteArray);
    jobject webResourceResponse = env->NewObject(
        webResourceResponseClass, constructor, mimeType, encoding, statusCode, reasonPhrase, headers, inputStream);
    env->DeleteLocalRef(mimeType);
    env->DeleteLocalRef(encoding);
    env->DeleteLocalRef(reasonPhrase);
    env->DeleteLocalRef(byteArray);
    env->DeleteLocalRef(inputStream);
    env->DeleteLocalRef(byteArrayInputStreamClass);
    env->DeleteLocalRef(webResourceResponseClass);
    return webResourceResponse;
}

jobject WebAdapterJni::NativeOnObjectEventWithObjectReturn(
    JNIEnv* env, jobject clazz, jstring id, jstring param, jobject object)
{
    CHECK_NULL_RETURN(env, nullptr);
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
    RefPtr<OHOS::Ace::WebResponse> response =
        WebObjectEventManager::GetInstance().OnObjectEventWithResponseReturn(eventId, eventParam, (void*)&object);
    return ConvertWebResponseToJObject(env, response);
}
} // namespace OHOS::Ace
