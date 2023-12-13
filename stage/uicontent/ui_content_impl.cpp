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

#include "adapter/android/stage/uicontent/ui_content_impl.h"

#include <algorithm>

#include "ability.h"
#include "ability_context.h"
#include "ability_info.h"
#include "js_runtime_utils.h"
#include "res_config.h"
#include "resource_manager.h"
#include "stage_asset_provider.h"

#include "adapter/android/entrance/java/jni/ace_application_info_impl.h"
#include "adapter/android/entrance/java/jni/apk_asset_provider.h"
#include "adapter/android/entrance/java/jni/pack_asset_provider.h"
#include "adapter/android/osal/accessibility_manager_impl.h"
#include "adapter/android/osal/file_asset_provider.h"
#include "adapter/android/osal/page_url_checker_android.h"
#include "adapter/android/stage/uicontent/ace_container_sg.h"
#include "adapter/android/stage/uicontent/ace_view_sg.h"
#include "adapter/android/stage/uicontent/platform_event_callback.h"
#include "base/log/ace_trace.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "core/common/ace_engine.h"
#include "core/common/ace_view.h"
#include "core/common/asset_manager_impl.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/event/touch_event.h"
#include "core/image/image_file_cache.h"
#include "frameworks/bridge/declarative_frontend/ng/declarative_frontend_ng.h"

namespace OHOS::Ace::Platform {
namespace {
const std::string START_PARAMS_KEY = "__startParams";
// Device type, same as w/ java in AceView
constexpr int32_t ORIENTATION_PORTRAIT = 1;
constexpr int32_t ORIENTATION_LANDSCAPE = 2;
} // namespace

using ContentFinishCallback = std::function<void()>;
using ContentStartAbilityCallback = std::function<void(const std::string& address)>;
class ContentEventCallback final : public Platform::PlatformEventCallback {
public:
    explicit ContentEventCallback(ContentFinishCallback onFinish) : onFinish_(onFinish) {}
    ContentEventCallback(ContentFinishCallback onFinish, ContentStartAbilityCallback onStartAbility)
        : onFinish_(onFinish), onStartAbility_(onStartAbility)
    {}
    ~ContentEventCallback() override = default;

    void OnFinish() const override
    {
        LOGI("UIContent OnFinish");
        CHECK_NULL_VOID(onFinish_);
        onFinish_();
    }

    void OnStartAbility(const std::string& address) override
    {
        LOGI("UIContent OnStartAbility");
        CHECK_NULL_VOID(onStartAbility_);
        onStartAbility_(address);
    }

    void OnStatusBarBgColorChanged(uint32_t color) override
    {
        LOGI("UIContent OnStatusBarBgColorChanged");
    }

private:
    ContentFinishCallback onFinish_;
    ContentStartAbilityCallback onStartAbility_;
};

class OccupiedAreaChangeListener : public OHOS::Rosen::IOccupiedAreaChangeListener {
public:
    explicit OccupiedAreaChangeListener(int32_t instanceId) : instanceId_(instanceId) {}
    ~OccupiedAreaChangeListener() = default;

