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

#include "adapter/android/stage/uicontent/ace_container_sg.h"

#include "adapter/android/entrance/java/jni/ace_application_info_impl.h"
#include "adapter/android/entrance/java/jni/apk_asset_provider.h"
#include "adapter/android/entrance/java/jni/ace_platform_plugin_jni.h"
#include "adapter/android/stage/uicontent/ace_view_sg.h"
#include "base/log/ace_trace.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/common/ace_view.h"
#include "core/common/connect_server_manager.h"
#include "core/common/container_scope.h"
#include "core/common/flutter/flutter_asset_manager.h"
#include "core/common/flutter/flutter_task_executor.h"
#include "core/common/font_manager.h"
#include "core/common/platform_window.h"
#include "core/common/text_field_manager.h"
#include "core/common/thread_checker.h"
#include "core/common/watch_dog.h"
#include "core/common/window.h"
#include "core/components/theme/app_theme.h"
#include "core/components/theme/theme_constants.h"
#include "core/components/theme/theme_manager_impl.h"
#ifdef ENABLE_ROSEN_BACKEND
#include "core/components_ng/render/adapter/rosen_window.h"
#endif
#include "core/pipeline/base/element.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "event_handler.h"
#include "event_runner.h"
#ifdef NG_BUILD
#include "frameworks/bridge/declarative_frontend/ng/declarative_frontend_ng.h"
#else
#include "frameworks/bridge/declarative_frontend/declarative_frontend.h"
#endif
#include "frameworks/bridge/common/utils/engine_helper.h"
#include "frameworks/bridge/js_frontend/engine/common/js_engine_loader.h"
#include "frameworks/bridge/js_frontend/js_frontend.h"

