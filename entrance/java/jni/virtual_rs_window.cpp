/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "virtual_rs_window.h"

#include <cstdint>
#include <memory>

#include "ability_context.h"
#include "display_info_jni.h"
#include "foundation/appframework/arkui/uicontent/ui_content.h"
#include "hilog.h"
#include "mmi_event_convertor.h"
#include "napi/native_api.h"
#include "render_service_client/core/pipeline/rs_render_thread.h"
#include "subwindow_manager_jni.h"
#include "transaction/rs_interfaces.h"
#include "window_view_adapter.h"
#include "window_view_jni.h"

#include "adapter/android/entrance/java/jni/ace_env_jni.h"
#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/utils/utils.h"
#include "core/event/touch_event.h"

using namespace OHOS::Ace::Platform;

namespace OHOS::Rosen {
namespace {
static constexpr Rect EMPTY_RECT = {0, 0, 0, 0};
static constexpr int32_t ORIENTATION_LANDSCAPE = 0;
static constexpr int32_t ORIENTATION_PORTRAIT = 1;
}  // namespace
void DummyWindowRelease(Window* window)
{
    window->DecStrongRef(window);
    LOGI("Rosenwindow rsWindow_Window: dummy release");
}

Ace::KeyCode KeyCodeToAceKeyCode(int keyCode)
{
    Ace::KeyCode aceKeyCode = Ace::KeyCode::KEY_UNKNOWN;
    const static std::map<int32_t, Ace::KeyCode> TO_OHOS_KEYCODE_MAP = {
        { 4 /* KEYCODE_BACK */, Ace::KeyCode::KEY_ESCAPE },
        { 7 /* KEYCODE_0 */, Ace::KeyCode::KEY_0 },
        { 8 /* KEYCODE_1 */, Ace::KeyCode::KEY_1 },
        { 9 /* KEYCODE_2 */, Ace::KeyCode::KEY_2 },
        { 10 /* KEYCODE_3 */, Ace::KeyCode::KEY_3 },
        { 11 /* KEYCODE_4 */, Ace::KeyCode::KEY_4 },
        { 12 /* KEYCODE_5 */, Ace::KeyCode::KEY_5 },
        { 13 /* KEYCODE_6 */, Ace::KeyCode::KEY_6 },
        { 14 /* KEYCODE_7 */, Ace::KeyCode::KEY_7 },
        { 15 /* KEYCODE_8 */, Ace::KeyCode::KEY_8 },
        { 16 /* KEYCODE_9 */, Ace::KeyCode::KEY_9 },
        { 19 /* KEYCODE_DPAD_UP */, Ace::KeyCode::KEY_DPAD_UP },
        { 20 /* KEYCODE_DPAD_DOWN */, Ace::KeyCode::KEY_DPAD_DOWN },
        { 21 /* KEYCODE_DPAD_LEFT */, Ace::KeyCode::KEY_DPAD_LEFT },
        { 22 /* KEYCODE_DPAD_RIGHT */, Ace::KeyCode::KEY_DPAD_RIGHT },
        { 23 /* KEYCODE_DPAD_CENTER */, Ace::KeyCode::KEY_DPAD_CENTER },
        { 29 /* KEYCODE_A */, Ace::KeyCode::KEY_A },
        { 30 /* KEYCODE_B */, Ace::KeyCode::KEY_B },
        { 31 /* KEYCODE_C */, Ace::KeyCode::KEY_C },
        { 32 /* KEYCODE_D */, Ace::KeyCode::KEY_D },
        { 33 /* KEYCODE_E */, Ace::KeyCode::KEY_E },
        { 34 /* KEYCODE_F */, Ace::KeyCode::KEY_F },
        { 35 /* KEYCODE_G */, Ace::KeyCode::KEY_G },
        { 36 /* KEYCODE_H */, Ace::KeyCode::KEY_H },
        { 37 /* KEYCODE_I */, Ace::KeyCode::KEY_I },
        { 38 /* KEYCODE_J */, Ace::KeyCode::KEY_J },
        { 39 /* KEYCODE_K */, Ace::KeyCode::KEY_K },
        { 40 /* KEYCODE_L */, Ace::KeyCode::KEY_L },
        { 41 /* KEYCODE_M */, Ace::KeyCode::KEY_M },
        { 42 /* KEYCODE_N */, Ace::KeyCode::KEY_N },
        { 43 /* KEYCODE_O */, Ace::KeyCode::KEY_O },
        { 44 /* KEYCODE_P */, Ace::KeyCode::KEY_P },
        { 45 /* KEYCODE_Q */, Ace::KeyCode::KEY_Q },
        { 46 /* KEYCODE_R */, Ace::KeyCode::KEY_R },
        { 47 /* KEYCODE_S */, Ace::KeyCode::KEY_S },
        { 48 /* KEYCODE_T */, Ace::KeyCode::KEY_T },
        { 49 /* KEYCODE_U */, Ace::KeyCode::KEY_U },
        { 50 /* KEYCODE_V */, Ace::KeyCode::KEY_V },
        { 51 /* KEYCODE_W */, Ace::KeyCode::KEY_W },
        { 52 /* KEYCODE_X */, Ace::KeyCode::KEY_X },
        { 53 /* KEYCODE_Y */, Ace::KeyCode::KEY_Y },
        { 54 /* KEYCODE_Z */, Ace::KeyCode::KEY_Z },
        { 55 /* KEYCODE_COMMA */, Ace::KeyCode::KEY_COMMA },
        { 56 /* KEYCODE_PERIOD */, Ace::KeyCode::KEY_PERIOD },
        { 57 /* KEYCODE_ALT_LEFT */, Ace::KeyCode::KEY_ALT_LEFT },
        { 58 /* KEYCODE_ALT_RIGHT */, Ace::KeyCode::KEY_ALT_RIGHT },
        { 59 /* KEYCODE_SHIFT_LEFT */, Ace::KeyCode::KEY_SHIFT_LEFT },
        { 60 /* KEYCODE_SHIFT_RIGHT */, Ace::KeyCode::KEY_SHIFT_RIGHT },
        { 61 /* KEYCODE_TAB */, Ace::KeyCode::KEY_TAB },
        { 62 /* KEYCODE_SPACE */, Ace::KeyCode::KEY_SPACE },
        { 66 /* KEYCODE_ENTER */, Ace::KeyCode::KEY_ENTER },
        { 67 /* KEYCODE_DEL */, Ace::KeyCode::KEY_DEL },
        { 68 /* KEYCODE_GRAVE */, Ace::KeyCode::KEY_GRAVE },
        { 69 /* KEYCODE_MINUS */, Ace::KeyCode::KEY_MINUS },
        { 70 /* KEYCODE_EQUALS */, Ace::KeyCode::KEY_EQUALS },
        { 71 /* KEYCODE_LEFT_BRACKET */, Ace::KeyCode::KEY_LEFT_BRACKET },
        { 72 /* KEYCODE_RIGHT_BRACKET */, Ace::KeyCode::KEY_RIGHT_BRACKET },
        { 73 /* KEYCODE_BACKSLASH */, Ace::KeyCode::KEY_BACKSLASH },
        { 74 /* KEYCODE_SEMICOLON */, Ace::KeyCode::KEY_SEMICOLON },
        { 75 /* KEYCODE_APOSTROPHE */, Ace::KeyCode::KEY_APOSTROPHE },
        { 76 /* KEYCODE_SLASH */, Ace::KeyCode::KEY_SLASH },
        { 82 /* KEYCODE_MENU */, (Ace::KeyCode)2466 },
        { 84 /* KEYCODE_SEARCH */, Ace::KeyCode::KEY_SEARCH },
        { 92 /* KEYCODE_PAGE_UP */, Ace::KeyCode::KEY_PAGE_UP },
        { 93 /* KEYCODE_PAGE_DOWN */, Ace::KeyCode::KEY_PAGE_DOWN },
        { 111 /* KEYCODE_ESCAPE */, Ace::KeyCode::KEY_ESCAPE },
        { 112 /* KEYCODE_FORWARD_DEL */, Ace::KeyCode::KEY_FORWARD_DEL },
        { 113 /* KEYCODE_CTRL_LEFT */, Ace::KeyCode::KEY_CTRL_LEFT },
        { 114 /* KEYCODE_CTRL_RIGHT */, Ace::KeyCode::KEY_CTRL_RIGHT },
        { 115 /* KEYCODE_CAPS_LOCK */, Ace::KeyCode::KEY_CAPS_LOCK },
        { 116 /* KEYCODE_SCROLL_LOCK */, Ace::KeyCode::KEY_SCROLL_LOCK },
        { 117 /* KEYCODE_META_LEFT */, Ace::KeyCode::KEY_META_LEFT },
        { 118 /* KEYCODE_META_RIGHT */, Ace::KeyCode::KEY_META_RIGHT },
        { 120 /* KEYCODE_SYSRQ */, Ace::KeyCode::KEY_SYSRQ },
        { 121 /* KEYCODE_BREAK */, Ace::KeyCode::KEY_BREAK },
        { 122 /* KEYCODE_MOVE_HOME */, Ace::KeyCode::KEY_MOVE_HOME },
        { 123 /* KEYCODE_MOVE_END */, Ace::KeyCode::KEY_MOVE_END },
        { 124 /* KEYCODE_INSERT */, Ace::KeyCode::KEY_INSERT },
        { 131 /* KEYCODE_F1 */, Ace::KeyCode::KEY_F1 },
        { 132 /* KEYCODE_F2 */, Ace::KeyCode::KEY_F2 },
        { 133 /* KEYCODE_F3 */, Ace::KeyCode::KEY_F3 },
        { 134 /* KEYCODE_F4 */, Ace::KeyCode::KEY_F4 },
        { 135 /* KEYCODE_F5 */, Ace::KeyCode::KEY_F5 },
        { 136 /* KEYCODE_F6 */, Ace::KeyCode::KEY_F6 },
        { 137 /* KEYCODE_F7 */, Ace::KeyCode::KEY_F7 },
        { 138 /* KEYCODE_F8 */, Ace::KeyCode::KEY_F8 },
        { 139 /* KEYCODE_F9 */, Ace::KeyCode::KEY_F9 },
        { 140 /* KEYCODE_F10 */, Ace::KeyCode::KEY_F10 },
        { 141 /* KEYCODE_F11 */, Ace::KeyCode::KEY_F11 },
        { 142 /* KEYCODE_F12 */, Ace::KeyCode::KEY_F12 },
        { 143 /* KEYCODE_NUM_LOCK */, Ace::KeyCode::KEY_NUM_LOCK },
        { 144 /* KEYCODE_NUMPAD_0 */, Ace::KeyCode::KEY_NUMPAD_0 },
        { 145 /* KEYCODE_NUMPAD_1 */, Ace::KeyCode::KEY_NUMPAD_1 },
        { 146 /* KEYCODE_NUMPAD_2 */, Ace::KeyCode::KEY_NUMPAD_2 },
        { 147 /* KEYCODE_NUMPAD_3 */, Ace::KeyCode::KEY_NUMPAD_3 },
        { 148 /* KEYCODE_NUMPAD_4 */, Ace::KeyCode::KEY_NUMPAD_4 },
        { 149 /* KEYCODE_NUMPAD_5 */, Ace::KeyCode::KEY_NUMPAD_5 },
        { 150 /* KEYCODE_NUMPAD_6 */, Ace::KeyCode::KEY_NUMPAD_6 },
        { 151 /* KEYCODE_NUMPAD_7 */, Ace::KeyCode::KEY_NUMPAD_7 },
        { 152 /* KEYCODE_NUMPAD_8 */, Ace::KeyCode::KEY_NUMPAD_8 },
        { 153 /* KEYCODE_NUMPAD_9 */, Ace::KeyCode::KEY_NUMPAD_9 },
        { 154 /* KEYCODE_NUMPAD_DIVIDE */, Ace::KeyCode::KEY_NUMPAD_DIVIDE },
        { 155 /* KEYCODE_NUMPAD_MULTIPLY */, Ace::KeyCode::KEY_NUMPAD_MULTIPLY },
        { 156 /* KEYCODE_NUMPAD_SUBTRACT */, Ace::KeyCode::KEY_NUMPAD_SUBTRACT },
        { 157 /* KEYCODE_NUMPAD_ADD */, Ace::KeyCode::KEY_NUMPAD_ADD },
        { 158 /* KEYCODE_NUMPAD_DOT */, Ace::KeyCode::KEY_NUMPAD_DOT },
        { 160 /* KEYCODE_NUMPAD_ENTER */, Ace::KeyCode::KEY_NUMPAD_ENTER },
    };
    auto checkIter = TO_OHOS_KEYCODE_MAP.find(keyCode);
    if (checkIter != TO_OHOS_KEYCODE_MAP.end()) {
        aceKeyCode = checkIter->second;
    }
    return aceKeyCode;
}

const std::map<ColorSpace, GraphicColorGamut> COLOR_SPACE_JS_TO_GAMUT_MAP {
    { ColorSpace::COLOR_SPACE_DEFAULT, GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB },
    { ColorSpace::COLOR_SPACE_WIDE_GAMUT, GraphicColorGamut::GRAPHIC_COLOR_GAMUT_DCI_P3 },
};

std::map<uint32_t, std::vector<std::shared_ptr<Window>>> Window::subWindowMap_;
std::map<std::string, std::pair<uint32_t, std::shared_ptr<Window>>> Window::windowMap_;
std::map<uint32_t, std::vector<sptr<IOccupiedAreaChangeListener>>> Window::occupiedAreaChangeListeners_;
std::map<uint32_t, std::vector<sptr<IWindowLifeCycle>>> Window::lifecycleListeners_;
std::map<uint32_t, std::vector<sptr<IWindowChangeListener>>> Window::windowChangeListeners_;
std::map<uint32_t, std::vector<sptr<ITouchOutsideListener>>> Window::touchOutsideListeners_;
std::map<uint32_t, std::vector<sptr<IWindowSurfaceNodeListener>>> Window::surfaceNodeListeners_;
std::map<uint32_t, std::vector<sptr<IAvoidAreaChangedListener>>> Window::avoidAreaChangeListeners_;
std::map<uint32_t, std::vector<sptr<IWindowStatusChangeListener>>> Window::windowStatusChangeListeners_;
std::recursive_mutex Window::globalMutex_;
std::recursive_mutex g_sysBarPropMapMutex;
uint32_t g_KeyboardHeight = 0;
bool g_IsFullScreen = false;
bool g_IsStatusBarHide = false;

Window::Window(std::shared_ptr<AbilityRuntime::Platform::Context> context, uint32_t windowId)
    : context_(context), windowId_(windowId), brightness_(SubWindowManagerJni::GetAppScreenBrightness())
{}

Window::Window(std::shared_ptr<AbilityRuntime::Platform::Context> context)
    : context_(context), brightness_(SubWindowManagerJni::GetAppScreenBrightness())
{}

Window::~Window()
{
    LOGI("Rosenwindow rsWindow_Window: release id = %u", windowId_);
    ReleaseWindowView();
}

void Window::AddToWindowMap(std::shared_ptr<Window> window)
{
    DeleteFromWindowMap(window);
    windowMap_.insert(std::make_pair(
        window->GetWindowName(), std::pair<uint32_t, std::shared_ptr<Window>>(window->GetWindowId(), window)));
}

void Window::DeleteFromWindowMap(std::shared_ptr<Window> window)
{
    auto iter = windowMap_.find(window->GetWindowName());
    if (iter != windowMap_.end()) {
        windowMap_.erase(iter);
    }
}

void Window::DeleteFromWindowMap(Window* window)
{
    if (window == nullptr) {
        return;
    }
    auto iter = windowMap_.find(window->GetWindowName());
    if (iter != windowMap_.end()) {
        windowMap_.erase(iter);
    }
}

void Window::AddToSubWindowMap(std::shared_ptr<Window> window)
{
    if (window == nullptr) {
        LOGE("window is null");
        return;
    }
    if (window->GetType() != OHOS::Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW ||
        window->GetParentId() == INVALID_WINDOW_ID) {
        LOGE("window is not subwindow");
        return;
    }
    DeleteFromSubWindowMap(window);
    uint32_t parentId = window->GetParentId();
    subWindowMap_[parentId].push_back(window);
}

void Window::DeleteFromSubWindowMap(std::shared_ptr<Window> window)
{
    if (window == nullptr) {
        return;
    }
    uint32_t parentId = window->GetParentId();
    if (parentId == INVALID_WINDOW_ID) {
        return;
    }
    auto iter1 = subWindowMap_.find(parentId);
    if (iter1 == subWindowMap_.end()) {
        return;
    }
    auto subWindows = iter1->second;
    auto iter2 = subWindows.begin();
    while (iter2 != subWindows.end()) {
        if (*iter2 == window) {
            subWindows.erase(iter2);
            ((*iter2)->Destroy());
            break;
        }
        iter2++;
    }
}

std::shared_ptr<Window> Window::Create(
    std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context, JNIEnv* env, jobject windowView)
{
    std::string windowName = AbilityRuntime::Platform::WindowViewAdapter::GetInstance()->GetWindowName(windowView);
    if (windowName.empty()) {
        LOGE("Window::Create called failed due to null windowName");
        return nullptr;
    }

    LOGI("Window::Create called. windowName=%s", windowName.c_str());

    auto window = std::shared_ptr<Window>(new Window(context), DummyWindowRelease);
    window->SetWindowView(env, windowView);
    window->SetWindowName(windowName);

    auto abilityContext =
        OHOS::AbilityRuntime::Platform::Context::ConvertTo<OHOS::AbilityRuntime::Platform::AbilityContext>(context);
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> info;
    CHECK_NULL_RETURN(abilityContext, nullptr);
    info = abilityContext->GetAbilityInfo();
    if (info) {
        LOGI("info->name = %s, info->instanceId = %d", info->name.c_str(), info->instanceId);
        window->SetWindowId(info->instanceId);
    }
    window->IncStrongRef(window.get());
    AddToWindowMap(window);
    AvoidAreaMapInit(window);
    return window;
}

void Window::AvoidAreaMapInit(std::shared_ptr<Window> window)
{
    for (auto type : {
        AvoidAreaType::TYPE_SYSTEM,
        AvoidAreaType::TYPE_CUTOUT,
        AvoidAreaType::TYPE_SYSTEM_GESTURE,
        AvoidAreaType::TYPE_KEYBOARD,
        AvoidAreaType::TYPE_NAVIGATION_INDICATOR
    }) {
        auto avoidArea  = std::make_shared<Rosen::AvoidArea>();
        window->GetAvoidAreaByType(type, *avoidArea);
        window->avoidAreaMap_.insert(std::make_pair(type, *avoidArea));
    }
}

std::shared_ptr<Window> Window::CreateDragWindow(std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context)
{
    LOGI("CreateDragWindow");
    auto mainWindow = OHOS::Rosen::Window::GetTopWindow(context);
    CHECK_NULL_RETURN(mainWindow, nullptr);

    std::string windowName = mainWindow->GetWindowName() + ":Drag";
    std::shared_ptr<OHOS::Rosen::WindowOption> option = std::make_shared<OHOS::Rosen::WindowOption>();
    option->SetWindowType(OHOS::Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(OHOS::Rosen::WindowMode::WINDOW_MODE_FLOATING);
    option->SetParentId(mainWindow->GetWindowId());
    option->SetWindowName(windowName);

    std::shared_ptr<OHOS::Rosen::Window> window = OHOS::Rosen::Window::CreateSubWindow(context, option);
    CHECK_NULL_RETURN(window, nullptr);

    window->SetFocusable(false);
    window->SetTouchable(false);
    window->SetFullScreen(true);
    window->SetOnTop(true);

    return window;
}

std::shared_ptr<Window> Window::CreateSubWindow(
    std::shared_ptr<OHOS::AbilityRuntime::Platform::Context> context, std::shared_ptr<OHOS::Rosen::WindowOption> option)
{
    if (option == nullptr) {
        LOGE("Window::CreateSubWindow called failed due to null option");
        return nullptr;
    }

    LOGI("Window::CreateSubWindow called. windowName=%s", option->GetWindowName().c_str());

    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    bool result = SubWindowManagerJni::CreateSubWindow(option);
    if (result) {
        jobject view = SubWindowManagerJni::GetContentView(option->GetWindowName());
        uint32_t windowId = SubWindowManagerJni::GetWindowId(option->GetWindowName());

        auto window = std::shared_ptr<Window>(new Window(context), DummyWindowRelease);
        window->SetWindowId(windowId);
        window->SetWindowName(option->GetWindowName());
        window->SetWindowType(option->GetWindowType());
        window->SetParentId(option->GetParentId());
        window->SetWindowMode(option->GetWindowMode());
        window->SetRect(option);
        window->IncStrongRef(window.get());
        AddToSubWindowMap(window);
        AddToWindowMap(window);
        AvoidAreaMapInit(window);

        window->SetSubWindowView(env, view);

        return window;
    }

    LOGI("Window::CreateSubWindow: failed");
    return nullptr;
}

WMError Window::Destroy()
{
    LOGI("Window::Destroy: %s", this->GetWindowName().c_str());
    if (uiContent_ != nullptr) {
        uiContent_->Destroy();
        uiContent_ = nullptr;
    }

    if (GetType() == OHOS::Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW && !GetWindowName().empty()) {
        SubWindowManagerJni::DestroyWindow(GetWindowName());
    }

    NotifyBeforeDestroy(GetWindowName());

    // Remove subWindows of current window from subWindowMap_
    if (subWindowMap_.count(GetWindowId()) > 0) {
        auto& subWindows = subWindowMap_.at(GetWindowId());
        for (auto iter = subWindows.begin(); iter != subWindows.end(); iter = subWindows.begin()) {
            if ((*iter) == nullptr) {
                subWindows.erase(iter);
                continue;
            }

            auto windowPtr = (*iter);
            subWindows.erase(iter);
            LOGI("Window::Destroy SubWindow %s", (windowPtr)->GetWindowName().c_str());
            DeleteFromWindowMap(windowPtr);
            (windowPtr)->Destroy();
        }
        subWindowMap_[GetWindowId()].clear();
        subWindowMap_.erase(GetWindowId());
    }

    // Rmove current window from subWindowMap_ of parent window
    if (subWindowMap_.count(GetParentId()) > 0) {
        auto& subWindows = subWindowMap_.at(GetParentId());
        for (auto iter = subWindows.begin(); iter < subWindows.end(); ++iter) {
            if ((*iter) == nullptr) {
                continue;
            }
            if ((*iter)->GetWindowId() == GetWindowId()) {
                subWindows.erase(iter);
                break;
            }
        }
    }

    // Remove current window from windowMap_
    if (windowMap_.count(GetWindowName()) > 0) {
        DeleteFromWindowMap(this);
    }

    NotifyAfterBackground();
    ReleaseWindowView();
    ClearListenersById(GetWindowId());
    return WMError::WM_OK;
}

void Window::RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func)
{
    LOGD("Start register");
    notifyNativefunc_ = std::move(func);
}

WMError Window::RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
{
    LOGD("Start register");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(occupiedAreaChangeListeners_[GetWindowId()], listener);
}

WMError Window::UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
{
    LOGD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(occupiedAreaChangeListeners_[GetWindowId()], listener);
}

WMError Window::RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener)
{
    LOGD("Start register");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(windowChangeListeners_[GetWindowId()], listener);
}

