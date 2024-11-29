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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JS_ACCESSIBILITY_MANAGER_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JS_ACCESSIBILITY_MANAGER_JNI_H

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "adapter/android/osal/js_accessibility_manager.h"
#include "base/utils/noncopyable.h"
#include "jni.h"

namespace OHOS::Ace::Platform {
struct JsAccessibilityManagerStruct {
    std::unordered_map<uint32_t, jobject> objectMap;
    jmethodID isEnabledMethod;
    jmethodID registerJsAccessibilityManagerMethod;
    jmethodID unregisterJsAccessibilityManagerMethod;
    jmethodID sendAccessibilityEventJsonMethod;
    jmethodID registerJsInteractionOperationMethod;
    jmethodID unregisterJsInteractionOperationMethod;
    jmethodID isTouchExplorationEnabledMethod;
};

class JsAccessibilityManagerJni {
public:
    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static bool SendAccessibilityEvent(int32_t nodeId, int32_t eventType, std::string jsonValue, int32_t windowId);
    static bool isEnabled(int32_t windowId);
    static bool RegisterJsAccessibilityStateObserver(void* jsAccessibilityManager, int32_t windowId);
    static bool UnregisterJsAccessibilityStateObserver(int32_t windowId);
    static bool RegisterJsInteractionOperation(
        int32_t windowId, const std::shared_ptr<Framework::JsAccessibilityManager::JsInteractionOperation>& callback);
    static bool UnregisterJsInteractionOperation(int32_t windowId);
    static bool isTouchExplorationEnabled(int32_t windowId);
    static void SetupJsAccessibilityManagerJni(JNIEnv* env, jobject obj, jint windowId);
    static jstring OnCreateAccessibilityNodeInfo(JNIEnv* env, jobject obj, jint nodeId, jint windowId);
    static bool PerformAction(JNIEnv* env, jobject obj, jintArray intArray, jstring jBundleString);
    static void OnAccessibilityStateChanged(JNIEnv* env, jobject obj, jboolean accessibilityEnabled, jlong objectPtr);
    static jstring FindFocusedElementInfo(JNIEnv* env, jobject obj, jint focusType, jint windowId);
    static jintArray GetTreeIdArray(JNIEnv* env, jobject obj, jint windowId);
    static void OnTouchExplorationStateChange(
        JNIEnv* env, jobject obj, jboolean jTouchExplorationStateChange, jint windowId);
    static int32_t OnGetRootElementId(JNIEnv* env, jobject obj, jint jwindowId);
    static bool OnRelease(JNIEnv* env, jobject obj, jint jwindowId);

private:
    static JsAccessibilityManagerStruct jsAccessibilityManagerStruct_;
    static std::mutex jsInteractionOperationMapMutex_;
    static std::mutex jsAccessibilityManagerStructObjectMapMutex_;
    static std::unordered_map<int32_t, std::shared_ptr<Framework::JsAccessibilityManager::JsInteractionOperation>>
        jsInteractionOperationMap_;

    ACE_DISALLOW_COPY_AND_MOVE(JsAccessibilityManagerJni);
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JS_ACCESSIBILITY_MANAGER_JNI_H