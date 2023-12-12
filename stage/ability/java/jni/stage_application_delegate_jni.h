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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_APPLICATION_DELEGATE_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_APPLICATION_DELEGATE_JNI_H

#include <memory>

#include "jni.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class StageApplicationDelegateJni {
public:
    StageApplicationDelegateJni() = delete;
    ~StageApplicationDelegateJni() = delete;

    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static void SetNativeAssetManager(JNIEnv* env, jclass myclass, jobject assetManager);
    static void SetHapPath(JNIEnv* env, jclass myclass, jstring str);
    static void SetAssetsFileRelativePath(JNIEnv* env, jclass myclass, jstring str);
    static void LaunchApplication(JNIEnv* env, jclass clazz, jboolean isCopyNativeLibs);
    static void SetCacheDir(JNIEnv* env, jclass myclass, jstring str);
    static void SetFileDir(JNIEnv* env, jclass myclass, jstring str);
    static void SetAppLibDir(JNIEnv* env, jclass myclass, jstring str);
    static void SetResourcesFilePrefixPath(JNIEnv* env, jclass myclass, jstring str);
    static void SetPidAndUid(JNIEnv* env, jclass myclass, jint pid, jint uid);
    static void InitConfiguration(JNIEnv* env, jclass myclass, jstring data);
    static void OnConfigurationChanged(JNIEnv* env, jclass myclass, jstring data);
    static void SetLocale(JNIEnv* env, jclass myclass, jstring jlanguage, jstring jcountry, jstring jscript);
    static void AttachStageApplicationDelegate(JNIEnv* env, jclass myclass, jobject object);
    static void SetPackageName(JNIEnv* env, jclass myclass, jstring object);
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ABILITY_JAVA_JNI_STAGE_APPLICATION_DELEGATE_JNI_H