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

#include "adapter/android/entrance/java/jni/ace_container.h"

#include "flutter/fml/platform/android/jni_util.h"

#ifdef NG_BUILD
#include "ace_shell/shell/common/window_manager.h"
#else
#include "flutter/lib/ui/ui_dart_state.h"
#endif

#include "adapter/android/entrance/java/jni/ace_application_info_impl.h"
#include "adapter/android/entrance/java/jni/apk_asset_provider.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/ace_trace.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/resource/shared_image_manager.h"
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
#include "core/components/theme/theme_manager.h"
#include "core/pipeline/base/element.h"
#ifdef NG_BUILD
#include "core/pipeline_ng/pipeline_context.h"
#else
#include "core/pipeline/pipeline_context.h"
#endif
#include "frameworks/bridge/card_frontend/card_frontend.h"
#include "frameworks/bridge/common/utils/engine_helper.h"
#ifdef NG_BUILD
#include "frameworks/bridge/declarative_frontend/ng/declarative_frontend_ng.h"
#else
#include "frameworks/bridge/declarative_frontend/declarative_frontend.h"
#endif
#include "frameworks/bridge/js_frontend/engine/common/js_engine_loader.h"
#include "frameworks/bridge/js_frontend/js_frontend.h"

#ifndef WEARABLE_PRODUCT
#include "core/event/multimodal/fake_multimodal_subscriber.h"
#endif