namespace OHOS::Ace::Platform {
namespace {
const std::string ORI_MODE_KEY { "orientation" };
const std::string ORI_MODE_PORTRAIT { "PORTRAIT" };
const std::string ORI_MODE_LANDSCAPE { "LANDSCAPE" };
const std::string DENSITY_KEY { "densityDpi" };
constexpr double DPI_BASE { 160.0f };
constexpr int THEME_ID_LIGHT = 117440515;
constexpr int THEME_ID_DARK = 117440516;
} // namespace
AceContainerSG::AceContainerSG(int32_t instanceId, FrontendType type,
    std::weak_ptr<OHOS::AbilityRuntime::Platform::Context> runtimeContext,
    std::weak_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo, std::unique_ptr<PlatformEventCallback> callback,
    bool useCurrentEventRunner)
    : messageBridge_(AceType::MakeRefPtr<PlatformBridge>()), type_(type), instanceId_(instanceId),
      runtimeContext_(std::move(runtimeContext)), abilityInfo_(std::move(abilityInfo)),
      useCurrentEventRunner_(useCurrentEventRunner)
{
    ACE_DCHECK(callback);

    SetUseNewPipeline();

    useStageModel_ = true;

    auto flutterTaskExecutor = Referenced::MakeRefPtr<FlutterTaskExecutor>();
    flutterTaskExecutor->InitPlatformThread(useCurrentEventRunner_, useStageModel_);

    if (type_ == FrontendType::DECLARATIVE_JS) {
        GetSettings().useUIAsJSThread = true;
    } else {
        flutterTaskExecutor->InitJsThread();
    }

    taskExecutor_ = flutterTaskExecutor;

    platformEventCallback_ = std::move(callback);
}

void AceContainerSG::Initialize()
{
    // For Declarative_js frontend use UI as JS thread, so initializeFrontend after UI thread's creation
    ContainerScope scope(instanceId_);
    if (type_ != FrontendType::DECLARATIVE_JS) {
        InitializeFrontend();
    }
}

void AceContainerSG::Destroy()
{
    CHECK_NULL_VOID(pipelineContext_);
    CHECK_NULL_VOID(taskExecutor_);

    ContainerScope scope(instanceId_);
    // 1. Destroy Pipeline on UI Thread
    auto weak = AceType::WeakClaim(AceType::RawPtr(pipelineContext_));
    taskExecutor_->PostTask(
        [weak, taskExecutor = taskExecutor_]() {
            auto context = weak.Upgrade();
            CHECK_NULL_VOID(context);
            context->Destroy();
        },
        TaskExecutor::TaskType::UI);
    // 2.Destroy Frontend on JS Thread
    RefPtr<Frontend> frontend;
    frontend_.Swap(frontend);
    if (frontend) {
        taskExecutor_->PostTask(
            [frontend, id = instanceId_]() {
                frontend->UpdateState(Frontend::State::ON_DESTROY);
                frontend->Destroy();
                EngineHelper::RemoveEngine(id);
            },
            TaskExecutor::TaskType::JS);
    }

    // 3. Clear the data of this container
    messageBridge_.Reset();
    resRegister_.Reset();
    assetManager_.Reset();
    pipelineContext_.Reset();
    aceView_ = nullptr;
}

void AceContainerSG::InitializeFrontend()
{
    if (type_ == FrontendType::DECLARATIVE_JS) {
#ifdef NG_BUILD
        frontend_ = AceType::MakeRefPtr<NG::DeclarativeFrontend>();
        auto declarativeFrontend = AceType::DynamicCast<NG::DeclarativeFrontend>(frontend_);
#else
        frontend_ = AceType::MakeRefPtr<DeclarativeFrontend>();
        auto declarativeFrontend = AceType::DynamicCast<DeclarativeFrontend>(frontend_);
#endif
        auto& loader = Framework::JsEngineLoader::GetDeclarative(nullptr);
        RefPtr<Framework::JsEngine> jsEngine;
        if (GetSettings().usingSharedRuntime) {
            jsEngine = loader.CreateJsEngineUsingSharedRuntime(instanceId_, sharedRuntime_);
            LOGI("Create engine using sharedruntime, engine %{public}p", RawPtr(jsEngine));
        } else {
            jsEngine = loader.CreateJsEngine(instanceId_);
            LOGI("Create engine using own runtime, engine %{public}p", RawPtr(jsEngine));
        }
        declarativeFrontend->SetJsEngine(jsEngine);
        EngineHelper::AddEngine(instanceId_, jsEngine);
        declarativeFrontend->SetNeedDebugBreakPoint(AceApplicationInfo::GetInstance().IsNeedDebugBreakPoint());
        declarativeFrontend->SetDebugVersion(AceApplicationInfo::GetInstance().IsDebugVersion());
    } else {
        LOGE("Frontend Type not supported");
        EventReport::SendAppStartException(AppStartExcepType::FRONTEND_TYPE_ERR);
        return;
    }

    ACE_DCHECK(frontend_);
    frontend_->Initialize(type_, taskExecutor_);
    if (assetManager_) {
        frontend_->SetAssetManager(assetManager_);
    }

    LOGI("InitializeFrontend finished.");
}

void AceContainerSG::InitPiplineContext(std::unique_ptr<Window> window, double density, int32_t width, int32_t height)
{
    LOGI("init piplinecontext start.");
    ACE_DCHECK(aceView_ && window && taskExecutor_ && assetManager_ && resRegister_ && frontend_);
    auto instanceId = aceView_->GetInstanceId();
    LOGI("New pipeline version creating...");
    pipelineContext_ = AceType::MakeRefPtr<NG::PipelineContext>(
        std::move(window), taskExecutor_, assetManager_, resRegister_, frontend_, instanceId);

    pipelineContext_->SetRootSize(density, width, height);
    pipelineContext_->SetTextFieldManager(AceType::MakeRefPtr<TextFieldManager>());
    pipelineContext_->SetIsRightToLeft(AceApplicationInfo::GetInstance().IsRightToLeft());
    pipelineContext_->SetMessageBridge(messageBridge_);
    pipelineContext_->SetWindowModal(windowModal_);
    pipelineContext_->SetDrawDelegate(aceView_->GetDrawDelegate());
    pipelineContext_->SetFontScale(resourceInfo_.GetResourceConfiguration().GetFontRatio());
    pipelineContext_->SetIsJsCard(type_ == FrontendType::JS_CARD);

    LOGI("init piplinecontext end.");
}

void AceContainerSG::InitializeCallback()
{
    ACE_FUNCTION_TRACE();
    ACE_DCHECK(aceView_ && taskExecutor_ && pipelineContext_);
    auto weak = AceType::WeakClaim(AceType::RawPtr(pipelineContext_));
    auto instanceId = aceView_->GetInstanceId();
    auto&& touchEventCallback = [weak, instanceId](const TouchEvent& event, const std::function<void()>& markProcess) {
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);

        ContainerScope scope(instanceId);
        auto bombId = GetMilliseconds();
        AceEngine::Get().BuriedBomb(instanceId, bombId);
        AceEngine::Get().DefusingBomb(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, event]() {
                auto context = weak.Upgrade();
                CHECK_NULL_VOID(context);
                context->OnTouchEvent(event);
                context->NotifyDispatchTouchEventDismiss(event);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterTouchEventCallback(touchEventCallback);

    auto&& keyEventCallback = [weak, instanceId](const KeyEvent& event) {
        bool result = false;
        auto context = weak.Upgrade();
        CHECK_NULL_RETURN(context, result);

        ContainerScope scope(instanceId);
        auto bombId = GetMilliseconds();
        AceEngine::Get().BuriedBomb(instanceId, bombId);
        AceEngine::Get().DefusingBomb(instanceId);
        context->GetTaskExecutor()->PostSyncTask(
            [context, event, &result]() { result = context->OnKeyEvent(event); }, TaskExecutor::TaskType::UI);
        return result;
    };
    aceView_->RegisterKeyEventCallback(keyEventCallback);

    auto&& mouseEventCallback = [weak, instanceId](const MouseEvent& event, const std::function<void()>& markProcess) {
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);

        ContainerScope scope(instanceId);
        auto bombId = GetMilliseconds();
        AceEngine::Get().BuriedBomb(instanceId, bombId);
        AceEngine::Get().DefusingBomb(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, event]() {
                auto context = weak.Upgrade();
                CHECK_NULL_VOID(context);
                context->OnMouseEvent(event);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterMouseEventCallback(mouseEventCallback);

    auto&& rotationEventCallback = [weak, instanceId](const RotationEvent& event) {
        bool result = false;
        auto context = weak.Upgrade();
        CHECK_NULL_RETURN(context, result);
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostSyncTask(
            [context, event, &result]() { result = context->OnRotationEvent(event); }, TaskExecutor::TaskType::UI);
        return result;
    };
    aceView_->RegisterRotationEventCallback(rotationEventCallback);

    auto&& viewChangeCallback = [weak, instanceId](int32_t width, int32_t height, WindowSizeChangeReason reason,
                                    const std::shared_ptr<Rosen::RSTransaction> rsTransaction) {
        ACE_SCOPED_TRACE("ViewChangeCallback(%d, %d)", width, height);
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, width, height, reason]() {
                auto context = weak.Upgrade();
                CHECK_NULL_VOID(context);
                context->OnSurfaceChanged(width, height, reason);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterViewChangeCallback(viewChangeCallback);

    auto&& viewPositionChangeCallback = [weak, instanceId](int32_t posX, int32_t posY) {
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);
        ContainerScope scope(instanceId);
        ACE_SCOPED_TRACE("ViewPositionChangeCallback(%d, %d)", posX, posY);
        context->GetTaskExecutor()->PostTask(
            [weak, posX, posY]() {
                auto context = weak.Upgrade();
                CHECK_NULL_VOID(context);
                context->OnSurfacePositionChanged(posX, posY);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterViewPositionChangeCallback(viewPositionChangeCallback);

    auto&& densityChangeCallback = [weak, instanceId](double density) {
        ACE_SCOPED_TRACE("DensityChangeCallback(%lf)", density);
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, density]() {
                auto context = weak.Upgrade();
                CHECK_NULL_VOID(context);
                context->OnSurfaceDensityChanged(density);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterDensityChangeCallback(densityChangeCallback);

    auto&& systemBarHeightChangeCallback = [weak, instanceId](double statusBar, double navigationBar) {
        ACE_SCOPED_TRACE("SytemBarHeighChangeCallback(%lf, %lf)", statusBar, navigationBar);
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, statusBar, navigationBar]() {
                auto context = weak.Upgrade();
                CHECK_NULL_VOID(context);
                context->OnSystemBarHeightChanged(statusBar, navigationBar);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterSystemBarHeightChangeCallback(systemBarHeightChangeCallback);

    auto&& surfaceDestroyCallback = [weak, instanceId]() {
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);

        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak]() {
                auto context = weak.Upgrade();
                CHECK_NULL_VOID(context);
                context->OnSurfaceDestroyed();
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterSurfaceDestroyCallback(surfaceDestroyCallback);

    auto&& idleCallback = [weak, instanceId](int64_t deadline) {
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);

        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, deadline]() {
                auto context = weak.Upgrade();
                CHECK_NULL_VOID(context);
                context->OnIdle(deadline);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterIdleCallback(idleCallback);

    auto&& preDrawCallback = [weak, instanceId]() {
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak]() {
                auto context = weak.Upgrade();
                CHECK_NULL_VOID(context);
                context->NotifyOnPreDraw();
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterPreDrawCallback(preDrawCallback);
}

void AceContainerSG::InitializeEventHandler()
{
    ACE_DCHECK(aceView_ && pipelineContext_);
    auto instanceId = aceView_->GetInstanceId();
    InitializeFinishEventHandler(instanceId);
    InitializeStatusBarEventHandler(instanceId);
}

void AceContainerSG::InitializeFinishEventHandler(int32_t instanceId)
{
    auto&& finishEventHandler = [weak = WeakClaim(this), instanceId] {
        auto container = weak.Upgrade();
        CHECK_NULL_VOID(container);
        auto context = container->GetPipelineContext();
        CHECK_NULL_VOID(context);
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak = WeakPtr<AceContainerSG>(container)] {
                auto container = weak.Upgrade();
                CHECK_NULL_VOID(container);
                container->OnFinish();
            },
            TaskExecutor::TaskType::PLATFORM);
    };
    pipelineContext_->SetFinishEventHandler(finishEventHandler);
}

void AceContainerSG::InitializeStatusBarEventHandler(int32_t instanceId)
{
    auto&& setStatusBarEventHandler = [weak = WeakClaim(this), instanceId](const Color& color) {
        auto container = weak.Upgrade();
        CHECK_NULL_VOID(container);
        auto context = container->GetPipelineContext();
        CHECK_NULL_VOID(context);
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, color = color.GetValue()]() {
                auto container = weak.Upgrade();
                CHECK_NULL_VOID(container);
                if (container->platformEventCallback_) {
                    container->platformEventCallback_->OnStatusBarBgColorChanged(color);
                }
            },
            TaskExecutor::TaskType::PLATFORM);
    };
    pipelineContext_->SetStatusBarEventHandler(setStatusBarEventHandler);
}

void AceContainerSG::SetGetViewScaleCallback()
{
    ACE_DCHECK(aceView_ && pipelineContext_);
    auto instanceId = aceView_->GetInstanceId();
    pipelineContext_->SetGetViewScaleCallback([weak = WeakClaim(this), instanceId](float& scaleX, float& scaleY) {
        auto container = weak.Upgrade();
        CHECK_NULL_RETURN(container, false);
        ContainerScope scope(instanceId);
        if (container->aceView_) {
            return container->aceView_->GetScale(scaleX, scaleY);
        }
        return false;
    });
}

void AceContainerSG::Dispatch(
    const std::string& group, std::vector<uint8_t>&& data, int32_t id, bool replyToComponent) const
{}

void AceContainerSG::DispatchSync(
    const std::string& group, std::vector<uint8_t>&& data, uint8_t** resData, int64_t& position) const
{}

void AceContainerSG::DispatchPluginError(int32_t callbackId, int32_t errorCode, std::string&& errorMessage) const
{
    auto front = GetFrontend();
    CHECK_NULL_VOID(front);
    auto weakFront = AceType::WeakClaim(AceType::RawPtr(front));
    ContainerScope scope(instanceId_);
    taskExecutor_->PostTask(
        [weakFront, callbackId, errorCode, errorMessage = std::move(errorMessage)]() mutable {
            auto front = weakFront.Upgrade();
            CHECK_NULL_VOID(front);
            front->TransferJsPluginGetError(callbackId, errorCode, std::move(errorMessage));
        },
        TaskExecutor::TaskType::BACKGROUND);
}

bool AceContainerSG::Dump(const std::vector<std::string>& params, std::vector<std::string>& info)
{
    ContainerScope scope(instanceId_);
    if (aceView_ && aceView_->Dump(params)) {
        return true;
    }

    if (pipelineContext_) {
        pipelineContext_->Dump(params);
        return true;
    }

    return false;
}

void AceContainerSG::SetView(
    AceView* view, double density, int32_t width, int32_t height, OHOS::Rosen::Window* rsWindow)
{
    CHECK_NULL_VOID(view);
    auto container = AceType::DynamicCast<AceContainerSG>(AceEngine::Get().GetContainer(view->GetInstanceId()));
    CHECK_NULL_VOID(container);

#ifdef ENABLE_ROSEN_BACKEND
    auto* aceView = static_cast<AceViewSG*>(view);
    CHECK_NULL_VOID(aceView);
    auto eventHandler = std::make_shared<OHOS::AppExecFwk::EventHandler>(OHOS::AppExecFwk::EventRunner::Current());
    rsWindow->CreateVSyncReceiver(eventHandler);
    aceView->SetRSWinodw(rsWindow);
    auto window = std::make_unique<NG::RosenWindow>(rsWindow, container->GetTaskExecutor(), view->GetInstanceId());
    container->AttachView(std::move(window), view, density, width, height);
#endif
}

void AceContainerSG::AttachView(
    std::unique_ptr<Window> window, AceView* view, double density, int32_t width, int32_t height)
{
    aceView_ = view;
    auto instanceId = aceView_->GetInstanceId();
#ifdef ENABLE_ROSEN_BACKEND
    auto* aceView = static_cast<Platform::AceViewSG*>(aceView_);
    CHECK_NULL_VOID(aceView);
    auto flutterTaskExecutor = AceType::DynamicCast<FlutterTaskExecutor>(taskExecutor_);
    CHECK_NULL_VOID(flutterTaskExecutor);
    flutterTaskExecutor->InitOtherThreads(aceView->GetThreadModel());
#endif
    ContainerScope scope(instanceId);
    if (type_ == FrontendType::DECLARATIVE_JS) {
        // for declarative js frontend display ui in js thread
        flutterTaskExecutor->InitJsThread(false);
        InitializeFrontend();
        auto front = GetFrontend();
        if (front) {
#ifdef NG_BUILD
            auto jsFront = AceType::DynamicCast<DeclarativeFrontendNG>(front);
#else
            auto jsFront = AceType::DynamicCast<DeclarativeFrontend>(front);
#endif
            jsFront->SetInstanceName(GetInstanceName());
            front->UpdateState(Frontend::State::ON_CREATE);
            front->SetJsMessageDispatcher(AceType::Claim(this));
        }
    }

    auto resResgister = AcePlatformPluginJni::GetResRegister(instanceId);
    auto* aceViewSG = static_cast<AceViewSG*>(aceView_);
    aceViewSG->SetPlatformResRegister(resResgister);
    resRegister_ = aceView_->GetPlatformResRegister();

    InitPiplineContext(std::move(window), density, width, height);
    if (resRegister_) {
        resRegister_->SetPipelineContext(pipelineContext_);
    }
    InitializeCallback();
    InitializeEventHandler();
    SetGetViewScaleCallback();
    InitThemeManager();
    SetupRootElement();

    aceView_->Launch();
    frontend_->AttachPipelineContext(pipelineContext_);
}

void AceContainerSG::UpdateResourceConfiguration(const std::string& jsonStr)
{
    uint32_t updateFlags = 0;
    auto resConfig = resourceInfo_.GetResourceConfiguration();
    ContainerScope scope(instanceId_);
    if (!resConfig.UpdateFromJsonString(jsonStr, updateFlags)) {
        return;
    }
    resourceInfo_.SetResourceConfiguration(resConfig);
    if (!ResourceConfiguration::TestFlag(updateFlags, ResourceConfiguration::COLOR_MODE_UPDATED_FLAG)) {
        SystemProperties::SetColorMode(resConfig.GetColorMode());
        if (frontend_) {
            frontend_->FlushReload();
            frontend_->SetColorMode(resConfig.GetColorMode());
        }
    }

    std::unique_ptr<JsonValue> jsonConfig = JsonUtil::ParseJsonString(jsonStr);
    if (jsonConfig->Contains(ORI_MODE_KEY)) {
        auto oriMode = jsonConfig->GetValue(ORI_MODE_KEY);
        if (oriMode && oriMode->IsString()) {
            auto strOriMode = oriMode->GetString();
            if (strOriMode == ORI_MODE_PORTRAIT && resConfig.GetOrientation() != DeviceOrientation::PORTRAIT) {
                resConfig.SetOrientation(DeviceOrientation::PORTRAIT);
            } else if (strOriMode == ORI_MODE_LANDSCAPE && resConfig.GetOrientation() != DeviceOrientation::LANDSCAPE) {
                resConfig.SetOrientation(DeviceOrientation::LANDSCAPE);
            }
        }
    }
    if (jsonConfig->Contains(DENSITY_KEY)) {
        auto jsonDensity = jsonConfig->GetValue(DENSITY_KEY);
        if (jsonDensity && jsonDensity->IsNumber()) {
            double densityDpi = jsonDensity->GetInt();
            double density = densityDpi / DPI_BASE;
            resConfig.SetDensity(density);
        }
    }

    CHECK_NULL_VOID(pipelineContext_);
    auto themeManager = pipelineContext_->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    themeManager->UpdateConfig(resConfig);
    taskExecutor_->PostTask(
        [weakThemeManager = WeakPtr<ThemeManager>(themeManager), colorScheme = colorScheme_, config = resConfig,
            weakContext = WeakPtr<PipelineBase>(pipelineContext_)]() {
            auto themeManager = weakThemeManager.Upgrade();
            auto context = weakContext.Upgrade();
            if (!themeManager || !context) {
                return;
            }
            themeManager->LoadResourceThemes();
            themeManager->ParseSystemTheme();
            themeManager->SetColorScheme(colorScheme);
            context->NotifyConfigurationChange();
            context->FlushReload();
        },
        TaskExecutor::TaskType::UI);
    if (frontend_) {
        frontend_->RebuildAllPages();
    }
}

void AceContainerSG::InitThemeManager()
{
    LOGI("Init theme manager");
    auto initThemeManagerTask = [pipelineContext = pipelineContext_, assetManager = assetManager_,
                                    colorScheme = colorScheme_, resourceInfo = resourceInfo_,
                                    aceView = aceView_]() {
        ACE_SCOPED_TRACE("OHOS::LoadThemes()");
        LOGI("UIContent load theme");
        ThemeConstants::InitDeviceType();
        auto themeManager = AceType::MakeRefPtr<ThemeManagerImpl>();
        pipelineContext->SetThemeManager(themeManager);
        themeManager->InitResource(resourceInfo);
        themeManager->LoadSystemTheme(resourceInfo.GetThemeId());
        themeManager->SetColorScheme(colorScheme);
        themeManager->LoadCustomTheme(assetManager);
        themeManager->LoadResourceThemes();
        aceView->SetBackgroundColor(themeManager->GetBackgroundColor());
    };

    if (GetSettings().usePlatformAsUIThread) {
        initThemeManagerTask();
    } else {
        taskExecutor_->PostTask(initThemeManagerTask, TaskExecutor::TaskType::UI);
    }
}

void AceContainerSG::SetupRootElement()
{
    LOGI("Setup Root Element.");
    ContainerScope scope(instanceId_);
    auto weakContext = AceType::WeakClaim(AceType::RawPtr(pipelineContext_));
    auto setupRootElementTask = [weakContext]() {
        LOGI("execute SetupRootElement task start.");
        auto context = weakContext.Upgrade();
        CHECK_NULL_VOID(context);
        context->SetupRootElement();
        LOGI("execute SetupRootElement task end.");
    };

    if (GetSettings().usePlatformAsUIThread) {
        setupRootElementTask();
    } else {
        taskExecutor_->PostTask(setupRootElementTask, TaskExecutor::TaskType::UI);
    }
}

void AceContainerSG::SetInstanceName(const std::string& name)
{
    instanceName_ = name;
}

void AceContainerSG::SetHostClassName(const std::string& name)
{
    hostClassName_ = name;
}

void AceContainerSG::TriggerGarbageCollection()
{
    ContainerScope scope(instanceId_);
    taskExecutor_->PostTask([] { PurgeMallocCache(); }, TaskExecutor::TaskType::UI);
    taskExecutor_->PostTask(
        [frontend = WeakPtr<Frontend>(frontend_)]() {
            auto frontendRef = frontend.Upgrade();
            if (frontendRef) {
                frontendRef->TriggerGarbageCollection();
            }
            PurgeMallocCache();
        },
        TaskExecutor::TaskType::JS);
}

void AceContainerSG::NotifyFontNodes()
{
    ContainerScope scope(instanceId_);
    if (pipelineContext_ && pipelineContext_->GetFontManager()) {
        pipelineContext_->GetFontManager()->NotifyVariationNodes();
    }
}

void AceContainerSG::NotifyAppStorage(const std::string& key, const std::string& value)
{
    ContainerScope scope(instanceId_);
    if (frontend_) {
        frontend_->NotifyAppStorage(key, value);
    }
}

bool AceContainerSG::OnBackPressed(int32_t instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_RETURN(container, false);

    ContainerScope scope(instanceId);
    auto baseContext = container->GetPipelineContext();
    auto context = DynamicCast<NG::PipelineContext>(baseContext);

    CHECK_NULL_RETURN(context, false);
    if (context->PopPageStackOverlay()) {
        return true;
    }
    return context->CallRouterBackToPopPage();
}

void AceContainerSG::SetWindowPos(int32_t left, int32_t top)
{
    CHECK_NULL_VOID(frontend_);
    auto accessibilityManager = frontend_->GetAccessibilityManager();
    CHECK_NULL_VOID(accessibilityManager);
    accessibilityManager->SetWindowPos(left, top, windowId_);
}

void AceContainerSG::OnShow(int32_t instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_VOID(container);

    ContainerScope scope(instanceId);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [container]() {
            // When it is subContainer, no need call the OnShow,
            auto front = container->GetFrontend();
            auto pipelineBase = container->GetPipelineContext();
            CHECK_NULL_VOID(pipelineBase);
            pipelineBase->OnShow();
            pipelineBase->SetForegroundCalled(true);
        },
        TaskExecutor::TaskType::UI);
}

void AceContainerSG::OnHide(int32_t instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_VOID(container);

    ContainerScope scope(instanceId);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);

    taskExecutor->PostTask(
        [container]() {
            auto front = container->GetFrontend();
            if (front) {
                front->UpdateState(Frontend::State::ON_HIDE);
                front->OnHide();
            }
            container->TriggerGarbageCollection();
            auto pipelineContext = container->GetPipelineContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->OnHide();
        },
        TaskExecutor::TaskType::UI);
}

void AceContainerSG::OnActive(int32_t instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_VOID(container);
    ContainerScope scope(instanceId);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [container]() {
            // When it is subContainer, no need call the OnActive.
            auto front = container->GetFrontend();
            auto pipelineContext = container->GetPipelineContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->WindowFocus(true);
        },
        TaskExecutor::TaskType::UI);
}

void AceContainerSG::OnInactive(int32_t instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_VOID(container);
    ContainerScope scope(instanceId);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);

    taskExecutor->PostTask(
        [container]() {
            // When it is subContainer, no need call the OnInactive.
            auto front = container->GetFrontend();
            auto pipelineContext = container->GetPipelineContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->WindowFocus(false);
        },
        TaskExecutor::TaskType::UI);
}

void AceContainerSG::OnConfigurationUpdated(int32_t instanceId, const std::string& configuration)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_VOID(container);

