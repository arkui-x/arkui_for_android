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

#include "adapter/android/entrance/java/jni/ace_container_jni.h"

#include <string>

#include "adapter/android/entrance/java/jni/ace_container.h"
#include "adapter/android/entrance/java/jni/apk_asset_provider.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/ace_trace.h"
#include "base/log/log.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/common/ace_engine.h"
#include "core/common/ace_view.h"
#include "core/common/container_scope.h"
#include "core/common/flutter/flutter_asset_manager.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/render/adapter/flutter_window.h"
#include "core/pipeline/pipeline_context.h"

namespace OHOS::Ace::Platform {

namespace {

const char LANGUAGE_TAG[] = "language";
const char COUNTRY_TAG[] = "countryOrRegion";
const char DIRECTION_TAG[] = "dir";
const char UNICODE_SETTING_TAG[] = "unicodeSetting";
const char LOCALE_DIR_LTR[] = "ltr";
const char LOCALE_DIR_RTL[] = "rtl";
const char LOCALE_KEY[] = "locale";

} // namespace

jboolean AceContainerJni::HandlePage(JNIEnv* env, jint instanceId, jstring content, jstring params, HandlePageType type)
{
    if (!env) {
        LOGE("JNIEnv is null");
        return false;
    }

    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        return false;
    }
    auto front = container->GetFrontend();
    if (!front) {
        std::string handleType = type == HandlePageType::RUN_PAGE ? "run page" : "push page";
        LOGE("frontend is null when %{public}s", handleType.c_str());
        return false;
    }

    const char* contentStr = env->GetStringUTFChars(content, nullptr);
    const char* paramsStr = env->GetStringUTFChars(params, nullptr);
    ContainerScope scope(instanceId);
    if (type == HandlePageType::RUN_PAGE) {
        front->RunPage(0, contentStr != nullptr ? contentStr : "", paramsStr != nullptr ? paramsStr : "");
    } else {
        front->PushPage(contentStr != nullptr ? contentStr : "", paramsStr != nullptr ? paramsStr : "");
    }

    if (contentStr != nullptr) {
        env->ReleaseStringUTFChars(content, contentStr);
    }
    if (paramsStr != nullptr) {
        env->ReleaseStringUTFChars(params, paramsStr);
    }

    return true;
}