    void OnSizeChange(const OHOS::Rosen::Rect& rect, const OHOS::Rosen::OccupiedAreaType type)
    {
        Rect keyboardRect = Rect(rect.posX_, rect.posY_, rect.width_, rect.height_);
        if (type == OHOS::Rosen::OccupiedAreaType::TYPE_INPUT) {
            auto container = Platform::AceContainerSG::GetContainer(instanceId_);
            CHECK_NULL_VOID(container);
            auto taskExecutor = container->GetTaskExecutor();
            CHECK_NULL_VOID(taskExecutor);
            ContainerScope scope(instanceId_);
            taskExecutor->PostTask(
                [container, keyboardRect] {
                    auto context = container->GetPipelineContext();
                    CHECK_NULL_VOID(context);
                    context->OnVirtualKeyboardAreaChange(keyboardRect);
                },
                TaskExecutor::TaskType::UI);
        }
    }

private:
    int32_t instanceId_ = -1;
};

UIContentImpl::UIContentImpl(OHOS::AbilityRuntime::Platform::Context* context, NativeEngine* runtime)
    : runtime_(reinterpret_cast<void*>(runtime))
{
    CHECK_NULL_VOID(context);
    const auto& obj = context->GetBindingObject();
    auto ref = obj->Get<NativeReference>();
    void* result = nullptr;
    napi_unwrap(reinterpret_cast<napi_env>(runtime), ref->GetNapiValue(), &result);
    auto weak = static_cast<std::weak_ptr<AbilityRuntime::Platform::Context>*>(result);
    context_ = *weak;
    LOGI("Create UIContentImpl successfully.");
}

void UIContentImpl::DestroyCallback() const
{
    auto container = Platform::AceContainerSG::GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->SetNextFrameLayoutCallback(nullptr);
    LOGI("DestroyCallback called.");
}

void UIContentImpl::Initialize(OHOS::Rosen::Window* window, const std::string& url, napi_value storage)
{
    if (window) {
        CommonInitialize(window, url, storage);
    }
    LOGI("Initialize startUrl = %{public}s", startUrl_.c_str());

    Platform::AceContainerSG::RunPage(
        instanceId_, Platform::AceContainerSG::GetContainer(instanceId_)->GeneratePageId(), startUrl_, "");
    LOGI("RunPage UIContentImpl done.");
}

napi_value UIContentImpl::GetUINapiContext()
{
    auto container = Platform::AceContainerSG::GetContainer(instanceId_);
    ContainerScope scope(instanceId_);
    auto frontend = container->GetFrontend();
    CHECK_NULL_RETURN(frontend, nullptr);
    if (frontend->GetType() == FrontendType::DECLARATIVE_JS) {
        auto declarativeFrontend = AceType::DynamicCast<DeclarativeFrontendNG>(frontend);
        CHECK_NULL_RETURN(declarativeFrontend, nullptr);
        return declarativeFrontend->GetContextValue();
    }

    return nullptr;
}

void UIContentImpl::CommonInitialize(OHOS::Rosen::Window* window, const std::string& url, napi_value storage)
{
    ACE_FUNCTION_TRACE();
    window_ = window;
    startUrl_ = url;
    CHECK_NULL_VOID(window_);

    InitOnceAceInfo();
    InitAceInfoFromResConfig();

    auto context = context_.lock();
    CHECK_NULL_VOID(context);

    auto abilityContext =
        OHOS::AbilityRuntime::Platform::Context::ConvertTo<OHOS::AbilityRuntime::Platform::AbilityContext>(context);
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> info;
    CHECK_NULL_VOID(abilityContext);
    info = abilityContext->GetAbilityInfo();
    if (info) {
        AceApplicationInfo::GetInstance().SetAbilityName(info->name);
    }

    RefPtr<AssetManagerImpl> assetManagerImpl = Referenced::MakeRefPtr<AssetManagerImpl>();
    bool isModelJson = info != nullptr ? info->isModuleJson : false;
    std::string moduleName = info != nullptr ? info->moduleName : "";
    auto appInfo = context->GetApplicationInfo();
    auto bundleName = info != nullptr ? info->bundleName : "";
    std::string moduleHapPath = info != nullptr ? info->hapPath : "";
    std::string pageProfile;
    LOGI("Initialize UIContent isModelJson:%{public}s", isModelJson ? "true" : "false");
    if (isModelJson) {
        auto hapPath = info != nullptr ? info->hapPath : "";
        hapPath = hapPath.empty() ? "arkui-x/" + moduleName : hapPath;
        if (hapPath.at(hapPath.size() - 1) == '/') {
            hapPath = hapPath.substr(0, hapPath.size() - 1);
        }
        LOGI("hapPath:%{public}s", hapPath.c_str());
        // first use hap provider
        if (assetManagerImpl && !hapPath.empty()) {
            auto assetProvider = AbilityRuntime::Platform::StageAssetProvider::GetInstance();
            CHECK_NULL_VOID(assetProvider);
            auto dynamicLoadFlag = true;
            std::string moduleNameMark = "/" + moduleName + "/";
            auto allFilePath = assetProvider->GetAllFilePath();
            for (auto& path : allFilePath) {
                if (path.find(moduleNameMark) != std::string::npos) {
                    dynamicLoadFlag = false;
                    break;
                }
            }

            auto env = JniEnvironment::GetInstance().GetJniEnv();
            std::vector<std::string> hapAssetPaths { "", "/ets", "/ets/share", "/resources/base/profile" };
            if (dynamicLoadFlag) {
                auto fileAssetProvider = AceType::MakeRefPtr<FileAssetProvider>();
                if (fileAssetProvider->Initialize(
                        assetProvider->GetAppDataModuleDir() + "/" + moduleName, hapAssetPaths)) {
                    LOGD("Push AssetProvider to queue.");
                    assetManagerImpl->PushBack(std::move(fileAssetProvider));
                }
            } else {
                for (const auto& path : hapAssetPaths) {
                    auto apkAssetProvider =
                        AceType::MakeRefPtr<ApkAssetProvider>(std::make_unique<PackAssetProvider>(env.get(),
                                                                  assetProvider->GetAssetManager(), hapPath + path),
                            hapPath + path);
                    apkAssetProvider->SetAssetManager(
                        AAssetManager_fromJava(env.get(), assetProvider->GetAssetManager()));
                    assetManagerImpl->PushBack(std::move(apkAssetProvider));
                }
            }
        }
        auto hapInfo = context->GetHapModuleInfo();
        if (hapInfo) {
            pageProfile = hapInfo->pages;
            const std::string profilePrefix = "$profile:";
            if (pageProfile.compare(0, profilePrefix.size(), profilePrefix) == 0) {
                pageProfile = pageProfile.substr(profilePrefix.length()).append(".json");
            }
            LOGI("In stage mode, pageProfile:%{public}s", pageProfile.c_str());
        } else {
            LOGE("In stage mode, can't get hap info.");
        }
    }

    // create container
    if (info) {
        instanceId_ = window->IsSubWindow() ? window->GetWindowId() : info->instanceId;
        LOGI("acecontainer init instanceId_:%{public}d", instanceId_);
        Ace::Platform::UIContent::AddUIContent(instanceId_, this);
    }

    AceTraceBegin("CreateAndInitConatienr");
    auto container = AceType::MakeRefPtr<Platform::AceContainerSG>(instanceId_, FrontendType::DECLARATIVE_JS, context_,
        info,
        std::make_unique<ContentEventCallback>(
            [context = context_] {
                auto sharedContext = context.lock();
                if (!sharedContext) {
                    return;
                }
                auto abilityContext =
                    OHOS::AbilityRuntime::Platform::Context::ConvertTo<OHOS::AbilityRuntime::Platform::AbilityContext>(
                        sharedContext);
                if (abilityContext) {
                    LOGI("callback abilitycontext to terminate self.");
                    abilityContext->TerminateSelf();
                }
            },
            [](const std::string& address) { LOGI("start ability with url = %{private}s", address.c_str()); }),
        true);

    CHECK_NULL_VOID(container);
    AceEngine::Get().AddContainer(instanceId_, container);
    container->SetInstanceName(info->name);
    container->SetHostClassName(info->name);
    if (runtime_) {
        LOGI("settings:setUsingSharedRuntime.");
        container->GetSettings().SetUsingSharedRuntime(true);
        container->SetSharedRuntime(runtime_);
    } else {
        LOGI("settings:set not UsingSharedRuntime.");
        container->GetSettings().SetUsingSharedRuntime(false);
    }
    container->SetPageProfile(pageProfile);
    container->Initialize();
    ContainerScope Initializescope(instanceId_);
    auto front = container->GetFrontend();
    if (front) {
        front->UpdateState(Frontend::State::ON_CREATE);
        front->SetJsMessageDispatcher(container);
    }

    double density = SystemProperties::GetResolution();
    auto aceResCfg = container->GetResourceConfiguration();
    aceResCfg.SetOrientation(SystemProperties::GetDeviceOrientation());
    aceResCfg.SetDensity(density);
    aceResCfg.SetDeviceType(SystemProperties::GetDeviceType());
    aceResCfg.SetColorMode(SystemProperties::GetColorMode());
    aceResCfg.SetDeviceAccess(SystemProperties::GetDeviceAccess());
    container->SetResourceConfiguration(aceResCfg);
    container->SetAssetManagerIfNull(assetManagerImpl);
    container->SetBundlePath(context->GetBundleCodeDir());
    container->SetFilesDataPath(context->GetFilesDir());
    container->SetModuleName(moduleName);
    container->SetIsModule(info->compileMode == AppExecFwk::CompileMode::ES_MODULE);
    container->SetPageUrlChecker(AceType::MakeRefPtr<PageUrlCheckerAndroid>());

    std::vector<std::string> resourcePaths;
    std::string sysResPath { "" };
    abilityContext->GetResourcePaths(resourcePaths, sysResPath);
    container->SetResPaths(resourcePaths, sysResPath, SystemProperties::GetColorMode());
    AceTraceEnd();

    AceTraceBegin("CreateAndSetView");
    auto aceView = Platform::AceViewSG::CreateView(instanceId_);
    if (!window_) {
        Platform::AceViewSG::SurfaceCreated(aceView, window_);
    }
    // set view
    Platform::AceContainerSG::SetView(aceView, density, 0, 0, window_);
    AceTraceEnd();
    if (window_) {
        occupiedAreaChangeListener_ = new OccupiedAreaChangeListener(instanceId_);
        window_->RegisterOccupiedAreaChangeListener(occupiedAreaChangeListener_);
    }

    // Set sdk version in module json mode
    if (isModelJson) {
        auto pipeline = container->GetPipelineContext();
        if (pipeline && appInfo) {
            LOGI("SetMinPlatformVersion code is %{public}d", appInfo->apiCompatibleVersion);
            pipeline->SetMinPlatformVersion(appInfo->apiCompatibleVersion);
        }
    }

    if (runtime_) {
        auto nativeEngine = reinterpret_cast<NativeEngine*>(runtime_);
        if (!storage) {
            container->SetLocalStorage(nullptr, context->GetBindingObject()->Get<NativeReference>());
        } else {
            auto env = reinterpret_cast<napi_env>(nativeEngine);
            napi_ref ref = nullptr;
            napi_create_reference(env, storage, 1, &ref);
            container->SetLocalStorage(
                reinterpret_cast<NativeReference*>(ref), context->GetBindingObject()->Get<NativeReference>());
        }
    }
}

void UIContentImpl::InitOnceAceInfo()
{
    LOGI("Initialize UIContentImpl start.");
    auto context = context_.lock();
    CHECK_NULL_VOID(context);
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&context]() {
        LOGI("Initialize for current process.");
        Container::UpdateCurrent(INSTANCE_ID_PLATFORM);
        AceApplicationInfo::GetInstance().SetProcessName(context->GetBundleName());
        AceApplicationInfo::GetInstance().SetPackageName(context->GetBundleName());
        AceApplicationInfo::GetInstance().SetDataFileDirPath(context->GetFilesDir());
        AceApplicationInfo::GetInstance().SetUid(context->GetApplicationInfo()->uid);
        AceApplicationInfo::GetInstance().SetPid(context->GetApplicationInfo()->pid);
        ImageFileCache::GetInstance().SetImageCacheFilePath(context->GetCacheDir());
        ImageFileCache::GetInstance().SetCacheFileInfo();
    });
}

