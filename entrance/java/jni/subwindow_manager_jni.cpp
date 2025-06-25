/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "subwindow_manager_jni.h"

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "adapter/android/entrance/java/jni/virtual_rs_window.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {
namespace {
static constexpr uint32_t ERROR_ENV = 0;
} // namespace

SubWindowManagerStruct SubWindowManagerJni::subWindowManagerStruct_;

bool SubWindowManagerJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    static const JNINativeMethod methods[] = { {
        .name = "nativeSetupSubWindowManager",
        .signature = "()V",
        .fnPtr = reinterpret_cast<void*>(&SetupSubWindowManager),
    } };
    static const JNINativeMethod subWindowMethods[] = {
        {
            .name = "nativeOnWindowTouchOutside",
            .signature = "(J)V",
            .fnPtr = reinterpret_cast<void*>(&OnWindowTouchOutside),
        },
        {
            .name = "nativeOnSubWindowHide",
            .signature = "(J)V",
            .fnPtr = reinterpret_cast<void*>(&OnSubWindowHide),
        }
    };

    if (!env) {
        LOGE("JNI Window: null java env");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/stage/ability/adapter/SubWindowManager");
    if (clazz == nullptr) {
        LOGE("JNI: can't find java class Window");
        return false;
    }

    const jclass subWindowClazz = env->FindClass("ohos/stage/ability/adapter/SubWindow");
    if (subWindowClazz == nullptr) {
        LOGE("JNI: can't find java class SubWindow");
        return false;
    }

    bool ret = env->RegisterNatives(clazz, methods, Ace::ArraySize(methods)) == 0;
    if (ret) {
        ret = env->RegisterNatives(subWindowClazz, subWindowMethods, Ace::ArraySize(subWindowMethods)) == 0;
    }

    env->DeleteLocalRef(clazz);
    return ret;
}