bool AceContainerJni::Register()
{
    static const JNINativeMethod methods[] = { // JNI methods
        {
            .name = "nativeCreateContainer",
            .signature = "(IILohos/ace/adapter/AceEventCallback;Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&CreateContainer),
        },
        {
            .name = "nativeDestroyContainer",
            .signature = "(I)V",
            .fnPtr = reinterpret_cast<void*>(&DestroyContainer),
        },
        {
            .name = "nativeRun",
            .signature = "(ILjava/lang/String;Ljava/lang/String;)Z",
            .fnPtr = reinterpret_cast<void*>(&RunPage),
        },
        {
            .name = "nativePush",
            .signature = "(ILjava/lang/String;Ljava/lang/String;)Z",
            .fnPtr = reinterpret_cast<void*>(&PushPage),
        },
        {
            .name = "nativeOnBackPressed",
            .signature = "(I)Z",
            .fnPtr = reinterpret_cast<void*>(&NativeOnBackPressed),
        },
        {
            .name = "nativeOnShow",
            .signature = "(I)V",
            .fnPtr = reinterpret_cast<void*>(&NativeOnShow),
        },
        {
            .name = "nativeOnHide",
            .signature = "(I)V",
            .fnPtr = reinterpret_cast<void*>(&NativeOnHide),
        },
        {
            .name = "nativeOnActive",
            .signature = "(I)V",
            .fnPtr = reinterpret_cast<void*>(&NativeOnActive),
        },
        {
            .name = "nativeOnInactive",
            .signature = "(I)V",
            .fnPtr = reinterpret_cast<void*>(&NativeOnInactive),
        },
        {
            .name = "nativeOnNewRequest",
            .signature = "(ILjava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&NativeOnNewRequest),
        },
        {
            .name = "nativeOnMemoryLevel",
            .signature = "(II)V",
            .fnPtr = reinterpret_cast<void*>(&NativeOnMemoryLevel),
        },
        {
            .name = "nativeOnConfigurationUpdated",
            .signature = "(ILjava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&NativeOnConfigurationUpdated),
        },
        {
            .name = "nativeAddAssetPath",
            .signature = "(ILjava/lang/Object;Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&AddAssetPath),
        },
        {
            .name = "nativeSetView",
            .signature = "(IJFII)V",
            .fnPtr = reinterpret_cast<void*>(&SetView),
        },
        {
            .name = "nativeSetFontScale",
            .signature = "(IF)V",
            .fnPtr = reinterpret_cast<void*>(&SetFontScale),
        },
        {
            .name = "nativeSetWindowStyle",
            .signature = "(III)V",
            .fnPtr = reinterpret_cast<void*>(&SetWindowStyle),
        },
        {
            .name = "nativeSetSemiModalCustomStyle",
            .signature = "(III)V",
            .fnPtr = reinterpret_cast<void*>(&SetSemiModalCustomStyle),
        },
        {
            .name = "nativeSetColorMode",
            .signature = "(II)V",
            .fnPtr = reinterpret_cast<void*>(&SetColorMode),
        },
        {
            .name = "nativeSetHostClassName",
            .signature = "(ILjava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&SetHostClassName),
        },
        {
            .name = "nativeInitDeviceInfo",
            .signature = "(IIIIFZII)V",
            .fnPtr = reinterpret_cast<void*>(&InitDeviceInfo),
        },
        {
            .name = "nativeInitResourceManager",
            .signature = "(IILjava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&InitResourceManager),
        },
        {
            .name = "nativeSetLibPath",
            .signature = "(ILjava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&SetLibPath),
        }
    };

    // Get JNI environment of current thread.
    auto jniEnv = JniEnvironment::GetInstance().GetJniEnv();
    if (!jniEnv) {
        LOGE("JNI Initialize: failed to get JNI environment");
        return false;
    }

    const jclass clazz = jniEnv->FindClass("ohos/ace/adapter/AceContainer");
    if (clazz == nullptr) {
        LOGE("JNI Initialize: failed to get AceContainer class");
        return false;
    }

    auto res = jniEnv->RegisterNatives(clazz, methods, ArraySize(methods)) == 0;
    jniEnv->DeleteLocalRef(clazz);
    return res;
}

void AceContainerJni::CreateContainer(
    JNIEnv* env, jclass clazz, jint instanceId, jint type, jobject callback, jstring name)
{
    LOGI("JNI CreateContainer start");
    Container::UpdateCurrent(INSTANCE_ID_PLATFORM);
    FrontendType frontendType = static_cast<FrontendType>(type);
    auto aceContainer = AceType::MakeRefPtr<AceContainer>(instanceId, frontendType, callback);
    if (env == nullptr) {
        LOGE("CreateContainer null env");
        return;
    }
    const char* namePtr = env->GetStringUTFChars(name, nullptr);
    if (namePtr == nullptr) {
        LOGE("CreateContainer instance name is null");
        return;
    }

    AceEngine::Get().AddContainer(instanceId, aceContainer);
    ContainerScope scope(instanceId);
    aceContainer->Initialize();
    aceContainer->SetInstanceName(namePtr);

    auto front = aceContainer->GetFrontend();
    if (front) {
        front->UpdateState(Frontend::State::ON_CREATE);
        front->SetJsMessageDispatcher(aceContainer);
    }

    env->ReleaseStringUTFChars(name, namePtr);
    LOGI("JNI CreateContainer end");
}

void AceContainerJni::DestroyContainer(JNIEnv* env, jclass clazz, jint instanceId)
{
    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        LOGE("no this AceContainer %{public}d in AceEngine", instanceId);
        return;
    }
    LOGI("JNI DestroyContainer begin");
    container->Destroy();
    // Wait for the async tasks in UI/JS, then remove the container
    auto tastExecutor = container->GetTaskExecutor();
    if (tastExecutor) {
        tastExecutor->PostSyncTask([] { LOGI("Wait UI thread..."); }, TaskExecutor::TaskType::UI);
        tastExecutor->PostSyncTask([] { LOGI("Wait JS thread..."); }, TaskExecutor::TaskType::JS);
    }
    AceEngine::Get().RemoveContainer(instanceId);
    LOGI("JNI DestroyContainer end");
}