WMError Window::UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener)
{
    LOGD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(windowChangeListeners_[GetWindowId()], listener);
}

std::vector<std::shared_ptr<Window>> Window::GetSubWindow(uint32_t parentId)
{
    LOGI("Window::GetSubWindow called. parentId=%d", parentId);
    if (subWindowMap_.find(parentId) == subWindowMap_.end()) {
        return std::vector<std::shared_ptr<Window>>();
    }
    return subWindowMap_[parentId];
}

std::shared_ptr<Window> Window::FindWindow(const std::string& name)
{
    LOGI("Window::FindWindow called. name=%s", name.c_str());
    auto iter = windowMap_.find(name);
    if (iter == windowMap_.end()) {
        return nullptr;
    }
    return iter->second.second;
}

std::shared_ptr<Window> Window::GetTopWindow(const std::shared_ptr<OHOS::AbilityRuntime::Platform::Context>& context)
{
    LOGI("Window::GetTopWindow");

    auto iter = windowMap_.begin();
    while (iter != windowMap_.end()) {
        std::pair<uint32_t, std::shared_ptr<Window>> pair = iter->second;
        std::shared_ptr<Window> window = pair.second;
        if (context.get() == window->GetContext().get() &&
            window->GetType() != OHOS::Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW) {
            return window;
        }

        iter++;
    }

    LOGE("Window::GetTopWindow no window on forground");
    return nullptr;
}