void UIContentImpl::InitAceInfoFromResConfig()
{
    ACE_FUNCTION_TRACE();
    auto context = context_.lock();
    CHECK_NULL_VOID(context);
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    auto resourceManager = context->GetResourceManager();
    if (resourceManager != nullptr) {
        resourceManager->GetResConfig(*resConfig);
        auto localeInfo = resConfig->GetLocaleInfo();
        Platform::AceApplicationInfoImpl::GetInstance().SetResourceManager(resourceManager);
        if (localeInfo != nullptr) {
            auto language = localeInfo->getLanguage();
            auto region = localeInfo->getCountry();
            auto script = localeInfo->getScript();
            AceApplicationInfo::GetInstance().SetLocale((language == nullptr) ? "" : language,
                (region == nullptr) ? "" : region, (script == nullptr) ? "" : script, "");
        } else {
            LOGI("localeInfo is nullptr, set localeInfo to default");
            AceApplicationInfo::GetInstance().SetLocale("", "", "", "");
        }
        if (resConfig->GetColorMode() == OHOS::Global::Resource::ColorMode::DARK) {
            SystemProperties::SetColorMode(ColorMode::DARK);
            LOGI("UIContent set dark mode");
        } else {
            SystemProperties::SetColorMode(ColorMode::LIGHT);
            LOGI("UIContent set light mode");
        }
        if (resConfig->GetDirection() == OHOS::Global::Resource::Direction::DIRECTION_VERTICAL) {
            SystemProperties::SetDeviceOrientation(ORIENTATION_PORTRAIT);
        } else if (resConfig->GetDirection() == OHOS::Global::Resource::Direction::DIRECTION_HORIZONTAL) {
            SystemProperties::SetDeviceOrientation(ORIENTATION_LANDSCAPE);
        }
        SystemProperties::SetResolution(resConfig->GetScreenDensity());
        SystemProperties::SetDeviceAccess(
            resConfig->GetInputDevice() == Global::Resource::InputDevice::INPUTDEVICE_POINTINGDEVICE);
    }
}