void SubWindowManagerJni::SetupSubWindowManager(JNIEnv* env, jobject obj)
{
    LOGI("SubWindowManagerJni::SetupDisplayInfo called");

    jclass clazz = env->GetObjectClass(obj);
    subWindowManagerStruct_.object = env->NewGlobalRef(obj);
    subWindowManagerStruct_.clazz = (jclass)env->NewGlobalRef(clazz);
    subWindowManagerStruct_.createSubWindowMethod =
        env->GetMethodID(clazz, "createSubWindow", "(Ljava/lang/String;IIIIIIII)Z");
    subWindowManagerStruct_.getContentViewMethod =
        env->GetMethodID(clazz, "getContentView", "(Ljava/lang/String;)Landroid/view/View;");
    subWindowManagerStruct_.resizeMethod = env->GetMethodID(clazz, "resize", "(Ljava/lang/String;II)Z");
    subWindowManagerStruct_.showWindowMethod = env->GetMethodID(clazz, "showWindow", "(Ljava/lang/String;)Z");
    subWindowManagerStruct_.moveWindowToMethod = env->GetMethodID(clazz, "moveWindowTo", "(Ljava/lang/String;II)Z");
    subWindowManagerStruct_.destroyWindowMethod = env->GetMethodID(clazz, "destroyWindow", "(Ljava/lang/String;)Z");
    subWindowManagerStruct_.getWindowIdMethod = env->GetMethodID(clazz, "getWindowId", "(Ljava/lang/String;)I");
    subWindowManagerStruct_.getTopWindowMethod = env->GetMethodID(clazz, "getTopWindow", "()Landroid/view/View;");

    subWindowManagerStruct_.setBackgroundColorMethod = env->GetMethodID(clazz, "setBackgroundColor", "(I)Z");
    subWindowManagerStruct_.setAppScreenBrightnessMethod = env->GetMethodID(clazz, "setAppScreenBrightness", "(F)Z");
    subWindowManagerStruct_.getAppScreenBrightnessMethod = env->GetMethodID(clazz, "getAppScreenBrightness", "()F");
    subWindowManagerStruct_.setKeepScreenOnMethod = env->GetMethodID(clazz, "setKeepScreenOn", "(Z)Z");
    subWindowManagerStruct_.isKeepScreenOnMethod = env->GetMethodID(clazz, "isKeepScreenOn", "()Z");
    subWindowManagerStruct_.requestOrientationMethod = env->GetMethodID(clazz, "requestOrientation", "(I)Z");
    subWindowManagerStruct_.setStatusBarStatusMethod = env->GetMethodID(clazz, "setStatusBarStatus", "(Z)Z");
    subWindowManagerStruct_.setActionBarStatusMethod = env->GetMethodID(clazz, "setActionBarStatus", "(Z)Z");
    subWindowManagerStruct_.isWindowShowingMethod = env->GetMethodID(clazz, "isShowing", "(Ljava/lang/String;)Z");
    subWindowManagerStruct_.setWindowLayoutFullScreenMethod =
        env->GetMethodID(clazz, "setWindowLayoutFullScreen", "(Z)Z");
    subWindowManagerStruct_.setNavigationBarStatusMethod = env->GetMethodID(clazz, "setNavigationBarStatus", "(Z)Z");
    subWindowManagerStruct_.setNavigationIndicatorStatusMethod =
        env->GetMethodID(clazz, "setNavigationIndicatorStatus", "(Z)Z");
    subWindowManagerStruct_.getStatusBarHeightMethod = env->GetMethodID(clazz, "getStatusBarHeight", "()I");
    subWindowManagerStruct_.getCutoutBarHeightMethod = env->GetMethodID(clazz, "getCutoutBarHeight", "()I");
    subWindowManagerStruct_.getNavigationBarStatusMethod = env->GetMethodID(clazz, "getNavigationBarStatus", "()Z");
    subWindowManagerStruct_.getNavigationIndicatorStatusMethod =
        env->GetMethodID(clazz, "getNavigationIndicatorStatus", "()Z");
    subWindowManagerStruct_.getScreenOrientationMethod = env->GetMethodID(clazz, "getScreenOrientation", "()I");
    subWindowManagerStruct_.getSafeAreaMethod = env->GetMethodID(clazz, "getSafeArea", "()Landroid/graphics/Rect;");
    subWindowManagerStruct_.hideMethod = env->GetMethodID(clazz, "hide", "(Ljava/lang/String;)Z");
    subWindowManagerStruct_.setFocusableMethod = env->GetMethodID(clazz, "setFocusable", "(Ljava/lang/String;Z)Z");
    subWindowManagerStruct_.setTouchableMethod = env->GetMethodID(clazz, "setTouchable", "(Ljava/lang/String;Z)Z");
    subWindowManagerStruct_.requestFocusMethod = env->GetMethodID(clazz, "requestFocus", "(Ljava/lang/String;)Z");
    subWindowManagerStruct_.setTouchHotAreaMethod =
        env->GetMethodID(clazz, "setTouchHotArea", "(Ljava/lang/String;[Landroid/graphics/Rect;)Z");
    subWindowManagerStruct_.setFullScreenMethod = env->GetMethodID(clazz, "setFullScreen", "(Ljava/lang/String;Z)Z");
    subWindowManagerStruct_.setOnTopMethod = env->GetMethodID(clazz, "setOnTop", "(Ljava/lang/String;Z)Z");
    subWindowManagerStruct_.registerSubWindowMethod =
        env->GetMethodID(clazz, "registerSubWindow", "(Ljava/lang/String;J)Z");
    subWindowManagerStruct_.unregisterSubWindowMethod =
        env->GetMethodID(clazz, "unregisterSubWindow", "(Ljava/lang/String;)Z");
    subWindowManagerStruct_.setStatusBarMethod = env->GetMethodID(clazz, "setStatusBar", "(II)Z");
    subWindowManagerStruct_.setNavigationBarMethod = env->GetMethodID(clazz, "setNavigationBar", "(II)Z");
    subWindowManagerStruct_.setWindowPrivacyModeMethod = env->GetMethodID(clazz, "setWindowPrivacyMode", "(Z)Z");
    subWindowManagerStruct_.getSurfaceRectMethod =
        env->GetMethodID(clazz, "getSurfaceRect", "()Landroid/graphics/Rect;");
}

