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

#include "js_accessibility_manager_jni.h"

#include <mutex>
#include <string>

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "adapter/android/osal/js_accessibility_manager.h"
#include "adapter/android/osal/mock/accessible_ability_operator_callback_impl.h"
#include "base/log/log.h"
#include "base/utils/utils.h"
#include "nlohmann/json.hpp"

using Json = nlohmann::json;

namespace OHOS::Ace::Platform {
const int32_t PERFORM_ACTION_ARRAY_LEN = 3;
JsAccessibilityManagerStruct JsAccessibilityManagerJni::jsAccessibilityManagerStruct_;
std::mutex JsAccessibilityManagerJni::jsInteractionOperationMapMutex_;
std::unordered_map<int32_t, std::shared_ptr<Framework::JsAccessibilityManager::JsInteractionOperation>>
    JsAccessibilityManagerJni::jsInteractionOperationMap_;
std::mutex JsAccessibilityManagerJni::jsAccessibilityManagerStructObjectMapMutex_;
static const JNINativeMethod METHODS[] = {
    {
        .name = "nativeSetupJsAccessibilityManager",
        .signature = "(I)V",
        .fnPtr = reinterpret_cast<void*>(&JsAccessibilityManagerJni::SetupJsAccessibilityManagerJni),
    },
    {
        .name = "nativeCreateAccessibilityNodeInfo",
        .signature = "(II)Ljava/lang/String;",
        .fnPtr = reinterpret_cast<void*>(&JsAccessibilityManagerJni::OnCreateAccessibilityNodeInfo),
    },
    {
        .name = "nativePerformAction",
        .signature = "([ILjava/lang/String;)Z",
        .fnPtr = reinterpret_cast<void*>(&JsAccessibilityManagerJni::PerformAction),
    },
    {
        .name = "nativeAccessibilityStateChanged",
        .signature = "(ZJ)V",
        .fnPtr = reinterpret_cast<void*>(&JsAccessibilityManagerJni::OnAccessibilityStateChanged),
    },
    {
        .name = "nativeFindFocusedElementInfo",
        .signature = "(II)Ljava/lang/String;",
        .fnPtr = reinterpret_cast<void*>(&JsAccessibilityManagerJni::FindFocusedElementInfo),
    },
    {
        .name = "nativeGetTreeIdArray",
        .signature = "(I)[I",
        .fnPtr = reinterpret_cast<void*>(&JsAccessibilityManagerJni::GetTreeIdArray),
    },
    {
        .name = "nativeTouchExplorationStateChange",
        .signature = "(ZI)V",
        .fnPtr = reinterpret_cast<void*>(&JsAccessibilityManagerJni::OnTouchExplorationStateChange),
    },
    {
        .name = "nativeGetRootElementId",
        .signature = "(I)I",
        .fnPtr = reinterpret_cast<void*>(&JsAccessibilityManagerJni::OnGetRootElementId),
    },
    {
        .name = "nativeRelease",
        .signature = "(I)Z",
        .fnPtr = reinterpret_cast<void*>(&JsAccessibilityManagerJni::OnRelease),
    }
};

bool JsAccessibilityManagerJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::Register: null java env");
        return false;
    }

    const jclass jsAccessibilityManagerClazz = env->FindClass("ohos/ace/adapter/AccessibilityCrossPlatformBridge");
    if (jsAccessibilityManagerClazz == nullptr) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY,
            "JsAccessibilityManagerJni::Register: can't find java class AccessibilityCrossPlatformBridge");
        return false;
    }

    bool ret = env->RegisterNatives(jsAccessibilityManagerClazz, METHODS, Ace::ArraySize(METHODS)) == 0;
    env->DeleteLocalRef(jsAccessibilityManagerClazz);
    return ret;
}

