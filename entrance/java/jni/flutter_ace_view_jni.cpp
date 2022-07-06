/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "adapter/android/entrance/java/jni/flutter_ace_view_jni.h"

#include "flutter/fml/platform/android/jni_weak_ref.h"
#ifdef NG_BUILD
#include "flutter/lib/ui/window/viewport_metrics.h"
#endif

#include "adapter/android/entrance/java/jni/ace_resource_register.h"
#include "adapter/android/entrance/java/jni/flutter_ace_view.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "adapter/android/entrance/java/jni/native_window_surface.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/macros.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/components/calendar/calendar_data_adapter.h"

namespace OHOS::Ace::Platform {
namespace {

static const JNINativeMethod ANDROID_METHODS[] = {
    {
        .name = "nativeCreateSurfaceHandle",
        .signature = "(Lohos/ace/adapter/AceViewAosp;I)J",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::CreateAndroidViewHandle),
    },
    {
        .name = "nativeSurfaceCreated",
        .signature = "(JLandroid/view/Surface;)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::SurfaceCreated),
    },
};

static const JNINativeMethod COMMON_METHODS[] = {
    {
        .name = "nativeDestroySurfaceHandle",
        .signature = "(J)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::DestroySurfaceHandle),
    },
    {
        .name = "nativeSurfaceChanged",
        .signature = "(JIII)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::SurfaceChanged),
    },
    {
        .name = "nativeSurfaceDestroyed",
        .signature = "(J)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::SurfaceDestroyed),
    },
    {
        .name = "nativeSetViewportMetrics",
        .signature = "(JFIIIIIIIIIIIIII)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::SetViewportMetrics),
    },
    {
        .name = "nativeDispatchPointerDataPacket",
        .signature = "(JLjava/nio/ByteBuffer;I)Z",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::DispatchPointerDataPacket),
    },
    {
        .name = "nativeRegisterTexture",
        .signature = "(JJLjava/lang/Object;)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::RegisterTexture),
    },
    {
        .name = "nativeRegisterSurface",
        .signature = "(JJLjava/lang/Object;)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::RegisterSurface),
    },
    {
        .name = "nativeMarkTextureFrameAvailable",
        .signature = "(JJ)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::MarkTextureFrameAvailable),
    },
    {
        .name = "nativeUnregisterTexture",
        .signature = "(JJ)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::UnregisterTexture),
    },
    {
        .name = "nativeUnregisterSurface",
        .signature = "(JJ)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::UnregisterSurface),
    },
    {
        .name = "nativeInitResRegister",
        .signature = "(JLohos/ace/adapter/AceResourceRegister;)J",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::InitResRegister),
    },
    {
        .name = "nativeInitCacheFilePath",
        .signature = "(JLjava/lang/String;Ljava/lang/String;)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::InitCacheFilePath),
    },
    {
        .name = "nativeDispatchKeyEvent",
        .signature = "(JIIIJJ)Z",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::DispatchKeyEvent),
    },
    {
        .name = "nativeInitDeviceType",
        .signature = "(I)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::InitDeviceType),
    },
    {
        .name = "nativeGetBackgroundColor",
        .signature = "(J)I",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::GetViewBackgoundColor),
    },
    {
        .name = "nativeSetCallback",
        .signature = "(JLjava/lang/Object;)V",
        .fnPtr = reinterpret_cast<void*>(&FlutterAceViewJni::SetNativeView),
    },
};

} // namespace

jlong FlutterAceViewJni::CreateAndroidViewHandle(JNIEnv* env, jclass myClass, jobject view, jint instanceId)
{
    return CreateViewHandle(env, myClass, view, instanceId, 0); // 0 is for android platfrom
}

