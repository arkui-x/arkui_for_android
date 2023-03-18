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
#include "core/common/container_scope.h"
#include "core/components/calendar/calendar_data_adapter.h"
#include "core/components/theme/theme_constants.h"
#include "core/event/event_convertor.h"
#include "core/event/mouse_event.h"
#include "core/event/touch_event.h"
#include "core/image/image_cache.h"
#ifndef NG_BUILD
#include "core/pipeline/layers/flutter_scene_builder.h"
#endif

namespace OHOS::Ace::Platform {

void FlutterAceView::RegisterTouchEventCallback(TouchEventCallback&& callback)
{
    ACE_DCHECK(callback);
    touchEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterKeyEventCallback(KeyEventCallback&& callback)
{
    ACE_DCHECK(callback);
    keyEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterMouseEventCallback(MouseEventCallback&& callback)
{
    ACE_DCHECK(callback);
    mouseEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterRotationEventCallback(RotationEventCallBack&& callback)
{
    ACE_DCHECK(callback);
    rotationEventCallback_ = std::move(callback);
}

void FlutterAceView::Launch()
{
    LOGD("Launch shell holder");
    if (!viewLaunched_) {
#ifndef ENABLE_ROSEN_BACKEND
#ifdef NG_BUILD
        shellHolder_->Launch();
#else
        flutter::RunConfiguration config;
        shellHolder_->Launch(std::move(config));
#endif
#endif
        viewLaunched_ = true;
    }
}

#ifndef ENABLE_ROSEN_BACKEND
void FlutterAceView::SetShellHolder(std::unique_ptr<AndroidShellHolder> holder)
{
    shellHolder_ = std::move(holder);
}
#endif

bool FlutterAceView::ProcessTouchEvent(std::unique_ptr<flutter::PointerDataPacket> packet)
{
    std::vector<TouchEvent> touchEvents;
    ConvertTouchEvent(packet->data(), touchEvents);
    LOGD(" ProcessTouchEvent event size%zu", touchEvents.size());
    bool forbiddenToPlatform = false;
    for (const auto& point : touchEvents) {
        if (point.type == TouchType::UNKNOWN) {
            LOGW("Unknown event");
            continue;
        }
        if (touchEventCallback_) {
            touchEventCallback_(point, nullptr);
        }
    }
    // if it is last page, let os know to quit app
    return forbiddenToPlatform || (!IsLastPage());
}

void FlutterAceView::ProcessMouseEvent(std::unique_ptr<flutter::PointerDataPacket> packet)
{
    MouseEvent mouseEvent;
    ConvertMouseEvent(packet->data(), mouseEvent);
    LOGD(" ProcessMouseEvent event size");
    if (mouseEventCallback_) {
        mouseEventCallback_(mouseEvent, nullptr);
    }
}

bool FlutterAceView::ProcessKeyEvent(int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp,
    int64_t timeStampStart, int32_t metaKey, int32_t sourceDevice, int32_t deviceId)
{
    if (!keyEventCallback_) {
        return false;
    }

    auto keyEvents = keyEventRecognizer_.GetKeyEvents(
        keyCode, keyAction, repeatTime, timeStamp, timeStampStart, metaKey, sourceDevice, deviceId);
    // distribute special event firstly
    // because platform receives a raw event, the special event processing is ignored
    if (keyEvents.size() > 1) {
        keyEventCallback_(keyEvents.back());
    }
    return keyEventCallback_(keyEvents.front());
}

void FlutterAceView::ProcessIdleEvent(int64_t deadline)
{
    if (idleCallback_) {
        idleCallback_(deadline);
    }
}

bool FlutterAceView::ProcessRotationEvent(float rotationValue)
{
    if (!rotationEventCallback_) {
        return false;
    }

    RotationEvent event { .value = rotationValue * ROTATION_DIVISOR };

    return rotationEventCallback_(event);
}

bool FlutterAceView::Dump(const std::vector<std::string>& params)
{
    if (params.empty() || params[0] != "-drawcmd") {
        return false;
    }
    return false;
}

bool FlutterAceView::IsLastPage() const
{
    auto container = AceEngine::Get().GetContainer(instanceId_);
    if (!container) {
        return false;
    }
    ContainerScope scope(instanceId_);
    auto context = container->GetPipelineContext();
    if (!context) {
        return false;
    }
    auto taskExecutor = context->GetTaskExecutor();

    bool isLastPage = false;
    if (taskExecutor) {
        taskExecutor->PostSyncTask(
            [context, &isLastPage]() { isLastPage = context->IsLastPage(); }, TaskExecutor::TaskType::UI);
    }
    return isLastPage;
}

std::unique_ptr<DrawDelegate> FlutterAceView::GetDrawDelegate()
{
#ifndef ENABLE_ROSEN_BACKEND
    auto drawDelegate = std::make_unique<DrawDelegate>();
    drawDelegate->SetDrawFrameCallback([this](RefPtr<Flutter::Layer>& layer, const Rect& dirty) {
        if (!layer) {
            return;
        }
#ifndef NG_BUILD
        RefPtr<Flutter::FlutterSceneBuilder> flutterSceneBuilder = AceType::MakeRefPtr<Flutter::FlutterSceneBuilder>();
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
#endif
    });
    return drawDelegate;
#else
    return nullptr;
#endif
}

std::unique_ptr<PlatformWindow> FlutterAceView::GetPlatformWindow()
{
#ifdef NG_BUILD
    // for ng version, don't need PlatformWindow any more
    return nullptr;
#else
    return PlatformWindow::Create(this);
#endif
}

const void* FlutterAceView::GetNativeWindowById(uint64_t textureId)
{
    auto it = nativeWindowMap_.find(textureId);
    if (it != nativeWindowMap_.end()) {
        return it->second;
    }
    return nullptr;
}

void FlutterAceView::SetViewCallback(JNIEnv* env, jobject jObject) {}

} // namespace OHOS::Ace::Platform