void UIContentImpl::Foreground()
{
    LOGI("UIContentImpl: window foreground");
    Platform::AceContainerSG::OnShow(instanceId_);
    // set the flag isForegroundCalled to be true
    auto container = Platform::AceContainerSG::GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->SetForegroundCalled(true);
}

void UIContentImpl::Background()
{
    LOGI("UIContentImpl: window background");
    Platform::AceContainerSG::OnHide(instanceId_);
}

void UIContentImpl::Focus()
{
    LOGI("UIContentImpl: window focus");
    Platform::AceContainerSG::OnActive(instanceId_);
}

void UIContentImpl::UnFocus()
{
    LOGI("UIContentImpl: window unFocus");
    Platform::AceContainerSG::OnInactive(instanceId_);
}

void UIContentImpl::Destroy()
{
    LOGI("UIContentImpl: window destroy");
    auto container = AceEngine::Get().GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    Platform::AceContainerSG::DestroyContainer(instanceId_);
}

void UIContentImpl::OnNewWant(const OHOS::AAFwk::Want& want)
{
    LOGI("UIContent OnNewWant");
    Platform::AceContainerSG::OnShow(instanceId_);
    std::string params = want.GetStringParam(START_PARAMS_KEY);
    Platform::AceContainerSG::OnNewRequest(instanceId_, params);
}