jlong FlutterAceViewJni::CreateViewHandle(JNIEnv* env, jclass myClass, jobject view, jint instanceId, int32_t platform)
{
    if (env == nullptr) {
        LOGW("env is null");
        return 0;
    }

    LOGI("Create FlutterAceView");
    auto id = static_cast<int32_t>(instanceId);
    auto refAceView = Referenced::MakeRefPtr<FlutterAceView>(id);
    FlutterAceView* aceView = Referenced::RawPtr(refAceView);
    flutter::Settings settings;
#ifndef NG_BUILD
    fml::jni::JavaObjectWeakGlobalRef java_object(env, view);
    settings.instanceId = id;
    settings.platform = static_cast<flutter::AcePlatform>(platform);
    settings.idle_notification_callback = [weak = Referenced::WeakClaim(aceView)](int64_t deadline) {
        auto refPtr = weak.Upgrade();
        if (refPtr) {
            refPtr->ProcessIdleEvent(deadline);
        }
    };
    auto shell_holder = std::make_unique<flutter::AndroidShellHolder>(settings, java_object, false);
#else
    auto shell_holder = std::make_unique<AndroidShellHolder>(settings, id, false);
#endif
    aceView->SetShellHolder(std::move(shell_holder));
    aceView->IncRefCount();
    return PointerToJavaLong(aceView);
}

void FlutterAceViewJni::SurfaceCreated(JNIEnv* env, jobject myObject, jlong view, jobject jsurface)
{
    fml::jni::ScopedJavaLocalFrame scopedFrame(env);
    auto window = fml::MakeRefCounted<flutter::AndroidNativeWindow>(NativeWindowFromSurface::GetWindow(env, jsurface));
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr != nullptr) {
        auto platformView = viewPtr->GetShellHolder()->GetPlatformView();
        if (platformView) {
            platformView->NotifyCreated(std::move(window));
        }
    }
}

void FlutterAceViewJni::SurfaceChanged(
    JNIEnv* env, jobject myObject, jlong view, jint width, jint height, jint orientation)
{
    SystemProperties::SetDeviceOrientation(orientation);
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    ACE_DCHECK(viewPtr != nullptr);
    if (viewPtr != nullptr) {
        viewPtr->NotifySurfaceChanged(width, height);
        auto platformView = viewPtr->GetShellHolder()->GetPlatformView();
        if (platformView) {
            platformView->NotifyChanged(SkISize::Make(width, height));
        }
    }
}

void FlutterAceViewJni::SurfaceDestroyed(JNIEnv* env, jobject myObject, jlong view)
{
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    ACE_DCHECK(viewPtr != nullptr);
    if (viewPtr != nullptr) {
        viewPtr->NotifySurfaceDestroyed();
        auto platformView = viewPtr->GetShellHolder()->GetPlatformView();
        if (platformView) {
            platformView->NotifyDestroyed();
        }
    }
}

void FlutterAceViewJni::DestroySurfaceHandle(JNIEnv* env, jobject myObject, jlong view)
{
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    ACE_DCHECK(viewPtr != nullptr);
    if (viewPtr != nullptr) {
        viewPtr->DecRefCount();
    }
}

void FlutterAceViewJni::SetViewportMetrics(JNIEnv* env, jobject myObject, jlong view, jfloat devicePixelRatio,
    jint physicalWidth, jint physicalHeight, jint physicalPaddingTop, jint physicalPaddingRight,
    jint physicalPaddingBottom, jint physicalPaddingLeft, jint physicalViewInsetTop, jint physicalViewInsetRight,
    jint physicalViewInsetBottom, jint physicalViewInsetLeft, jint systemGestureInsetTop, jint systemGestureInsetRight,
    jint systemGestureInsetBottom, jint systemGestureInsetLeft)
{
    LOGI("SetViewPortMetrics");
    const flutter::ViewportMetrics metrics {
        static_cast<double>(devicePixelRatio), static_cast<double>(physicalWidth), static_cast<double>(physicalHeight),
        static_cast<double>(physicalPaddingTop), static_cast<double>(physicalPaddingRight),
        static_cast<double>(physicalPaddingBottom), static_cast<double>(physicalPaddingLeft),
        static_cast<double>(physicalViewInsetTop), static_cast<double>(physicalViewInsetRight),
        static_cast<double>(physicalViewInsetBottom), static_cast<double>(physicalViewInsetLeft),
        static_cast<double>(systemGestureInsetTop), static_cast<double>(systemGestureInsetRight),
        static_cast<double>(systemGestureInsetBottom), static_cast<double>(systemGestureInsetLeft),
#ifdef NG_BUILD
        0.0, // touch slop
#endif
    };
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr != nullptr) {
        viewPtr->NotifyDensityChanged(static_cast<double>(devicePixelRatio));
        viewPtr->NotifySystemBarHeightChanged(
            static_cast<double>(physicalPaddingTop), static_cast<double>(physicalViewInsetBottom));
        auto platformView = viewPtr->GetShellHolder()->GetPlatformView();
        if (platformView) {
            platformView->SetViewportMetrics(metrics);
        }
    }
}