namespace OHOS::Ace::Platform {

AceContainer::AceContainer(jint instanceId, FrontendType type, jobject callback)
    : messageBridge_(AceType::MakeRefPtr<PlatformBridge>()), type_(type), instanceId_(instanceId)
{
    ACE_DCHECK(callback);
#ifdef NG_BUILD
    LOGD("AceContainer created use new pipeline");
    SetUseNewPipeline();
#endif
    auto flutterTaskExecutor = Referenced::MakeRefPtr<FlutterTaskExecutor>();
    flutterTaskExecutor->InitPlatformThread();
    // no need to create JS thread for DELCARATIVE_JS
    if (type_ != FrontendType::DECLARATIVE_JS) {
        flutterTaskExecutor->InitJsThread();
    }
    taskExecutor_ = flutterTaskExecutor;
    CHECK_RUN_ON(PLATFORM);

    platformEventCallback_ = std::make_unique<JavaEventCallback>(callback);
    if (!platformEventCallback_->Initialize()) {
        LOGE("failed to initialize the callback");
        platformEventCallback_.reset();
        return;
    }
}

void AceContainer::Initialize()
{
    // For Declarative_js frontend use UI as JS thread, so initializeFrontend after UI thread's creation
    ContainerScope scope(instanceId_);
    if (type_ != FrontendType::DECLARATIVE_JS) {
        InitializeFrontend(isArk_);
    }
}

void AceContainer::Destroy()
{
    if (!pipelineContext_) {
        LOGE("no context found in %{private}d container", instanceId_);
        return;
    }

    if (!taskExecutor_) {
        LOGE("no taskExecutor found in %{private}d container", instanceId_);
        return;
    }

    ContainerScope scope(instanceId_);
    // 1. Destroy Pipeline on UI Thread
    auto weak = AceType::WeakClaim(AceType::RawPtr(pipelineContext_));
    taskExecutor_->PostTask(
        [weak, taskExecutor = taskExecutor_]() {
            auto context = weak.Upgrade();
            if (context == nullptr) {
                LOGE("context is null");
                return;
            }
            context->Destroy();
        },
        TaskExecutor::TaskType::UI);
    // 2.Destroy Frontend on JS Thread
    RefPtr<Frontend> frontend;
    frontend_.Swap(frontend);
    if (frontend) {
        taskExecutor_->PostTask(
            [frontend]() {
                frontend->UpdateState(Frontend::State::ON_DESTROY);
                frontend->Destroy();
            },
            TaskExecutor::TaskType::JS);
    }

    // 3. Clear the data of this container
    screenOnEvents_.clear();
    screenOffEvents_.clear();
    sharedImageManager_.Reset();
    messageBridge_.Reset();
    resRegister_.Reset();
    assetManager_.Reset();
    pipelineContext_.Reset();
    aceView_ = nullptr;
    EngineHelper::RemoveEngine(instanceId_);
}

void AceContainer::InitializeFrontend(bool isArkApp)
{
    if (type_ == FrontendType::JS) {
#ifdef NG_BUILD
        LOGE("NG veriosn not support js frontend yet!");
#else
        frontend_ = Frontend::Create();
        auto jsFrontend = AceType::DynamicCast<JsFrontend>(frontend_);
        auto& loader = Framework::JsEngineLoader::Get(nullptr);
        auto jsEngine = loader.CreateJsEngine(instanceId_);
        jsFrontend->SetJsEngine(jsEngine);
        EngineHelper::AddEngine(instanceId_, jsEngine);
        jsFrontend->SetNeedDebugBreakPoint(AceApplicationInfo::GetInstance().IsNeedDebugBreakPoint());
        jsFrontend->SetDebugVersion(AceApplicationInfo::GetInstance().IsDebugVersion());
#endif
    } else if (type_ == FrontendType::JS_CARD) {
#ifdef NG_BUILD
        LOGE("NG veriosn not support js frontend yet!");
#else
        AceApplicationInfo::GetInstance().SetCardType();
        frontend_ = AceType::MakeRefPtr<CardFrontend>();
#endif
    } else if (type_ == FrontendType::DECLARATIVE_JS) {
#ifdef NG_BUILD
        frontend_ = AceType::MakeRefPtr<DeclarativeFrontendNG>();
        auto declarativeFrontend = AceType::DynamicCast<DeclarativeFrontendNG>(frontend_);
#else
        frontend_ = AceType::MakeRefPtr<DeclarativeFrontend>();
        auto declarativeFrontend = AceType::DynamicCast<DeclarativeFrontend>(frontend_);
#endif
        auto& loader = Framework::JsEngineLoader::GetDeclarative(nullptr);
        auto jsEngine = loader.CreateJsEngine(instanceId_);
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
}

void AceContainer::InitializeCallback()
{
    ACE_FUNCTION_TRACE();
    ACE_DCHECK(aceView_ && taskExecutor_ && pipelineContext_);
    auto weak = AceType::WeakClaim(AceType::RawPtr(pipelineContext_));
    auto instanceId = aceView_->GetInstanceId();
    auto&& touchEventCallback = [weak, instanceId](const TouchEvent& event, const std::function<void()>& markProcess) {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return;
        }

        ContainerScope scope(instanceId);
        auto bombId = GetMilliseconds();
        AceEngine::Get().BuriedBomb(instanceId, bombId);
        AceEngine::Get().DefusingBomb(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, event]() {
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null");
                    return;
                }
                context->OnTouchEvent(event);
                context->NotifyDispatchTouchEventDismiss(event);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterTouchEventCallback(touchEventCallback);

    auto&& keyEventCallback = [weak, instanceId](const KeyEvent& event) {
        bool result = false;
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return result;
        }

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
        if (context == nullptr) {
            LOGE("context is null");
            return;
        }

        ContainerScope scope(instanceId);
        auto bombId = GetMilliseconds();
        AceEngine::Get().BuriedBomb(instanceId, bombId);
        AceEngine::Get().DefusingBomb(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, event]() {
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null");
                    return;
                }
                context->OnMouseEvent(event);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterMouseEventCallback(mouseEventCallback);

    auto&& rotationEventCallback = [weak, instanceId](const RotationEvent& event) {
        bool result = false;
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return result;
        }
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostSyncTask(
            [context, event, &result]() { result = context->OnRotationEvent(event); }, TaskExecutor::TaskType::UI);
        return result;
    };
    aceView_->RegisterRotationEventCallback(rotationEventCallback);

    auto&& viewChangeCallback = [weak, instanceId](int32_t width, int32_t height, WindowSizeChangeReason reason) {
        ACE_SCOPED_TRACE("ViewChangeCallback(%d, %d)", width, height);
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return;
        }
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, width, height, reason]() {
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null ");
                    return;
                }
                context->OnSurfaceChanged(width, height, reason);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterViewChangeCallback(viewChangeCallback);

