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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ACE_CONTAINER_SG_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ACE_CONTAINER_SG_H

#include <chrono>
#include <memory>

#include "ability_context.h"
#include "adapter/android/entrance/java/jni/virtual_rs_window.h"

#include "adapter/android/entrance/java/jni/ace_resource_register.h"
#include "adapter/android/stage/uicontent/platform_event_callback.h"
#include "base/resource/asset_manager.h"
#include "base/thread/task_executor.h"
#include "base/utils/noncopyable.h"
#include "base/utils/resource_configuration.h"
#include "core/common/ace_view.h"
#include "core/common/container.h"
#include "core/common/js_message_dispatcher.h"
#include "core/common/platform_bridge.h"
#include "core/components/theme/theme_manager.h"
#include "core/pipeline_ng/pipeline_context.h"

#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"

namespace OHOS::Ace::Platform {
using UIEnvCallback = std::function<void(const OHOS::Ace::RefPtr<OHOS::Ace::PipelineContext>& context)>;

struct ParsedConfig {
    std::string colorMode;
    std::string deviceAccess;
    std::string languageTag;
    std::string direction;
    std::string densitydpi;
    std::string themeTag;
    std::string fontFamily;
    std::string fontScale;
    std::string fontWeightScale;
    std::string colorModeIsSetByApp;
    std::string mcc;
    std::string mnc;
    std::string preferredLanguage;
    std::string fontId;
    bool IsValid() const
    {
        return !(colorMode.empty() && deviceAccess.empty() && languageTag.empty() && direction.empty() &&
                 densitydpi.empty() && themeTag.empty() && fontScale.empty() && fontWeightScale.empty() &&
                 colorModeIsSetByApp.empty() && mcc.empty() && mnc.empty() && fontFamily.empty() &&
                 preferredLanguage.empty() && fontId.empty());
    }
};
// AceContainerSG is the instance which has its own pipeline and thread models, it can contain multiple pages.
class AceContainerSG : public Container, public JsMessageDispatcher {
    DECLARE_ACE_TYPE(AceContainerSG, Container, JsMessageDispatcher);

public:
    AceContainerSG(int32_t instanceId, FrontendType type,
        std::weak_ptr<OHOS::AbilityRuntime::Platform::Context> runtimeContext,
        std::weak_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo, std::unique_ptr<PlatformEventCallback> callback,
        bool useCurrentEventRunner = false, bool isSubContainer = false);

    ~AceContainerSG() override = default;

    void Initialize() override;

    void Destroy() override;

    void DestroyView() override;

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

    void SetSharedRuntime(void* runtime) override
    {
        sharedRuntime_ = runtime;
    }

    void SetPageProfile(const std::string& pageProfile)
    {
        pageProfile_ = pageProfile;
    }

    RefPtr<Frontend> GetFrontend() const override
    {
        std::lock_guard<std::mutex> lock(frontendMutex_);
        return frontend_;
    }