jboolean FlutterAceViewJni::DispatchPointerDataPacket(
    JNIEnv* env, jobject myObject, jlong view, jobject buffer, jint position)
{
    if (env == nullptr) {
        LOGW("env is null");
        return false;
    }

    uint8_t* data = static_cast<uint8_t*>(env->GetDirectBufferAddress(buffer));
    auto packet = std::make_unique<flutter::PointerDataPacket>(data, position);
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr == nullptr) {
        LOGE("view is null");
        return false;
    }

    return viewPtr->ProcessTouchEvent(std::move(packet));
}

jboolean FlutterAceViewJni::DispatchKeyEvent(JNIEnv* env, jobject myObject, jlong view, jint keyCode, jint action,
    jint repeatTme, jlong timeStamp, jlong timeStampStart, jint metaKey, jint keySource, jint deviceId)
{
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr == nullptr) {
        LOGE("view is null");
        return false;
    }

    return viewPtr->ProcessKeyEvent(
        keyCode, action, repeatTme, timeStamp, timeStampStart, metaKey, keySource, deviceId);
}

void FlutterAceViewJni::DispatchMouseEvent(JNIEnv* env, jobject myObject, jlong view, jobject buffer, jint position)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }

    uint8_t* data = static_cast<uint8_t*>(env->GetDirectBufferAddress(buffer));
    auto packet = std::make_unique<flutter::PointerDataPacket>(data, position);
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr != nullptr) {
        viewPtr->ProcessMouseEvent(std::move(packet));
    }
}

void FlutterAceViewJni::RegisterTexture(
    JNIEnv* env, jobject myObject, jlong view, jlong texture_id, jobject surface_texture)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr != nullptr) {
        auto platformView = viewPtr->GetShellHolder()->GetPlatformView();
        if (platformView) {
            platformView->RegisterExternalTexture(
                static_cast<int64_t>(texture_id), fml::jni::JavaObjectWeakGlobalRef(env, surface_texture));
        }
    }
}

void FlutterAceViewJni::RegisterSurface(JNIEnv* env, jobject myObject, jlong view, jlong texture_id, jobject surface)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }

    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr != nullptr) {
        auto nativeWindow = reinterpret_cast<void*>(ANativeWindow_fromSurface(env, surface));
        viewPtr->RegisterSurface(static_cast<int64_t>(texture_id), nativeWindow);
    }
}

void FlutterAceViewJni::MarkTextureFrameAvailable(JNIEnv* env, jobject myObject, jlong view, jlong texture_id)
{
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr != nullptr) {
        auto platformView = viewPtr->GetShellHolder()->GetPlatformView();
        if (platformView) {
            platformView->MarkTextureFrameAvailable(static_cast<int64_t>(texture_id));
        }
    }
}

void FlutterAceViewJni::UnregisterTexture(JNIEnv* env, jobject myObject, jlong view, jlong texture_id)
{
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr != nullptr) {
        auto platformView = viewPtr->GetShellHolder()->GetPlatformView();
        if (platformView) {
            platformView->UnregisterTexture(static_cast<int64_t>(texture_id));
        }
    }
}

