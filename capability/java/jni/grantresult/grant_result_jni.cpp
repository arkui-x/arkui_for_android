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

#include "adapter/android/capability/java/jni/grantresult/grant_result_jni.h"

#include <jni.h>

#include "adapter/android/capability/java/jni/grantresult/grant_result_manager.h"
#include "base/log/log.h"

namespace OHOS::Ace::Platform {
namespace {
const char CLASS_NAME[] = "ohos/ace/adapter/capability/grantresult/GrantResult";

static const JNINativeMethod METHODS[] = {
    { "onRequestPremissionCallback", "([Ljava/lang/String;[I)V",
        reinterpret_cast<void*>(GrantResultJni::OnRequestPremissionCallback) },
};
} // namespace
bool GrantResultJni::Register(std::shared_ptr<JNIEnv> env)
{
    if (!env) {
        LOGE("Clipborad JNI: null env");
        return false;
    }
    jclass cls = env->FindClass(CLASS_NAME);
    if (cls == nullptr) {
        LOGE("GrantResultJni JNI: class not found");
        return false;
    }

    bool ret = env->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    env->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("GrantResultJni JNI: register failed");
        return false;
    }
    return true;
}

void GrantResultJni::OnRequestPremissionCallback(JNIEnv* env, jobject obj, jobjectArray stringArray, jintArray intArray)
{
    jsize arrayLength = env->GetArrayLength(stringArray);
    std::vector<std::string> stringVector;
    for (int i = 0; i < arrayLength; i++) {
        jstring stringObject = (jstring)env->GetObjectArrayElement(stringArray, i);
        const char* stringChars = env->GetStringUTFChars(stringObject, nullptr);
        stringVector.push_back(stringChars);
        env->ReleaseStringUTFChars(stringObject, stringChars);
        env->DeleteLocalRef(stringObject);
    }

    jsize intArrayLength = env->GetArrayLength(intArray);
    std::vector<int> intVector(intArrayLength);
    jint* intArrayElements = env->GetIntArrayElements(intArray, nullptr);
    for (int i = 0; i < intArrayLength; i++) {
        intVector[i] = intArrayElements[i];
    }
    env->ReleaseIntArrayElements(intArray, intArrayElements, JNI_ABORT);
    GrantResultManager::OnRequestPremissionCallback(stringVector, intVector);
}
} // namespace OHOS::Ace::Platform