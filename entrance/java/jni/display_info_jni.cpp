/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "display_info_jni.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {

DisplayInfoStruct DisplayInfoJni::displayInfoStruct_;

bool DisplayInfoJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    static const JNINativeMethod methods[] = { {
        .name = "nativeSetupDisplayInfo",
        .signature = "()V",
        .fnPtr = reinterpret_cast<void*>(&SetupDisplayInfo),
    } };

    if (!env) {
        LOGE("JNI Window: null java env");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/ace/adapter/DisplayInfo");
    if (clazz == nullptr) {
        LOGE("JNI: can't find java class Window");
        return false;
    }
    bool ret = env->RegisterNatives(clazz, methods, ArraySize(methods)) == 0;
    env->DeleteLocalRef(clazz);
    return ret;
}

void DisplayInfoJni::SetupDisplayInfo(JNIEnv* env, jobject obj)
{
    LOGI("DisplayInfoJni::SetupDisplayInfo called");

    ReleaseDisplayInfo(env, obj);

    jclass clazz = env->GetObjectClass(obj);
    displayInfoStruct_.object = env->NewGlobalRef(obj);
    displayInfoStruct_.clazz = (jclass)env->NewGlobalRef(clazz);
    displayInfoStruct_.getDisplayIdMethod = env->GetMethodID(clazz, "getDisplayId", "()I");
    displayInfoStruct_.getOrentationMethod = env->GetMethodID(clazz, "getOrentation", "()I");
    displayInfoStruct_.getWidthMethod = env->GetMethodID(clazz, "getWidth", "()I");
    displayInfoStruct_.getHeightMethod = env->GetMethodID(clazz, "getHeight", "()I");
    displayInfoStruct_.getRefreshRateMethod = env->GetMethodID(clazz, "getRefreshRate", "()F");
    displayInfoStruct_.getDensityPixelsMethod = env->GetMethodID(clazz, "getDensityPixels", "()F");
    displayInfoStruct_.getDensityDpiMethod = env->GetMethodID(clazz, "getDensityDpi", "()I");
    displayInfoStruct_.getScaledDensityMethod = env->GetMethodID(clazz, "getScaledDensity", "()F");
    displayInfoStruct_.getXDpiMethod = env->GetMethodID(clazz, "getXDpi", "()F");
    displayInfoStruct_.getYDpiMethod = env->GetMethodID(clazz, "getYDpi", "()F");
    displayInfoStruct_.getRotationMethod = env->GetMethodID(clazz, "getRotation", "()I");
    displayInfoStruct_.getDisplayIdWithIdMethod = env->GetMethodID(clazz, "getDisplayId", "(I)I");
    displayInfoStruct_.getRotationWithIdMethod = env->GetMethodID(clazz, "getRotation", "(I)I");
    displayInfoStruct_.getOrentationWithIdMethod = env->GetMethodID(clazz, "getOrentation", "(I)I");
    displayInfoStruct_.getWidthWithIdMethod = env->GetMethodID(clazz, "getWidth", "(I)I");
    displayInfoStruct_.getHeightWithIdMethod = env->GetMethodID(clazz, "getHeight", "(I)I");
    displayInfoStruct_.getRefreshRateWithIdMethod = env->GetMethodID(clazz, "getRefreshRate", "(I)F");
    displayInfoStruct_.getDensityPixelsWithIdMethod = env->GetMethodID(clazz, "getDensityPixels", "(I)F");
    displayInfoStruct_.getDensityDpiWithIdMethod = env->GetMethodID(clazz, "getDensityDpi", "(I)I");
    displayInfoStruct_.getScaledDensityWithIdMethod = env->GetMethodID(clazz, "getScaledDensity", "(I)F");
    displayInfoStruct_.getXDpiWithIdMethod = env->GetMethodID(clazz, "getXDpi", "(I)F");
    displayInfoStruct_.getYDpiWithIdMethod = env->GetMethodID(clazz, "getYDpi", "(I)F");
}

int DisplayInfoJni::getDisplayId()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::getDisplayId env is NULL");
        return -1;
    }

    jint displayId = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getDisplayIdMethod);

    return (int)displayId;
}

int32_t DisplayInfoJni::getDisplayWidth()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::getDisplayWidth env is NULL");
        return -1;
    }

    jint width = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getWidthMethod);

    return (int32_t)width;
}

int32_t DisplayInfoJni::getDisplayHeight()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::getDisplayHeight env is NULL");
        return -1;
    }

    jint height = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getHeightMethod);

    return (int32_t)height;
}

int DisplayInfoJni::getOrentation()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::getOrentation env is NULL");
        return -1;
    }

    jint orentation = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getOrentationMethod);

    return (int)orentation;
}

float DisplayInfoJni::getRefreshRate()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::getRefreshRate env is NULL");
        return -1;
    }

    jfloat refreshRate = env->CallFloatMethod(displayInfoStruct_.object, displayInfoStruct_.getRefreshRateMethod);

    return (float)refreshRate;
}

float DisplayInfoJni::getDensityPixels()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::getDensityPixels env is NULL");
        return -1;
    }

    jfloat refreshRate = env->CallFloatMethod(displayInfoStruct_.object, displayInfoStruct_.getDensityPixelsMethod);

    return (float)refreshRate;
}