void JsAccessibilityManagerJni::SetupJsAccessibilityManagerJni(JNIEnv* env, jobject obj, jint windowId)
{
    std::lock_guard<std::mutex> lock(jsAccessibilityManagerStructObjectMapMutex_);
    jsAccessibilityManagerStruct_.objectMap[windowId] = env->NewGlobalRef(obj);
    jclass clazz = env->GetObjectClass(obj);
    jsAccessibilityManagerStruct_.isEnabledMethod = env->GetMethodID(clazz, "isEnabled", "()Z");
    jsAccessibilityManagerStruct_.registerJsAccessibilityManagerMethod =
        env->GetMethodID(clazz, "registerJsAccessibilityStateObserver", "(J)Z");
    jsAccessibilityManagerStruct_.unregisterJsAccessibilityManagerMethod =
        env->GetMethodID(clazz, "unregisterJsAccessibilityStateObserver", "()Z");
    jsAccessibilityManagerStruct_.sendAccessibilityEventJsonMethod =
        env->GetMethodID(clazz, "onSendAccessibilityEvent", "(IILjava/lang/String;)Z");
    jsAccessibilityManagerStruct_.isTouchExplorationEnabledMethod =
        env->GetMethodID(clazz, "isTouchExplorationEnabled", "()Z");
    env->DeleteLocalRef(clazz);
}

jintArray JsAccessibilityManagerJni::GetTreeIdArray(JNIEnv* env, jobject obj, jint windowId)
{
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::GetTreeIdArray: env null");
        return env->NewIntArray(0);
    }

    size_t componentSize = 0;
    std::vector<int> allComponents;
    if (jsInteractionOperationMap_.find(windowId) != jsInteractionOperationMap_.end() &&
        jsInteractionOperationMap_[windowId] != nullptr) {
        auto res = jsInteractionOperationMap_[windowId]->GetAllComponentsId(0, allComponents);
        if (res) {
            componentSize = allComponents.size();
        }
    }

    jintArray intArray = env->NewIntArray(componentSize);
    jint temp[componentSize];
    for (size_t i = 0; i < componentSize; ++i) {
        temp[i] = allComponents[i];
    }
    env->SetIntArrayRegion(intArray, 0, componentSize, temp);

    return intArray;
}

jstring JsAccessibilityManagerJni::OnCreateAccessibilityNodeInfo(JNIEnv* env, jobject obj, jint nodeId, jint windowId)
{
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::OnCreateAccessibilityNodeInfo: env null");
        return env->NewStringUTF("");
    }

    int32_t nodeIdValue = static_cast<int32_t>(nodeId);
    std::string retJson = std::string("");
    if (jsInteractionOperationMap_.find(windowId) != jsInteractionOperationMap_.end() &&
        jsInteractionOperationMap_[windowId] != nullptr) {
        jsInteractionOperationMap_[windowId]->SearchElementInfoByAccessibilityId(nodeIdValue, retJson);
    }

    return env->NewStringUTF(retJson.c_str());
}

bool JsAccessibilityManagerJni::PerformAction(JNIEnv* env, jobject obj, jintArray intArray, jstring jBundleString)
{
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::PerformAction: env null");
        return false;
    }

    jsize intArrayLength = env->GetArrayLength(intArray);
    if (intArrayLength < PERFORM_ACTION_ARRAY_LEN) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::PerformAction: wrong number of parameters.");
        return false;
    }
    jint* intArrayElements = env->GetIntArrayElements(intArray, nullptr);
    int32_t virtualViewIdValue = intArrayElements[0];
    int32_t accessibilityActionValue = intArrayElements[1];
    int32_t windowId = intArrayElements[2];
    env->ReleaseIntArrayElements(intArray, intArrayElements, JNI_ABORT);
    std::string bundleValue;
    auto bundleStr = env->GetStringUTFChars(jBundleString, nullptr);
    if (bundleStr != nullptr) {
        bundleValue = bundleStr;
        env->ReleaseStringUTFChars(jBundleString, bundleStr);
    }
    std::map<std::string, std::string> actionArgumentsMap;
    if (bundleValue.length() > 0 && bundleValue.compare("{}") != 0) {
        Json bundleJson = Json::parse(bundleValue, nullptr, false);
        for (const auto& item : bundleJson.items()) {
            if (item.value().is_string()) {
                actionArgumentsMap[item.key()] = item.value().get<std::string>();
            } else if (item.value().is_number_integer()) {
                actionArgumentsMap[item.key()] = std::to_string(item.value().get<int>());
            } else {
                TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
                    "JsAccessibilityManagerJni::PerformAction: parse json: unknown type.");
            }
        }
    }
    if (jsInteractionOperationMap_.find(windowId) != jsInteractionOperationMap_.end() &&
        jsInteractionOperationMap_[windowId] != nullptr) {
        OHOS::Accessibility::ElementOperatorCallbackImpl callback;
        jsInteractionOperationMap_[windowId]->ExecuteAction(
            virtualViewIdValue, accessibilityActionValue, actionArgumentsMap, 0, callback);
    }

    return true;
}

