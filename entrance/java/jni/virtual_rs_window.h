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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_VIRTUAL_RS_WINDOW_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_VIRTUAL_RS_WINDOW_H

#include <memory>

#include "flutter/shell/common/vsync_waiter.h"
#include "refbase.h"
#include "render_service_client/core/ui/rs_surface_node.h"

#include "adapter/android/entrance/java/jni/window_view_jni.h"
#include "base/log/log.h"
#include "base/utils/noncopyable.h"

class NativeValue;
class NativeEngine;

namespace OHOS {
namespace AbilityRuntime::Platform {
class Context;
class Ability;
}

namespace Ace::Platform {
class UIContent;
}

namespace AppExecFwk {
class EventHandler;
}

namespace Rosen {
constexpr uint32_t INVALID_WINDOW_ID = 0;
using OnCallback = std::function<void(int64_t)>;
struct VsyncCallback {
    OnCallback onCallback;
};
class VSyncReceiver;

enum class WindowState : uint32_t {
    STATE_INITIAL,
    STATE_CREATED,
    STATE_SHOWN,
    STATE_HIDDEN,
    STATE_FROZEN,
    STATE_UNFROZEN,
    STATE_DESTROYED,
    STATE_BOTTOM = STATE_DESTROYED, // Add state type after STATE_DESTROYED is not allowed
};

enum class WindowSizeChangeReason : uint32_t {
    UNDEFINED = 0,
    MAXIMIZE,
    RECOVER,
    ROTATION,
    DRAG,
    DRAG_START,
    DRAG_END,
    RESIZE,
    MOVE,
    HIDE,
    TRANSFORM,
    CUSTOM_ANIMATION_SHOW,
    FULL_TO_SPLIT,
    SPLIT_TO_FULL,
    END,
};

class Window : public RefBase {
public:
    static std::shared_ptr<Window> Create(
        std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context, JNIEnv* env, jobject windowView);

    explicit Window(const flutter::TaskRunners& taskRunners);
    explicit Window(std::shared_ptr<AbilityRuntime::Platform::Context> context);
    ~Window() override;

    bool CreateVSyncReceiver(std::shared_ptr<AppExecFwk::EventHandler> handler);
    void RequestNextVsync(std::function<void(int64_t, void*)> callback);

    virtual void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback);

    void CreateSurfaceNode(void* nativeWindow);
    void NotifySurfaceChanged(int32_t width, int32_t height);
    void NotifySurfaceDestroyed();

    int SetUIContent(const std::string& contentInfo, NativeEngine* engine,
        NativeValue* storage, bool isdistributed, AbilityRuntime::Platform::Ability* ability);

    std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const
    {
        return surfaceNode_;
    }

private:
    void SetWindowView(JNIEnv* env, jobject windowView);
    void ReleaseWindowView();

    void DelayNotifyUIContentIfNeeded();

    int32_t surfaceWidth_ = 0;
    int32_t surfaceHeight_ = 0;
    std::shared_ptr<RSSurfaceNode> surfaceNode_;
    std::shared_ptr<flutter::VsyncWaiter> vsyncWaiter_;

    jobject windowView_ = nullptr;
    std::shared_ptr<AbilityRuntime::Platform::Context> context_;
    std::unique_ptr<OHOS::Ace::Platform::UIContent> uiContent_;

    std::shared_ptr<VSyncReceiver> receiver_ = nullptr;

    bool delayNotifySurfaceCreated_ = false;
    bool delayNotifySurfaceChanged_ = false;
    bool delayNotifySurfaceDestroyed_ = false;

    WindowState state_ { WindowState::STATE_INITIAL };

    DISALLOW_COPY_AND_MOVE(Window);
};

} // namespace Rosen
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_VIRTUAL_RS_WINDOW_H