void UIContentImpl::Finish()
{
    LOGI("UIContent Finish");
    auto container = Platform::AceContainerSG::GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    container->OnFinish();
}

uint32_t UIContentImpl::GetBackgroundColor()
{
    auto container = Platform::AceContainerSG::GetContainer(instanceId_);
    CHECK_NULL_RETURN(container, 0x000000);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_RETURN(taskExecutor, 0x000000);

    ContainerScope scope(instanceId_);
    uint32_t bgColor = 0x000000;
    taskExecutor->PostSyncTask(
        [&bgColor, container]() {
            CHECK_NULL_VOID(container);
            auto pipelineContext = container->GetPipelineContext();
            CHECK_NULL_VOID(pipelineContext);
            bgColor = pipelineContext->GetAppBgColor().GetValue();
        },
        TaskExecutor::TaskType::UI);

    LOGI("UIContentImpl::GetBackgroundColor, value is %{public}u", bgColor);
    return bgColor;
}

void UIContentImpl::SetBackgroundColor(uint32_t color)
{
    LOGI("UIContentImpl: SetBackgroundColor color is %{public}u", color);
    auto container = AceEngine::Get().GetContainer(instanceId_);
    CHECK_NULL_VOID(container);

    ContainerScope scope(instanceId_);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostSyncTask(
        [container, bgColor = color]() {
            auto pipelineContext = container->GetPipelineContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->SetAppBgColor(Color(bgColor));
        },
        TaskExecutor::TaskType::UI);
}

