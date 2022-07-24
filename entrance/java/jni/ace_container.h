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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_CONTAINER_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_ACE_CONTAINER_H

#include <chrono>
#include <memory>

#include "flutter/fml/synchronization/waitable_event.h"

#include "adapter/android/entrance/java/jni/ace_resource_register.h"
#include "adapter/android/entrance/java/jni/java_event_callback.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/resource/asset_manager.h"
#include "base/thread/task_executor.h"
#include "base/utils/noncopyable.h"
#include "base/utils/resource_configuration.h"
#include "core/common/ace_view.h"
#include "core/common/container.h"
#include "core/common/js_message_dispatcher.h"
#include "core/common/platform_bridge.h"

namespace OHOS::Ace::Platform {

// AceContainer is the instance which has its own pipeline and thread models, it can contain multiple pages.
class AceContainer : public Container, public JsMessageDispatcher {
    DECLARE_ACE_TYPE(AceContainer, Container, JsMessageDispatcher);

public:
    AceContainer(jint instanceId, FrontendType type, jobject callback);

    ~AceContainer() override = default;

    void Initialize() override;

    void Destroy() override;

    int32_t GetInstanceId() const override
    {
        return instanceId_;
    }

    std::string GetInstanceName() const
    {
        return instanceName_;
    }

    std::string GetHostClassName() const override
    {
        return hostClassName_;
    }

    RefPtr<Frontend> GetFrontend() const override
    {
        return frontend_;
    }

    RefPtr<TaskExecutor> GetTaskExecutor() const override
    {
        return taskExecutor_;
    }

    void SetAssetManagerIfNull(RefPtr<AssetManager> assetManager)
    {
        if (assetManager_ == nullptr) {
            assetManager_ = assetManager;
            if (frontend_) {
                frontend_->SetAssetManager(assetManager);
            }
        }
    }

    RefPtr<AssetManager> GetAssetManager() const override
    {
        return assetManager_;
    }

    RefPtr<PlatformResRegister> GetPlatformResRegister() const override
    {
        return resRegister_;
    }

    RefPtr<PipelineBase> GetPipelineContext() const override
    {
        return pipelineContext_;
    }

    int32_t GetViewWidth() const override
    {
        return aceView_ ? aceView_->GetWidth() : 0;
    }

    int32_t GetViewHeight() const override
    {
        return aceView_ ? aceView_->GetHeight() : 0;
    }

    void* GetView() const override
    {
        return static_cast<void*>(aceView_);
    }

    void SetWindowModal(WindowModal windowModal)
    {
        windowModal_ = windowModal;
    }

    void SetColorScheme(ColorScheme colorScheme)
    {
        colorScheme_ = colorScheme;
    }

    void SetSemiModalHeight(int32_t modalHeight)
    {
        modalHeight_ = modalHeight;
    }

    void SetSemiModalColor(uint32_t modalColor)
    {
        modalColor_ = modalColor;
    }

    void Dispatch(
        const std::string& group, std::vector<uint8_t>&& data, int32_t id, bool replyToComponent) const override;

    void DispatchSync(
        const std::string& group, std::vector<uint8_t>&& data, uint8_t** resData, long& position) const override;

    void DispatchPluginError(int32_t callbackId, int32_t errorCode, std::string&& errroMessage) const override;

    bool Dump(const std::vector<std::string>& params) override;

    void TriggerGarbageCollection() override;

    void NotifyFontNodes() override;

    void NotifyAppStorage(const std::string& key, const std::string& value) override;

    void SetActionCallback(jobject callback);

    void OnFinish()
    {
        if (platformEventCallback_) {
            platformEventCallback_->OnFinish();
        }
    }

    RefPtr<PlatformBridge> GetMessageBridge() const
    {
        return messageBridge_;
    }

    void UpdateThemeConfig(const ResourceConfiguration& config);

    ResourceConfiguration GetResourceConfiguration() const
    {
        return resourceInfo_.GetResourceConfiguration();
    }

    void SetResourceConfiguration(const ResourceConfiguration& config)
    {
        resourceInfo_.SetResourceConfiguration(config);
    }

    void UpdateResourceConfiguration(const std::string& jsonStr) override;

    uintptr_t GetMutilModalPtr() const override
    {
        return reinterpret_cast<uintptr_t>(multiModalPtr_);
    }

    void UpdateColorMode(ColorMode colorMode);

    void SetThemeResourceInfo(const std::string& path, int32_t themeId);
    void SetHostClassName(const std::string& name);
    void SetInstanceName(const std::string& name);
    void SetViewFirstUpdating(std::chrono::time_point<std::chrono::high_resolution_clock> time) override
    {
        aceView_->SetFirstUpDating(time);
    }

    void SetSessionID(const std::string& sessionID);

    void AttachView(std::unique_ptr<Window> window, AceView* view, double density, int32_t width, int32_t height);

    void InitThemeManager();

private:
    void InitializeFrontend(bool isArkApp);

    void InitializeCallback();

    AceView* aceView_ = nullptr;
    RefPtr<TaskExecutor> taskExecutor_;
    RefPtr<AssetManager> assetManager_;
    RefPtr<PlatformResRegister> resRegister_;
    RefPtr<PipelineBase> pipelineContext_;
    RefPtr<Frontend> frontend_;
    RefPtr<PlatformBridge> messageBridge_;
    FrontendType type_ { FrontendType::JSON };

    std::unique_ptr<JavaEventCallback> platformEventCallback_;
    WindowModal windowModal_ { WindowModal::NORMAL };
    ColorScheme colorScheme_ { ColorScheme::SCHEME_LIGHT };

    int32_t modalHeight_ = 0;
    uint32_t modalColor_ = 0xff000000;
    ResourceInfo resourceInfo_;
    int32_t instanceId_ = 0;
    bool isArk_ = false;

    std::string instanceName_;
    std::string hostClassName_;
    RefPtr<SharedImageManager> sharedImageManager_;

    std::vector<std::function<void()>> screenOnEvents_;
    std::vector<std::function<void()>> screenOffEvents_;

    void* multiModalPtr_ = nullptr;
    RefPtr<ThemeManager> themeManager_;
    std::shared_ptr<fml::ManualResetWaitableEvent> themeLatch_;

    ACE_DISALLOW_COPY_AND_MOVE(AceContainer);
};

} // namespace OHOS::Ace::Platform

#endif