    auto&& densityChangeCallback = [weak, instanceId](double density) {
        ACE_SCOPED_TRACE("DensityChangeCallback(%lf)", density);
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return;
        }
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, density]() {
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null");
                    return;
                }
                context->OnSurfaceDensityChanged(density);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterDensityChangeCallback(densityChangeCallback);

    auto&& systemBarHeightChangeCallback = [weak, instanceId](double statusBar, double navigationBar) {
        ACE_SCOPED_TRACE("SytemBarHeighChangeCallback(%lf, %lf)", statusBar, navigationBar);
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return;
        }
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, statusBar, navigationBar]() {
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null");
                    return;
                }
                context->OnSystemBarHeightChanged(statusBar, navigationBar);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterSystemBarHeightChangeCallback(systemBarHeightChangeCallback);

    auto&& surfaceDestroyCallback = [weak, instanceId]() {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is nullptr");
            return;
        }

        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak]() {
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is nullptr");
                    return;
                }
                context->OnSurfaceDestroyed();
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterSurfaceDestroyCallback(surfaceDestroyCallback);

    auto&& idleCallback = [weak, instanceId](int64_t deadline) {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return;
        }

        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, deadline]() {
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null");
                    return;
                }
                context->OnIdle(deadline);
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterIdleCallback(idleCallback);

    auto&& preDrawCallback = [weak, instanceId]() {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null while trying to post task to notifyonPreDraw");
            return;
        }
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak]() {
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null while trying to notifyonPreDraw");
                    return;
                }
                context->NotifyOnPreDraw();
            },
            TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterPreDrawCallback(preDrawCallback);

    auto&& requestFrame = [weak, instanceId]() {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null while trying to post task to requestframe");
            return;
        }
        ContainerScope scope(instanceId);
        context->RequestFrame();
    };
    aceView_->RegisterRequestFrameCallback(requestFrame);
}

void AceContainer::Dispatch(
    const std::string& group, std::vector<uint8_t>&& data, int32_t id, bool replyToComponent) const
{}

void AceContainer::DispatchSync(
    const std::string& group, std::vector<uint8_t>&& data, uint8_t** resData, long& position) const
{}

void AceContainer::DispatchPluginError(int32_t callbackId, int32_t errorCode, std::string&& errorMessage) const
{
    auto front = GetFrontend();
    if (!front) {
        LOGE("the front jni is null");
        return;
    }

    auto weakFront = AceType::WeakClaim(AceType::RawPtr(front));
    ContainerScope scope(instanceId_);
    taskExecutor_->PostTask(
        [weakFront, callbackId, errorCode, errorMessage = std::move(errorMessage)]() mutable {
            auto front = weakFront.Upgrade();
            if (front == nullptr) {
                LOGE("front is null");
                return;
            }
            front->TransferJsPluginGetError(callbackId, errorCode, std::move(errorMessage));
        },
        TaskExecutor::TaskType::BACKGROUND);
}

bool AceContainer::Dump(const std::vector<std::string>& params)
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