void SubWindowManagerJni::OnWindowTouchOutside(JNIEnv* env, jobject obj, jlong window)
{
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr != nullptr) {
        windowPtr->NotifyTouchOutside();
    }
}

void SubWindowManagerJni::OnSubWindowHide(JNIEnv* env, jobject obj, jlong window)
{
    auto windowPtr = JavaLongToPointer<Rosen::Window>(window);
    if (windowPtr != nullptr) {
        windowPtr->SubWindowHide();
    }
}

bool SubWindowManagerJni::CreateSubWindow(std::shared_ptr<OHOS::Rosen::WindowOption> option)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::CreateSubWindow: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(option->GetWindowName().c_str());
    uint32_t parentId = option->GetParentId();
    int windowType = (int)option->GetWindowType();
    int windowMode = (int)option->GetWindowMode();
    int windowTag = (int)option->GetWindowTag();
    int width = option->GetWindowRect().width_;
    int height = option->GetWindowRect().height_;
    int x = option->GetWindowRect().posX_;
    int y = option->GetWindowRect().posY_;

    jboolean ret = env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.createSubWindowMethod,
        windowName, windowType, windowMode, windowTag, (int)parentId, x, y, width, height);

    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::CreateSubWindow: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::CreateSubWindow: failed");
        return false;
    }
}

jobject SubWindowManagerJni::GetContentView(const std::string& name)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::GetContentView: env is NULL");
        return nullptr;
    }

    jstring windowName = env->NewStringUTF(name.c_str());

    jobject view =
        env->CallObjectMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.getContentViewMethod, windowName);
    LOGI("SubWindowManagerJni::GetContentView: %d", view != nullptr);

    env->DeleteLocalRef(windowName);
    return view;
}

uint32_t SubWindowManagerJni::GetWindowId(const std::string& name)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::GetWindowId: env is NULL");
        return OHOS::Rosen::INVALID_WINDOW_ID;
    }

    jstring windowName = env->NewStringUTF(name.c_str());

    jint windowId =
        env->CallIntMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.getWindowIdMethod, windowName);
    env->DeleteLocalRef(windowName);
    return (uint32_t)windowId;
}

bool SubWindowManagerJni::ShowWindow(const std::string& name)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::ShowWindow: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());

    jboolean ret =
        env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.showWindowMethod, windowName);

    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::ShowWindow: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::ShowWindow: failed");
        return false;
    }
}

bool SubWindowManagerJni::MoveWindowTo(const std::string& name, int32_t x, int32_t y)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::MoveWindowTo: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());

    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.moveWindowToMethod, windowName, (int)x, (int)y);

    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::MoveWindowTo: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::MoveWindowTo: failed");
        return false;
    }
}

bool SubWindowManagerJni::ResizeWindowTo(const std::string& name, int32_t width, int32_t height)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::ResizeWindowTo: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());

    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.resizeMethod, windowName, (int)width, (int)height);

    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::ResizeWindowTo: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::ResizeWindowTo: failed");
        return false;
    }
}

bool SubWindowManagerJni::DestroyWindow(const std::string& name)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::DestroyWindow: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());

    jboolean ret =
        env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.destroyWindowMethod, windowName);

    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::DestroyWindow: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::DestroyWindow: failed");
        return false;
    }
}

bool SubWindowManagerJni::SetBackgroundColor(uint32_t color)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::setBackgroundColor: env is NULL");
        return false;
    }

    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setBackgroundColorMethod, (int)color);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::setBackgroundColor: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::setBackgroundColor: failed");
        return false;
    }
}

bool SubWindowManagerJni::SetAppScreenBrightness(float brightness)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::setAppScreenBrightness: env is NULL");
        return false;
    }

    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setAppScreenBrightnessMethod, brightness);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::setAppScreenBrightness: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::setAppScreenBrightness: failed");
        return false;
    }
}

