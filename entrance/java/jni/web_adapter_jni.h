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

#ifndef FOUNDATION_ACE_ADAPTER_COMMON_JNI_WEB_ADAPTER_JNI_H
#define FOUNDATION_ACE_ADAPTER_COMMON_JNI_WEB_ADAPTER_JNI_H
#include <string>
#include <map>

#include "jni.h"
#include "base/memory/ace_type.h"

namespace OHOS::Ace::Platform {
namespace JNI_TOOL {
    std::string GetStringFromJNI(const jobject& obj, std::string funcName);
    int GetIntFromJNI(const jobject& obj, std::string funcName);
    float GetFloatFromJNI(const jobject& obj, std::string funcName);
    long GetLongFromJNI(const jobject& obj, std::string funcName);
    bool GetBoolFromJNI(const jobject& obj, std::string funcName);
    std::map<std::string, std::string> GetStringMapFromJNI(const jobject& obj, std::string funcName);
    std::vector<std::string> GetStringVectorFromJNI(const jobject& obj, std::string funcName);
    void CallVoidMethodFromJNI(const jobject& obj, std::string funcName);
}

class WebAdapterJni final {
public:
    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static void NativeOnObjectEvent(
        JNIEnv* env, jobject clazz, jstring enventId, jstring param, jobject object);
    static bool NativeOnObjectEventWithBoolReturn(
        JNIEnv* env, jobject clazz, jstring enventId, jstring param, jobject object);
};
}
#endif