jboolean AceContainerJni::RunPage(JNIEnv* env, jclass clazz, jint instanceId, jstring content, jstring params)
{
    ACE_FUNCTION_TRACE();
    return HandlePage(env, instanceId, content, params, HandlePageType::RUN_PAGE);
}

jboolean AceContainerJni::PushPage(JNIEnv* env, jclass clazz, jint instanceId, jstring content, jstring params)
{
    ACE_FUNCTION_TRACE();
    return HandlePage(env, instanceId, content, params, HandlePageType::PUSH_PAGE);
}

jboolean AceContainerJni::NativeOnBackPressed(JNIEnv* env, jclass clazz, jint instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (!container) {
        LOGW("JNI Backpressed null container");
        return false;
    }

    ContainerScope scope(instanceId);
    auto context = container->GetPipelineContext();
    if (!context) {
        LOGW("JNI Backpressed null context");
        return false;
    }
    return context->CallRouterBackToPopPage();
}

void AceContainerJni::NativeOnShow(JNIEnv* env, jclass clazz, jint instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (!container) {
        return;
    }

    ContainerScope scope(instanceId);
    auto front = container->GetFrontend();
    if (front) {
        front->UpdateState(Frontend::State::ON_SHOW);
        front->OnShow();
    }

    auto context = container->GetPipelineContext();
    if (!context) {
        LOGW("JNI OnShow null context");
        return;
    }
    context->OnShow();
}

void AceContainerJni::NativeOnHide(JNIEnv* env, jclass clazz, jint instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (!container) {
        return;
    }

    ContainerScope scope(instanceId);
    auto front = container->GetFrontend();
    if (front) {
        front->UpdateState(Frontend::State::ON_HIDE);
        front->OnHide();
    }

    container->TriggerGarbageCollection();

    auto context = container->GetPipelineContext();
    if (!context) {
        LOGW("JNI OnHide null context");
        return;
    }
    context->OnHide();
}

void AceContainerJni::NativeOnConfigurationUpdated(JNIEnv* env, jclass clazz, jint instanceId, jstring data)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (!container) {
        return;
    }

    ContainerScope scope(instanceId);
    auto front = container->GetFrontend();
    if (!front) {
        return;
    }
    const char* dataStr = env->GetStringUTFChars(data, nullptr);
    if (dataStr == nullptr) {
        LOGW("JNI configuration updated, null data");
    }
    if (front->GetType() == FrontendType::DECLARATIVE_JS) {
        front->OnConfigurationUpdated(dataStr);
        container->UpdateResourceConfiguration(dataStr);
        env->ReleaseStringUTFChars(data, dataStr);
        return;
    }

    std::unique_ptr<JsonValue> value = JsonUtil::ParseJsonData(dataStr);
    if (!value) {
        env->ReleaseStringUTFChars(data, dataStr);
        return;
    }
    std::unique_ptr<JsonValue> localeValue = JsonUtil::Create(false);
    localeValue->Put(LANGUAGE_TAG, AceApplicationInfo::GetInstance().GetLanguage().c_str());
    localeValue->Put(COUNTRY_TAG, AceApplicationInfo::GetInstance().GetCountryOrRegion().c_str());
    localeValue->Put(
        DIRECTION_TAG, AceApplicationInfo::GetInstance().IsRightToLeft() ? LOCALE_DIR_RTL : LOCALE_DIR_LTR);
    localeValue->Put(UNICODE_SETTING_TAG, AceApplicationInfo::GetInstance().GetUnicodeSetting().c_str());
    value->Put(LOCALE_KEY, localeValue);
    front->OnConfigurationUpdated(value->ToString());
    env->ReleaseStringUTFChars(data, dataStr);
}

void AceContainerJni::NativeOnActive(JNIEnv* env, jclass clazz, jint instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (!container) {
        return;
    }

    ContainerScope scope(instanceId);
    auto front = container->GetFrontend();
    if (front) {
        front->OnActive();
    }
}

void AceContainerJni::NativeOnInactive(JNIEnv* env, jclass clazz, jint instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (!container) {
        return;
    }

    ContainerScope scope(instanceId);
    auto front = container->GetFrontend();
    if (front) {
        front->OnInactive();
    }
}

