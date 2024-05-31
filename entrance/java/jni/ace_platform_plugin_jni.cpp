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

#include "adapter/android/entrance/java/jni/ace_platform_plugin_jni.h"

#include "flutter/fml/platform/android/jni_weak_ref.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "adapter/android/entrance/java/jni/native_window_surface.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/macros.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {
using NativeWindowMap = std::unordered_map<int64_t, void*>;
using NativeTextureMap = std::unordered_map<int64_t, fml::jni::JavaObjectWeakGlobalRef>;
std::unordered_map<int, RefPtr<AceResourceRegister>> g_resRegisters;
std::unordered_map<int, NativeWindowMap> g_nativeWindowMaps;
std::unordered_map<int, NativeTextureMap> g_surfaceTextureMaps;
bool AcePlatformPluginJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeInitResRegister",
            .signature = "(Lohos/ace/adapter/AceResourceRegister;I)J",
            .fnPtr = reinterpret_cast<void*>(&AcePlatformPluginJni::InitResRegister),
        },
        {
            .name = "nativeRegisterSurface",
            .signature = "(IJLjava/lang/Object;)V",
            .fnPtr = reinterpret_cast<void*>(&AcePlatformPluginJni::RegisterSurface),
        },
        {
            .name = "nativeUnregisterSurface",
            .signature = "(IJ)V",
            .fnPtr = reinterpret_cast<void*>(&AcePlatformPluginJni::UnregisterSurface),
        },
        {
            .name = "nativeRegisterTexture",
            .signature = "(IJLjava/lang/Object;)V",
            .fnPtr = reinterpret_cast<void*>(&AcePlatformPluginJni::RegisterTexture),
        },
        {
            .name = "nativeUnregisterTexture",
            .signature = "(IJ)V",
            .fnPtr = reinterpret_cast<void*>(&AcePlatformPluginJni::UnregisterTexture),
        },
    };

    if (!env) {
        LOGE("JNIEnv is null when register AcePlatformPluginJni jni！");
        return false;
    }

    const jclass myClass = env->FindClass("ohos/ace/adapter/AcePlatformPlugin");
    if (myClass == nullptr) {
        LOGE("Failed to find the AcePlatformPlugin Class");
        return false;
    }

    return env->RegisterNatives(myClass, methods, ArraySize(methods)) == 0;
}

jlong AcePlatformPluginJni::InitResRegister(JNIEnv* env, jobject myObject, jobject resRegister, jint instanceId)
{
    if (env == nullptr) {
        LOGE("env is null");
        return 0;
    }
    auto aceResRegister = Referenced::MakeRefPtr<AceResourceRegister>(resRegister, instanceId);
    if (aceResRegister && !aceResRegister->Initialize(env)) {
        LOGE("Failed to initialize the AcerResourceRegister");
        return 0;
    }
    
    g_resRegisters.emplace(static_cast<int32_t>(instanceId), aceResRegister);
    return PointerToJavaLong(AceType::RawPtr(aceResRegister));
}

RefPtr<AceResourceRegister> AcePlatformPluginJni::GetResRegister(int32_t instanceId)
{
    return g_resRegisters[instanceId];
}

void* AcePlatformPluginJni::GetNativeWindow(int32_t instanceId, int64_t textureId)
{
    auto iter = g_nativeWindowMaps.find(static_cast<int32_t>(instanceId));
    if (iter != g_nativeWindowMaps.end()) {
        auto map =  iter->second;
        auto nativeWindowIter = map.find(static_cast<int64_t>(textureId));
        if (nativeWindowIter != map.end()) {
            return nativeWindowIter->second;
        }
    }
    return nullptr;
}

void AcePlatformPluginJni::RegisterSurface(JNIEnv* env, jobject myObject,
    jint instanceId, jlong textureId, jobject surface)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }

    auto nativeWindow = reinterpret_cast<void*>(ANativeWindow_fromSurface(env, surface));
    auto iter = g_nativeWindowMaps.find(static_cast<int32_t>(instanceId));
    if (iter != g_nativeWindowMaps.end()) {
        iter->second.emplace(static_cast<int64_t>(textureId), nativeWindow);
    } else {
        std::unordered_map<int64_t, void*> nativeWindowMap;
        nativeWindowMap.emplace(static_cast<int64_t>(textureId), nativeWindow);
        g_nativeWindowMaps.emplace(static_cast<int32_t>(instanceId), nativeWindowMap);
    }
}

void AcePlatformPluginJni::UnregisterSurface(JNIEnv* env, jobject myObject, jint instanceId, jlong textureId)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }
    
    auto iter = g_nativeWindowMaps.find(static_cast<int32_t>(instanceId));
    if (iter == g_nativeWindowMaps.end()) {
        LOGW("UnregisterSurface fail, instanceId :%{public}d", instanceId);
        return;
    }

    iter->second.erase(static_cast<int64_t>(textureId));
}

void AcePlatformPluginJni::RegisterTexture(JNIEnv* env, jobject myObject,
    jint instanceId, jlong textureId, jobject surfaceTexture)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }

    auto nativeTexture = fml::jni::JavaObjectWeakGlobalRef(env, surfaceTexture);
    auto iter = g_surfaceTextureMaps.find(static_cast<int32_t>(instanceId));
    if (iter != g_surfaceTextureMaps.end()) {
        iter->second.emplace(static_cast<int64_t>(textureId), nativeTexture);
    } else {
        NativeTextureMap nativeTextureMap;
        nativeTextureMap.emplace(static_cast<int64_t>(textureId), nativeTexture);
        g_surfaceTextureMaps.emplace(static_cast<int32_t>(instanceId), nativeTextureMap);
    }
}

void AcePlatformPluginJni::UnregisterTexture(JNIEnv* env, jobject myObject, jint instanceId, jlong textureId)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }
    
    auto iter = g_surfaceTextureMaps.find(static_cast<int32_t>(instanceId));
    if (iter == g_surfaceTextureMaps.end()) {
        LOGW("UnregisterSurface fail, instanceId :%{public}d", instanceId);
        return;
    }

    iter->second.erase(static_cast<int64_t>(textureId));
}

void AcePlatformPluginJni::ReleaseInstance(int32_t instanceId)
{
    g_nativeWindowMaps.erase(instanceId);
    g_resRegisters.erase(instanceId);
    g_surfaceTextureMaps.erase(instanceId);
}
} // namespace OHOS::Ace::Platform