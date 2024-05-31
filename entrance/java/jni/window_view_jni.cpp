/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "adapter/android/entrance/java/jni/window_view_jni.h"

#include "adapter/android/entrance/java/jni/virtual_rs_window.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "adapter/android/entrance/java/jni/native_window_surface.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {
namespace {
static jmethodID gRegisterWindowMethodID;
static jmethodID gUnRegisterWindowMethodID;

static const JNINativeMethod ANDROID_METHODS[] = {
    {
        .name = "nativeSurfaceCreated",
        .signature = "(JLandroid/view/Surface;)V",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::SurfaceCreated),
    },
};

static const JNINativeMethod COMMON_METHODS[] = {
    {
        .name = "nativeSurfaceChanged",
        .signature = "(JIIF)V",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::SurfaceChanged),
    },
    {
        .name = "nativeKeyboardHeightChanged",
        .signature = "(JI)V",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::KeyboardHeightChanged),
    },
    {
        .name = "nativeSurfaceDestroyed",
        .signature = "(J)V",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::SurfaceDestroyed),
    },
    {
        .name = "nativeOnWindowFocusChanged",
        .signature = "(JZ)V",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::OnWindowFocusChanged),
    },
    {
        .name = "nativeForeground",
        .signature = "(J)V",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::Foreground),
    },
    {
        .name = "nativeBackground",
        .signature = "(J)V",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::Background),
    },
    {
        .name = "nativeDestroy",
        .signature = "(J)V",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::Destroy),
    },
    {
        .name = "nativeBackPressed",
        .signature = "(J)Z",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::BackPressed),
    },
    {
        .name = "nativeDispatchPointerDataPacket",
        .signature = "(JLjava/nio/ByteBuffer;I)Z",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::DispatchPointerDataPacket),
    },
    {
        .name = "nativeDispatchMouseDataPacket",
        .signature = "(JLjava/nio/ByteBuffer;I)Z",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::DispatchMouseDataPacket),
    },
    {
        .name = "nativeDispatchKeyEvent",
        .signature = "(JIIIJJIII)Z",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::DispatchKeyEvent),
    },
};
} // namespace

void WindowViewJni::SurfaceCreated(JNIEnv* env, jobject myObject, jlong window, jobject jsurface)
{
    auto nativeWindow = static_cast<void*>(ANativeWindow_fromSurface(env, jsurface));
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr != nullptr) {
        windowPtr->CreateSurfaceNode(nativeWindow);
    }
}

void WindowViewJni::SurfaceChanged(
    JNIEnv* env, jobject myObject, jlong window, jint width, jint height, jfloat density)
{
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr != nullptr) {
        windowPtr->NotifySurfaceChanged(width, height, density);
    }
}

void WindowViewJni::KeyboardHeightChanged(
    JNIEnv* env, jobject myObject, jlong window, jint height)
{
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr != nullptr) {
        windowPtr->NotifyKeyboardHeightChanged(height);
    }
}

void WindowViewJni::SurfaceDestroyed(JNIEnv* env, jobject myObject, jlong window)
{
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr != nullptr) {
        windowPtr->NotifySurfaceDestroyed();
    }
}

void WindowViewJni::RegisterWindow(JNIEnv* env, void* window, jobject windowView)
{
    jlong windowHandle = PointerToJavaLong(window);
    env->CallVoidMethod(windowView, gRegisterWindowMethodID, windowHandle);
}

void WindowViewJni::UnRegisterWindow(JNIEnv* env, jobject windowView)
{
    env->CallVoidMethod(windowView, gUnRegisterWindowMethodID);
}

void WindowViewJni::OnWindowFocusChanged(JNIEnv* env, jobject myObject, jlong window, jboolean hasWindowFocus)
{
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr != nullptr) {
        windowPtr->WindowFocusChanged(hasWindowFocus);
    }
}

void WindowViewJni::Foreground(JNIEnv* env, jobject myObject, jlong window)
{
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr != nullptr) {
        windowPtr->Foreground();
    }
}

void WindowViewJni::Background(JNIEnv* env, jobject myObject, jlong window)
{
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr != nullptr) {
        windowPtr->Background();
    }
}

void WindowViewJni::Destroy(JNIEnv* env, jobject myObject, jlong window)
{
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr != nullptr) {
        windowPtr->Destroy();
    }
}

jboolean WindowViewJni::BackPressed(JNIEnv* env, jobject myObject, jlong window)
{
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr == nullptr) {
        LOGE("BackPressed window is nullptr");
        return false;
    }
    return windowPtr->ProcessBackPressed();
}

jboolean WindowViewJni::DispatchPointerDataPacket(
    JNIEnv* env, jobject myObject, jlong window, jobject buffer, jint position)
{
    if (env == nullptr) {
        LOGW("env is null");
        return false;
    }

    uint8_t* data = static_cast<uint8_t*>(env->GetDirectBufferAddress(buffer));
    std::vector<uint8_t> packet(data, data + position);
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr == nullptr) {
        LOGE("DispatchPointerDataPacket window is nullptr");
        return false;
    }

    return windowPtr->ProcessPointerEvent(packet);
}

jboolean WindowViewJni::DispatchMouseDataPacket(
    JNIEnv* env, jobject myObject, jlong window, jobject buffer, jint position)
{
    if (env == nullptr) {
        LOGW("env is null");
        return false;
    }

    uint8_t* data = static_cast<uint8_t*>(env->GetDirectBufferAddress(buffer));
    std::vector<uint8_t> packet(data, data + position);
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr == nullptr) {
        LOGE("DispatchMouseDataPacket window is nullptr");
        return false;
    }
    return windowPtr->ProcessMouseEvent(packet);
}

jboolean WindowViewJni::DispatchKeyEvent(JNIEnv* env, jobject myObject, jlong window, jint keyCode, jint action,
    jint repeatTime, jlong timeStamp, jlong timeStampStart, jint source, jint deviceId, jint metaKey)
{
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr == nullptr) {
        LOGE("DispatchKeyEvent window is nullptr");
        return false;
    }

    return windowPtr->ProcessKeyEvent(keyCode, action, repeatTime, timeStamp, timeStampStart, source, deviceId, metaKey);
}

bool WindowViewJni::RegisterCommonNatives(JNIEnv* env, const jclass myClass)
{
    if (myClass == nullptr) {
        LOGE("Failed to find WindowViewJni Class");
        return false;
    }
    return env->RegisterNatives(myClass, COMMON_METHODS, ArraySize(COMMON_METHODS)) == 0;
}

bool WindowViewJni::RegisterNatives(JNIEnv* env)
{
    if (env == nullptr) {
        LOGE("env is null");
        return false;
    }

    jclass myClass = nullptr;
    myClass = env->FindClass("ohos/ace/adapter/WindowView");
    if (myClass == nullptr) {
        LOGE("Failed to find Activity WindowViewJni Class");
        return false;
    }

    if (env->RegisterNatives(myClass, ANDROID_METHODS, ArraySize(ANDROID_METHODS)) != 0) {
        LOGE("Failed to register android platform natives");
        return false;
    }

    if (!RegisterCommonNatives(env, myClass)) {
        LOGE("Failed to register common natives");
        return false;
    }

    gRegisterWindowMethodID = env->GetMethodID(myClass, "registerWindow", "(J)V");
    gUnRegisterWindowMethodID = env->GetMethodID(myClass, "unRegisterWindow", "()V");

    env->DeleteLocalRef(myClass);
    return true;
}
} // namespace OHOS::Ace::Platform
