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

#include "adapter/android/entrance/java/jni/flutter_ace_view.h"

#include "flutter/fml/platform/android/jni_weak_ref.h"
#include "flutter/shell/platform/android/apk_asset_provider.h"

#include "adapter/android/entrance/java/jni/ace_resource_register.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "adapter/android/entrance/java/jni/native_window_surface.h"
#include "base/log/dump_log.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/macros.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/components/calendar/calendar_data_adapter.h"
#include "core/components/theme/theme_constants.h"
#include "core/event/event_convertor.h"
#include "core/event/mouse_event.h"
#include "core/event/touch_event.h"
#include "core/image/image_cache.h"
#include "core/pipeline/layers/flutter_scene_builder.h"

namespace OHOS::Ace::Platform {

void FlutterAceView::RegisterTouchEventCallback(TouchEventCallback&& callback) {

    ACE_DCHECK(callback);
    touchEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterKeyEventCallback(KeyEventCallback&& callback) {

    ACE_DCHECK(callback);
    keyEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterMouseEventCallback(MouseEventCallback&& callback) {

    ACE_DCHECK(callback);
    mouseEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterRotationEventCallback(KeyEventCallback&& callback) {

    ACE_DCHECK(callback);
    rotationEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterCardViewPositionCallback(CardViewPositionCallback&& callback) {

    ACE_DCHECK(callback);
    cardViewPositionCallback_ = std::move(callback);
}

void FlutterAceView::Launch() {

    LOGD("Launch shell holder");
    if (!viewLaunched_) {
        flutter::RunConfiguration config;
        shell_holder_->Launch(std::move(config));
        viewLaunched_ = true;
    }
}

void FlutterAceView::SetShellHolder(std::unique_ptr<flutter::AndroidShellHolder> holder) {

    shellHolder_ = std::move(holder);
}

bool FlutterAceView::ProcessTouchEvent(std::unique_ptr<flutter::PointerDataPacket> packet) {

    std::vector<TouchEvent> touchEvents;
    ConvertTouchEvent(packet->data(),touchEvents);
    LOGD(" ProcessTouchEvent event size%zu", touchEvents.size());
    bool forbiddenToPlatform = false;
    for (const auto& point : touchEvents) {
        if (point.type == TouchType::UNKNOWN) {
            LOGW("Unknown event");
            continue;
        }
        if (touchEventCallback_) {
            touchEventCallback_(point);
        }
    }
    // if it is last page, let os know to quit app
    return forbiddenToPlatform || (!IsLastPage());
}

bool FlutterAceView::ProcessMouseEvent(std::unique_ptr<flutter::PointerDataPacket> packet) {

    MouseEvent mouseEvent;
    ConvertMouseEvent(packet->data(),mouseEvent);
    LOGD(" ProcessMouseEvent event size");
    if (mouseEventCallback_) {
            mouseEventCallback_(point);
    }
}


bool FlutterAceView::ProcessKeyEvent(int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp, 
                                     int64_t timeStampStart, int32_t metaKey, int32_t sourceDevice, int32_t deviceId) {

    if (!keyEventCallback_) {
        return false;
    }
    
    auto keyEvents = keyEventRecognizer_.GetKeyEvents(keyCode, keyAction, repeatTime, timeStamp, timeStampStart,
                                                      metaKey, sourceDevice, deviceId);
    // distribute special event firstly
    // because platform receives a raw event, the special event processing is ignored
    if (keyEvents.size() > 1) {
        keyEventCallback_(keyEvents.back());
    }
    return keyEventCallback_(keyEvents.front());
}

void FlutterAceView::ProcessIdleEvent(int64_t deadline) {
    
    if (idleCallback_) {
        idleCallback_(deadline);
    }
} 

bool FlutterAceView::ProcessRotationEvent(float rotationValue) {
    
    if (!rotationEventCallback_) {
        return false;
    }
    
    RotationEvent event { .value = rotationValue * ROTATION_DIVISOR };

    return rotationEventCallback_(event); 
}

bool FlutterAceView::Dump(const std::vector<std::string>& params) {

    if (params.empty() || params[0] != "-drawcmd") {
        return false;
    }
    //TODO
    return false;
}

bool FlutterAceView::IsLastPage() const {

    auto container = AceEngine::Get().GetContainer(instanceId_);
    if (!container) {
        return false;
    }

    auto context = container->GetPipelineContext();
    if (!context) {
        return false;
    }

    return context->IsLastPage();
}



std::unique_ptr<DrawDelegate> FlutterAceView::GetDrawDelegate() {

    auto drawDelegate = std::make_unique<DrawDelegate>();
    drawDelegate->SetDrawFrameCallback([this](RefPtr<Flutter::Layer>& layer, const Rect& dirty){
        if (!layer) {
            return;
        }
        RefPtr<Flutter::FlutterSceneBuilder> flutterSceneBuilder = AceType::MakeRefPtf<Flutter::FlutterSceneBuilder>();
        layer->AddToScene(*flutterSceneBuilder, 0.0, 0.0);
        auto scene = flutterSceneBuilder->Build();
        if (!flutter::UIDartState::Current()) {
            LOGE("UIDartState is null");
            return;
        }
        auto window = flutter::UIDartState::Current()->window();
        if (window != nullptr && window->client() != nullptr) {
            window->client()->Render(scene.get());      
        }
    });
    return drawDelegate;
}


std::unique_ptr<PlatformWindow> FlutterAceView::GetPlatformWindow() {
    return PlatformWindow::Create(this);
}

const void* FlutterAceView::GetNativeWindowById(uint64_t textureId) {
    return nullptr;
}

void FlutterAceView::SetViewCallback(JNIEnv* env, jobject jObject) {
    
    if (env == nullptr) {
        LOGE("env is null");
        return;
    }

    object_ = JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), jObject);
    if (object_ = nullptr) {
        LOGE("Failed to make global ref ");
        return;
    }

    const jclass myClass = env->GetObjectClass(object_.get());
    if (myClass == nullptr) {
        LOGE("class is null");
        return;
    }
    
    updateWindowBlurDrawOp_ = env->GetMethodId(myClass, "updateWindowDrawOp", "()V");
    if (updateWindowBlurDrawOp_ == nullptr) {
        LOGE("Failed to get method id : updateWindowDrawOp");
    }

    updateWindowBlurRegion_ = env->GetMethodId(myClass, "updateWindowBlurRegion", "()V");
    if (updateWindowBlurRegion_ == nullptr) {
        LOGE("Failed to get method id : updateWindowBlurRegion");
    }

    env->DeleteLocalRef(myClass);
}


void FlutterAceView::UpdateWindowBlurRegion(const std::vector<std::vector<float>>& blurRRects) {

    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("Failed to get jni env");
        return;
    }
    if (!updateWindowBlurRegion_) {
        LOGE("updateWindowBlurRegion_ is null");
        return;
    }
    
    auto cls = env->FindClass("[F");
    auto javaBlurRRects = (env)->NewObjectArray(blurRRects.size(), cls, NULL);
    for(uint32_t i = 0; i < blurRRects.size(); i++) {
        uint32_t size = blurRRects[i].size();
        auto tmp = env->NewFloatArray(size);
        env->SetFloatArrayRegion(tmp, 0, size, &blurRRects[i][0]);
        env->SetObjectArrayElement(javaBlurRRects, i, tmp);
        env->DeleteLocalRef(tmp);
    }
    
    env->CallVoidMethod(object_.get(), updateWindowBlurRegion_, javaBlurRRects);
    if (env->ExceptionCheck()) {
        LOGE("exception occurred when updateWindowBlurRegion");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void FlutterAceView::UpdateWindowBlurDrawOp() {

    auto env = JniEnvironment::GetInstance().GetJniEnv();
    if (env == nullptr) {
        LOGE("Failed to get jni env");
        return;
    }
    if (!updateWindowBlurDrawOp_) {
        LOGE("updateWindowBlurDrawOp_ is null");
        return;
    }
    
    env->CallVoidMethod(object_.get(), updateWindowBlurDrawOp_);
    if (env->ExceptionCheck()) {
        LOGE("exception occurred when updateWindowBlurDrawOp");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

} // namespace