    bool IsUseStageModel() const override
    {
        return useStageModel_;
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
        std::lock_guard<std::mutex> lock(pipelineMutex_);
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

    RefPtr<AceView> GetAceView() const
    {
        RefPtr<AceView> ref_aceView_;
        if (aceView_ != nullptr) {
            ref_aceView_ = aceView_;
        }
        return ref_aceView_;
    }

    AceView* GetAceViewEx() const
    {
        return aceView_;
    }

    int32_t GetViewPosX() const override
    {
        return aceView_ ? aceView_->GetPosX() : 0;
    }

    int32_t GetViewPosY() const override
    {
        return aceView_ ? aceView_->GetPosY() : 0;
    }

    uint32_t GetWindowId() const override
    {
        return windowId_;
    }

    void SetWindowId(uint32_t windowId) override
    {
        windowId_ = windowId;
    }

    void SetWindowPos(int32_t left, int32_t top);

    void SetWindowModal(WindowModal windowModal)
    {
        windowModal_ = windowModal;
    }

    void SetColorScheme(ColorScheme colorScheme)
    {
        colorScheme_ = colorScheme;
    }

    void SetWindowName(const std::string& name)
    {
        windowName_ = name;
    }

    std::string& GetWindowName()
    {
        return windowName_;
    }

    void* GetSharedRuntime() override
    {
        return sharedRuntime_;
    }

    void SetParentId(int32_t parentId)
    {
        parentId_ = parentId;
    }

    int32_t GetParentId() const
    {
        return parentId_;
    }

    void SetIsSubContainer(bool isSubContainer)
    {
        isSubContainer_ = isSubContainer;
    }

    bool IsSubContainer() const override
    {
        return isSubContainer_;
    }

    void Dispatch(
        const std::string& group, std::vector<uint8_t>&& data, int32_t id, bool replyToComponent) const override;

    void DispatchSync(
        const std::string& group, std::vector<uint8_t>&& data, uint8_t** resData, int64_t& position) const override;

    void DispatchPluginError(int32_t callbackId, int32_t errorCode, std::string&& errroMessage) const override;

    bool Dump(const std::vector<std::string>& params, std::vector<std::string>& info) override;

    void TriggerGarbageCollection() override;

    void NotifyFontNodes() override;

    void NotifyAppStorage(const std::string& key, const std::string& value) override;

    void SetActionCallback(jobject callback);

    void SetLocalStorage(NativeReference* storage, NativeReference* context);

    void OnFinish()
    {
        if (platformEventCallback_) {
            platformEventCallback_->OnFinish();
        }
    }

    ResourceConfiguration GetResourceConfiguration() const override
    {
        return resourceInfo_.GetResourceConfiguration();
    }

    void SetResourceConfiguration(const ResourceConfiguration& config)
    {
        resourceInfo_.SetResourceConfiguration(config);
    }

    void UpdateResourceConfiguration(const std::string& jsonStr) override {};

    void UpdateConfiguration(Platform::ParsedConfig& parsedConfig);

    uintptr_t GetMutilModalPtr() const override
    {
        return reinterpret_cast<uintptr_t>(multiModalPtr_);
    }

    void SetInstanceName(const std::string& name);
    void SetHostClassName(const std::string& name);
    void SetViewFirstUpdating(std::chrono::time_point<std::chrono::high_resolution_clock> time) override
    {
        aceView_->SetFirstUpDating(time);
    }

    void AttachView(std::unique_ptr<Window> window, AceView* view, double density, int32_t width, int32_t height,
        uint32_t windowId);

    // For stage mode
    static void DestroyContainer(int32_t instanceId, const std::function<void()>& destroyCallback = nullptr);
    static void OnNewRequest(int32_t instanceId, const std::string& data);
    static void OnConfigurationUpdated(int32_t instanceId, const std::string& configuration);
    static RefPtr<AceContainerSG> GetContainer(int32_t instanceId);
    static void OnShow(int32_t instanceId);
    static void OnHide(int32_t instanceId);
    static void OnActive(int32_t instanceId);
    static void OnInactive(int32_t instanceId);
    static bool OnBackPressed(int32_t instanceId);
    static bool RunPage(int32_t instanceId, int32_t pageId, const std::string& content,
        const std::string& params, bool isNamedRouter = false);
    static void SetView(AceView* view, double density,
        int32_t width, int32_t height, OHOS::Rosen::Window* rsWindow);

    static void SetUIWindow(int32_t instanceId, sptr<OHOS::Rosen::Window> uiWindow);
    static sptr<OHOS::Rosen::Window> GetUIWindow(int32_t instanceId);
    void InitializeSubContainer(int32_t parentContainerId);

    int32_t GeneratePageId()
    {
        return pageId_++;
    }

    std::string GetHapPath() const override
    {
        return resourceInfo_.GetHapPath();
    }

    void SetHapPath(const std::string& hapPath)
    {
        resourceInfo_.SetHapPath(hapPath);
    }

    std::string GetPackagePathStr() const
    {
        return resourceInfo_.GetPackagePath();
    }

    void SetPackagePathStr(const std::string& packagePath)
    {
        resourceInfo_.SetPackagePath(packagePath);
    }

    void SetResPaths(const std::vector<std::string>& hapPath, const std::string& path, const ColorMode& colorMode);

    bool WindowIsShow() const override
    {
        return true;
    }

    void SetCurPointerEvent(const std::shared_ptr<MMI::PointerEvent>& currentEvent);

    bool GetCurPointerEventInfo(DragPointerEvent& dragPointerEvent, StopDragCallback&& stopDragCallback) override;

    bool GetLastMovingPointerPosition(DragPointerEvent& dragPointerEvent) override;
private:
    virtual bool MaybeRelease() override;
    void InitializeFrontend();
    void InitializeCallback();
    void InitPiplineContext(std::unique_ptr<Window> window, double density, int32_t width, int32_t height,
        uint32_t windowId);
    void InitializeEventHandler();
    void InitializeFinishEventHandler(int32_t instanceId);
    void InitializeStatusBarEventHandler(int32_t instanceId);
    void SetGetViewScaleCallback();
    void InitThemeManager();
    void SetupRootElement();
    std::string GetOldLanguageTag() const;

    void SetUIWindowInner(sptr<OHOS::Rosen::Window> uiWindow);
    sptr<OHOS::Rosen::Window> GetUIWindowInner() const;
    void RegisterStopDragCallback(int32_t pointerId, StopDragCallback&& stopDragCallback);
    void SetFontAndScale(Platform::ParsedConfig& parsedConfig, ConfigurationChange& configurationChange);
    void SetLanguage(Platform::ParsedConfig& parsedConfig, ConfigurationChange& configurationChange,
        ResourceConfiguration& resConfig);
    void SetDirectionAndDensity(Platform::ParsedConfig& parsedConfig, ConfigurationChange& configurationChange,
        ResourceConfiguration& resConfig);
    void SetColor(Platform::ParsedConfig& parsedConfig, ConfigurationChange& configurationChange,
        ResourceConfiguration& resConfig);

    AceView* aceView_ { nullptr };
    RefPtr<TaskExecutor> taskExecutor_;
    RefPtr<AssetManager> assetManager_;
    RefPtr<PlatformResRegister> resRegister_;
    RefPtr<PipelineBase> pipelineContext_;
    RefPtr<Frontend> frontend_;
    RefPtr<PlatformBridge> messageBridge_;
    FrontendType type_ { FrontendType::JSON };

    std::unique_ptr<PlatformEventCallback> platformEventCallback_;
    WindowModal windowModal_ { WindowModal::NORMAL };
    ColorScheme colorScheme_ { ColorScheme::SCHEME_LIGHT };

    ResourceInfo resourceInfo_;
    int32_t instanceId_ { 0 };

    std::string instanceName_;
    std::string hostClassName_;

    void* multiModalPtr_ { nullptr };
    RefPtr<ThemeManager> themeManager_;

    // for stage mode
    uint32_t windowId_ { OHOS::Rosen::INVALID_WINDOW_ID };
    std::weak_ptr<OHOS::AbilityRuntime::Platform::Context> runtimeContext_;
    std::weak_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo_;
    void* sharedRuntime_ { nullptr };
    std::string pageProfile_;
    bool useCurrentEventRunner_ { false };
    int32_t pageId_ { 0 };
    bool useStageModel_ = false;

    mutable std::mutex frontendMutex_;
    mutable std::mutex pipelineMutex_;

    int32_t parentId_ = 0;
    sptr<OHOS::Rosen::Window> uiWindow_ = nullptr;
    std::string windowName_;
    bool isSubContainer_ = false;

    std::mutex pointerEventMutex_;
    std::shared_ptr<MMI::PointerEvent> currentPointerEvent_;
    std::unordered_map<int32_t, std::list<StopDragCallback>> stopDragCallbackMap_;
    std::map<int32_t, std::shared_ptr<MMI::PointerEvent>> currentEvents_;
    ACE_DISALLOW_COPY_AND_MOVE(AceContainerSG);
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_ACE_CONTAINER_SG_H