    ContainerScope scope(instanceId);
    auto front = container->GetFrontend();
    CHECK_NULL_VOID(front);
    front->OnConfigurationUpdated(configuration);
}

void AceContainerSG::OnNewRequest(int32_t instanceId, const std::string& data)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_VOID(container);

    ContainerScope scope(instanceId);
    auto front = container->GetFrontend();
    if (front) {
        front->OnNewRequest(data);
    }
}

void AceContainerSG::DestroyView()
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_VOID_NOLOG(aceView_);
    auto aceView = static_cast<AceViewSG*>(aceView_);
    if (aceView) {
        aceView->DecRefCount();
    }
    aceView_ = nullptr;
}

void AceContainerSG::DestroyContainer(int32_t instanceId, const std::function<void()>& destroyCallback)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_VOID(container);
    container->Destroy();
    // unregister watchdog before stop thread to avoid UI_BLOCK report
    AceEngine::Get().UnRegisterFromWatchDog(instanceId);
    auto taskExecutor = container->GetTaskExecutor();
    if (taskExecutor) {
        taskExecutor->PostSyncTask([] { LOGI("Wait UI thread..."); }, TaskExecutor::TaskType::UI);
        taskExecutor->PostSyncTask([] { LOGI("Wait JS thread..."); }, TaskExecutor::TaskType::JS);
    }
    container->DestroyView(); // Stop all threads(ui,gpu,io) for current ability.
    auto removeContainerTask = [instanceId, destroyCallback] {
        LOGI("Remove on Platform thread...");
        EngineHelper::RemoveEngine(instanceId);
        AceEngine::Get().RemoveContainer(instanceId);
        if (destroyCallback) {
            destroyCallback();
        }
    };
    if (container->GetSettings().usePlatformAsUIThread) {
        removeContainerTask();
    } else {
        taskExecutor->PostTask(removeContainerTask, TaskExecutor::TaskType::PLATFORM);
    }
}

