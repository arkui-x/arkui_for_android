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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_UI_CONTENT_IMPL_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_UI_CONTENT_IMPL_H

#include "adapter/android/entrance/java/jni/virtual_rs_window.h"

#include "base/utils/macros.h"
#include "core/accessibility/accessibility_node.h"
#include "core/event/key_event.h"
#include "foundation/appframework/arkui/uicontent/component_info.h"
#include "foundation/appframework/arkui/uicontent/ui_content.h"
#include "interfaces/inner_api/ace/viewport_config.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

#include "core/event/touch_event.h"

namespace OHOS {
class Window;
} // namespace OHOS

namespace OHOS::Ace::Platform {
class ACE_FORCE_EXPORT UIContentImpl : public UIContent {
public:
    UIContentImpl(OHOS::AbilityRuntime::Platform::Context* context, NativeEngine* runtime);
    ~UIContentImpl()
    {
        DestroyCallback();
        Ace::Platform::UIContent::RemoveUIContent(instanceId_);
    }
    std::unique_ptr<NativeEngine> nativeEngine_;
    // UI content lifecycles
    void Initialize(OHOS::Rosen::Window* window, const std::string& url, napi_value storage) override;
    napi_value GetUINapiContext() override;
    void Foreground() override;
    void Background() override;
    void Focus() override;
    void UnFocus() override;
    void Destroy() override;
    void OnNewWant(const OHOS::AAFwk::Want& want) override;
    void Finish() override;

    // UI content event process
    bool ProcessBackPressed() override;
    bool ProcessBasicEvent(const std::vector<TouchEvent>& touchEvents) override;
    bool ProcessPointerEvent(const std::vector<uint8_t>& data) override;
    bool ProcessMouseEvent(const std::vector<uint8_t>& data) override;
    bool ProcessKeyEvent(int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp = 0,
        int64_t timeStampStart = 0, int32_t metaKey = 0, int32_t sourceDevice = 0, int32_t deviceId = 0) override;

    void NotifySurfaceCreated() override;
    void NotifySurfaceDestroyed() override;
    void UpdateViewportConfig(const ViewportConfig& config, OHOS::Rosen::WindowSizeChangeReason reason) override;

    void UpdateConfiguration(const std::shared_ptr<OHOS::AbilityRuntime::Platform::Configuration>& config) override;

    // Window color
    uint32_t GetBackgroundColor() override;
    void SetBackgroundColor(uint32_t color) override;

    // Control filtering
    bool GetAllComponents(NodeId nodeID, OHOS::Ace::Platform::ComponentInfo& components) override;

    void DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override;

    // Set UIContent callback for custom window animation
    void SetNextFrameLayoutCallback(std::function<void()>&& callback) override;

    // Receive memory level notification
    void NotifyMemoryLevel(int32_t level) override;

private:
    void CommonInitialize(OHOS::Rosen::Window* window, const std::string& url, napi_value storage);

    void DestroyCallback() const;

    void InitOnceAceInfo();
    void InitAceInfoFromResConfig();

    std::weak_ptr<OHOS::AbilityRuntime::Platform::Context> context_;

    void* runtime_ = nullptr;
    OHOS::Rosen::Window* window_ = nullptr;
    std::string startUrl_;
    int32_t instanceId_ = -1;
    OHOS::Rosen::IOccupiedAreaChangeListener* occupiedAreaChangeListener_ = nullptr;
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_UI_CONTENT_IMPL_H
