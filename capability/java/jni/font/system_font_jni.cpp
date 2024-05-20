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
#include "adapter/android/capability/java/jni/font/system_font_jni.h"

#include "adapter/android/capability/java/jni/font/font_platform_proxy_impl.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#ifdef USE_PLATFORM_FONT
#include "core/common/font/font_platform_proxy.h"
#endif

namespace OHOS::Ace::Platform {
namespace {

static const char* const SYSTEM_FONT_MANAGER_PLUGIN_CLASS_NAME = "ohos/ace/adapter/capability/font/SystemFontManager";

static const char* const METHOD_GET_SYSTEM_FONT_INFO_LIST = "getSystemFontInfoList";
static const char* const SIGNATURE_GET_SYSTEM_FONT_INFO_LIST = "()[Lohos/ace/adapter/capability/font/SystemFontInfo;";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(SystemFontJni::NativeInit) },
};

static const char* const SYSTEM_FONT_INFO_PLUGIN_CLASS_NAME = "ohos/ace/adapter/capability/font/SystemFontInfo";

static const char* const METHOD_SYSTEM_FONT_INFO_GET_NAME = "getName";
static const char* const SIGNATURE_SYSTEM_FONT_INFO_GET_NAME = "()Ljava/lang/String;";
static const char* const METHOD_SYSTEM_FONT_INFO_GET_PATH = "getPath";
static const char* const SIGNATURE_SYSTEM_FONT_INFO_GET_PATH = "()Ljava/lang/String;";
static const char* const METHOD_SYSTEM_FONT_INFO_GET_WEIGHT = "getWeight";
static const char* const SIGNATURE_SYSTEM_FONT_INFO_GET_WEIGHT = "()I";
static const char* const METHOD_SYSTEM_FONT_INFO_IS_ITALIC = "isItalic";
static const char* const SIGNATURE_SYSTEM_FONT_INFO_IS_ITALIC = "()Z";

JniEnvironment::JavaGlobalRef g_jobject(nullptr, nullptr);

struct {
    jmethodID getSystemFontInfoList;
    jmethodID getName;
    jmethodID getPath;
    jmethodID getWeight;
    jmethodID isItalic;
} g_pluginClass;

} // namespace

bool SystemFontJni::Register(std::shared_ptr<JNIEnv> env)
{
    if (!env) {
        LOGW("SystemFont Jni: env not ready");
        return false;
    }

    jclass cls = env->FindClass(SYSTEM_FONT_MANAGER_PLUGIN_CLASS_NAME);
    if (!cls) {
        LOGW("SystemFont Jni: cannot find SYSTEM_FONT_MANAGER class.");
        return false;
    }

    bool ret = env->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    env->DeleteLocalRef(cls);
    if (!ret) {
        LOGW("SystemFont Jni: RegisterNatives failed.");
        return false;
    }
    OnJniRegistered();
    return true;
}

void SystemFontJni::OnJniRegistered()
{
#ifdef USE_PLATFORM_FONT
    FontPlatformProxy::GetInstance().SetDelegate(std::make_unique<FontPlatformProxyImpl>());
#endif
}

void SystemFontJni::NativeInit(JNIEnv* env, jobject jobj)
{
    if (!env) {
        LOGW("SystemFont Jni: env not ready.");
        return;
    }
    InitFontInfo(env);

    g_jobject = JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jobj);

    // Find classes & methods in Java class.
    jclass cls = env->GetObjectClass(jobj);
    if (cls == nullptr) {
        LOGE("SystemFont Jni: get object class failed.");
        return;
    }
    g_pluginClass.getSystemFontInfoList =
        env->GetMethodID(cls, METHOD_GET_SYSTEM_FONT_INFO_LIST, SIGNATURE_GET_SYSTEM_FONT_INFO_LIST);
    if (!g_pluginClass.getSystemFontInfoList) {
        LOGW("SystemFont Jni: cannot find GET_SYSTEM_FONT_INFO method.");
    }
    env->DeleteLocalRef(cls);
}

