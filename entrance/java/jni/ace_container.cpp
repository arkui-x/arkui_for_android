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
#include "flutter/lib/ui/ui_dart_state.h"

#include "adapter/android/entrance/java/jni/ace_application_info_impl.h"
#include "adapter/android/entrance/java/jni/apk_asset_provider.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "adapter/android/capability/java/jni/editing/text_input_jni.h"

#include "base/log/ace_trance.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/resource/shared_image_manager.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/common/ace_view.h"
#include "core/common/connect_server_manager.h"
#include "core/common/flutter/flutter_asset_manager.h"
#include "core/common/flutter/flutter_task_executor.h"
#include "core/common/platform_window.h"
#include "core/common/text_field_manager.h"
#include "core/common/thread_checker.h"
#include "core/common/watch_dog.h"
#include "core/common/window.h"
#include "core/components/theme/app_theme.h"
#include "core/components/theme/theme_constants.h"
#include "core/components/theme/theme_manager.h"
#include "core/pipeline/base/element.h"
#include "core/pipeline/pipeline_context.h"
#include "frameworks/bridge/card_frontend/card_frontend.h"
#include "frameworks/bridge/declarative_frontend/declarative_frontend.h"
#include "frameworks/bridge/js_frontend/engine/common/js_engine_loader.h"
#include "frameworks/bridge/js_frontend/js_frontend.h"

#ifndef WEARABLE_PRODUCT
#include "core/event/multimodal/fake_multimodal_subscriber.h"
#endif

namespace OHOS::Ace::Platform {

AceContainer::AceContainer(jint instanceId, FrontendType type, jobject callback)
    : messageBridge_(AceType::MakeRefPtr<PlatformBridge>()), type_(type), instanceId_(instanceId) {

    ACE_DCHECK(callback);
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
        EventReport::SendAppStartException(AppStartExepType::JAVA_EVENT_CALLBACK_INIT_ERR);
        platformEventCallback_.reset();
        return;
    }
}

void AceContainer::Initialize() {
    // For Declarative_js frontend use UI as JS thread, so initializeFrontend after UI thread's creation
    if (type_ != FrontendType::DECLARATIVE_JS) {
        InitializeFrontend(isArk_);
    }
}

void AceContainer::Destroy(){
    if (!pipelineContext_) {
        LOGE("no context found in %{private}d container", instanceId_);
        return;
    }

    if （!taskExecutor_){
        LOGE("no taskExecutor found in %{private}d container", instanceId_);
        return;
    }

    //1. Destroy Pipeline on UI Thread
    auto weak = AceType::WeakClaim(AceType::RawPtr(pipelineContext_));
    taskExecutor_->PostTask(
        [weak, taskExecutor = taskExecutor_]() {
            auto context = weak.Upgrade();
            if (context == nullptr) {
                LOGE("context is null");
                return;
            }
            context->Destory();
        },
        TaskExecutor::TaskType::UI);
    //2.Destroy Frontend on JS Thread
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

    //3. Clear the data of this container
    screenOnEvents_.clear();
    screenOffEvents_.clear();
    sharedImageManager_Reset();
    messageBridge_.Reset();
    resRegister_.Reset();
    assetManager_.Reset();
    pipelineContext_.Reset();
    aceView_ = nullptr;

}

void AceContainer::InitializeFrontend(bool isArkApp) {
    if (type_ == FrontendType::JS) {
        frontend_ = Frontend::Create();
        auto jsFrontend = AceType::DynamicCast<JsFrontend>(frontend_);
        auto& loader = Framework::JsEngineLoader::Get(nullptr);
        jsFrontend->SetJsEngine(loader.CreateJsEngine(instanceId_));
        jsFrontend->SetNeedDebugBreakPoint(AceApplicationInfo::GetInstance().IsNeedDebugBreakPoint());
        jsFrontend->SetDebugVersion(AceApplicationInfo::GetInstance().IsDebugVersion());
    } else if (type_ == FrontendType::JS_CARD){
        AceApplicationInfo::GetInstance().SetCardType();
        frontend_ = AceType::MakeRefPtr<CardFrontend>();
    } else if (type_ == FrontendType::DELCARATIVE_JS) {
        frontend_ = AceType::MakeRefPtr<DelcarativeFrontend>();
        auto declarativeFrontend = AceType::DynamicCast<DeclarativeFrontend>(frontend_)；
        auto& loader = Framework::JsEngineLoader::GetDeclarative(nullptr);
        declarativeFrontend->SetJsEngine(loader.CreateJsEngine(instanceId_));
        declarativeFrontend->SetNeedDebugBreakPoint(AceApplicationInfo::GetInstance().IsNeedDebugBreakPoint());
        declarativeFrontend->SetDebugVersion(AceApplicationInfo::GetInstance().IsDebugVersion());
        if (instanceId_ != abilityId_ && abilityId_ != -1){
            declarativeFrontend->MarkIsSubWindow(true);
        }
    } else {
        LOGE("Frontend Type not supported")l
        EventReport::SendAppStartException(AppStartExcepType::FRONTEND_TYPE_ERR);
        return;
    }

    ACE_DCHECK(frontend_);
    frontend_->Initialize(type_, taskExecutor_);
    if (assetManger_) {
        frontend_->SetAssetManager(assetManager_);
    }
}

