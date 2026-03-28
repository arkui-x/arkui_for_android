/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "adapter/android/capability/java/jni/vibrator/audio_haptic_player_jni.h"

#include <atomic>

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "ui/base/utils/utils.h"

namespace OHOS::Ace::Platform {
namespace {
constexpr const char* AUDIO_HAPTIC_PLAYER_JNI_CLASS = "ohos/ace/adapter/capability/vibrator/AudioHapticPlayer";
constexpr const char* METHOD_PRELOAD = "preload";
constexpr const char* METHOD_PLAY = "play";
constexpr const char* METHOD_RELEASE = "release";
constexpr const char* METHOD_GET_STREAM_VOLUME_DB = "getStreamVolumeDb";

constexpr const char* SIGNATURE_PRELOAD = "(Ljava/lang/String;)V";
constexpr const char* SIGNATURE_PLAY = "(F)V";
constexpr const char* SIGNATURE_RELEASE = "()V";
constexpr const char* SIGNATURE_GET_STREAM_VOLUME_DB = "()F";

const JniEnvironment::JniMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(AudioHapticPlayerJni::NativeInit) },
};

JniEnvironment::JavaGlobalRef g_jobject(nullptr, nullptr);
std::atomic<bool> g_initialized { false };

struct MethodIds {
    std::atomic<jmethodID> preload { nullptr };
    std::atomic<jmethodID> play { nullptr };
    std::atomic<jmethodID> release { nullptr };
    std::atomic<jmethodID> getStreamVolumeDb { nullptr };

    bool IsValid() const
    {
        return preload.load() != nullptr && play.load() != nullptr &&
               release.load() != nullptr && getStreamVolumeDb.load() != nullptr;
    }
} g_methodIds;

template<typename Invoker>
bool CallJavaVoid(jmethodID method, Invoker&& invoker)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_RETURN(env, false);

    invoker(env.get(), method);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

template<typename ResultType, typename Invoker>
ResultType CallJavaMethod(jmethodID method, ResultType defaultValue, Invoker&& invoker)
{
    auto env = JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_RETURN(env, defaultValue);
    CHECK_NULL_RETURN(method, defaultValue);

    ResultType result = invoker(env.get(), method);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return defaultValue;
    }
    return result;
}
} // namespace

void AudioHapticPlayerJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(jobj);

    auto newJobject = JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jobj);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    jmethodID preload = env->GetMethodID(cls, METHOD_PRELOAD, SIGNATURE_PRELOAD);
    jmethodID play = env->GetMethodID(cls, METHOD_PLAY, SIGNATURE_PLAY);
    jmethodID release = env->GetMethodID(cls, METHOD_RELEASE, SIGNATURE_RELEASE);
    jmethodID getStreamVolumeDb = env->GetMethodID(cls, METHOD_GET_STREAM_VOLUME_DB, SIGNATURE_GET_STREAM_VOLUME_DB);
    env->DeleteLocalRef(cls);
    
    CHECK_NULL_VOID(preload);
    CHECK_NULL_VOID(play);
    CHECK_NULL_VOID(release);
    CHECK_NULL_VOID(getStreamVolumeDb);

    g_methodIds.preload.store(preload);
    g_methodIds.play.store(play);
    g_methodIds.release.store(release);
    g_methodIds.getStreamVolumeDb.store(getStreamVolumeDb);
    g_jobject = std::move(newJobject);
    g_initialized.store(true);
}

bool AudioHapticPlayerJni::Register(std::shared_ptr<JNIEnv> env)
{
    CHECK_NULL_RETURN(env, false);

    jclass cls = env->FindClass(AUDIO_HAPTIC_PLAYER_JNI_CLASS);
    CHECK_NULL_RETURN(cls, false);

    bool ret = (env->RegisterNatives(
        cls, JniEnvironment::ToJniNativeMethod(METHODS), sizeof(METHODS) / sizeof(METHODS[0])) == JNI_OK);
    env->DeleteLocalRef(cls);
    return ret;
}

void AudioHapticPlayerJni::Preload(const std::string& uri)
{
    CHECK_EQUAL_VOID(g_initialized.load(), false);
    CHECK_EQUAL_VOID(g_methodIds.IsValid(), false);

    CallJavaVoid(g_methodIds.preload.load(), [uri](auto* env, auto method) {
        jstring jUri = env->NewStringUTF(uri.c_str());
        CHECK_EQUAL_VOID(jUri, nullptr);

        env->CallVoidMethod(g_jobject.get(), method, jUri);
        env->DeleteLocalRef(jUri);
    });
}

void AudioHapticPlayerJni::Play(float volume)
{
    CHECK_EQUAL_VOID(g_initialized.load(), false);
    CHECK_EQUAL_VOID(g_methodIds.IsValid(), false);

    CallJavaVoid(g_methodIds.play.load(), [volume](auto* env, auto method) {
        env->CallVoidMethod(g_jobject.get(), method, volume);
    });
}

float AudioHapticPlayerJni::GetStreamVolumeDb()
{
    CHECK_EQUAL_RETURN(g_initialized.load(), false, 0.0f);
    CHECK_EQUAL_RETURN(g_methodIds.IsValid(), false, 0.0f);

    return CallJavaMethod<jfloat>(g_methodIds.getStreamVolumeDb.load(), 0.0f,
        [](auto* env, auto method) {
            return env->CallFloatMethod(g_jobject.get(), method);
        });
}

void AudioHapticPlayerJni::Release()
{
    CHECK_EQUAL_VOID(g_initialized.load(), false);
    CHECK_EQUAL_VOID(g_methodIds.IsValid(), false);

    CallJavaVoid(g_methodIds.release.load(), [](auto* env, auto method) {
        env->CallVoidMethod(g_jobject.get(), method);
    });

    g_initialized.store(false);
}
} // namespace OHOS::Ace::Platform