bool UIContentImpl::ProcessBackPressed()
{
    LOGI("UIContentImpl: ProcessBackPressed: Platform::AceContainerSG::OnBackPressed called");
    auto container = AceEngine::Get().GetContainer(instanceId_);
    CHECK_NULL_RETURN(container, false);

    LOGI("UIContentImpl::ProcessBackPressed AceContainerSG");
    if (Platform::AceContainerSG::OnBackPressed(instanceId_)) {
        LOGI("UIContentImpl::ProcessBackPressed AceContainerSG return true");
        return true;
    }
    LOGI("ProcessBackPressed: Platform::AceContainerSG::OnBackPressed return false");
    return false;
}

bool UIContentImpl::ProcessBasicEvent(const std::vector<TouchEvent>& touchEvents)
{
    auto container = AceEngine::Get().GetContainer(instanceId_);
    CHECK_NULL_RETURN(container, false);

    auto aceView = static_cast<Platform::AceViewSG*>(container->GetView());
    CHECK_NULL_RETURN(aceView, false);

    return aceView->DispatchBasicEvent(touchEvents);
}

bool UIContentImpl::ProcessPointerEvent(const std::vector<uint8_t>& data)
{
    LOGI("UIContentImpl::ProcessPointerEvent called");
    auto container = AceEngine::Get().GetContainer(instanceId_);
    CHECK_NULL_RETURN(container, false);

    auto aceView = static_cast<Platform::AceViewSG*>(container->GetView());
    CHECK_NULL_RETURN(aceView, false);

    return aceView->DispatchTouchEvent(data);
}

bool UIContentImpl::ProcessMouseEvent(const std::vector<uint8_t>& data)
{
    LOGI("UIContentImpl::ProcessMouseEvent called");
    auto container = AceEngine::Get().GetContainer(instanceId_);
    CHECK_NULL_RETURN(container, false);

    auto aceView = static_cast<Platform::AceViewSG*>(container->GetView());
    CHECK_NULL_RETURN(aceView, false);
    return aceView->DispatchMouseEvent(data);
}

bool UIContentImpl::ProcessKeyEvent(int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp,
    int64_t timeStampStart, int32_t metaKey, int32_t sourceDevice, int32_t deviceId)
{
    LOGI("UIContentImpl: OnKeyUp called");
    auto container = AceEngine::Get().GetContainer(instanceId_);
    CHECK_NULL_RETURN(container, false);

    auto aceView = static_cast<Platform::AceViewSG*>(container->GetView());
    CHECK_NULL_RETURN(aceView, false);

    return aceView->DispatchKeyEvent(
        { keyCode, keyAction, repeatTime, timeStamp, timeStampStart, metaKey, sourceDevice, deviceId });
}

void UIContentImpl::UpdateConfiguration(const std::shared_ptr<OHOS::AbilityRuntime::Platform::Configuration>& config)
{
    LOGI("UIContentImpl: UpdateConfiguration called");
    CHECK_NULL_VOID(config);
    auto container = Platform::AceContainerSG::GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [weakContainer = WeakPtr<Platform::AceContainerSG>(container), config]() {
            auto container = weakContainer.Upgrade();
            CHECK_NULL_VOID(container);
            auto colorMode = config->GetItem(OHOS::AbilityRuntime::Platform::ConfigurationInner::SYSTEM_COLORMODE);
            auto direction = config->GetItem(OHOS::AbilityRuntime::Platform::ConfigurationInner::APPLICATION_DIRECTION);
            auto densityDpi =
                config->GetItem(OHOS::AbilityRuntime::Platform::ConfigurationInner::APPLICATION_DENSITYDPI);
            auto language = config->GetItem(OHOS::AbilityRuntime::Platform::ConfigurationInner::APPLICATION_LANGUAGE);
            container->UpdateConfiguration(colorMode, direction, densityDpi, language);
        },
        TaskExecutor::TaskType::UI);
    LOGI("UIContentImpl: UpdateConfiguration called End");
}

