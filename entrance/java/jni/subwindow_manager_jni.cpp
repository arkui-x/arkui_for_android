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

#include "subwindow_manager_jni.h"
#include "base/log/log.h"
#include "base/utils/utils.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"

namespace OHOS::Ace::Platform {

SubWindowManagerStruct SubWindowManagerJni::subWindowManagerStruct_;

bool SubWindowManagerJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    static const JNINativeMethod methods[] = { {
        .name = "nativeSetupSubWindowManager",
        .signature = "()V",
        .fnPtr = reinterpret_cast<void*>(&SetupSubWindowManager),
    } };

    if (!env) {
        LOGE("JNI Window: null java env");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/ace/adapter/SubWindowManager");
    if (clazz == nullptr) {
        LOGE("JNI: can't find java class Window");
        return false;
    }

    bool ret = env->RegisterNatives(clazz, methods, Ace::ArraySize(methods)) == 0;
    env->DeleteLocalRef(clazz);
    return ret;
}

void SubWindowManagerJni::SetupSubWindowManager(JNIEnv* env, jobject obj)
{
    LOGI("SubWindowManagerJni::SetupDisplayInfo called");

    jclass clazz = env->GetObjectClass(obj);
    subWindowManagerStruct_.object = env->NewGlobalRef(obj);
    subWindowManagerStruct_.clazz = (jclass)env->NewGlobalRef(clazz);
    subWindowManagerStruct_.createSubWindowMethod = env->GetMethodID(clazz, "createSubWindow",
                                                        "(Ljava/lang/String;IIIIIIII)V");
    subWindowManagerStruct_.getContentViewMethod = env->GetMethodID(clazz, "getContentView",
                                                        "(Ljava/lang/String;)Landroid/view/View;");
    subWindowManagerStruct_.resizeMethod = env->GetMethodID(clazz, "resize", "(Ljava/lang/String;II)Z");
    subWindowManagerStruct_.showWindowMethod = env->GetMethodID(clazz, "showWindow", "(Ljava/lang/String;)Z");
    subWindowManagerStruct_.moveWindowToMethod = env->GetMethodID(clazz, "moveWindowTo", "(Ljava/lang/String;II)Z");
    subWindowManagerStruct_.destroyWindowMethod = env->GetMethodID(clazz, "destroyWindow", "(Ljava/lang/String;)Z");
    subWindowManagerStruct_.getWindowIdMethod = env->GetMethodID(clazz, "getWindowId", "(Ljava/lang/String;)I");
    subWindowManagerStruct_.getTopWindowMethod = env->GetMethodID(clazz, "getTopWindow", "()Landroid/view/View;");
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void SubWindowManagerJni::CreateSubWindow(std::shared_ptr<OHOS::Rosen::WindowOption> option)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::CreateSubWindow: env is NULL");
        return;
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

    env->CallVoidMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.createSubWindowMethod,
                        windowName, windowType, windowMode, windowTag, (int)parentId, width, height, x, y);

    LOGI("SubWindowManagerJni::CreateSubWindow: success");

    env->DeleteLocalRef(windowName);
}

jobject SubWindowManagerJni::GetContentView(const std::string& name)
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("SubWindowManagerJni::GetContentView: env is NULL");
        return nullptr;
    }

    jstring windowName = env->NewStringUTF(name.c_str());

    jobject view = env->CallObjectMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.getContentViewMethod,
                                         windowName);
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

    jint windowId = env->CallIntMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.getWindowIdMethod,
                                        windowName);
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

    jboolean ret = env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.showWindowMethod,
                                            windowName);

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

     jboolean ret = env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.moveWindowToMethod,
                                                windowName, (int)x, (int)y);

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

    jboolean ret = env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.resizeMethod,
                                                windowName, (int)width, (int)height);

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

    jboolean ret = env->CallBooleanMethod(subWindowManagerStruct_.object, subWindowManagerStruct_.destroyWindowMethod,
                                            windowName);

    env->DeleteLocalRef(windowName);
    if (ret == JNI_TRUE) {
        LOGI("SubWindowManagerJni::DestroyWindow: success");
        return true;
    } else {
        LOGI("SubWindowManagerJni::DestroyWindow: failed");
        return false;
    }
}

} // namespace OHOS::Ace::Platform