void JsAccessibilityManagerJni::OnAccessibilityStateChanged(
    JNIEnv* env, jobject obj, jboolean accessibilityEnabled, jlong objectPtr)
{
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::OnAccessibilityStateChanged: env null");
        return;
    }

    bool accessibilityEnabledValue = static_cast<bool>(accessibilityEnabled);
    auto accessibilityStateObserver =
        JavaLongToPointer<OHOS::Ace::Framework::JsAccessibilityManager::JsAccessibilityStateObserver>(objectPtr);
    if (accessibilityStateObserver != nullptr) {
        accessibilityStateObserver->OnStateChanged(accessibilityEnabledValue);
    }
}

jstring JsAccessibilityManagerJni::FindFocusedElementInfo(JNIEnv* env, jobject obj, jint jfocusType, jint windowId)
{
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::FindFocusedElementInfo: env null");
        return env->NewStringUTF("");
    }

    auto focusType = static_cast<int32_t>(jfocusType);
    std::string retJson = "";
    if (jsInteractionOperationMap_.find(windowId) != jsInteractionOperationMap_.end() &&
        jsInteractionOperationMap_[windowId] != nullptr) {
        jsInteractionOperationMap_[windowId]->FindFocusedElementInfo(-1, focusType, retJson);
    }

    return env->NewStringUTF(retJson.c_str());
}

void JsAccessibilityManagerJni::OnTouchExplorationStateChange(
    JNIEnv* env, jobject obj, jboolean jTouchExplorationStateChange, jint windowId)
{
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::OnTouchExplorationStateChange: env null");
        return;
    }

    bool touchExplorationStateChange = static_cast<bool>(jTouchExplorationStateChange);
    if (jsInteractionOperationMap_.find(windowId) != jsInteractionOperationMap_.end() &&
        jsInteractionOperationMap_[windowId] != nullptr) {
        jsInteractionOperationMap_[windowId]->OnTouchExplorationStateChange(touchExplorationStateChange);
    }
}

int32_t JsAccessibilityManagerJni::OnGetRootElementId(JNIEnv* env, jobject obj, jint jwindowId)
{
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::OnGetRootElementId: env null");
        return -1;
    }

    int32_t windowId = static_cast<int32_t>(jwindowId);
    int32_t elementId = -1;
    if (jsInteractionOperationMap_.find(windowId) != jsInteractionOperationMap_.end() &&
        jsInteractionOperationMap_[windowId] != nullptr) {
        elementId = jsInteractionOperationMap_[windowId]->GetRootElementId();
    }

    return elementId;
}

bool JsAccessibilityManagerJni::OnRelease(JNIEnv* env, jobject obj, jint jwindowId)
{
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::OnRelease: env null");
        return false;
    }

    int32_t windowId = static_cast<int32_t>(jwindowId);
    std::lock_guard<std::mutex> lock(jsAccessibilityManagerStructObjectMapMutex_);
    if (jsAccessibilityManagerStruct_.objectMap.find(windowId) != jsAccessibilityManagerStruct_.objectMap.end() &&
        jsAccessibilityManagerStruct_.objectMap[windowId] != nullptr) {
        jsAccessibilityManagerStruct_.objectMap.erase(windowId);
    }

    return true;
}

bool JsAccessibilityManagerJni::SendAccessibilityEvent(
    int32_t nodeId, int32_t eventType, std::string jsonValue, int32_t windowId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::sendAccessibilityEvent: env is NULL");
        return false;
    }

    jint eventName = static_cast<jint>(eventType);
    jint nodeIdValue = static_cast<jint>(nodeId);
    jstring jsonStrValue = env->NewStringUTF(jsonValue.c_str());
    jboolean ret = JNI_FALSE;
    if (jsAccessibilityManagerStruct_.objectMap.find(windowId) != jsAccessibilityManagerStruct_.objectMap.end() &&
        jsAccessibilityManagerStruct_.objectMap[windowId] != nullptr) {
        ret = env->CallBooleanMethod(jsAccessibilityManagerStruct_.objectMap[windowId],
            jsAccessibilityManagerStruct_.sendAccessibilityEventJsonMethod, nodeIdValue, eventName, jsonStrValue);
    }

    env->DeleteLocalRef(jsonStrValue);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

    return ret == JNI_TRUE;
}