void AceContainer::AttachView(
    std::unique_ptr<Window> window, AceView* view, double density, int32_t width, int32_t height)
{
    aceView_ = view;
    auto instanceId = aceView_->GetInstanceId();
#ifdef NG_BUILD
    auto state = flutter::ace::WindowManager::GetWindow(instanceId);
    CHECK_NULL_VOID(state);
#else
    auto state = flutter::UIDartState::Current()->GetStateById(instanceId);
    ACE_DCHECK(state != nullptr);
#endif
    auto flutterTaskExecutor = AceType::DynamicCast<FlutterTaskExecutor>(taskExecutor_);
    flutterTaskExecutor->InitOtherThreads(state->GetTaskRunners());

    ContainerScope scope(instanceId);
    if (type_ == FrontendType::DECLARATIVE_JS) {
        // for declarative js frontend display ui in js thread
        flutterTaskExecutor->InitJsThread(false);
        LOGD(" initialize frontend isArk_= %d", isArk_);
        InitializeFrontend(isArk_);
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

    } else if (type_ == FrontendType::JS_CARD) {
        aceView_->SetCreateTime(createTime_);
    }

    resRegister_ = aceView_->GetPlatformResRegister();
#ifdef NG_BUILD
    LOGI("New pipeline version creating...");
    pipelineContext_ = AceType::MakeRefPtr<NG::PipelineContext>(
        std::move(window), taskExecutor_, assetManager_, resRegister_, frontend_, instanceId);
#else
    auto pipelineContext = AceType::MakeRefPtr<PipelineContext>(
        std::move(window), taskExecutor_, assetManager_, resRegister_, frontend_, instanceId);
    pipelineContext_ = pipelineContext;
#endif

    pipelineContext_->SetRootSize(density, width, height);
    pipelineContext_->SetTextFieldManager(AceType::MakeRefPtr<TextFieldManager>());
    pipelineContext_->SetIsRightToLeft(AceApplicationInfo::GetInstance().IsRightToLeft());
#ifndef NG_BUILD
    pipelineContext->SetMessageBridge(messageBridge_);
    pipelineContext->SetWindowModal(windowModal_);
    pipelineContext->SetModalHeight(modalHeight_);
    pipelineContext->SetModalColor(modalColor_);
    pipelineContext->SetDrawDelegate(aceView_->GetDrawDelegate());
    pipelineContext->SetPhotoCachePath(aceView_->GetCachePath());
#endif
    pipelineContext_->SetFontScale(resourceInfo_.GetResourceConfiguration().GetFontRatio());
    pipelineContext_->SetIsJsCard(type_ == FrontendType::JS_CARD);

    if (resRegister_) {
        resRegister_->SetPipelineContext(pipelineContext_);
    }
    InitializeCallback();

    auto&& finishEventHandler = [weak = WeakClaim(this), instanceId] {
        auto container = weak.Upgrade();
        if (!container) {
            LOGE("FinishEventHandler container is null");
            return;
        }
        auto context = container->GetPipelineContext();
        if (!context) {
            LOGE("FinishEventHandler context is null");
            return;
        }
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak = WeakPtr<AceContainer>(container)] {
                auto container = weak.Upgrade();
                if (!container) {
                    LOGE("Finish Task, container is null");
                    return;
                }
                container->OnFinish();
            },
            TaskExecutor::TaskType::PLATFORM);
    };
    pipelineContext_->SetFinishEventHandler(finishEventHandler);

    auto&& setStatusBarEventHandler = [weak = WeakClaim(this), instanceId](const Color& color) {
        auto container = weak.Upgrade();
        if (!container) {
            LOGE("StatusBarEventHandler container is null");
            return;
        }
        auto context = container->GetPipelineContext();
        if (!context) {
            LOGE("StatusBarEventHandler context is null");
            return;
        }
        ContainerScope scope(instanceId);
        context->GetTaskExecutor()->PostTask(
            [weak, color = color.GetValue()]() {
                auto container = weak.Upgrade();
                if (!container) {
                    LOGE("StatusBarEventHandler container is null");
                    return;
                }
                if (container->platformEventCallback_) {
                    container->platformEventCallback_->OnStatusBarBgColorChanged(color);
                }
            },
            TaskExecutor::TaskType::PLATFORM);
    };
    pipelineContext_->SetStatusBarEventHandler(setStatusBarEventHandler);

    pipelineContext_->SetGetViewScaleCallback([weak = WeakClaim(this), instanceId](float& scaleX, float& scaleY) {
        auto container = weak.Upgrade();
        if (!container) {
            LOGE("getViewScale container is null");
            return false;
        }
        ContainerScope scope(instanceId);
        if (container->aceView_) {
            return container->aceView_->GetScale(scaleX, scaleY);
        }
        return false;
    });

    InitThemeManager();

    auto weakContext = AceType::WeakClaim(AceType::RawPtr(pipelineContext_));
    taskExecutor_->PostTask(
        [weakContext]() {
            auto context = weakContext.Upgrade();
            if (context == nullptr) {
                LOGE("context is null");
                return;
            }
            context->SetupRootElement();
        },
        TaskExecutor::TaskType::UI);
    aceView_->Launch();

    frontend_->AttachPipelineContext(pipelineContext_);
    auto cardFrontend = AceType::DynamicCast<CardFrontend>(frontend_);
    if (cardFrontend) {
        cardFrontend->SetDensity(static_cast<double>(density));
        taskExecutor_->PostTask(
            [weakContext, width, height]() {
                auto context = weakContext.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null");
                    return;
                }
                context->OnSurfaceChanged(width, height);
            },
            TaskExecutor::TaskType::UI);
    }

    AceEngine::Get().RegisterToWatchDog(instanceId, taskExecutor_);
}