void AceContainerJni::NativeOnNewRequest(JNIEnv* env, jclass clazz, jint instanceId, jstring data)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (!container) {
        return;
    }

    ContainerScope scope(instanceId);
    auto front = container->GetFrontend();
    if (front) {
        const char* dataStr = env->GetStringUTFChars(data, nullptr);
        if (dataStr != nullptr) {
            front->OnNewRequest(dataStr);
        } else {
            front->OnNewRequest("");
        }
    }
}

void AceContainerJni::NativeOnMemoryLevel(JNIEnv* env, jclass clazz, jint instanceId, jint level)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (!container) {
        return;
    }

    ContainerScope scope(instanceId);
    auto front = container->GetFrontend();
    if (front) {
        front->OnMemoryLevel(level);
    }
}

void AceContainerJni::AddAssetPath(JNIEnv* env, jclass clazz, jint instanceId, jobject assetManager, jstring path)
{
    if (!env) {
        LOGW("JNI addAssetPath, null env");
        return;
    }

    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        LOGW("JNI addAssetPath, null container");
        return;
    }

    ContainerScope scope(instanceId);
    LOGI("JNI addAssetPath start");
    auto pathStr = env->GetStringUTFChars(path, nullptr);
    if (pathStr != nullptr) {
        auto flutterAssetManager = Referenced::MakeRefPtr<FlutterAssetManager>();
        container->SetAssetManagerIfNull(flutterAssetManager);
        flutterAssetManager = AceType::DynamicCast<FlutterAssetManager>(container->GetAssetManager());

        if (flutterAssetManager) {
            auto apkAssetProvider = AceType::MakeRefPtr<ApkAssetProvider>(
                std::make_unique<flutter::APKAssetProvider>(env, assetManager, pathStr));
            flutterAssetManager->PushBack(std::move(apkAssetProvider));
        }
        env->ReleaseStringUTFChars(path, pathStr);
    }
    LOGI("JNI addAssetPath end");
}

void AceContainerJni::SetView(
    JNIEnv* env, jclass clazz, jint instanceId, jlong nativePtr, jfloat density, jint width, jint height)
{
    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        LOGW("JNI setView, null container");
        return;
    }

    ContainerScope scope(instanceId);
    auto view = JavaLongToPointer<AceView>(nativePtr);
    if (view == nullptr) {
        LOGE("JNI setView: null view");
        return;
    }
#ifdef NG_BUILD
    std::unique_ptr<Window> window = std::make_unique<NG::FlutterWindow>(container->GetTaskExecutor(), instanceId);
#else
    auto platformWindow = view->GetPlatformWindow();
    if (!platformWindow) {
        LOGE("JNI setView: null platformWindow");
        return;
    }
    std::unique_ptr<Window> window = std::make_unique<Window>(std::move(platformWindow));
#endif
    container->AttachView(std::move(window), view, static_cast<double>(density), width, height);
}

void AceContainerJni::SetFontScale(JNIEnv* env, jclass clazz, jint instanceId, jfloat fontScale)
{
    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        LOGW("JNI setFontScale, null container");
        return;
    }

    ContainerScope scope(instanceId);
    auto config = container->GetResourceConfiguration();
    if (NearEqual(config.GetFontRatio(), fontScale)) {
        return;
    }
    config.SetFontRatio(fontScale);
    container->SetResourceConfiguration(config);
    auto pipelineContext = container->GetPipelineContext();
    if (!pipelineContext) {
        LOGW("JNI setFontScale, null pipelineContext");
        return;
    }
    pipelineContext->SetFontScale(fontScale);
}

void AceContainerJni::SetWindowStyle(JNIEnv* env, jclass clazz, jint instanceId, jint windowModal, jint colorScheme)
{
    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        LOGW("JNI setWindowStyle, null container");
        return;
    }

    ContainerScope scope(instanceId);
    if (windowModal >= static_cast<int32_t>(WindowModal::FIRST_VALUE) &&
        windowModal <= static_cast<int32_t>(WindowModal::LAST_VALUE)) {
        container->SetWindowModal(static_cast<WindowModal>(windowModal));
    }
    if (colorScheme >= static_cast<int32_t>(ColorScheme::FIRST_VALUE) &&
        colorScheme <= static_cast<int32_t>(ColorScheme::LAST_VALUE)) {
        container->SetColorScheme(static_cast<ColorScheme>(colorScheme));
    }
}

