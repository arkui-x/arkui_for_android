/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <numeric>

#include "adapter/android/entrance/java/jni/ace_application_info_impl.h"
#include "adapter/android/entrance/java/jni/ace_platform_plugin_jni.h"
#include "adapter/android/entrance/java/jni/apk_asset_provider.h"
#include "adapter/android/entrance/java/jni/jni_registry.h"
#include "adapter/android/stage/uicontent/ace_view_sg.h"
#include "application_context.h"
#include "base/i18n/localization.h"
#include "base/log/ace_trace.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/subwindow/subwindow_manager.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/common/ace_view.h"
#include "core/common/connect_server_manager.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/common/font_manager.h"
#include "core/common/platform_window.h"
#include "core/common/resource/resource_manager.h"
#include "core/common/task_executor_impl.h"
#include "core/common/thread_checker.h"
#include "core/common/watch_dog.h"
#include "core/common/window.h"
#include "core/components/theme/app_theme.h"
#include "core/components/theme/theme_constants.h"
#include "core/components/theme/theme_manager_impl.h"
#include "core/components_ng/pattern/text_field/text_field_manager.h"
#ifdef ENABLE_ROSEN_BACKEND
#include "core/components_ng/render/adapter/rosen_window.h"
#endif
#include "event_handler.h"
#include "event_runner.h"

#include "core/pipeline/base/element.h"
#include "core/pipeline_ng/pipeline_context.h"
#ifdef NG_BUILD
#include "frameworks/bridge/declarative_frontend/ng/declarative_frontend_ng.h"
#else
#include "frameworks/bridge/declarative_frontend/declarative_frontend.h"
#endif
#include "frameworks/bridge/common/utils/engine_helper.h"
#include "frameworks/bridge/js_frontend/engine/common/js_engine_loader.h"
#include "frameworks/bridge/js_frontend/js_frontend.h"
#include "unicode/locid.h"