void AceContainer::UpdateThemeConfig(const ResourceConfiguration& config)
{
    if (!pipelineContext_) {
        return;
    }
    ContainerScope scope(instanceId_);
    auto themeManager = pipelineContext_->GetThemeManager();
    if (!themeManager) {
        return;
    }
    themeManager->UpdateConfig(config);
}

void AceContainer::UpdateResourceConfiguration(const std::string& jsonStr)
{
    uint32_t updateFlags = 0;
    auto resConfig = resourceInfo_.GetResourceConfiguration();
    ContainerScope scope(instanceId_);
    if (!resConfig.UpdateFromJsonString(jsonStr, updateFlags) || !updateFlags) {
        return;
    }
    resourceInfo_.SetResourceConfiguration(resConfig);
    if (!ResourceConfiguration::TestFlag(updateFlags, ResourceConfiguration::COLOR_MODE_UPDATED_FLAG)) {
        SystemProperties::SetColorMode(resConfig.GetColorMode());
        if (frontend_) {
            frontend_->SetColorMode(resConfig.GetColorMode());
        }
    }

    if (!pipelineContext_) {
        return;
    }

    auto themeManager = pipelineContext_->GetThemeManager();
    if (!themeManager) {
        return;
    }
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
            context->RefreshRootBgColor();
            context->UpdateFontWeightScale();
            context->SetFontScale(config.GetFontRatio());
        },
        TaskExecutor::TaskType::UI);
    if (frontend_) {
        frontend_->RebuildAllPages();
    }
}

void AceContainer::UpdateColorMode(ColorMode colorMode)
{
    auto resConfig = resourceInfo_.GetResourceConfiguration();
    ContainerScope scope(instanceId_);
    SystemProperties::SetColorMode(colorMode);
    if (resConfig.GetColorMode() == colorMode) {
        return;
    }
    resConfig.SetColorMode(colorMode);
    resourceInfo_.SetResourceConfiguration(resConfig);
    if (!pipelineContext_) {
        return;
    }
    auto themeManager = pipelineContext_->GetThemeManager();
    if (!themeManager) {
        return;
    }
    themeManager->UpdateConfig(resConfig);
    taskExecutor_->PostTask(
        [weakThemeManager = WeakPtr<ThemeManager>(themeManager), colorScheme = colorScheme_,
            weakContext = WeakPtr<PipelineBase>(pipelineContext_)]() {
            auto themeManager = weakThemeManager.Upgrade();
            auto context = weakContext.Upgrade();
            if (!themeManager || !context) {
                return;
            }
            themeManager->LoadResourceThemes();
            themeManager->ParseSystemTheme();
            themeManager->SetColorScheme(colorScheme);
            context->RefreshRootBgColor();
        },
        TaskExecutor::TaskType::UI);
    if (frontend_) {
        frontend_->SetColorMode(colorMode);
        frontend_->RebuildAllPages();
    }
}