void AceContainerJni::SetSemiModalCustomStyle(
    JNIEnv* env, jclass clazz, jint instanceId, jint modalHeight, jint modalColor)
{
    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        LOGW("JNI setSemiModalCustomStyle, null container");
        return;
    }

    ContainerScope scope(instanceId);
    if (modalHeight > 0) {
        container->SetSemiModalHeight(modalHeight);
    }
    container->SetSemiModalColor(modalColor);
}

void AceContainerJni::SetColorMode(JNIEnv* env, jclass clazz, jint instanceId, jint colorMode)
{
    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        LOGW("JNI setColorMode, null container");
        return;
    }
    ContainerScope scope(instanceId);
    container->UpdateColorMode(static_cast<ColorMode>(colorMode));
}

void AceContainerJni::SetHostClassName(JNIEnv* env, jclass clazz, jint instanceId, jstring hostClassName)
{
    if (!env) {
        LOGW("JNI setHostClassName, null env");
        return;
    }

    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        LOGW("JNI setHostClassName, null container");
        return;
    }

    ContainerScope scope(instanceId);
    const char* hostClassNameStr = env->GetStringUTFChars(hostClassName, nullptr);
    container->SetHostClassName(hostClassNameStr != nullptr ? hostClassNameStr : "");
    if (hostClassNameStr != nullptr) {
        env->ReleaseStringUTFChars(hostClassName, hostClassNameStr);
    }
}

void AceContainerJni::InitDeviceInfo(JNIEnv* env, jclass clazz, jint instanceId, jint deviceWidth, jint deviceHeight,
    jint orientation, jfloat density, jboolean isRound, jint mcc, jint mnc)
{
    ContainerScope scope(instanceId);
    SystemProperties::InitDeviceInfo(deviceWidth, deviceHeight, orientation, density, isRound);
    SystemProperties::InitMccMnc(mcc, mnc);
    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        LOGW("JNI initDeviceInfo, null container");
        return;
    }
    auto config = container->GetResourceConfiguration();
    config.SetOrientation(SystemProperties::GetDeviceOrientation());
    config.SetDensity(density);
    config.SetDeviceType(SystemProperties::GetDeviceType());
    container->SetResourceConfiguration(config);
}

void AceContainerJni::InitResourceManager(JNIEnv* env, jclass clazz, jint instanceId, jint themeId, jstring path)
{
    if (!env) {
        LOGW("JNI initResourceManager, null env");
        return;
    }

    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        LOGW("JNI initResourceManager, null container");
        return;
    }
    ContainerScope scope(instanceId);
    auto pathStr = env->GetStringUTFChars(path, nullptr);
    std::string pkgPath;
    if (pathStr != nullptr) {
        pkgPath = pathStr;
        env->ReleaseStringUTFChars(path, pathStr);
    }
    container->SetThemeResourceInfo(pkgPath, themeId);
}

void AceContainerJni::SetLibPath(JNIEnv* env, jclass clazz, jint instanceId, jstring path)
{
    if (!env) {
        LOGW("JNI SetLibPath, null env");
        return;
    }

    auto container = AceType::DynamicCast<AceContainer>(AceEngine::Get().GetContainer(instanceId));
    if (!container) {
        LOGW("JNI SetLibPath, null container");
        return;
    }

    auto libPathStr = env->GetStringUTFChars(path, nullptr);
    std::string libPath;
    if (libPathStr != nullptr) {
        libPath = libPathStr;
        env->ReleaseStringUTFChars(path, libPathStr);
    }

    RefPtr<FlutterAssetManager> flutterAssetManager;
    if (container->GetAssetManager()) {
        flutterAssetManager = AceType::DynamicCast<FlutterAssetManager>(container->GetAssetManager());
    } else {
        flutterAssetManager = Referenced::MakeRefPtr<FlutterAssetManager>();
        container->SetAssetManagerIfNull(flutterAssetManager);
    }
    if (flutterAssetManager) {
        flutterAssetManager->SetLibPath({ libPath });
    }
}

}; // namespace OHOS::Ace::Platform