RefPtr<AceContainerSG> AceContainerSG::GetContainer(int32_t instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (container != nullptr) {
        auto aceContainerSG = AceType::DynamicCast<AceContainerSG>(container);
        return aceContainerSG;
    }
    return nullptr;
}

bool AceContainerSG::RunPage(int32_t instanceId, int32_t pageId, const std::string& content, const std::string& params)
{
    LOGI("RunPage content=[%{private}s] start", content.c_str());
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_RETURN(container, false);
    ContainerScope scope(instanceId);
    auto front = container->GetFrontend();
    if (front) {
        LOGD("RunPage content=[%{private}s]", content.c_str());
        front->RunPage(pageId, content, params);
        return true;
    }

    LOGI("RunPage content=[%{private}s] end", content.c_str());
    return false;
}

void AceContainerSG::SetResPaths(const std::string& hapResPath,
    const std::string& sysResPath, const ColorMode& colorMode)
{
    LOGI("SetResPaths, Use hap path to load resource");
    resourceInfo_.SetHapPath(hapResPath);
    // use package path to load system resource.
    auto sysFisrtPos = sysResPath.find_last_of('/');
    auto sysResourcePath = sysResPath.substr(0, sysFisrtPos);
    resourceInfo_.SetPackagePath(sysResourcePath);

    auto themeId = colorMode == ColorMode::LIGHT ? THEME_ID_LIGHT : THEME_ID_DARK;
    resourceInfo_.SetThemeId(themeId);
    colorScheme_ = colorMode == ColorMode::LIGHT ? ColorScheme::SCHEME_LIGHT : ColorScheme::SCHEME_DARK;
}
} // namespace OHOS::Ace::Platform