int DisplayInfoJni::getDensityDpi()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::getDensityDpi env is NULL");
        return -1;
    }

    jfloat refreshRate = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getDensityDpiMethod);

    return (int)refreshRate;
}

float DisplayInfoJni::getScaledDensity()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::getScaledDensity env is NULL");
        return -1;
    }

    jfloat refreshRate = env->CallFloatMethod(displayInfoStruct_.object, displayInfoStruct_.getScaledDensityMethod);

    return (float)refreshRate;
}

float DisplayInfoJni::getXDpi()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::getXDpi env is NULL");
        return -1;
    }
    jfloat xDpi = env->CallFloatMethod(displayInfoStruct_.object, displayInfoStruct_.getXDpiMethod);
    if (env->ExceptionCheck()) {
        LOGE("GetXDpi JNI has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return (float)xDpi;
}

float DisplayInfoJni::getYDpi()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::getYDpi env is NULL");
        return -1;
    }
    jfloat yDpi = env->CallFloatMethod(displayInfoStruct_.object, displayInfoStruct_.getYDpiMethod);
    if (env->ExceptionCheck()) {
        LOGE("GetYDpi JNI has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return (float)yDpi;
}

uint32_t DisplayInfoJni::GetRotation()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetRotation env is NULL");
        return 0;
    }

    jint rotation = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getRotationMethod);

    return static_cast<uint32_t>(rotation);
}

int DisplayInfoJni::GetDisplayId(int displayId)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetDisplayId env is NULL");
        return -1;
    }

    jint displayIdValue = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getDisplayIdWithIdMethod,
        static_cast<jint>(displayId));

    return static_cast<int>(displayIdValue);
}

uint32_t DisplayInfoJni::GetRotation(int displayId)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetRotation env is NULL");
        return 0;
    }

    jint rotation = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getRotationWithIdMethod,
        static_cast<jint>(displayId));

    return static_cast<uint32_t>(rotation);
}

int DisplayInfoJni::GetOrentation(int displayId)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetOrentation env is NULL");
        return -1;
    }

    jint orentation = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getOrentationWithIdMethod,
        static_cast<jint>(displayId));

    return (int)orentation;
}

int32_t DisplayInfoJni::GetDisplayWidth(int displayId)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetDisplayWidth env is NULL");
        return -1;
    }

    jint width = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getWidthWithIdMethod,
        static_cast<jint>(displayId));

    return (int32_t)width;
}

int32_t DisplayInfoJni::GetDisplayHeight(int displayId)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetDisplayHeight env is NULL");
        return -1;
    }

    jint height = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getHeightWithIdMethod,
        static_cast<jint>(displayId));

    return (int32_t)height;
}

float DisplayInfoJni::GetRefreshRate(int displayId)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetRefreshRate env is NULL");
        return -1;
    }

    jfloat refreshRate = env->CallFloatMethod(displayInfoStruct_.object, displayInfoStruct_.getRefreshRateWithIdMethod,
        static_cast<jint>(displayId));

    return (float)refreshRate;
}

float DisplayInfoJni::GetDensityPixels(int displayId)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetDensityPixels env is NULL");
        return -1;
    }

    jfloat refreshRate = env->CallFloatMethod(displayInfoStruct_.object,
        displayInfoStruct_.getDensityPixelsWithIdMethod,
        static_cast<jint>(displayId));

    return (float)refreshRate;
}

int DisplayInfoJni::GetDensityDpi(int displayId)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetDensityDpi env is NULL");
        return -1;
    }

    jfloat refreshRate = env->CallIntMethod(displayInfoStruct_.object, displayInfoStruct_.getDensityDpiWithIdMethod,
        static_cast<jint>(displayId));

    return (int)refreshRate;
}

float DisplayInfoJni::GetScaledDensity(int displayId)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetScaledDensity env is NULL");
        return -1;
    }

    jfloat refreshRate = env->CallFloatMethod(displayInfoStruct_.object,
        displayInfoStruct_.getScaledDensityWithIdMethod,
        static_cast<jint>(displayId));

    return (float)refreshRate;
}

float DisplayInfoJni::GetXDpi(int displayId)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetXDpi env is NULL");
        return -1;
    }
    jfloat xDpi = env->CallFloatMethod(displayInfoStruct_.object, displayInfoStruct_.getXDpiWithIdMethod,
        static_cast<jint>(displayId));
    if (env->ExceptionCheck()) {
        LOGE("GetXDpi JNI has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return (float)xDpi;
}

float DisplayInfoJni::GetYDpi(int displayId)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo::GetYDpi env is NULL");
        return -1;
    }
    jfloat yDpi = env->CallFloatMethod(displayInfoStruct_.object, displayInfoStruct_.getYDpiWithIdMethod,
        static_cast<jint>(displayId));
    if (env->ExceptionCheck()) {
        LOGE("GetYDpi JNI has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return (float)yDpi;
}

void DisplayInfoJni::ReleaseDisplayInfo(JNIEnv* env, jobject obj)
{
    if (displayInfoStruct_.object != nullptr) {
        env->DeleteGlobalRef(displayInfoStruct_.object);
        displayInfoStruct_.object = nullptr;
    }
    if (displayInfoStruct_.clazz != nullptr) {
        env->DeleteGlobalRef(displayInfoStruct_.clazz);
        displayInfoStruct_.clazz = nullptr;
    }
}

} // namespace OHOS::Ace::Platform