float SubWindowManagerJni::GetAppScreenBrightness()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::GetAppScreenBrightness: env is NULL");
        return 0.0f;
    }

    jfloat ret =
        env->CallFloatMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.getAppScreenBrightnessMethod);
    return ret;
}

bool SubWindowManagerJni::SetKeepScreenOn(bool keepScreenOn)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::setKeepScreenOn: env is NULL");
        return false;
    }

    jboolean ret = env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.setKeepScreenOnMethod,
        keepScreenOn ? JNI_TRUE : JNI_FALSE);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::setKeepScreenOn: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::setKeepScreenOn: failed");
        return false;
    }
}

bool SubWindowManagerJni::IsKeepScreenOn()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::isKeepScreenOn: env is NULL");
        return false;
    }

    jboolean ret = env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.isKeepScreenOnMethod);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::isKeepScreenOn: true");
        return true;
    } else {
        LOGI("SubWindowManagerJni::isKeepScreenOn: false");
        return false;
    }
}

bool SubWindowManagerJni::RequestOrientation(Rosen::Orientation orientation)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::requestOrentation: env is NULL");
        return false;
    }

    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.requestOrientationMethod, (int)orientation);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::requestOrentation: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::requestOrentation: failed");
        return false;
    }
}

bool SubWindowManagerJni::SetWindowLayoutFullScreen(bool isFullScreen)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetWindowLayoutFullScreen: env is NULL");
        return false;
    }

    jboolean result = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setWindowLayoutFullScreenMethod, isFullScreen);

    bool conversionResult = static_cast<bool>(result);
    LOGI("SubWindowManagerJni::SetWindowLayoutFullScreen: result:%{public}d", conversionResult);
    return conversionResult;
}

bool SubWindowManagerJni::SetNavigationBarStatus(bool hide)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetNavigationBarStatus: env is NULL");
        return false;
    }

    jboolean result = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setNavigationBarStatusMethod, hide);

    bool conversionResult = static_cast<bool>(result);
    LOGI("SubWindowManagerJni::SetNavigationBarStatus: result:%{public}d", conversionResult);
    return conversionResult;
}

bool SubWindowManagerJni::SetNavigationIndicatorStatus(bool hide)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetNavigationIndicatorStatus: env is NULL");
        return false;
    }

    jboolean result = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setNavigationIndicatorStatusMethod, hide);

    bool conversionResult = static_cast<bool>(result);
    LOGI("SubWindowManagerJni::SetNavigationIndicatorStatus: result:%{public}d", conversionResult);
    return conversionResult;
}

bool SubWindowManagerJni::SetStatusBarStatus(bool hide)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::setStatusBarStatus: env is NULL");
        return false;
    }

    jboolean result = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setStatusBarStatusMethod, hide ? JNI_TRUE : JNI_FALSE);
    bool conversionResult = static_cast<bool>(result);
    LOGI("SubWindowManagerJni::SetStatusBarStatus: result:%{public}d", conversionResult);
    return conversionResult;
}

bool SubWindowManagerJni::SetActionBarStatus(bool hide)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::setActionBarStatus: env is NULL");
        return false;
    }

    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setActionBarStatusMethod, hide ? JNI_TRUE : JNI_FALSE);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::setActionBarStatus: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::setActionBarStatus: failed");
        return false;
    }
}

bool SubWindowManagerJni::IsWindowShowing(const std::string& name)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::IsWindowShowing: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());
    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.isWindowShowingMethod, windowName);
    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::IsWindowShowing: show");
        return true;
    } else {
        LOGI("SubWindowManagerJni::IsWindowShowing: hide");
        return false;
    }
}

uint32_t SubWindowManagerJni::GetStatusBarHeight()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::GetStatusBarHeight: env is NULL");
        return ERROR_ENV;
    }
    jint statusBarHeight =
        env->CallIntMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.getStatusBarHeightMethod);
    return static_cast<uint32_t>(statusBarHeight);
}