void AceContainer::SetThemeResourceInfo(const std::string& path, int32_t themeId)
{
    ACE_FUNCTION_TRACE();
    ContainerScope scope(instanceId_);
    resourceInfo_.SetThemeId(themeId);
    resourceInfo_.SetPackagePath(path);
    ThemeConstants::InitDeviceType();
    themeManager_ = AceType::MakeRefPtr<ThemeManager>();
    if (themeManager_) {
        // init resource, load theme map , do not parse yet
        themeManager_->InitResource(resourceInfo_);
        themeManager_->LoadSystemTheme(resourceInfo_.GetThemeId());
        auto weakTheme = AceType::WeakClaim(AceType::RawPtr(themeManager_));
        themeLatch_ = std::make_shared<fml::ManualResetWaitableEvent>();
        taskExecutor_->PostTask(
            [weakTheme, latch = themeLatch_]() {
                ACE_SCOPED_TRACE("ParseSystemTheme");
                auto themeManager = weakTheme.Upgrade();
                if (themeManager == nullptr) {
                    LOGE("themeManager is null");
                    latch->Signal();
                    return;
                }
                themeManager->ParseSystemTheme();
                latch->Signal();
            },
            TaskExecutor::TaskType::BACKGROUND);
    }
}

void AceContainer::InitThemeManager()
{
    LOGI("Init theme manager");
    ContainerScope scope(instanceId_);
    // only init global resource here
    if (pipelineContext_ && !pipelineContext_->GetThemeManager() && themeManager_) {
        pipelineContext_->SetThemeManager(themeManager_);
        auto weakTheme = AceType::WeakClaim(AceType::RawPtr(themeManager_));
        auto weakAsset = AceType::WeakClaim(AceType::RawPtr(assetManager_));
        taskExecutor_->PostTask(
            [weakTheme, weakAsset, colorScheme = colorScheme_, aceView = aceView_, latch = themeLatch_]() {
                auto themeManager = weakTheme.Upgrade();
                if (themeManager == nullptr || aceView == nullptr) {
                    LOGE("themeManager or aceView is null");
                    return;
                }
                latch->Wait();
                themeManager->SetColorScheme(colorScheme);
                themeManager->LoadCustomTheme(weakAsset.Upgrade());
                // get background color
                aceView->SetBackgroundColor(themeManager->GetBackgroundColor());
            },
            TaskExecutor::TaskType::UI);
    }
}

void AceContainer::SetSessionID(const std::string& sessionID)
{
    if (aceView_) {
        aceView_->SetSessionID(sessionID);
    }
}

void AceContainer::SetHostClassName(const std::string& name)
{
    hostClassName_ = name;
}

void AceContainer::SetInstanceName(const std::string& name)
{
    instanceName_ = name;
}

void AceContainer::TriggerGarbageCollection()
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

void AceContainer::NotifyFontNodes()
{
    ContainerScope scope(instanceId_);
    if (pipelineContext_ && pipelineContext_->GetFontManager()) {
        pipelineContext_->GetFontManager()->NotifyVariationNodes();
    }
}

void AceContainer::NotifyAppStorage(const std::string& key, const std::string& value)
{
    ContainerScope scope(instanceId_);
    if (frontend_) {
        frontend_->NotifyAppStorage(key, value);
    }
}

} // namespace OHOS::Ace::Platform