void AceContainer::InitializeCallback(){
    ACE_FUNCTION_TRACE();
    ACE_DCHECK(aceView_ && taskExecutor_ && pipelineContext_);
    auto weak = AceType::WeakClaim(ActType::RawPtr(pipelineContext_));
    auto instanceId = aceView_->GetInstanceId();
    auto&& touchEventCallback = [weak, instanceId](const TouchEvnet& event) {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return;
        }
        
        auto bombId = GetMilliseconds();
        AceEngine::Get().BuriedBomb(instanceId,bombId);
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

    auto bombId = GetMilliseconds();
    AceEngine::Get().BuriedBomb(instanceId,bombId);
    AceEngine::Get().DefusingBomb(instanceId);
    context->GetTaskExecutor()->PostSyncTask(
        [context, event, &result]() {
            result = context->OnKeyEvent(event);}, TaskExecutor::TaskType::UI);
    return result;
    };
    aceView_->RegisterKeyEventCallback(keyEventCallback);
    
    auto&& mouseEventCallback = [weak, instanceId](const MouseEvent& event) {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return ;
        }
        auto bombId = GetMilliseconds();
        AceEngine::Get().BuriedBomb(instanceId,bombId);
        AceEngine::Get().DefusingBomb(instanceId)；
        context->GetTaskExecutor()->PostTask(
            [weak, event]() {
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null");
                    return;
                }
                context->onMouseEvent(event);
            },
            TaskExecutor::TaskType::UI);
        
    };
    aceView_->RegisterMouseEventCallback(mouseEventCallback);

    auto&& rotationEventCallback = [weak](const RotationEvent& event) {
        bool result = false;
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return result;
        }
        context->GetTaskExecutor()->PostSyncTask(
            [context, event, &result](){
                result = context->OnRotationEvent(event);}, TaskExecutor::TaskType::UI);
        return result;    
    };
    aceView_->RegisterRotationEventCallback(rotationEventCallback);

    auto&& cardViewPositionCallback = [weak](int id, float offsetX, float offsetY) {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null"); 
            return;
        }
        context->GetTaskExecutor()->PostSyncTask(
            [context, id, offsetX, offsetY]() {
                context->SetCardViewPosition(id, offsetX, offsetY);}， TaskExecutor::TaskType::UI);
    };
    aceview_->RegisterCardViewPositionCallback(cardViewPositionCallback);
    
    auto&& cardViewParamsCallback = [weak](const std::string& key, bool focus) {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return;
        }
        context->GetTaskExecutor()->PostSyncTask(
            [context, key, focus]() {
                context->SetCardViewAccessbilityParams(key, focus);}, TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterCardViewAccessiblityParamsCallback(cardViewParamsCallback);

    auto&& viewChangeCallback = [weak](int32_t width, int32_t height) {
        ACE_SCOPE_TRACE("ViewChangeCallback(%d,%d)", width, height);
        auto context = weak.Upgrade();
        if(context == nullptr) {
            LOGE("context is null");
            return;
        }
        context->GetTaskExecutor()->PostTask(
            [weak, width, height]() {
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null ");
                    return;
                }
                context->OnSurfaceChanged(width, height);
            }, TaskExecutor::TaskType::UI);
    }；
    aceView_->RegisterViewChangeCallback(viewChangeCallback);
    
    auto&& densityChangeCallback = [weak](double denisty) {
        ACE_SCOPED_TRACE("DensityChangeCallback(%lf)", denisty);
        if (context == nullptr) {
            LOGE("context is null");
            return;
        }
        context->GetTaskExecutor()->PostTask(
            [weak, density](){
                auto context = weak.Upgrade();
                if (context == nullptr){
                    LOGE("context is null");
                    return;
                }
                context->OnSurfaceDensityChanged(denisty);
            },TaskExecutor::TaskType::UI);
        
    };
    aceView_->RegisterDensityChangeCallback(densityChangeCallback);

    auto&& sytemBarHeightChangeCallback = [weak](double statusBar, double navigationBar) {
        ACE_SCOPED_TRACE("SytemBarHeighChangeCallback(%lf, %lf)", statusBar, navigationBar);
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null");
            return;
        }
        context->GetTaskExecutor()->PostTask(
            [weak,statusBar, navigationBar](){
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null");
                    return;
                }
                context->OnSystemBarHeightChanged(statusBar, navigationBar);
            },TaskExecutor::TaskType::UI);
    };
    aceView_->RegisterSystemBarHeightChangeCallback(systemBarHeightChangeCallback);

    auto&& surfaceDestoryCallback = [weak]() {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is nullptr");
            return;
        }
        context->GetTaskExcecutor()->PostTask(
            [weak](){
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
    
    auto&& idleCallback = [weak](int64_t deadline) {
        auto context = weak.Upgrade();
        if (context == nullptr){
            LOGE("context is null");
            return;
        }
        context->GetTaskExecutor()->PostTask(
            [weak,deadline]() {
                auto context = weak.Upgrade();
                if (context == nullptr) {
                    LOGE("context is null");
                    return;
                }
                context->OnIdle(deadline);
            },
            TaskExecutor::TaskType::UI;
    };
    aceView_->RegisterIdelCallbck(idleCallback);

    auto&& preDrawCallback = [weak]() {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null while trying to post task to notifyonPreDraw");
            return;
        }
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

    auto&& requestFrame = [weak]() {
        auto context = weak.Upgrade();
        if (context == nullptr) {
            LOGE("context is null while trying to post task to requestframe");
            return;
        }
        context->RequestFrame();
    };
    aceView_->RegisterRequestFrameCallback(requestFrame);
}

void AceContainer::Dispatch(
    const std::string& group, std::vector<unit8_t>&& data, int32_t id, bool replyToComponent) const{

}

void AceContainer::DispatchSync(
    const std::string& group, std::vector<unit8_t>&& data, unit8_t** resData, long& position) const{

}


void AceContainer::DispatchPluginError(int32_t callbackId, int32_t errorCode, std::string&& errorMessage) const {
    auto front = GetFrontend();
    if (!front) {
        LOGE("the front jni is null");
        return;
    }

    auto weakFront = AceType::WeakClaim(AceType::RawPtr(front));
    taskExecutor_->PostTask(
        [weakFront, callbackId, errorCode, errorMessage = std::move(errrorMessage)]() mutable {
            auto front = weakFront.Upgrade();
            if (front == nullptr) {
                LOGE("front is null");
                return;
            }
            front->TransferJsPluginGetError(callbackId, errorCode, std::move(errorMessage));
        },
        TaskExecutor::TaskType::BACKGROUND);
}

bool AceContainer::Dump(const std::vector<std::string>& params) {
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
    std::unique_ptr<Window> window, AceView* view, double density, int32_t width, int32_t height){
    aceView_ = view;
    if (instanceId_ != abilityId_ && abilityId_ != -1){
        taskExecutor_->PostTask([aceView=aceView_](){
            aceView->SetBackgroundColor(Color::TRANSPARENT);
        }, TaskExecutor::TaskType::UI);
    }
    auto instanceId = aceView_->GetInstanceId();
    auto state = flutter::UIDartState::Current()->GetStateById(instanceId);
    ACE_DCHECK(state != nullptr);
    auto flutterTaskExecutor = AceType::DynamicCast<FlutterTaskExecutor>(taskExecutor_);
    flutterTaskExecutor->InitOtherThreads(state->GetTaskRunners());

    if (type_ == FrontendType::DECLARATIVE_JS) {
        // for declarative js frontend display ui in js thread temporarily
        flutterTaskExecutor->InitJsThread(false);
        LOGD(" initialize frontend isArk_= %d", isArk_);
        InitializeFrontend(isArk_);
        auto front = GetFrontend();
        if (front) {
            // SetInstanceName is called in AceContainer::CreateContainer in ace 1.0
            auto jsFront = AceType::DynamicCast<DeclarativeFrontend>(front);
            jsFront->SetInstanceName(GetInstanceName());
            front->UpdateState(Frontend::State::ON_CREATE);
            front->SetJsMessageDispatcher(AceType::Claim(this));
        }

    } else if (type_ == FrontendType::JS_CARD) {
        aceView_->SetCreateTime(createTime_);
    } 

    resRegister_ = aceVeiw_->GetPlatformResRegister();
    pipelineContext_ = AceType::MakeRefPtr<PipelineContext>(
        std::move(window), taskExecutor_, assetManager_, resRegister_, frontend_,instanceId);
    
    pipelineContext_->SetRootSize(density, width, height);
    pipelineContext_->SetTextFieldManager(AceTyp::MakeRefPtr<TextFieldManager>());
    pipelineContext_->SetIsRightToLeft(AceApplicationInfo::GetInstance().IsRightToLeft());
    pipelineContext_->SetMessageBridge(messageBridge_);
    pipelineContext_->SetWindowModal(windowModal_);
    pipelineContext_->SetModalHeight(modalHeight_);
    pipelineContext_->SetModalColor(modalColor_);
    pipelineContext_->SetDrawDelegate(aceView_->GetDrawDelegate());
    pipelineContext_->SetFontScale(resourceInfo_.GetResourceConfiguration().GetFrontRatio());
    pipelineContext_->SetIsJsCard(type_ == FrontendType::JS_CARD);
    pipelineContext_->SetPhotoCachePath(aceView_->GetCachePath());
    pipelineContext_->SetScreenOnCallback([weak = AceType::WeakClaim(this)](std::function<void()>&& func) {
        auto container = weak.Upgrade();
        if (!container) {
            return;
        }
        if (container->screenOnEvents_.empty() && container->screenOffEvents_.empty() && container->aceView_) {
            container->aceView_->RegisterScreenBroadcast();
        }
        container->screenOnEvents_.emplace_back(std::move(func));
    });
    pipelineContext_->SetScreenOffCallback([weak = AceType::WeakClaim(this)](std::function<void()> && func){
        auto container = weak.Upgrade();
        if (!container) {
            return;
        }
        if (container->screenOnEvents_.empty() && container->screenOffEvents_empty() && container->aceView_) {
            container->aceView_->RegisterScreenBroadCast();
        }
        container->screenOffEvents_.emplace_back(std::move(func));
    });
    
    pipelineContext_->SetQueryIfWindowInScreenCallback(
        [weak = AceType::WeakClaim(this), pipelineCtxWp = WeakClaim(RawPtr(pipelineContext_))]() {
            auto container = weak.Upgrade();
            auto pipelineCtx = pipelineCtxWp.Upgrade();
            if (container && container->aceView_ && pipelineCtx) {
                pipelineCtx->SetIsWindowInScreen(container->aceView_->QueryIfWindowInScreen());
            }
        });
    if (resRegister_) {
        resRegister_->SetPipelineContext(pipelineContext_);
    }
    InitializeCallback();

    auto&& actionEventHandler = [weak = WeakClaim(this)](const std::string& action) {
        auto container = weak.Upgrade();
        if (!container) {
            LOGE("ActionEventHandler container is null");
            return;
        }
        auto context = container->GetPipelineContext();
        if (!context) {
            LOGE("ActionEventHandler context is null");
            return;
        }

        context->GetTaskExecutor()->PostTask(
            [weak = WeakPtr<AceContainer>(container), action] {
                auto container = weak.Upgrade();
                if (!container) {
                    LOGE("Finish task, container is null");
                    return;
                }
                container->OnActionEvent(action);
            },
            TaskExecutor::TaskType::PLATFORM);
    };
    pipelineContext_->SetActionEventHandler(actionEventHandler);

    auto&& finishEventHandler = [weak = WeakClaim(this)] {
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

    auto&& setStatusBarEventHandler = [weak = WeakClaim(this)](const Color& color) {
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
    auto requestDataProviderResImpl = [weak = WeakClaim(this)](
                                           const std::string& uriStr) -> std::unique_ptr<DataProviderRes> {
        auto container = weak.Upgrade();
        if (!container) {
            LOGE("requestDataProviderResImpl container is null");
            return nullptr;
        }
        if (!container->platformEventCallback_) {
            LOGE("requestDataProviderResImpl platformEventCallback is null");
            return nullptr;
        }
        return container->platformEventCallback_->OnGetDataProviderRes(uriStr);
    };
    pipelineContext_->SetDataProviderManager(MakeRefPtr<DataProviderManager>(requestDataProviderResImpl));

    auto&& updateWindowBlurRegionHandler = [weak = WeakClaim(this)](const std::vector<std::vector<float>>& rect) {
        auto container = weak.Upgrade();
        if (!container) {
            LOGE("updateWindowBlurRegionHandler container is null");
            return;
        }
        auto context = container->GetPipelineContext();
        if (!context) {
            LOGE("updateWindowBlurRegionHandler context is null");
            return;
        }
        context->GetTaskExecutor()->PostTask(
            [weak, rect](){
                auto container = weak.Upgrade();
                if (!container) {
                    LOGE("updateWindowBlurRegionHandler container is null");
                    return;
                }
        }, 
        TaskExecutor::TaskType::PLATFORM);
    };
    pipelineContext_->SetUpdateWindowBlurRegionHandler(updateWindowBlurRegionHandler);

    auto&& updateWindowBlurDrawOpHandler = [weak = WeakClaim(this)]() {
        auto container = weak.Upgrade();
        if (!container) {
            LOGE("updateWindowBlurDrawOpHandler container is null");
            return;
        }
        auto context = container->GetPipelineContext();
        if (!context) {
            LOGE("updateWindowBlurDrawOpHandler context is null");
            return;
        }
        context->GetTaskExecutor()->PostTask(
            [weak](){
                auto container = weak.Upgrade();
                if (!container) {
                    LOGE("updateWindowBlurDrawOpHandler container is null");
                    return;
                }
                if (container->aceView_) {
                    container_>aceView_->UpdateWindowBlurDrawOp();
                }
        }, 
        TaskExecutor::TaskType::PLATFORM);
    };
    pipelineContext_->SetUpdateWindowBlurDrawOpHandler(updateWindowBlurDrawOpHandler);

    auto&& initDragListener = [weak = WeakClaim(this)] {
        auto container = weak.Upgrade();
        if (!container) {
            LOGE("initDragListener container is null");
            return;
        }
        auto context = container->GetPipelineContext();
        if (!context) {
            LOGE("initDragListener context is null");
            return;
        }

        context->GetTaskExecutor()->PostTask(
            [weak]() {
                auto container = weak.Upgrade();
                if (!container) {
                    LOGE("initDragListener container is null");
                    return;
                }
                if (container->aceView_) {
                    container->aceView_->InitDragListener();
                }
            },
            TaskExecutor::TaskType::PLATFORM);
    };
    
    pipelineContext_->SetInitDragListener(initDragLiistener);



    auto&& startSystemDrag = [weak = WeakClaim(this)](const std::string& str, const RefPtr<PixelMap>& pixmap) {
        auto container = weak.Upgrade();
        if (!container) {
            LOGE("startSystemDrag container is null");
            return;
        }
        auto context = container->GetPipelineContext();
        if (!context) {
            LOGE("startSystemDrag context is null");
            return;
        }

        void* pixelMapManger = pixmap->GetPixelManager();
        int32_t byteCount = pixmap->GetByteCount();

        context->GetTaskExecutor()->PostTask(
            [weak, str,pixelMapManager, byteCount]() {
                auto container = weak.Upgrade();
                if (!container) {
                    LOGE("startSystemDrap container is null");
                    return;
                }
                if (container->aceView_) {
                    container->aceView_->StartSystemDrag(str,pixelMapManager, byteCount);
                }
            },
            TaskExecutor::TaskType::PLATFORM);
    };
    pipelineContext_->SetDragEventHandler(startSystemDrag);

    pipelineContext_->SetGetViewScaleCallback([weak = WeakClaim(this)](float& scaleX, float& scaleY) {
        auto container = weak.Upgrade();
        if (!container) {
            LOGE("getViewScale container is null");
            return false;
        }
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
                context->OnSurfaceChanged(width,height);
            },
            TaskExecutor::TaskType::UI);
    }

    AceEngine::Get().RegisterToWatchDog(instanceId, taskExecutor_);
}

void AceContainer::UpdateThemeConfig(const ResourceConfiguration& coonfig) {

    if (!pipelineContext_) {
        return;
    }
    auto themeManager = pipelineContext_->GetThemeManager();
    if (!themeManager) {
        return;
    }
    themeManager->UpdateConfig(config);
}


void AceContainer::SetActionCallback(jobject callback) {
    actionEventCallback_ = std::make_unique<ActionEventCallback>(callback);
    if (!actionEventCallback_->Initialize()) {
        LOGE("Fail to initialize the action callback");
        EventReport::SendFormException(FormExcepType::ACTION_EVENT_CALLBACK_ERR);
        actionEventCallback_.reset();
    }
}

void AceContainer::UpdateResourceConfiguration(const std::string& jsonStr) {
    uint32_t updateFlags = 0;
    auto resConfig = resourceInfo_GetResourceConfiguration();
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
        [weakThemeManager = WeakPtr<ThemeManager>(themeManager), colorScheme = colorScheme_, config = resConfig],
            weakContext = WeakPtr<PipelineContext>(pipelineContext_)](){
                auto themeManager = weakThemeManager.Upgrade();
                auto context = weakContext.Upgrade();
                if (!themeManager || !context) {
                    return;
                }
                themeManager->ReloadThemes();
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

void AceContainer::UpdateColorMode(ColorMode colorMode {
    
    auto resConfig = resourceInfo_.GetResourceConfiguration();
    SystemProperties::SetColorMode(colorMode);
    if (resConfig.GetColorMode() == colorMode) {
        return;
    }
    resConfig.SetColorMode(colorMode);
    resourceInfo_.SetResourceConfiguration(resConfig);
    if(!pipelineContext_) {
        return;
    }    
    auto themeManager = pipelineContext_->GetThemeManager();
    if (!themeManager) {
        return;
    }
    themeManager->UpdateConfig(resConfig);
    taskExecutor_->PostTask(
        [weakThemeManager = WeakPtr<ThemeManager>(themeManager), colorScheme = colorScheme_,
            weakContext = WeakPtr<PipelineContext>(pipelineContext_)]() {
            auto themeManager = weakThemeManager.Upgrade();
            auto context = weakContext.Upgrade();
            if (!themeManager || !context) {
                return;
            } 
            themeManager->ReloadThemes();
            themeManager->ParseSustemTheme();
            themeManager->SetColorScheme(colorScheme);
            context->RefreshRootBgColor();   
        },
        TaskExecutor::TaskType::UI);
    if (frontend_) {
        frontend_->SetColorMode(colorMode);
        frontend_->RebuildAllPages();
    }    
}

void AceContainer::SetThememResourceInfo(const std::string& path, int32_t themeId) {
    ACE_FUNCTION_TRACE();
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

void AceContainer::InitThemeManager() {
    LOGI("Init theme manager");
    
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
                //get background color
                aceView->SetBackgroundColor(themeManager->GetBackgroundColor());
            },
            TaskExecutor::TaskType::UI);
    }

}

void AceContainer::SetSessionID(const std::string& sessionID) {
    if (aceView_) {
        aceView_->SetSessionID(sessionID);
    }
}

void AceContainer::SetHostClassName(const std::string& name) {
    hostClassName_ = name;
}

void AceContainer::SetInstanceName(const std::string& name) {
    hostInstanceName_ = name;
}

void AceContainer::TriggerGarbageCollection(){
    // To do
}

void AceContainer::NotifyFontNodes() {
    if (pipelineContext_) {
        pipelineContext_->NotifyFontNodes();
    }
}

void AceContainer::ProcessScreenOnEvents() {
    taskExecutor_->PostTask(
        [wp = WeakClaim(this)]() {
            auto container = wp.Upgrade();
            if (!container) {
                return;
            }
            for (const auto& func : container->screenOnEvents_) {
                func();
            }
        },
        TaskExecutor::TaskType::UI);
}


void AceContainer::ProcessScreenOffEvents() {
    taskExecutor_->PostTask(
        [wp = WeakClaim(this)]() {
            auto container = wp.Upgrade();
            if (!container) {
                return;
            }
            for (const auto& func : container->screenOffEvents_) {
                func();
            }
        },
        TaskExecutor::TaskType::UI);
}

void AceContainer::NotifyAppStorage(const std::string& key, const std::string& value) {
    if (pipelineContext_) {
        pipelineContext_->NotifyAppStorage();
    }
}

} // namespace OHOS::Ace::Platform