uint32_t SubWindowManagerJni::GetCutoutBarHeight()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::GetCutoutBarHeight: env is NULL");
        return ERROR_ENV;
    }
    jint statusBarWidth =
        env->CallIntMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.getCutoutBarHeightMethod);
    return static_cast<uint32_t>(statusBarWidth);
}

bool SubWindowManagerJni::GetNavigationBarStatus()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::GetNavigationBarStatus: env is NULL");
        return true;
    }
    jboolean navigationBarIsShow =
        env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.getNavigationBarStatusMethod);
    return static_cast<bool>(navigationBarIsShow);
}

bool SubWindowManagerJni::GetNavigationIndicatorStatus()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::GetNavigationIndicatorStatus: env is NULL");
        return true;
    }
    jboolean navigationIndicatorIsShow = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.getNavigationIndicatorStatusMethod);
    return static_cast<bool>(navigationIndicatorIsShow);
}

int32_t SubWindowManagerJni::GetScreenOrientation()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::GetScreenOrientation: env is NULL");
        return ERROR_ENV;
    }
    jint orientationType =
        env->CallIntMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.getScreenOrientationMethod);
    return static_cast<int32_t>(orientationType);
}

OHOS::Rosen::Rect SubWindowManagerJni::GetSafeArea()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    OHOS::Rosen::Rect rect = { 0, 0, 0, 0 };
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::GetSafeArea: env is NULL");
        return rect;
    }
    jobject javaRect = env->CallObjectMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.getSafeAreaMethod);
    jclass rectClass = env->FindClass("android/graphics/Rect");
    jfieldID xField = env->GetFieldID(rectClass, "left", "I");
    jfieldID yField = env->GetFieldID(rectClass, "top", "I");
    jfieldID widthField = env->GetFieldID(rectClass, "right", "I");
    jfieldID heightField = env->GetFieldID(rectClass, "bottom", "I");

    int x = env->GetIntField(javaRect, xField);
    int y = env->GetIntField(javaRect, yField);
    int width = env->GetIntField(javaRect, widthField);
    int height = env->GetIntField(javaRect, heightField);
    rect = { x, y, width, height };
    return rect;
}

OHOS::Rosen::Rect SubWindowManagerJni::GetSurfaceRect()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    OHOS::Rosen::Rect rect = { 0, 0, 0, 0 };
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::GetSurfaceRect: env is NULL");
        return rect;
    }
    jobject javaRect = env->CallObjectMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.getSurfaceRectMethod);
    jclass rectClass = env->FindClass("android/graphics/Rect");
    jfieldID xField = env->GetFieldID(rectClass, "left", "I");
    jfieldID yField = env->GetFieldID(rectClass, "top", "I");
    jfieldID widthField = env->GetFieldID(rectClass, "right", "I");
    jfieldID heightField = env->GetFieldID(rectClass, "bottom", "I");

    int x = env->GetIntField(javaRect, xField);
    int y = env->GetIntField(javaRect, yField);
    int width = env->GetIntField(javaRect, widthField);
    int height = env->GetIntField(javaRect, heightField);
    rect = { x, y, width, height };
    return rect;
}

bool SubWindowManagerJni::Hide(const std::string& name)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::Hide: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());
    jboolean ret =
        env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.hideMethod, windowName);
    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::Hide: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::Hide: failed");
        return false;
    }
}

bool SubWindowManagerJni::SetFocusable(const std::string& name, bool isFocusable)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::setFocusable: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());
    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setFocusableMethod, windowName, isFocusable);
    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::setFocusable: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::setFocusable: failed");
        return false;
    }
}

bool SubWindowManagerJni::SetTouchable(const std::string& name, bool isTouchable)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetWindowTouchable: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());
    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setTouchableMethod, windowName, isTouchable);
    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::SetWindowTouchable: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::SetWindowTouchable: failed");
        return false;
    }
}

bool SubWindowManagerJni::RequestFocus(const std::string& name)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::RequestFocuse: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());
    jboolean ret =
        env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.requestFocusMethod, windowName);
    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::RequestFocus: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::RequestFocus: failed");
        return false;
    }
}