void FlutterAceViewJni::UnregisterSurface(JNIEnv* env, jobject myObject, jlong view, jlong texture_id)
{
    if (env == nullptr) {
        LOGW("env is null");
        return;
    }
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr != nullptr) {
        viewPtr->UnregisterSurface(static_cast<int64_t>(texture_id));
    }
}

bool FlutterAceViewJni::RegisterCommonNatives(JNIEnv* env, const jclass myClass)
{
    if (myClass == nullptr) {
        LOGE("Failed to find AceView Class");
        return false;
    }
    return env->RegisterNatives(myClass, COMMON_METHODS, ArraySize(COMMON_METHODS)) == 0;
}

bool FlutterAceViewJni::RegisterNatives(JNIEnv* env)
{
    if (env == nullptr) {
        LOGE("env is null");
        return false;
    }

    jclass myClass = nullptr;
    myClass = env->FindClass("ohos/ace/adapter/AceViewAosp");
    if (myClass == nullptr) {
        LOGE("Failed to find Activity AceView Class");
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

    env->DeleteLocalRef(myClass);
    return true;
}

jlong FlutterAceViewJni::InitResRegister(JNIEnv* env, jobject myObject, jlong view, jobject resRegister)
{
    if (env == nullptr) {
        LOGE("env is null");
        return 0;
    }
    auto aceResRegister = Referenced::MakeRefPtr<AceResourceRegister>(resRegister);
    if (!aceResRegister->Initialize(env)) {
        LOGE("Failed to initialize the AcerResourceRegister");
        return 0;
    }
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr == nullptr) {
        LOGE("viewPtr is null");
        return 0;
    }
    viewPtr->SetPlatformResRegister(aceResRegister);
    return PointerToJavaLong(AceType::RawPtr(aceResRegister));
}

void FlutterAceViewJni::InitCacheFilePath(
    JNIEnv* env, jobject myObject, jlong view, jstring imagePath, jstring filePath)
{
    if (env == nullptr) {
        LOGE("env is null");
        return;
    }
    const char* imagePathStr = env->GetStringUTFChars(imagePath, nullptr);
    if (imagePathStr != nullptr) {
        ImageCache::SetImageCacheFilePath(std::string(imagePathStr));
        ImageCache::SetCacheFileInfo();
        auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
        if (viewPtr != nullptr) {
            viewPtr->SetCachePath(std::string(imagePathStr));
        }
        env->ReleaseStringUTFChars(imagePath, imagePathStr);
    } else {
        LOGW("image cache path is null");
    }

    const char* filePathStr = env->GetStringUTFChars(filePath, nullptr);
    if (filePathStr != nullptr) {
        CalendarDataAdapter::SetCachePath(std::string(filePathStr));
        env->ReleaseStringUTFChars(filePath, filePathStr);
    } else {
        LOGW("file cache path is null");
    }
}

void FlutterAceViewJni::InitDeviceType(JNIEnv* env, jclass myClass, jint deviceType)
{
    if (deviceType != 0) {
        SystemProperties::InitDeviceType(DeviceType::TV);
    } else {
        SystemProperties::InitDeviceType(DeviceType::PHONE);
    }
}

jint FlutterAceViewJni::GetViewBackgoundColor(JNIEnv* env, jobject myObject, jlong view)
{
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr == nullptr) {
        return 0;
    }
    return viewPtr->GetBackgroundColor().GetValue();
}

void FlutterAceViewJni::SetNativeView(JNIEnv* env, jobject myObject, jlong view, jobject jObject)
{
    auto viewPtr = JavaLongToPointer<FlutterAceView>(view);
    if (viewPtr == nullptr) {
        return;
    }
    return viewPtr->SetViewCallback(env, jObject);
}

} // namespace OHOS::Ace::Platform