bool JsAccessibilityManagerJni::isEnabled(int32_t windowId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::isEnabled: env is NULL");
        return false;
    }
    jboolean ret = JNI_FALSE;
    if (jsAccessibilityManagerStruct_.objectMap.find(windowId) != jsAccessibilityManagerStruct_.objectMap.end() &&
        jsAccessibilityManagerStruct_.objectMap[windowId] != nullptr) {
        ret = env->CallBooleanMethod(
            jsAccessibilityManagerStruct_.objectMap[windowId], jsAccessibilityManagerStruct_.isEnabledMethod);
    }
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

    return ret == JNI_TRUE;
}

bool JsAccessibilityManagerJni::RegisterJsAccessibilityStateObserver(void* jsAccessibilityManager, int32_t windowId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY,
            "JsAccessibilityManagerJni::RegisterJsAccessibilityStateObserver: env is NULL");
        return false;
    }

    jlong jsAccessibilityManagerHandle = PointerToJavaLong(jsAccessibilityManager);
    jboolean ret = JNI_FALSE;
    if (jsAccessibilityManagerStruct_.objectMap.find(windowId) != jsAccessibilityManagerStruct_.objectMap.end() &&
        jsAccessibilityManagerStruct_.objectMap[windowId] != nullptr) {
        ret = env->CallBooleanMethod(jsAccessibilityManagerStruct_.objectMap[windowId],
            jsAccessibilityManagerStruct_.registerJsAccessibilityManagerMethod, jsAccessibilityManagerHandle);
    }
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

    return ret == JNI_TRUE;
}

bool JsAccessibilityManagerJni::UnregisterJsAccessibilityStateObserver(int32_t windowId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY,
            "JsAccessibilityManagerJni::UnregisterJsAccessibilityStateObserver: env is NULL");
        return false;
    }

    jboolean ret = JNI_FALSE;
    if (jsAccessibilityManagerStruct_.objectMap.find(windowId) != jsAccessibilityManagerStruct_.objectMap.end() &&
        jsAccessibilityManagerStruct_.objectMap[windowId] != nullptr) {
        ret = env->CallBooleanMethod(jsAccessibilityManagerStruct_.objectMap[windowId],
            jsAccessibilityManagerStruct_.unregisterJsAccessibilityManagerMethod);
    }
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

    return ret == JNI_TRUE;
}

bool JsAccessibilityManagerJni::RegisterJsInteractionOperation(
    int32_t windowId, const std::shared_ptr<Framework::JsAccessibilityManager::JsInteractionOperation>& callback)
{
    std::lock_guard<std::mutex> lock(jsInteractionOperationMapMutex_);
    jsInteractionOperationMap_[windowId] = callback;

    return true;
}

bool JsAccessibilityManagerJni::UnregisterJsInteractionOperation(int32_t windowId)
{
    std::lock_guard<std::mutex> lock(jsInteractionOperationMapMutex_);
    auto target = jsInteractionOperationMap_.find(windowId);
    if (target != jsInteractionOperationMap_.end()) {
        jsInteractionOperationMap_.erase(windowId);
    }

    return true;
}

bool JsAccessibilityManagerJni::isTouchExplorationEnabled(int32_t windowId)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "JsAccessibilityManagerJni::isTouchExplorationEnabled: env is NULL");
        return false;
    }

    jboolean ret = JNI_FALSE;
    if (jsAccessibilityManagerStruct_.objectMap.find(windowId) != jsAccessibilityManagerStruct_.objectMap.end() &&
        jsAccessibilityManagerStruct_.objectMap[windowId] != nullptr) {
        ret = env->CallBooleanMethod(jsAccessibilityManagerStruct_.objectMap[windowId],
            jsAccessibilityManagerStruct_.isTouchExplorationEnabledMethod);
    }
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

    return ret == JNI_TRUE;
}
} // namespace OHOS::Ace::Platform