bool SubWindowManagerJni::SetTouchHotAreas(const std::string& name, const std::vector<OHOS::Rosen::Rect>& rects)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetTouchHotAreas: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());
    jint size = rects.size();
    jclass rectClass = env->FindClass("android/graphics/Rect");
    jobjectArray javaRectArray = env->NewObjectArray(size, rectClass, nullptr);
    jmethodID rectConstructorID = env->GetMethodID(rectClass, "<init>", "(IIII)V");
    for (int i = 0; i < size; i++) {
        jobject javaRectObj = env->NewObject(rectClass, rectConstructorID, rects[i].posX_, rects[i].posY_,
            rects[i].posX_ + rects[i].width_, rects[i].posY_ + rects[i].height_);
        env->SetObjectArrayElement(javaRectArray, i, javaRectObj);
    }

    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setTouchHotAreaMethod, windowName, javaRectArray);
    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::SetTouchHotAreas: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::SetTouchHotAreas: failed");
        return false;
    }
}

bool SubWindowManagerJni::SetFullScreen(const std::string& name, bool status)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetFullScreen: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());
    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setFullScreenMethod, windowName, status);
    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::setFullScreen: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::setFullScreen: failed");
        return false;
    }
}

bool SubWindowManagerJni::SetOnTop(const std::string& name, bool status)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetOnTop: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());
    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.setOnTopMethod, windowName, status);
    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::setOnTop: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::setOnTop: failed");
        return false;
    }
}

bool SubWindowManagerJni::RegisterSubWindow(const std::string& name, void* subWindow)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetWindowTouchable: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());
    jlong windowHandle = PointerToJavaLong(subWindow);
    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.registerSubWindowMethod, windowName, windowHandle);
    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::RegisterSubWindow: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::RegisterSubWindow: failed");
        return false;
    }
}

bool SubWindowManagerJni::UnregisterSubWindow(const std::string& name)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetWindowTouchable: env is NULL");
        return false;
    }

    jstring windowName = env->NewStringUTF(name.c_str());
    jboolean ret = env->CallBooleanMethod(
        subWindowManagerStruct_.object, subWindowManagerStruct_.unregisterSubWindowMethod, windowName);
    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::UnregisterSubWindow: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::UnregisterSubWindow: failed");
        return false;
    }
}

bool SubWindowManagerJni::SetStatusBar(uint32_t backgroundColor, uint32_t contentColor)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetStatusBar: env is NULL");
        return false;
    }

    jboolean result = env->CallBooleanMethod(subWindowManagerStruct_.object,
        subWindowManagerStruct_.setStatusBarMethod, (int)backgroundColor, (int)contentColor);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    bool conversionResult = static_cast<bool>(result);
    LOGI("SubWindowManagerJni::SetStatusBar: result:%{public}d", conversionResult);
    return conversionResult;
}

bool SubWindowManagerJni::SetNavigationBar(uint32_t backgroundColor, uint32_t contentColor)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetNavigationBar: env is NULL");
        return false;
    }

    jboolean result = env->CallBooleanMethod(subWindowManagerStruct_.object,
        subWindowManagerStruct_.setNavigationBarMethod, (int)backgroundColor, (int)contentColor);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    bool conversionResult = static_cast<bool>(result);
    LOGI("SubWindowManagerJni::SetNavigationBar: result:%{public}d", conversionResult);
    return conversionResult;
}

bool SubWindowManagerJni::SetWindowPrivacyMode(bool isPrivacyMode)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::SetWindowPrivacyMode: env is NULL");
        return false;
    }

    jboolean ret = env->CallBooleanMethod(subWindowManagerStruct_.object,
        subWindowManagerStruct_.setWindowPrivacyModeMethod, isPrivacyMode ? JNI_TRUE : JNI_FALSE);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::SetWindowPrivacyMode: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::SetWindowPrivacyMode: failed");
        return false;
    }
}
} // namespace OHOS::Ace::Platform