namespace OHOS::Ace::Platform {
namespace {
const std::string ORI_MODE_KEY { "orientation" };
const std::string ORI_MODE_PORTRAIT { "PORTRAIT" };
const std::string ORI_MODE_LANDSCAPE { "LANDSCAPE" };
const std::string DENSITY_KEY { "densityDpi" };
constexpr double DPI_BASE { 160.0f };
constexpr int THEME_ID_LIGHT = 117440515;
constexpr int THEME_ID_DARK = 117440516;
constexpr int INDEX_LANGUAGE = 0;
constexpr int INDEX_REGION = 1;
constexpr int INDEX_SCRIPT = 2;
void ParseLocaleTag(const std::string& localeTag, std::string& language, std::string& script, std::string& region)
{
    if (localeTag.empty()) {
        return;
    }

    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(icu::StringPiece(localeTag), status);
    if (status == U_ZERO_ERROR && !locale.isBogus()) {
        language = locale.getLanguage();
        script = locale.getScript();
        region = locale.getCountry();
        return;
    }

    std::size_t previous = 0;
    std::size_t current = localeTag.find('_');
    std::vector<std::string> elems;
    while (current != std::string::npos) {
        if (current > previous) {
            elems.push_back(localeTag.substr(previous, current - previous));
        }
        previous = current + 1;
        current = localeTag.find('_', previous);
    }
    if (previous != localeTag.size()) {
        elems.push_back(localeTag.substr(previous));
    }
    if (elems.size() == INDEX_REGION + 1) {
        language = elems[INDEX_LANGUAGE];
        region = elems[INDEX_REGION];
    } else if (elems.size() == INDEX_SCRIPT + 1) {
        language = elems[INDEX_LANGUAGE];
        region = elems[INDEX_REGION];
        script = elems[INDEX_SCRIPT].substr(elems[INDEX_SCRIPT].find("#") + 1);
    }
}
} // namespace

AceContainerSG::AceContainerSG(int32_t instanceId, FrontendType type,
    std::weak_ptr<OHOS::AbilityRuntime::Platform::Context> runtimeContext,
    std::weak_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo, std::unique_ptr<PlatformEventCallback> callback,
    bool useCurrentEventRunner, bool isSubAceContainer)
    : messageBridge_(AceType::MakeRefPtr<PlatformBridge>()), type_(type), instanceId_(instanceId),
      runtimeContext_(std::move(runtimeContext)), abilityInfo_(std::move(abilityInfo)),
      useCurrentEventRunner_(useCurrentEventRunner), isSubContainer_(isSubAceContainer)
{
    ACE_DCHECK(callback);

    SetUseNewPipeline();

    useStageModel_ = true;
    if (!isSubContainer_) {
        auto taskExecutorImpl = Referenced::MakeRefPtr<TaskExecutorImpl>();
        taskExecutorImpl->InitPlatformThread(useCurrentEventRunner_, useStageModel_);
        if (type_ == FrontendType::DECLARATIVE_JS) {
            GetSettings().useUIAsJSThread = true;
        } else {
            taskExecutorImpl->InitJsThread();
        }
        taskExecutor_ = taskExecutorImpl;
    }
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
    LOGI("AceContainerSG::Destroy start");
    ContainerScope scope(instanceId_);
    if (pipelineContext_ && taskExecutor_) {
        // 1. Destroy Pipeline on UI thread.
        RefPtr<PipelineBase> context;
        {
            std::lock_guard<std::mutex> lock(pipelineMutex_);
            context.Swap(pipelineContext_);
            LOGI("AceContainerSG::Destroy pipelineContext_ start");
        }
        auto uiTask = [context]() { context->Destroy(); };
        if (GetSettings().usePlatformAsUIThread) {
            uiTask();
            LOGI("AceContainerSG::Destroy pipelineContext_ end");
        } else {
            taskExecutor_->PostTask(uiTask, TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGDestroyPipeline");
        }

        if (isSubContainer_) {
            // SubAceContainerSG just return.
            LOGI("Is sub container, just return.");
            return;
        }

        // 2. Destroy Frontend on JS thread.
        RefPtr<Frontend> frontend;
        {
            std::lock_guard<std::mutex> lock(frontendMutex_);
            frontend.Swap(frontend_);
        }
        auto jsTask = [frontend]() {
            auto lock = frontend->GetLock();
            LOGI("AceContainerSG::Destroy frontend start");
            frontend->Destroy();
        };
        frontend->UpdateState(Frontend::State::ON_DESTROY);
        if (GetSettings().usePlatformAsUIThread && GetSettings().useUIAsJSThread) {
            jsTask();
            LOGI("AceContainerSG::Destroy frontend end");
        } else {
            taskExecutor_->PostTask(jsTask, TaskExecutor::TaskType::JS, "ArkUI-XAceContainerSGDestroyFrontend");
        }
    }
    // Clear the data of this container
    messageBridge_.Reset();
    resRegister_.Reset();
    assetManager_.Reset();
    pipelineContext_.Reset();
    LOGI("AceContainerSG::Destroy end");
}

void AceContainerSG::InitializeFrontend()
{
    if (type_ == FrontendType::DECLARATIVE_JS) {
        if (isSubContainer_) {
            auto container = GetContainer(parentId_);
            CHECK_NULL_VOID(container);
            frontend_ = container->GetFrontend();
            return;
        }

#ifdef NG_BUILD
        frontend_ = AceType::MakeRefPtr<DeclarativeFrontendNG>();
        auto declarativeFrontend = AceType::DynamicCast<DeclarativeFrontendNG>(frontend_);
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
        declarativeFrontend->SetPageProfile(pageProfile_);
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

void AceContainerSG::InitPiplineContext(
    std::unique_ptr<Window> window, double density, int32_t width, int32_t height, uint32_t windowId)
{
    ACE_DCHECK(aceView_ && window && taskExecutor_ && assetManager_ && resRegister_ && frontend_);
    auto instanceId = aceView_->GetInstanceId();
    pipelineContext_ = AceType::MakeRefPtr<NG::PipelineContext>(
        std::move(window), taskExecutor_, assetManager_, resRegister_, frontend_, instanceId);

    pipelineContext_->SetRootSize(density, width, height);
    pipelineContext_->SetTextFieldManager(AceType::MakeRefPtr<NG::TextFieldManagerNG>());
    pipelineContext_->SetIsRightToLeft(AceApplicationInfo::GetInstance().IsRightToLeft());
    pipelineContext_->SetMessageBridge(messageBridge_);
    pipelineContext_->SetWindowId(windowId);
    pipelineContext_->SetWindowModal(windowModal_);
    pipelineContext_->SetDrawDelegate(aceView_->GetDrawDelegate());
    pipelineContext_->SetGetWindowRectImpl([window = uiWindow_]() -> Rect {
        Rect rect;
        CHECK_NULL_RETURN(window, rect);
        auto windowRect = window->GetRect();
        rect.SetRect(windowRect.posX_, windowRect.posY_, windowRect.width_, windowRect.height_);
        return rect;
    });
    auto applicationContext = AbilityRuntime::Platform::ApplicationContext::GetInstance();
    if (applicationContext != nullptr) {
        auto configuration = applicationContext->GetConfiguration();
        if (configuration != nullptr) {
            auto followSystem =
                configuration->GetItem(OHOS::AbilityRuntime::Platform::ConfigurationInner::APP_FONT_SIZE_SCALE);
            float fontScaleValue = resourceInfo_.GetResourceConfiguration().GetFontRatio();
            bool bIsFollowSystem = (followSystem == "followSystem");
            pipelineContext_->SetFollowSystem(bIsFollowSystem);
            if (!bIsFollowSystem) {
                fontScaleValue = 1.0f;
            }
            auto maxFontScale =
                configuration->GetItem(OHOS::AbilityRuntime::Platform::ConfigurationInner::APP_FONT_MAX_SCALE);
            if (!maxFontScale.empty()) {
                pipelineContext_->SetMaxAppFontScale(StringUtils::StringToFloat(maxFontScale));
            }
            pipelineContext_->SetFontScale(fontScaleValue);
        }
    }
    pipelineContext_->SetIsJsCard(type_ == FrontendType::JS_CARD);

    if (uiWindow_) {
        auto windowType = uiWindow_->GetType();
        pipelineContext_->SetIsAppWindow(
            windowType < Rosen::WindowType::SYSTEM_WINDOW_BASE && windowType >= Rosen::WindowType::APP_WINDOW_BASE);
    }

    if (isSubContainer_) {
        pipelineContext_->SetIsSubPipeline(true);
    }
}

void AceContainerSG::InitializeCallback()
{
    ACE_FUNCTION_TRACE();
    ACE_DCHECK(aceView_ && taskExecutor_ && pipelineContext_);
    auto weak = AceType::WeakClaim(AceType::RawPtr(pipelineContext_));
    auto instanceId = aceView_->GetInstanceId();
    auto&& touchEventCallback = [weak, instanceId](const TouchEvent& event, const std::function<void()>& markProcess,
                                    const RefPtr<OHOS::Ace::NG::FrameNode>& node) {
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);

        ContainerScope scope(instanceId);
        auto bombId = GetMilliseconds();
        AceEngine::Get().BuriedBomb(instanceId, bombId);
        AceEngine::Get().DefusingBomb(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, event, node]() {
                auto context = weak.Upgrade();
                CHECK_NULL_VOID(context);
                if (event.type == TouchType::HOVER_ENTER || event.type == TouchType::HOVER_MOVE ||
                    event.type == TouchType::HOVER_EXIT || event.type == TouchType::HOVER_CANCEL) {
                    context->OnAccessibilityHoverEvent(event, node);
                } else {
                    context->OnTouchEvent(event);
                }
                context->NotifyDispatchTouchEventDismiss(event);
            },
            TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGTouchEventCallback");
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
        context->GetTaskExecutor()->PostSyncTask([context, event, &result]() { result = context->OnNonPointerEvent(event); },
            TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGKeyEventCallback");
        return result;
    };
    aceView_->RegisterKeyEventCallback(keyEventCallback);

    auto&& mouseEventCallback = [weak, instanceId](const MouseEvent& event, const std::function<void()>& markProcess,
                                    const RefPtr<OHOS::Ace::NG::FrameNode>& node) {
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
            TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGMouseEventCallback");
    };
    aceView_->RegisterMouseEventCallback(mouseEventCallback);

    auto&& rotationEventCallback = [weak, instanceId](const RotationEvent& event) {
        bool result = false;
        auto context = weak.Upgrade();
        CHECK_NULL_RETURN(context, result);
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostSyncTask(
            [context, event, &result]() { result = context->OnRotationEvent(event); }, TaskExecutor::TaskType::UI,
            "ArkUI-XAceContainerSGRotationEventCallback");
        return result;
    };
    aceView_->RegisterRotationEventCallback(rotationEventCallback);

    auto&& viewChangeCallback = [weak, instanceId](int32_t width, int32_t height, WindowSizeChangeReason reason,
                                    const std::shared_ptr<Rosen::RSTransaction> rsTransaction) {
        ACE_SCOPED_TRACE("ViewChangeCallback(%d, %d)", width, height);
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);
        ContainerScope scope(instanceId);
        auto task = [weak, width, height, reason, id = instanceId]() {
            auto context = weak.Upgrade();
            CHECK_NULL_VOID(context);
            context->OnSurfaceChanged(width, height, reason);
            if (reason == WindowSizeChangeReason::ROTATION) {
                auto subwindow = SubwindowManager::GetInstance()->GetSubwindow(id);
                CHECK_NULL_VOID(subwindow);
                subwindow->ResizeWindow();
            }
        };
        if (Container::Current()->GetSettings().usePlatformAsUIThread) {
            task();
        } else {
            context->GetTaskExecutor()->PostTask(
                task, TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGViewChangeCallback");
        }
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
            TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGViewPositionChangeCallback");
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
            TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGDensityChangeCallback");
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
            TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGSytemBarHeighChangeCallback");
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
            TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGSurfaceDestroyCallback");
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
            TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGIdleCallback");
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
            TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGPreDrawCallback");
    };
    aceView_->RegisterPreDrawCallback(preDrawCallback);

    auto&& dragEventCallback = [weak, instanceId](const DragPointerEvent& pointerEvent, const DragEventAction& action,
                                   const RefPtr<NG::FrameNode>& node) {
        ContainerScope scope(instanceId);
        auto context = weak.Upgrade();
        CHECK_NULL_VOID(context);
        auto callback = [context, pointerEvent, action, node]() { context->OnDragEvent(pointerEvent, action, node); };
        auto taskExecutor = context->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        auto uiTaskRunner = SingleTaskExecutor::Make(taskExecutor, TaskExecutor::TaskType::UI);
        if (uiTaskRunner.IsRunOnCurrentThread()) {
            callback();
            return;
        }
        taskExecutor->PostTask(callback, TaskExecutor::TaskType::UI, "ArkUI-XAceContainerDragEvent", PriorityType::VIP);
    };
    aceView_->RegisterDragEventCallback(dragEventCallback);
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
            TaskExecutor::TaskType::PLATFORM, "ArkUI-XAceContainerSGFinishEventHandler");
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
            TaskExecutor::TaskType::PLATFORM, "ArkUI-XAceContainerSGStatusBarEventHandler");
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
        TaskExecutor::TaskType::BACKGROUND, "ArkUI-XAceContainerSGDispatchPluginError");
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
    auto window = std::make_unique<NG::RosenWindow>(rsWindow, container->GetTaskExecutor(), view->GetInstanceId());
    AceContainerSG::SetUIWindow(view->GetInstanceId(), rsWindow);
    container->AttachView(std::move(window), view, density, width, height, rsWindow->GetWindowId());
#endif
}

void AceContainerSG::AttachView(
    std::unique_ptr<Window> window, AceView* view, double density, int32_t width, int32_t height, uint32_t windowId)
{
    aceView_ = view;
    auto instanceId = aceView_->GetInstanceId();
#ifdef ENABLE_ROSEN_BACKEND
    auto taskExecutorImpl = AceType::DynamicCast<TaskExecutorImpl>(taskExecutor_);
    CHECK_NULL_VOID(taskExecutorImpl);
    if (!isSubContainer_) {
        auto* aceView = static_cast<Platform::AceViewSG*>(aceView_);
        CHECK_NULL_VOID(aceView);
        taskExecutorImpl->InitOtherThreads(aceView->GetThreadModel());
    }
#endif
    ContainerScope scope(instanceId);
    if (type_ == FrontendType::DECLARATIVE_JS) {
        // for declarative js frontend display ui in js thread
        taskExecutorImpl->InitJsThread(false);
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
    if (resResgister == nullptr) {
        LOGW("AcePlatformPluginJni::GetResRegister fail.");
    }
    auto* aceViewSG = static_cast<AceViewSG*>(aceView_);
    aceViewSG->SetPlatformResRegister(resResgister);
    resRegister_ = aceView_->GetPlatformResRegister();

    InitPiplineContext(std::move(window), density, width, height, windowId);
    if (resRegister_) {
        resRegister_->SetPipelineContext(pipelineContext_);
    }
    InitializeCallback();
    InitializeEventHandler();
    SetGetViewScaleCallback();
    InitThemeManager();
    SetupRootElement();

    aceView_->Launch();
    if (!isSubContainer_) {
        frontend_->AttachPipelineContext(pipelineContext_);
    } else {
        auto declarativeFrontend = AceType::DynamicCast<DeclarativeFrontendNG>(frontend_);
        if (declarativeFrontend) {
            declarativeFrontend->AttachSubPipelineContext(pipelineContext_);
        }
    }
    pipelineContext_->SetGetWindowRectImpl([window = uiWindow_, weak = WeakClaim(this)]() -> Rect {
        Rect rect;
        CHECK_NULL_RETURN(window, rect);
        auto windowRect = window->GetRect();
        if (windowRect.IsUninitializedRect()) {
            auto container = weak.Upgrade();
            CHECK_NULL_RETURN(container, rect);
            bool isScb = container->IsSceneBoardWindow();
            rect.SetRect(isScb ? 0 : container->GetViewPosX(), isScb ? 0 : container->GetViewPosY(),
                container->GetViewWidth(), container->GetViewHeight());
        } else {
            rect.SetRect(windowRect.posX_, windowRect.posY_, windowRect.width_, windowRect.height_);
        }
        return rect;
    });
}

std::string AceContainerSG::GetOldLanguageTag() const
{
    const std::string& oldLanguage = AceApplicationInfo::GetInstance().GetLanguage();
    const std::string& oldCountry = AceApplicationInfo::GetInstance().GetCountryOrRegion();

    std::stringstream ss;
    ss << oldLanguage << "_" << oldCountry;
    std::string oldLanguageTag = ss.str();
    oldLanguageTag.reserve(oldLanguage.size() + oldCountry.size() + 1);
    return oldLanguageTag;
}

void AceContainerSG::UpdateConfiguration(Platform::ParsedConfig& parsedConfig)
{
    if (!parsedConfig.IsValid()) {
        LOGW("AceContainerSG::OnConfigurationUpdated param is empty");
        return;
    }
    LOGI("AceContainerSG::UpdateConfiguration, colorMode:%{public}s, direction:%{public}s, densityDpi:%{public}s,"
         "language:%{public}s",
         parsedConfig.colorMode.c_str(), parsedConfig.direction.c_str(), parsedConfig.densitydpi.c_str(),
         parsedConfig.languageTag.c_str());
    CHECK_NULL_VOID(pipelineContext_);
    ContainerScope scope(instanceId_);
    auto themeManager = pipelineContext_->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    ConfigurationChange configurationChange;
    auto resConfig = GetResourceConfiguration();
    SetColor(parsedConfig, configurationChange, resConfig);
    SetDirectionAndDensity(parsedConfig, configurationChange, resConfig);
    SetLanguage(parsedConfig, configurationChange, resConfig);
    SetFontAndScale(parsedConfig, configurationChange);
    SetResourceConfiguration(resConfig);
    themeManager->UpdateConfig(resConfig);
    if (SystemProperties::GetResourceDecoupling()) {
        ResourceManager::GetInstance().UpdateResourceConfig(GetBundleName(), GetModuleName(), instanceId_, resConfig);
    }
    themeManager->LoadResourceThemes();
    themeManager->ParseSystemTheme();
    themeManager->SetColorScheme(colorScheme_);
    if (colorScheme_ == ColorScheme::SCHEME_DARK) {
        pipelineContext_->SetAppBgColor(Color::BLACK);
    } else {
        pipelineContext_->SetAppBgColor(Color::WHITE);
    }
    pipelineContext_->RefreshRootBgColor();
    pipelineContext_->NotifyConfigurationChange();
    pipelineContext_->FlushReload(configurationChange);
    pipelineContext_->FlushReloadTransition();
}

void AceContainerSG::SetColor(Platform::ParsedConfig& parsedConfig, ConfigurationChange& configurationChange,
    ResourceConfiguration& resConfig)
{
    if (!parsedConfig.colorMode.empty()) {
        configurationChange.colorModeUpdate = true;
        if (parsedConfig.colorMode == "dark") {
            SetColorMode(ColorMode::DARK);
            SetColorScheme(ColorScheme::SCHEME_DARK);
            resConfig.SetColorMode(ColorMode::DARK);
        } else {
            SetColorMode(ColorMode::LIGHT);
            SetColorScheme(ColorScheme::SCHEME_LIGHT);
            resConfig.SetColorMode(ColorMode::LIGHT);
        }
        if (frontend_) {
            frontend_->FlushReload();
            frontend_->SetColorMode(resConfig.GetColorMode());
        }
    }
}

void AceContainerSG::SetDirectionAndDensity(Platform::ParsedConfig& parsedConfig,
    ConfigurationChange& configurationChange, ResourceConfiguration& resConfig)
{
    if (!parsedConfig.direction.empty()) {
        if (parsedConfig.direction == "vertical") {
            resConfig.SetOrientation(DeviceOrientation::PORTRAIT);
        } else if (parsedConfig.direction == "horizontal") {
            resConfig.SetOrientation(DeviceOrientation::LANDSCAPE);
        }
        configurationChange.directionUpdate = true;

        CHECK_NULL_VOID(pipelineContext_);
        auto dragDropManager = AceType::DynamicCast<NG::PipelineContext>(pipelineContext_)->GetDragDropManager();
        if (dragDropManager && dragDropManager->IsItemDragging()) {
            dragDropManager->CancelItemDrag();
        }
    }
    if (!parsedConfig.densitydpi.empty()) {
        double density = std::stoi(parsedConfig.densitydpi) / DPI_BASE;
        LOGI("resconfig density : %{public}f", density);
        configurationChange.dpiUpdate = true;
        resConfig.SetDensity(density);
    }
}

void AceContainerSG::SetLanguage(Platform::ParsedConfig& parsedConfig, ConfigurationChange& configurationChange,
    ResourceConfiguration& resConfig)
{
    if (!parsedConfig.languageTag.empty()) {
        std::string language;
        std::string script;
        std::string region;
        ParseLocaleTag(parsedConfig.languageTag, language, script, region);
        LOGI("language:%{public}s, script:%{public}s, region:%{public}s", language.c_str(), script.c_str(),
            region.c_str());
        if (!language.empty() || !script.empty() || !region.empty()) {
            if (parsedConfig.languageTag != GetOldLanguageTag()) {
                configurationChange.languageUpdate = true;
            }
            AceApplicationInfo::GetInstance().SetLocale(language, region, script, "");
        }
    }
}

void AceContainerSG::SetFontAndScale(Platform::ParsedConfig& parsedConfig, ConfigurationChange& configurationChange)
{
    if (!parsedConfig.fontFamily.empty()) {
        auto fontManager = pipelineContext_->GetFontManager();
        CHECK_NULL_VOID(fontManager);
        configurationChange.fontUpdate = true;
        fontManager->SetAppCustomFont(parsedConfig.fontFamily);
    }
    if (!parsedConfig.fontScale.empty()) {
        configurationChange.fontUpdate = true;
        float fontScale = StringUtils::StringToFloat(parsedConfig.fontScale);
        float maxFontScale = pipelineContext_->GetMaxAppFontScale();
        fontScale = std::min(fontScale, maxFontScale);
        pipelineContext_->SetFontScale(std::stof(parsedConfig.fontScale));
    }
}


void InitResourceAndThemeManager(const RefPtr<PipelineBase>& pipelineContext, const RefPtr<AssetManager>& assetManager,
    const ColorScheme& colorScheme, const ResourceInfo& resourceInfo, AceView* aceView,
    const std::shared_ptr<OHOS::AbilityRuntime::Platform::Context>& context,
    const std::shared_ptr<OHOS::AppExecFwk::AbilityInfo>& abilityInfo)
{
    auto resourceAdapter = ResourceAdapter::CreateV2();
    resourceAdapter->Init(resourceInfo);

    ThemeConstants::InitDeviceType();
    auto themeManager = AceType::MakeRefPtr<ThemeManagerImpl>(resourceAdapter);
    pipelineContext->SetThemeManager(themeManager);
    themeManager->SetColorScheme(colorScheme);
    themeManager->LoadCustomTheme(assetManager);
    themeManager->LoadResourceThemes();
    aceView->SetBackgroundColor(themeManager->GetBackgroundColor());

    auto defaultBundleName = "";
    auto defaultModuleName = "";
    int32_t instanceId = pipelineContext->GetInstanceId();
    ResourceManager::GetInstance().AddResourceAdapter(
        defaultBundleName, defaultModuleName, instanceId, resourceAdapter);
    if (context) {
        auto bundleName = context->GetBundleName();
        auto moduleName = context->GetHapModuleInfo()->name;
        if (!bundleName.empty() && !moduleName.empty()) {
            ResourceManager::GetInstance().AddResourceAdapter(bundleName, moduleName, instanceId, resourceAdapter);
        }
    } else if (abilityInfo) {
        auto bundleName = abilityInfo->bundleName;
        auto moduleName = abilityInfo->moduleName;
        if (!bundleName.empty() && !moduleName.empty()) {
            ResourceManager::GetInstance().AddResourceAdapter(bundleName, moduleName, instanceId, resourceAdapter);
        }
    }
}

void AceContainerSG::InitThemeManager()
{
    LOGI("Init theme manager");
    auto initThemeManagerTask = [pipelineContext = pipelineContext_, assetManager = assetManager_,
                                    colorScheme = colorScheme_, resourceInfo = resourceInfo_, aceView = aceView_,
                                    context = runtimeContext_.lock(), abilityInfo = abilityInfo_.lock()]() {
        ACE_SCOPED_TRACE("OHOS::LoadThemes()");
        LOGI("UIContent load theme");
        if (SystemProperties::GetResourceDecoupling()) {
            InitResourceAndThemeManager(
                pipelineContext, assetManager, colorScheme, resourceInfo, aceView, context, abilityInfo);
        } else {
            ThemeConstants::InitDeviceType();
            auto themeManager = AceType::MakeRefPtr<ThemeManagerImpl>();
            LOGI("UIContent SetThemeManager enter.");
            pipelineContext->SetThemeManager(themeManager);
            LOGI("UIContent SetThemeManager end.");
            themeManager->InitResource(resourceInfo);
            LOGI("UIContent InitResource end.");
            themeManager->SetColorScheme(colorScheme);
            themeManager->LoadResourceThemes();
            LOGI("UIContent LoadResourceThemes end.");
            aceView->SetBackgroundColor(themeManager->GetBackgroundColor());
        }
        if (colorScheme == ColorScheme::SCHEME_DARK) {
            pipelineContext->SetAppBgColor(Color::BLACK);
        } else {
            pipelineContext->SetAppBgColor(Color::WHITE);
        }
    };

    if (GetSettings().usePlatformAsUIThread) {
        initThemeManagerTask();
    } else {
        taskExecutor_->PostTask(
            initThemeManagerTask, TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGInitThemeManager");
    }
}

void AceContainerSG::SetupRootElement()
{
    LOGI("Setup Root Element.");
    ContainerScope scope(instanceId_);
    auto weakContext = AceType::WeakClaim(AceType::RawPtr(pipelineContext_));
    auto setupRootElementTask = [weakContext, isSubContainer = isSubContainer_]() {
        LOGI("execute SetupRootElement task start.");
        auto context = weakContext.Upgrade();
        CHECK_NULL_VOID(context);
        if (!isSubContainer) {
            context->SetupRootElement();
        }
        LOGI("execute SetupRootElement task end.");
    };

    if (GetSettings().usePlatformAsUIThread) {
        setupRootElementTask();
    } else {
        taskExecutor_->PostTask(
            setupRootElementTask, TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGSetupRootElement");
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
    taskExecutor_->PostTask(
        [] { PurgeMallocCache(); }, TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGTriggerGarbageCollectionUI");
    taskExecutor_->PostTask(
        [frontend = WeakPtr<Frontend>(frontend_)]() {
            auto frontendRef = frontend.Upgrade();
            if (frontendRef) {
                frontendRef->TriggerGarbageCollection();
            }
            PurgeMallocCache();
        },
        TaskExecutor::TaskType::JS, "ArkUI-XAceContainerSGTriggerGarbageCollectionJS");
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

    // When the container is for overlay, it need close the overlay first.
    if (container->IsSubContainer()) {
        LOGI("Back press for remove overlay node");
        ContainerScope scope(instanceId);
        auto subPipelineContext = DynamicCast<NG::PipelineContext>(container->GetPipelineContext());
        CHECK_NULL_RETURN(subPipelineContext, false);
        auto overlayManager = subPipelineContext->GetOverlayManager();
        CHECK_NULL_RETURN(overlayManager, false);
        return overlayManager->RemoveOverlayInSubwindow();
    }

    // Remove overlay through SubwindowManager if subwindow unfocused.
    auto subwindow = SubwindowManager::GetInstance()->GetSubwindow(instanceId);
    if (subwindow) {
        if (subwindow->GetShown()) {
            auto overlayManager = subwindow->GetOverlayManager();
            CHECK_NULL_RETURN(overlayManager, false);
            return overlayManager->RemoveOverlayInSubwindow();
        }
    }

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
    if (!container->UpdateState(Frontend::State::ON_SHOW)) {
        return;
    }
    taskExecutor->PostTask(
        [container]() {
            // When it is subContainer, no need call the OnShow,
            auto front = container->GetFrontend();
            if (front && !container->IsSubContainer()) {
                front->UpdateState(Frontend::State::ON_SHOW);
                front->OnShow();
            }
            auto pipelineBase = container->GetPipelineContext();
            CHECK_NULL_VOID(pipelineBase);
            pipelineBase->OnShow();
            pipelineBase->SetForegroundCalled(true);
        },
        TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGOnShow");
}

void AceContainerSG::OnHide(int32_t instanceId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_VOID(container);

    ContainerScope scope(instanceId);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    if (!container->UpdateState(Frontend::State::ON_HIDE)) {
        return;
    }

    taskExecutor->PostTask(
        [container]() {
            auto front = container->GetFrontend();
            if (front && !container->IsSubContainer()) {
                front->UpdateState(Frontend::State::ON_HIDE);
                front->OnHide();
            }
            container->TriggerGarbageCollection();
            auto pipelineContext = container->GetPipelineContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->OnHide();
        },
        TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGOnHide");
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
            if (front && !container->IsSubContainer()) {
                front->UpdateState(Frontend::State::ON_ACTIVE);
                front->OnActive();
            }
            auto pipelineContext = container->GetPipelineContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->WindowFocus(true);
        },
        TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGOnActive");
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
            if (front && !container->IsSubContainer()) {
                front->UpdateState(Frontend::State::ON_INACTIVE);
                front->OnInactive();
            }
            auto pipelineContext = container->GetPipelineContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->WindowFocus(false);
        },
        TaskExecutor::TaskType::UI, "ArkUI-XAceContainerSGOnInactive");
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
    CHECK_NULL_VOID(aceView_);
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
        taskExecutor->PostSyncTask([] { LOGI("Wait UI thread..."); }, TaskExecutor::TaskType::UI,
            "ArkUI-XAceContainerSGDestroyContainerWaitUI");
        taskExecutor->PostSyncTask([] { LOGI("Wait JS thread..."); }, TaskExecutor::TaskType::JS,
            "ArkUI-XAceContainerSGDestroyContainerWaitJS");
    }
    container->DestroyView(); // Stop all threads(ui,gpu,io) for current ability.
    auto removeContainerTask = [instanceId, destroyCallback] {
        LOGI("Remove on Platform thread...");
        JniRegistry::ReleaseInstance(instanceId);
        EngineHelper::RemoveEngine(instanceId);
        AceEngine::Get().RemoveContainer(instanceId);
        if (destroyCallback) {
            destroyCallback();
        }
    };
    if (container->GetSettings().usePlatformAsUIThread) {
        removeContainerTask();
    } else {
        taskExecutor->PostTask(
            removeContainerTask, TaskExecutor::TaskType::PLATFORM, "ArkUI-XAceContainerSGDestroyContainer");
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

bool AceContainerSG::RunPage(
    int32_t instanceId, int32_t pageId, const std::string& content, const std::string& params, bool isNamedRouter)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_RETURN(container, false);
    ContainerScope scope(instanceId);
    auto front = container->GetFrontend();
    CHECK_NULL_RETURN(front, false);
    LOGI("RunPage content=[%{private}s]", content.c_str());
    if (isNamedRouter) {
        front->RunPageByNamedRouter(content, params);
    } else {
        front->RunPage(content, params);
    }
    return true;
}

void AceContainerSG::SetResPaths(
    const std::vector<std::string>& hapResPath, const std::string& sysResPath, const ColorMode& colorMode)
{
    LOGI("SetResPaths, Use hap path to load resource");
    resourceInfo_.SetHapPath(std::accumulate(hapResPath.begin(), hapResPath.end(), std::string(),
        [](const std::string& acc, const std::string& element) { return acc + (acc.empty() ? "" : ":") + element; }));
    // use package path to load system resource.
    auto sysFisrtPos = sysResPath.find_last_of('/');
    auto sysResourcePath = sysResPath.substr(0, sysFisrtPos);
    resourceInfo_.SetPackagePath(sysResourcePath);

    auto themeId = colorMode == ColorMode::LIGHT ? THEME_ID_LIGHT : THEME_ID_DARK;
    resourceInfo_.SetThemeId(themeId);
    colorScheme_ = colorMode == ColorMode::LIGHT ? ColorScheme::SCHEME_LIGHT : ColorScheme::SCHEME_DARK;
}

void AceContainerSG::SetLocalStorage(NativeReference* storage, NativeReference* context)
{
    ContainerScope scope(instanceId_);
    taskExecutor_->PostTask(
        [frontend = WeakPtr<Frontend>(frontend_), storage, context, id = instanceId_] {
            auto sp = frontend.Upgrade();
            CHECK_NULL_VOID(sp);
#ifdef NG_BUILD
            auto declarativeFrontend = AceType::DynamicCast<DeclarativeFrontendNG>(sp);
#else
            auto declarativeFrontend = AceType::DynamicCast<DeclarativeFrontend>(sp);
#endif
            CHECK_NULL_VOID(declarativeFrontend);
            auto jsEngine = declarativeFrontend->GetJsEngine();
            CHECK_NULL_VOID(jsEngine);
            if (context) {
                jsEngine->SetContext(id, context);
            }
            if (storage) {
                jsEngine->SetLocalStorage(id, storage);
            }
        },
        TaskExecutor::TaskType::JS, "ArkUI-XAceContainerSGSetLocalStorage");
}

bool AceContainerSG::MaybeRelease()
{
    CHECK_NULL_RETURN(taskExecutor_, true);
    if (taskExecutor_->WillRunOnCurrentThread(TaskExecutor::TaskType::PLATFORM)) {
        LOGI("Destroy AceContainer on PLATFORM thread.");
        return true;
    } else {
        LOGI("Post Destroy AceContainer Task to PLATFORM thread.");
        return !taskExecutor_->PostTask(
            [this] { delete this; }, TaskExecutor::TaskType::PLATFORM, "ArkUI-XAceContainerSGMaybeRelease");
    }
}

void AceContainerSG::SetUIWindow(int32_t instanceId, sptr<OHOS::Rosen::Window> uiWindow)
{
    CHECK_NULL_VOID(uiWindow);
    auto container = AceType::DynamicCast<AceContainerSG>(AceEngine::Get().GetContainer(instanceId));
    CHECK_NULL_VOID(container);
    container->SetUIWindowInner(uiWindow);
}

sptr<OHOS::Rosen::Window> AceContainerSG::GetUIWindow(int32_t instanceId)
{
    auto container = AceType::DynamicCast<AceContainerSG>(AceEngine::Get().GetContainer(instanceId));
    CHECK_NULL_RETURN(container, nullptr);
    return container->GetUIWindowInner();
}

void AceContainerSG::SetUIWindowInner(sptr<OHOS::Rosen::Window> uiWindow)
{
    uiWindow_ = uiWindow;
}

sptr<OHOS::Rosen::Window> AceContainerSG::GetUIWindowInner() const
{
    return uiWindow_;
}

void AceContainerSG::InitializeSubContainer(int32_t parentContainerId)
{
    auto parentContainer = AceEngine::Get().GetContainer(parentContainerId);
    CHECK_NULL_VOID(parentContainer);
    auto taskExec = parentContainer->GetTaskExecutor();
    taskExecutor_ = AceType::DynamicCast<TaskExecutorImpl>(std::move(taskExec));
    auto parentSettings = parentContainer->GetSettings();
    GetSettings().useUIAsJSThread = parentSettings.useUIAsJSThread;
    GetSettings().usePlatformAsUIThread = parentSettings.usePlatformAsUIThread;
    GetSettings().usingSharedRuntime = parentSettings.usingSharedRuntime;
}

void AceContainerSG::SetCurPointerEvent(const std::shared_ptr<MMI::PointerEvent>& currentEvent)
{
    std::lock_guard<std::mutex> lock(pointerEventMutex_);
    currentPointerEvent_ = currentEvent;
    MMI::PointerEvent::PointerItem pointerItem;
    currentEvent->GetPointerItem(currentEvent->GetPointerId(), pointerItem);
    int32_t originId = pointerItem.GetOriginPointerId();
    currentEvents_[originId] = currentEvent;
    auto callbacksIter = stopDragCallbackMap_.begin();
    while (callbacksIter != stopDragCallbackMap_.end()) {
        auto pointerId = callbacksIter->first;
        MMI::PointerEvent::PointerItem pointerItem;
        bool hasPointerItem = currentEvent->GetPointerItem(pointerId, pointerItem);
        bool isPressed = hasPointerItem ? pointerItem.IsPressed() : false;
        if (!hasPointerItem || !isPressed) {
            for (const auto& callback : callbacksIter->second) {
                if (callback) {
                    callback();
                }
            }
            callbacksIter = stopDragCallbackMap_.erase(callbacksIter);
        } else {
            ++callbacksIter;
        }
    }
}

bool AceContainerSG::GetCurPointerEventInfo(DragPointerEvent& dragPointerEvent, StopDragCallback&& stopDragCallback)
{
    std::lock_guard<std::mutex> lock(pointerEventMutex_);
    CHECK_NULL_RETURN(currentPointerEvent_, false);
    MMI::PointerEvent::PointerItem pointerItem;
    if (!currentPointerEvent_->GetPointerItem(dragPointerEvent.pointerId, pointerItem) || !pointerItem.IsPressed()) {
        return false;
    }
    dragPointerEvent.sourceType = currentPointerEvent_->GetSourceType();
    dragPointerEvent.displayX = pointerItem.GetDisplayX();
    dragPointerEvent.displayY = pointerItem.GetDisplayY();
    dragPointerEvent.sourceTool = static_cast<SourceTool>(pointerItem.GetToolType());
    RegisterStopDragCallback(dragPointerEvent.pointerId, std::move(stopDragCallback));
    return true;
}

bool AceContainerSG::GetLastMovingPointerPosition(DragPointerEvent& dragPointerEvent)
{
    std::lock_guard<std::mutex> lock(pointerEventMutex_);
    auto iter = currentEvents_.find(dragPointerEvent.originId);
    if (iter == currentEvents_.end()) {
        return false;
    }
    MMI::PointerEvent::PointerItem pointerItem;
    auto currentPointerEvent = iter->second;
    CHECK_NULL_RETURN(currentPointerEvent, false);
    if (!currentPointerEvent->GetPointerItem(currentPointerEvent->GetPointerId(), pointerItem) ||
        !pointerItem.IsPressed()) {
        return false;
    }
    dragPointerEvent.displayX = pointerItem.GetDisplayX();
    dragPointerEvent.displayY = pointerItem.GetDisplayY();
    return true;
}

void AceContainerSG::RegisterStopDragCallback(int32_t pointerId, StopDragCallback&& stopDragCallback)
{
    auto iter = stopDragCallbackMap_.find(pointerId);
    if (iter != stopDragCallbackMap_.end()) {
        iter->second.emplace_back(std::move(stopDragCallback));
    } else {
        std::list<StopDragCallback> list;
        list.emplace_back(std::move(stopDragCallback));
        stopDragCallbackMap_.emplace(pointerId, list);
    }
}
} // namespace OHOS::Ace::Platform