void UIContentImpl::UpdateViewportConfig(const ViewportConfig& config, OHOS::Rosen::WindowSizeChangeReason reason)
{
    LOGI("UIContentImpl: UpdateViewportConfig %{public}s", config.ToString().c_str());
    ACE_SCOPED_TRACE("UpdateViewportConfig %s", config.ToString().c_str());
    SystemProperties::SetResolution(config.Density());
    SystemProperties::SetDeviceOrientation(
        config.Height() >= config.Width() ? ORIENTATION_PORTRAIT : ORIENTATION_LANDSCAPE);
    auto container = Platform::AceContainerSG::GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    container->SetWindowPos(config.Left(), config.Top());
    auto pipelineContext = container->GetPipelineContext();
    if (pipelineContext) {
        pipelineContext->SetDisplayWindowRectInfo(
            Rect(Offset(config.Left(), config.Top()), Size(config.Width(), config.Height())));
    }
    auto aceView = static_cast<Platform::AceViewSG*>(container->GetAceView());
    CHECK_NULL_VOID(aceView);
    Platform::AceViewSG::SetViewportMetrics(aceView, config);
    Platform::AceViewSG::SurfaceChanged(
        aceView, config.Width(), config.Height(), config.Orientation(), static_cast<WindowSizeChangeReason>(reason));
    Platform::AceViewSG::SurfacePositionChanged(aceView, config.Left(), config.Top());
}

// Control filtering
bool UIContentImpl::GetAllComponents(NodeId nodeID, OHOS::Ace::Platform::ComponentInfo& components)
{
    LOGI("UIContentImpl::GetAllComponents enter.");
    auto container = Platform::AceContainerSG::GetContainer(instanceId_);
    CHECK_NULL_RETURN(container, false);
    if (container->GetPipelineContext()) {
        auto accessibilityManager = container->GetPipelineContext()->GetAccessibilityManager();
        if (accessibilityManager) {
            auto accessibilityNodeManager =
                AceType::DynamicCast<OHOS::Ace::Framework::AccessibilityNodeManager>(accessibilityManager);
            auto accessibilityManagerImpl =
                AceType::DynamicCast<OHOS::Ace::Framework::AccessibilityManagerImpl>(accessibilityNodeManager);
            auto ret = accessibilityManagerImpl->GetAllComponents(nodeID, components);
            LOGI("UIContentImpl::GetAllComponents ret = %d", ret);
            return ret;
        }
    }
    LOGI("UIContentImpl::GetAllComponents exit.");
    return false;
}

void UIContentImpl::DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info)
{
    auto container = Platform::AceContainerSG::GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    container->Dump(params, info);
}

void UIContentImpl::SetNextFrameLayoutCallback(std::function<void()>&& callback)
{
    CHECK_NULL_VOID(callback);
    auto container = Platform::AceContainerSG::GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->SetNextFrameLayoutCallback(std::move(callback));
}

void UIContentImpl::NotifyMemoryLevel(int32_t level)
{
    LOGI("Receive Memory level notification, level: %{public}d", level);
    auto container = Platform::AceContainerSG::GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    ContainerScope scope(instanceId_);
    pipelineContext->NotifyMemoryLevel(level);
}

void UIContentImpl::NotifySurfaceCreated()
{
    LOGI("UIContentImpl: NotifySurfaceCreated called.");
    auto container = AceEngine::Get().GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    auto pipeline = container->GetPipelineContext();
    CHECK_NULL_VOID(pipeline);
    ContainerScope scope(instanceId_);
    auto* window = pipeline->GetWindow();
    CHECK_NULL_VOID(window);
    window->Init();
    window->RequestFrame();
}

void UIContentImpl::NotifySurfaceDestroyed()
{
    LOGI("UIContentImpl: NotifySurfaceDestroyed called.");
    auto container = AceEngine::Get().GetContainer(instanceId_);
    CHECK_NULL_VOID(container);
    auto aceView = static_cast<Platform::AceViewSG*>(container->GetView());
    CHECK_NULL_VOID(aceView);
    aceView->NotifySurfaceDestroyed();
}

std::unique_ptr<UIContent> UIContent::Create(OHOS::AbilityRuntime::Platform::Context* context, NativeEngine* runtime)
{
    std::unique_ptr<UIContent> content;
    content.reset(new UIContentImpl(context, runtime));
    return content;
}
} // namespace OHOS::Ace::Platform