void SystemFontJni::InitFontInfo(JNIEnv* env)
{
    if (!env) {
        LOGW("SystemFont Jni: env not ready.");
        return;
    }
    jclass cls = env->FindClass(SYSTEM_FONT_INFO_PLUGIN_CLASS_NAME);
    if (!cls) {
        LOGW("SystemFont Jni: cannot find SYSTEM_FONT_INFO class.");
        return;
    }
    g_pluginClass.getName =
        env->GetMethodID(cls, METHOD_SYSTEM_FONT_INFO_GET_NAME, SIGNATURE_SYSTEM_FONT_INFO_GET_NAME);
    g_pluginClass.getPath =
        env->GetMethodID(cls, METHOD_SYSTEM_FONT_INFO_GET_PATH, SIGNATURE_SYSTEM_FONT_INFO_GET_PATH);
    g_pluginClass.getWeight =
        env->GetMethodID(cls, METHOD_SYSTEM_FONT_INFO_GET_WEIGHT, SIGNATURE_SYSTEM_FONT_INFO_GET_WEIGHT);
    g_pluginClass.isItalic =
        env->GetMethodID(cls, METHOD_SYSTEM_FONT_INFO_IS_ITALIC, SIGNATURE_SYSTEM_FONT_INFO_IS_ITALIC);
    if (!g_pluginClass.getName || !g_pluginClass.getPath || !g_pluginClass.getWeight || !g_pluginClass.isItalic) {
        LOGW("SystemFont Jni: SYSTEM_FONT_INFO method get failed.");
    }
    env->DeleteLocalRef(cls);
}

void SystemFontJni::ConvertFontInfo(jobjectArray jObjArray, std::vector<FontInfoAndroid>& fontInfos)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("SystemFont Jni: env not ready.");
        return;
    }
    if (!jObjArray ||
        !g_pluginClass.getName || !g_pluginClass.getPath || !g_pluginClass.getWeight || !g_pluginClass.isItalic) {
        return;
    }
    jsize length = env->GetArrayLength(jObjArray);
    for (jsize i = 0; i < length; i++) {
        FontInfoAndroid fontInfo;
        jobject jObjFont = env->GetObjectArrayElement(jObjArray, i);
        jstring jName = static_cast<jstring>(env->CallObjectMethod(jObjFont, g_pluginClass.getName));
        const char* nameChars = env->GetStringUTFChars(jName, nullptr);
        if (nameChars != nullptr) {
            fontInfo.name.assign(nameChars);
            env->ReleaseStringUTFChars(jName, nameChars);
        }
        jstring jPath = static_cast<jstring>(env->CallObjectMethod(jObjFont, g_pluginClass.getPath));
        const char* pathChars = env->GetStringUTFChars(jPath, nullptr);
        if (pathChars != nullptr) {
            fontInfo.path.assign(pathChars);
            env->ReleaseStringUTFChars(jPath, pathChars);
        }
        jint jWeight = env->CallIntMethod(jObjFont, g_pluginClass.getWeight);
        fontInfo.weight = static_cast<int>(jWeight);
        jboolean jItalic = env->CallBooleanMethod(jObjFont, g_pluginClass.isItalic);
        fontInfo.italic = static_cast<bool>(jItalic);
        if (env->ExceptionCheck()) {
            LOGW("SystemFont JNI: call SYSTEM_FONT_INFO method has exception");
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        fontInfos.push_back(fontInfo);
        env->DeleteLocalRef(jObjFont);
    }
}

// Called by C++
void SystemFontJni::GetSystemFontInfo(std::vector<FontInfoAndroid>& fontInfos)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (!env) {
        LOGW("SystemFont Jni: env not ready.");
        return;
    }

    if (!g_jobject || !g_pluginClass.getSystemFontInfoList) {
        return;
    }
    jobjectArray jResult =
        static_cast<jobjectArray>(env->CallObjectMethod(g_jobject.get(), g_pluginClass.getSystemFontInfoList));
    if (env->ExceptionCheck()) {
        LOGE("SystemFont Jni: call getSystemFontInfoList has exception.");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    ConvertFontInfo(jResult, fontInfos);
    if (jResult != nullptr) {
        env->DeleteLocalRef(jResult);
    }
}
} // namespace OHOS::Ace::Platform