WMError Window::ShowWindow()
{
    LOGI("Window::ShowWindow called.");
    if (this->GetWindowName().empty()) {
        LOGI("Window::ShowWindow called failed due to null option");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    bool result = SubWindowManagerJni::ShowWindow(this->GetWindowName());
    if (result) {
        NotifyAfterForeground();
        return WMError::WM_OK;
    } else {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

bool Window::IsWindowShow()
{
    if (this->GetWindowName().empty()) {
        LOGE("Window::IsWindowShow called failed due to null window name");
        return false;
    }
    return SubWindowManagerJni::IsWindowShowing(this->GetWindowName());
}

WMError Window::MoveWindowTo(int32_t x, int32_t y)
{
    LOGI("Window::MoveWindowTo called. x=%d, y=%d", x, y);
    if (this->GetWindowName().empty()) {
        LOGI("Window::MoveWindowTo called failed due to null option");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    rect_.posX_ = x;
    rect_.posY_ = y;

    bool result = SubWindowManagerJni::MoveWindowTo(this->GetWindowName(), x, y);
    if (result) {
        AvoidAreaChange();
        NotifySizeChange(rect_);
        return WMError::WM_OK;
    } else {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}
WMError Window::ResizeWindowTo(int32_t width, int32_t height)
{
    LOGI("Window::ResizeWindowTo called. width=%d, height=%d", width, height);

    if (this->GetWindowName().empty()) {
        LOGI("Window::ResizeWindowTo called failed due to null option");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    rect_.width_ = width;
    rect_.height_ = height;
    bool result = SubWindowManagerJni::ResizeWindowTo(this->GetWindowName(), width, height);
    if (result) {
        AvoidAreaChange();
        NotifySizeChange(rect_);
        return WMError::WM_OK;
    } else {
        LOGI("Window::ResizeWindowTo: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

WMError Window::SetBackgroundColor(uint32_t color)
{
    LOGI("Window::SetBackgroundColor called. color=%u", color);
    backgroundColor_ = color;

    if (uiContent_) {
        uiContent_->SetBackgroundColor(color);
        return WMError::WM_OK;
    }
    return WMError::WM_ERROR_INVALID_OPERATION;
}

WMError Window::SetBrightness(float brightness)
{
    LOGI("Window::SetBrightness called. brightness=%.3f", brightness);
    if (brightness < MINIMUM_BRIGHTNESS || brightness > MAXIMUM_BRIGHTNESS) {
        LOGE("invalid brightness value: %{public}f", brightness);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    bool result = SubWindowManagerJni::SetAppScreenBrightness(brightness);

    brightness_ = brightness;
    if (result) {
        return WMError::WM_OK;
    } else {
        LOGI("Window::SetBrightness: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}
WMError Window::SetKeepScreenOn(bool keepScreenOn)
{
    LOGI("Window::SetKeepScreenOn called. keepScreenOn=%d", keepScreenOn);

    bool result = SubWindowManagerJni::SetKeepScreenOn(keepScreenOn);
    if (result) {
        return WMError::WM_OK;
    } else {
        LOGI("Window::SetKeepScreenOn: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

ColorSpace Window::GetColorSpaceFromSurfaceGamut(GraphicColorGamut colorGamut) const
{
    for (auto& item : COLOR_SPACE_JS_TO_GAMUT_MAP) {
        if (item.second == colorGamut) {
            return item.first;
        }
    }
    return ColorSpace::COLOR_SPACE_DEFAULT;
}

GraphicColorGamut Window::GetSurfaceGamutFromColorSpace(ColorSpace colorSpace) const
{
    for (auto& item : COLOR_SPACE_JS_TO_GAMUT_MAP) {
        if (item.first == colorSpace) {
            return item.second;
        }
    }
    return GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB;
}

WMError Window::SetColorSpace(ColorSpace colorSpace)
{
    if (!surfaceNode_) {
        LOGE("Window::SetColorSpace called. surfaceNode_ is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto surfaceGamut = GetSurfaceGamutFromColorSpace(colorSpace);
    LOGI("Window::SetColorSpace called. colorSpace=%{public}d, surfaceGamut=%{public}d", colorSpace, surfaceGamut);
    surfaceNode_->SetColorSpace(surfaceGamut);
    return WMError::WM_OK;
}

ColorSpace Window::GetColorSpace() const
{
    if (!surfaceNode_) {
        LOGE("Window::GetColorSpace called. surfaceNode_ is null");
        return ColorSpace::COLOR_SPACE_DEFAULT;
    }
    GraphicColorGamut gamut = surfaceNode_->GetColorSpace();
    ColorSpace colorSpace = GetColorSpaceFromSurfaceGamut(gamut);
    return colorSpace;
}

bool Window::IsKeepScreenOn()
{
    LOGI("Window::IsKeepScreenOn called.");

    return SubWindowManagerJni::IsKeepScreenOn();
}

WMError Window::SetSystemBarProperty(WindowType type, const SystemBarProperty& property)
{
    bool hide = !property.enable_;
    bool result = false;

    if (type == WindowType::WINDOW_TYPE_NAVIGATION_BAR) {
        result = SubWindowManagerJni::SetNavigationBarStatus(hide);
    } else if (type == WindowType::WINDOW_TYPE_STATUS_BAR) {
        result = SubWindowManagerJni::SetStatusBarStatus(hide);
        if (result) {
            g_IsStatusBarHide = hide;
        }
    } else {
        LOGE("The WindowType is not set to SystemBarProperty. The WindowType is %{public}d", type);
    }

    std::lock_guard<std::recursive_mutex> lock(g_sysBarPropMapMutex);
    sysBarPropMap_[type] = property;

    if (result) {
        return WMError::WM_OK;
    } else {
        LOGE("Window::SetSystemBarProperty: failed. The WindowType is %{public}d", type);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

void Window::SetRequestedOrientation(Orientation orientation)
{
    LOGI("Window::SetRequestedOrientation called.");

    bool result = SubWindowManagerJni::RequestOrientation(orientation);
    if (!result) {
        LOGI("Window::SetRequestedOrientation: failed");
    }
}

SystemBarProperty Window::GetSystemBarPropertyByType(WindowType type) const
{
    LOGI("Window::GetSystemBarPropertyByType called.");
    for (auto& it : sysBarPropMap_) {
        if (it.first == type) {
            return it.second;
        }
    }
}

void Window::ClearListenersById(uint32_t winId)
{
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    ClearUselessListeners(lifecycleListeners_, winId);
    ClearUselessListeners(avoidAreaChangeListeners_, winId);
}

WMError Window::RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    LOGD("Start register");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(lifecycleListeners_[GetWindowId()], listener);
}

WMError Window::UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener)
{
    LOGD("Start unregister");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(lifecycleListeners_[GetWindowId()], listener);
}

WMError Window::RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener)
{
    LOGD("Start register TouchOutsideListener");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(touchOutsideListeners_[GetWindowId()], listener);
}

WMError Window::UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener)
{
    LOGD("Start unregister TouchOutsideListener");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(touchOutsideListeners_[GetWindowId()], listener);
}

WMError Window::RegisterSurfaceNodeListener(const sptr<IWindowSurfaceNodeListener>& listener)
{
    LOGI("Start register SurfaceNodeListener");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return RegisterListener(surfaceNodeListeners_[GetWindowId()], listener);
}

WMError Window::UnregisterSurfaceNodeListener(const sptr<IWindowSurfaceNodeListener>& listener)
{
    LOGI("Start unregister SurfaceNodeListener");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return UnregisterListener(surfaceNodeListeners_[GetWindowId()], listener);
}

WMError Window::RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    LOGI("Start register AvoidAreaChangeListener");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    WMError ret = RegisterListener(avoidAreaChangeListeners_[GetWindowId()], listener);
    return ret;
}

WMError Window::UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    LOGI("Start unregister AvoidAreaChangeListener");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    WMError ret = UnregisterListener(avoidAreaChangeListeners_[GetWindowId()], listener);
    return ret;
}

WMError Window::RegisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener)
{
    LOGI("Start register WindowChangeListener");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
}

WMError Window::UnregisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener)
{
    LOGI("Start unregister WindowChangeListener");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
}

template<typename T>
WMError Window::RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    if (listener == nullptr) {
        LOGE("listener is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (std::find(holder.begin(), holder.end(), listener) != holder.end()) {
        LOGE("Listener already registered");
        return WMError::WM_OK;
    }
    holder.emplace_back(listener);
    return WMError::WM_OK;
}

template<typename T>
WMError Window::UnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener)
{
    if (listener == nullptr) {
        LOGE("listener could not be null");
        return WMError::WM_ERROR_NULLPTR;
    }
    holder.erase(std::remove_if(holder.begin(), holder.end(),
                     [listener](sptr<T> registeredListener) { return registeredListener == listener; }),
        holder.end());
    return WMError::WM_OK;
}

void Window::RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    // stage model
    if (receiver_) {
        SetUpThreadInfo();
        auto callback = [vsyncCallback](int64_t timestamp, void*) { vsyncCallback->onCallback(timestamp, 0); };
        VSyncReceiver::FrameCallback fcb = {
            .userData_ = this,
            .callback_ = callback,
        };
        receiver_->RequestNextVSync(fcb);
        return;
    }
}

bool Window::CreateVSyncReceiver(std::shared_ptr<AppExecFwk::EventHandler> handler)
{
    if (receiver_) {
        return true;
    }
    auto& rsClient = Rosen::RSInterfaces::GetInstance();
    receiver_ = rsClient.CreateVSyncReceiver("Window_Andorid", handler);
    VsyncError ret = receiver_->Init();
    if (ret) {
        LOGE("Window_Andorid: vsync receiver init failed: %{public}d", ret);
        return false;
    }
    return true;
}

void Window::RequestNextVsync(std::function<void(int64_t, void*)> callback)
{
    if (!receiver_) {
        return;
    }
    VSyncReceiver::FrameCallback fcb = {
        .userData_ = this,
        .callback_ = callback,
    };
    receiver_->RequestNextVSync(fcb);
}

void Window::CreateSurfaceNode(void* nativeWindow)
{
    struct Rosen::RSSurfaceNodeConfig rsSurfaceNodeConfig = { .SurfaceNodeName = "arkui-x_surface",
        .additionalData = nativeWindow };
    surfaceNode_ = Rosen::RSSurfaceNode::Create(rsSurfaceNodeConfig);

    if (!uiContent_) {
        LOGW("Window Notify uiContent_ Surface Created, uiContent_ is nullptr, delay notify.");
    } else {
        LOGI("Window Notify uiContent_ Surface Created");
        uiContent_->NotifySurfaceCreated();
    }
    delayNotifySurfaceCreated_ = true;

    auto surfaceNodeListeners = GetListeners<IWindowSurfaceNodeListener>();
    for (auto& listener : surfaceNodeListeners) {
        if (listener != nullptr) {
            listener->OnSurfaceNodeCreated();
        }
    }
}

void Window::NotifySurfaceChanged(int32_t width, int32_t height, float density)
{
    rect_.width_ = width;
    rect_.height_ = height;
    NotifySizeChange(rect_);
    if (!surfaceNode_) {
        LOGE("Window Notify Surface Changed, surfaceNode_ is nullptr!");
        return;
    }
    LOGI("Window Notify Surface Changed wh:[%{public}d, %{public}d]", width, height);
    surfaceWidth_ = width;
    surfaceHeight_ = height;
    density_ = density;
    surfaceNode_->SetBoundsWidth(surfaceWidth_);
    surfaceNode_->SetBoundsHeight(surfaceHeight_);
    if (!uiContent_) {
        LOGW("Window Notify uiContent_ Surface Changed, uiContent_ is nullptr, delay notify.");
    } else {
        Ace::ViewportConfig config;
        config.SetDensity(density_);
        config.SetSize(surfaceWidth_, surfaceHeight_);
        LOGI("Window Notify uiContent_ Surface Changed %{public}s", config.ToString().c_str());
        uiContent_->UpdateViewportConfig(config, WindowSizeChangeReason::RESIZE);
    }
    delayNotifySurfaceChanged_ = true;

    auto surfaceNodeListeners = GetListeners<IWindowSurfaceNodeListener>();
    for (auto& listener : surfaceNodeListeners) {
        if (listener != nullptr) {
            listener->OnSurfaceNodeChanged(width, height, density);
        }
    }
}

void Window::NotifyKeyboardHeightChanged(int32_t height)
{
    g_KeyboardHeight = height;
    AvoidAreaChange();
    auto occupiedAreaChangeListeners = GetListeners<IOccupiedAreaChangeListener>();
    for (auto& listener : occupiedAreaChangeListeners) {
        if (listener != nullptr) {
            Rect rect = { 0, 0, 0, height };
            listener->OnSizeChange(rect, OccupiedAreaType::TYPE_INPUT);
        }
    }
}

void Window::NotifySizeChange(Rect rect)
{
    auto windowChangeListeners = GetListeners<IWindowChangeListener>();
    for (auto& listener : windowChangeListeners) {
        if (listener != nullptr) {
            listener->OnSizeChange(rect);
        }
    }
}

void Window::UpdateAvoidArea(const std::shared_ptr<Rosen::AvoidArea>& avoidArea, AvoidAreaType type)
{
    LOGD("UpdateAvoidArea WindowId:%{public}d, type:%{public}d, top:{%{public}d,%{public}d,%{public}d,%{public}d}, "
        "left:{%{public}d,%{public}d,%{public}d,%{public}d}, right:{%{public}d,%{public}d,%{public}d,%{public}d}, "
        "bottom:{%{public}d,%{public}d,%{public}d,%{public}d}", GetWindowId(),
        type, avoidArea->topRect_.posX_, avoidArea->topRect_.posY_, avoidArea->topRect_.width_,
        avoidArea->topRect_.height_, avoidArea->leftRect_.posX_, avoidArea->leftRect_.posY_,
        avoidArea->leftRect_.width_, avoidArea->leftRect_.height_, avoidArea->rightRect_.posX_,
        avoidArea->rightRect_.posY_, avoidArea->rightRect_.width_, avoidArea->rightRect_.height_,
        avoidArea->bottomRect_.posX_, avoidArea->bottomRect_.posY_, avoidArea->bottomRect_.width_,
        avoidArea->bottomRect_.height_);

    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    avoidAreaMap_[type] = *avoidArea;
    NotifyAvoidAreaChange(avoidArea, type);
}

void Window::NotifyAvoidAreaChange(const std::shared_ptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    auto avoidAreaChangeListeners = GetListeners<IAvoidAreaChangedListener>();
    for (auto& listener : avoidAreaChangeListeners) {
        if (listener != nullptr) {
            LOGD("type=%{public}u", type);
            listener->OnAvoidAreaChanged(*avoidArea, type);
        }
    }
}

void Window::NotifyTouchOutside()
{
    auto touchOutsideListeners = GetListeners<ITouchOutsideListener>();
    for (auto& listener : touchOutsideListeners) {
        if (listener != nullptr) {
            listener->OnTouchOutside();
        }
    }
}

void Window::SubWindowHide()
{
    NotifyAfterBackground();
}

void Window::NotifySurfaceDestroyed()
{
    surfaceNode_ = nullptr;

    if (!uiContent_) {
        LOGW("Window Notify Surface Destroyed, uiContent_ is nullptr, delay notify.");
        delayNotifySurfaceDestroyed_ = true;
    } else {
        LOGI("Window Notify uiContent_ Surface Destroyed");
        uiContent_->NotifySurfaceDestroyed();
    }

    auto surfaceNodeListeners = GetListeners<IWindowSurfaceNodeListener>();
    for (auto& listener : surfaceNodeListeners) {
        if (listener != nullptr) {
            listener->OnSurfaceNodeDestroyed();
        }
    }
}

void Window::WindowFocusChanged(bool hasWindowFocus)
{
    if (!uiContent_) {
        LOGW("Window::Focus uiContent_ is nullptr");
        return;
    }
    if (hasWindowFocus) {
        LOGI("Window(%{public}s): notify uiContent Focus", GetWindowName().c_str());
        uiContent_->Focus();
        NotifyAfterActive();
        isForground_ = true;
        isFocused_ = true;

        if (IsSubWindow() || subWindowMap_.count(GetWindowId()) == 0) {
            return;
        }
        auto windows = subWindowMap_.at(GetWindowId());
        for (auto const& window : windows) {
            if (!window->IsWindowShow()) {
                window->NotifyAfterInactive();
            }
        }
    } else {
        LOGI("Window(%{public}s): notify uiContent UnFocus", GetWindowName().c_str());
        uiContent_->UnFocus();
        NotifyAfterInactive();
        isForground_ = false;
        isFocused_ = false;
    }
}

void Window::Foreground()
{
    if (!uiContent_) {
        LOGW("Window::Foreground uiContent_ is nullptr");
        return;
    }
    LOGI("Window: notify uiContent Foreground");
    uiContent_->Foreground();
    NotifyAfterForeground();
    isForground_ = true;
}

void Window::Background()
{
    if (!uiContent_) {
        LOGW("Window::Background uiContent_ is nullptr");
        return;
    }
    LOGI("Window: notify uiContent Background");
    uiContent_->Background();
    NotifyAfterBackground();
    isForground_ = false;
}

bool Window::ProcessBackPressed()
{
    if (!uiContent_) {
        LOGW("Window::ProcessBackPressed uiContent_ is nullptr");
        return false;
    }
    return uiContent_->ProcessBackPressed();
}

bool Window::ProcessBasicEvent(const std::vector<Ace::TouchEvent>& touchEvents)
{
    if (!uiContent_) {
        LOGW("Window::ProcessBasicEvent uiContent_ is nullptr");
        return false;
    }
    return uiContent_->ProcessBasicEvent(touchEvents);
}

bool Window::ProcessPointerEvent(const std::vector<uint8_t>& data)
{
    if (!uiContent_) {
        LOGW("Window::ProcessPointerEvent uiContent_ is nullptr");
        return false;
    }
    std::vector<std::shared_ptr<MMI::PointerEvent>> pointerEvents;
    CreatePointerEventsFromBytes(pointerEvents, data);
    bool result = true;
    for (auto& pointerEvent : pointerEvents) {
        result &= uiContent_->ProcessPointerEvent(pointerEvent);
    }
    return result;
}

bool Window::ProcessMouseEvent(const std::vector<uint8_t>& data)
{
    if (!uiContent_) {
        LOGW("Window::ProcessMouseEvent uiContent_ is nullptr");
        return false;
    }
    return uiContent_->ProcessMouseEvent(data);
}

bool Window::ProcessKeyEvent(int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp,
    int64_t timeStampStart, int32_t source, int32_t deviceId, int32_t metaKey)
{
    if (!uiContent_) {
        LOGW("Window::ProcessKeyEvent uiContent_ is nullptr");
        return false;
    }
    Ace::KeyCode aceKeyCode = KeyCodeToAceKeyCode(keyCode);
    return uiContent_->ProcessKeyEvent(
        static_cast<int32_t>(aceKeyCode), keyAction, repeatTime, timeStamp, timeStampStart, metaKey, source, deviceId);
}

void Window::DelayNotifyUIContentIfNeeded()
{
    if (!uiContent_) {
        LOGE("Window Delay Notify uiContent_ is nullptr!");
        return;
    }
    if (delayNotifySurfaceCreated_) {
        LOGD("Window Delay Notify uiContent_ Surface Created");
        uiContent_->NotifySurfaceCreated();
    }
    if (delayNotifySurfaceChanged_) {
        LOGD("Window Delay Notify uiContent_ Surface Changed wh:[%{public}d, %{public}d]", surfaceWidth_,
            surfaceHeight_);
        Ace::ViewportConfig config;
        config.SetDensity(density_);
        config.SetSize(surfaceWidth_, surfaceHeight_);
        config.SetOrientation(surfaceHeight_ >= surfaceWidth_ ? 0 : 1);
        uiContent_->UpdateViewportConfig(config, WindowSizeChangeReason::RESIZE);
    }
    if (delayNotifySurfaceDestroyed_) {
        LOGD("Window Delay Notify uiContent_ Surface Destroyed");
        uiContent_->NotifySurfaceDestroyed();
        delayNotifySurfaceDestroyed_ = false;
    }
}

WMError Window::SetUIContent(const std::string& contentInfo, NativeEngine* engine, napi_value storage,
    bool isdistributed, AbilityRuntime::Platform::Ability* ability, bool loadContentByName)
{
    using namespace OHOS::Ace::Platform;
    (void)ability;
    if (uiContent_) {
        uiContent_->Destroy();
    }
    std::unique_ptr<UIContent> uiContent;
    uiContent = UIContent::Create(context_.get(), engine);
    if (uiContent == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }
    if (loadContentByName) {
        uiContent->InitializeByName(this, contentInfo, storage);
    } else {
        uiContent->Initialize(this, contentInfo, storage);
    }
    // make uiContent available after Initialize/Restore
    uiContent_ = std::move(uiContent);
    DelayNotifyUIContentIfNeeded();
    uiContent_->Foreground();
    return WMError::WM_OK;
}

UIContent* Window::GetUIContent()
{
    return uiContent_.get();
}

void Window::SetWindowView(JNIEnv* env, jobject windowView)
{
    if (windowView == nullptr) {
        LOGE("Window::SetWindowView: jobject of WindowView is nullptr!");
        return;
    }
    if (windowView_ != nullptr) {
        LOGW("Window::SetWindowView: windowView_ has already been set!");
        return;
    }
    windowView_ = env->NewGlobalRef(windowView);
    Ace::Platform::WindowViewJni::RegisterWindow(env, this, windowView);
}

void Window::SetSubWindowView(JNIEnv* env, jobject windowView)
{
    if (windowView == nullptr) {
        LOGE("Window::SetWindowView: jobject of WindowView is nullptr!");
        return;
    }
    windowView_ = env->NewGlobalRef(windowView);
    Ace::Platform::WindowViewJni::RegisterWindow(env, this, windowView);
    SubWindowManagerJni::RegisterSubWindow(GetWindowName(), this);
}

void Window::ReleaseWindowView()
{
    if (windowView_ == nullptr) {
        return;
    }
    auto jniEnv = Ace::Platform::JniEnvironment::GetInstance().GetJniEnv();
    Ace::Platform::WindowViewJni::UnRegisterWindow(jniEnv.get(), windowView_);
    if (IsSubWindow()) {
        SubWindowManagerJni::UnregisterSubWindow(GetWindowName());
    }
    Ace::Platform::JniEnvironment::DeleteJavaGlobalRef(windowView_);
    windowView_ = nullptr;
}

void Window::UpdateConfiguration(const std::shared_ptr<OHOS::AbilityRuntime::Platform::Configuration>& config)
{
    if (uiContent_ != nullptr) {
        LOGI("Window::UpdateConfiguration called.");
        uiContent_->UpdateConfiguration(config);
    }
}

void Window::SetUpThreadInfo()
{
    static int32_t renderTid = -1;
    if (renderTid < 0) {
        int32_t tid = -1;
        RSRenderThread::Instance().PostSyncTask([&tid]() { tid = gettid(); });
        renderTid = tid;
        bool ret = AceEnvJni::SetThreadInfo(renderTid);
        LOGI("Window::SetUpThreadInfo tid:%{public}d ret:%{public}d.", renderTid, ret);
    }
}

WMError Window::SetLayoutFullScreen(bool status)
{
    if (SubWindowManagerJni::SetWindowLayoutFullScreen(status)) {
        g_IsFullScreen = status;
        return WMError::WM_OK;
    } else {
        LOGE("Window::SetLayoutFullScreen: failed");
        return WMError::WM_ERROR_OPER_FULLSCREEN_FAILED;
    }
}

WMError Window::SetSpecificBarProperty(WindowType type, const SystemBarProperty& property)
{
    bool hide = !property.enable_;
    bool result = false;

    if (type == WindowType::WINDOW_TYPE_STATUS_BAR) {
        result = SubWindowManagerJni::SetStatusBarStatus(hide);
        if (result) {
            g_IsStatusBarHide = hide;
        }
    } else if (type == WindowType::WINDOW_TYPE_NAVIGATION_BAR) {
        result = SubWindowManagerJni::SetNavigationBarStatus(hide);
    } else if (type == WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR) {
        result = SubWindowManagerJni::SetNavigationIndicatorStatus(hide);
    } else {
        LOGE("The WindowType is not set to SpecificBarProperty. The WindowType is %{public}d", type);
    }

    std::lock_guard<std::recursive_mutex> lock(g_sysBarPropMapMutex);
    sysBarPropMap_[type] = property;

    if (result) {
        return WMError::WM_OK;
    } else {
        LOGE("Window::SetSpecificBarProperty: failed. The WindowType is %{public}d", type);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

Rect Window::GetTopRect(const Rect& safeAreaRect, const Rect& surfaceRect)
{
    Rect rect = EMPTY_RECT;
    if (safeAreaRect.posY_ > surfaceRect.posY_) {
        rect.posX_ = surfaceRect.posX_;
        rect.posY_ = surfaceRect.posY_;
        rect.width_ = surfaceRect.width_;
        rect.height_ = safeAreaRect.posY_ - surfaceRect.posY_;
    }
    return rect;
}

Rect Window::GetLeftRect(const Rect& safeAreaRect, const Rect& surfaceRect)
{
    Rect rect = EMPTY_RECT;
    if (safeAreaRect.posX_ > surfaceRect.posX_) {
        rect.posX_ = surfaceRect.posX_;
        rect.posY_ = surfaceRect.posY_;
        rect.width_ = safeAreaRect.posX_ - surfaceRect.posX_;
        rect.height_ = surfaceRect.height_;
    }
    return rect;
}

Rect Window::GetRightRect(const Rect& safeAreaRect, const Rect& surfaceRect)
{
    Rect rect = EMPTY_RECT;
    auto sumWidth = safeAreaRect.posX_ + safeAreaRect.width_;
    if (sumWidth < surfaceRect.posX_ + surfaceRect.width_) {
        rect.posX_ = sumWidth;
        rect.posY_= surfaceRect.posY_;
        rect.width_ = surfaceRect.posX_ + surfaceRect.width_ - sumWidth;
        rect.height_ = surfaceRect.height_;
    }
    return rect;
}

Rect Window::GetBottomRect(const Rect& safeAreaRect, const Rect& surfaceRect)
{
    Rect rect = EMPTY_RECT;
    auto sumHeight = safeAreaRect.posY_ + safeAreaRect.height_;
    if (sumHeight < surfaceRect.posY_ + surfaceRect.height_) {
        rect.posX_ = surfaceRect.posX_;
        rect.posY_ = sumHeight;
        rect.width_ = surfaceRect.width_;
        rect.height_ = surfaceRect.posY_ + surfaceRect.height_ - sumHeight;
    }
    return rect;
}

void Window::GetCutoutRect(const Rect& safeAreaRect, const Rect& surfaceRect, AvoidArea& avoidArea)
{
    auto cutoutBarHeight = SubWindowManagerJni::GetCutoutBarHeight();
    if (cutoutBarHeight == 0) {
        return;
    }
    auto orientationType = SubWindowManagerJni::GetScreenOrientation();
    if (orientationType == ORIENTATION_PORTRAIT) {
        avoidArea.topRect_ = GetTopRect(safeAreaRect, surfaceRect);
    } else {
        orientationType == ORIENTATION_LANDSCAPE ? avoidArea.leftRect_ = GetLeftRect(safeAreaRect, surfaceRect)
                                                 : avoidArea.rightRect_ = GetRightRect(safeAreaRect, surfaceRect);
    }
}

WMError Window::GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea)
{
    avoidArea.topRect_ = EMPTY_RECT;
    avoidArea.leftRect_ = EMPTY_RECT;
    avoidArea.rightRect_ = EMPTY_RECT;
    avoidArea.bottomRect_ = EMPTY_RECT;
    auto errorCode = WMError::WM_OK;
    const Rect safeAreaRect = SubWindowManagerJni::GetSafeArea();
    const Rect surfaceRect = SubWindowManagerJni::GetSurfaceRect();
    if (IsSubWindow()) {
        if (isFullScreen_ == false) {
            return errorCode;
        }
    }

    switch (type) {
        case AvoidAreaType::TYPE_SYSTEM:
            avoidArea.topRect_ = GetTopRect(safeAreaRect, surfaceRect);
            avoidArea.leftRect_ = GetLeftRect(safeAreaRect, surfaceRect);
            avoidArea.rightRect_ = GetRightRect(safeAreaRect, surfaceRect);
            avoidArea.bottomRect_ = GetBottomRect(safeAreaRect, surfaceRect);
            break;
        case AvoidAreaType::TYPE_CUTOUT:
            GetCutoutRect(safeAreaRect, surfaceRect, avoidArea);
            break;
        case AvoidAreaType::TYPE_SYSTEM_GESTURE:
            break;
        case AvoidAreaType::TYPE_KEYBOARD:
            if (g_KeyboardHeight > 0) {
                avoidArea.bottomRect_.posY_ = DisplayInfoJni::getDisplayHeight() - g_KeyboardHeight;
                avoidArea.bottomRect_.width_ = surfaceWidth_;
                avoidArea.bottomRect_.height_ = g_KeyboardHeight;
            }
            break;
        case AvoidAreaType::TYPE_NAVIGATION_INDICATOR:
            if (SubWindowManagerJni::GetNavigationIndicatorStatus()) {
                avoidArea.bottomRect_ = GetBottomRect(safeAreaRect, surfaceRect);
            }
            break;
        default:
            LOGE("Window::GetAvoidAreaByType, Set Invalid AvoidAreaByType. The type is %{public}d", type);
            errorCode = WMError::WM_ERROR_INVALID_PARAM;
            break;
    }
    return errorCode;
}

void Window::AvoidAreaChange()
{
    std::shared_ptr<Rosen::AvoidArea> avoidArea = std::make_shared<Rosen::AvoidArea>();
    auto WMErrorCode = WMError::WM_OK;
    LOGE("AvoidAreaChange is calling.");
    std::lock_guard<std::recursive_mutex> lock(globalMutex_);
    for (const auto& [type, avoidArea_] : avoidAreaMap_) {
        WMErrorCode = GetAvoidAreaByType(type, *avoidArea);
        if (WMErrorCode != WMError::WM_OK) {
            LOGE("Window::AvoidAreaChange, GetAvoidAreaByType failed. The type is %{public}d", type);
            return;
        }
        LOGE("GetAvoidAreaByType is over.");
        if (avoidArea_ != *avoidArea) {
            UpdateAvoidArea(avoidArea, type);
            LOGE("UpdateAvoidArea is over.");
        }
    }
}

WMError Window::UpdateSystemBarProperties(
    const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
    const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags)
{
    if (IsSubWindow()) {
        return WMError::WM_OK;
    }
    for (auto& [systemBarType, systemBarPropertyFlag] : systemBarPropertyFlags) {
        if (systemBarProperties.find(systemBarType) == systemBarProperties.end()) {
            LOGI("Window::UpdateSystemBarProperties system bar type is invalid");
            return WMError::WM_DO_NOTHING;
        }
        auto property = GetSystemBarPropertyByType(systemBarType);
        property.enable_ = systemBarPropertyFlag.enableFlag ?
            systemBarProperties.at(systemBarType).enable_ : property.enable_;
        
        property.backgroundColor_ = systemBarPropertyFlag.backgroundColorFlag ?
            systemBarProperties.at(systemBarType).backgroundColor_ : property.backgroundColor_;
        property.contentColor_ = systemBarPropertyFlag.contentColorFlag ?
            systemBarProperties.at(systemBarType).contentColor_ : property.contentColor_;
        property.enableAnimation_ = systemBarPropertyFlag.enableAnimationFlag ?
            systemBarProperties.at(systemBarType).enableAnimation_ : property.enableAnimation_;
        if (systemBarPropertyFlag.enableFlag) {
            property.settingFlag_ |= SystemBarSettingFlag::ENABLE_SETTING;
        }
        if (systemBarPropertyFlag.backgroundColorFlag || systemBarPropertyFlag.contentColorFlag) {
            property.settingFlag_ |= SystemBarSettingFlag::COLOR_SETTING;
        }
        if (systemBarPropertyFlag.enableFlag || systemBarPropertyFlag.backgroundColorFlag ||
            systemBarPropertyFlag.contentColorFlag || systemBarPropertyFlag.enableAnimationFlag) {
            if (systemBarType == WindowType::WINDOW_TYPE_STATUS_BAR) {
                SubWindowManagerJni::SetStatusBar(
                    property.backgroundColor_, property.contentColor_);
            } else if (systemBarType == WindowType::WINDOW_TYPE_NAVIGATION_BAR) {
                SubWindowManagerJni::SetNavigationBar(
                    property.backgroundColor_, property.contentColor_);
            } else {
                LOGE("The WindowType is not set to UpdateSystemBarProperties. The WindowType is %{public}d",
                    systemBarType);
            }
        }
        std::lock_guard<std::recursive_mutex> lock(g_sysBarPropMapMutex);
        sysBarPropMap_[systemBarType] = property;
    }
    return WMError::WM_OK;
}

WMError Window::SetWindowPrivacyMode(bool isPrivacyMode)
{
    if (IsSubWindow()) {
        return WMError::WM_OK;
    }
    LOGI("Window::SetWindowPrivacyMode called. isPrivacyMode=%d", isPrivacyMode);

    bool result = SubWindowManagerJni::SetWindowPrivacyMode(isPrivacyMode);
    if (result) {
        return WMError::WM_OK;
    } else {
        LOGI("Window::SetWindowPrivacyMode: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

WMError Window::Hide()
{
    LOGI("Window::Hide called.");
    if (GetWindowName().empty()) {
        LOGI("Window::Hide called failed due to null option");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    bool result = SubWindowManagerJni::Hide(GetWindowName());
    if (result) {
        NotifyAfterBackground();
        return WMError::WM_OK;
    } else {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

WMError Window::SetFocusable(bool isFocusable)
{
    LOGI("Window::SetFocusable called. isFocusable=%{public}d", isFocusable);
    bool result = SubWindowManagerJni::SetFocusable(this->GetWindowName(), isFocusable);
    if (result) {
        LOGI("Window::SetFocusable: success");
        return WMError::WM_OK;
    } else {
        LOGI("Window::SetFocusable: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

WMError Window::SetTouchable(bool isTouchable)
{
    LOGI("Window::SetTouchable called. isTouchable=%{public}d", isTouchable);
    bool result = SubWindowManagerJni::SetTouchable(GetWindowName(), isTouchable);
    if (result) {
        LOGI("Window::SetTouchable: success");
        return WMError::WM_OK;
    } else {
        LOGI("Window::SetTouchable: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

bool Window::RequestFocus()
{
    bool result = SubWindowManagerJni::RequestFocus(GetWindowName());
    LOGI("Window::RequestFocus return %{public}d", result);
    return result;
}

bool Window::IsFocused()
{
    LOGI("Window::IsFocused called(%{public}s). isFocused_=%{public}d", GetWindowName().c_str(), isFocused_);
    return isFocused_;
}

WMError Window::SetTouchHotAreas(const std::vector<Rect>& rects)
{
    LOGI("Window::SetTouchHotAreas called");
    if (rects.empty()) {
        LOGI("rects is empty");
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    bool result = SubWindowManagerJni::SetTouchHotAreas(GetWindowName(), rects);
    if (result) {
        LOGI("Window::SetTouchable: success");
        return WMError::WM_OK;
    } else {
        LOGI("Window::SetTouchable: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

WMError Window::SetFullScreen(bool status)
{
    LOGI("Window::SetFullScreen called. status=%{public}d", status);
    bool result = SubWindowManagerJni::SetFullScreen(GetWindowName(), status);
    if (result) {
        LOGI("Window::SetFullScreen: success");
        return WMError::WM_OK;
    } else {
        LOGI("Window::SetFullScreen: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

WMError Window::SetOnTop(bool status)
{
    LOGI("Window::SetOnTop called. status=%{public}d", status);
    bool result = SubWindowManagerJni::SetOnTop(GetWindowName(), status);
    if (result) {
        LOGI("Window::SetOnTop: success");
        return WMError::WM_OK;
    } else {
        LOGI("Window::SetOnTop: failed");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
}

} // namespace OHOS::Rosen
