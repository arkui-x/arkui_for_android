/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "core/common/ime/input_method_manager.h"

namespace OHOS::Ace {
std::unique_ptr<InputMethodManager> InputMethodManager::instance_ = nullptr;
std::mutex InputMethodManager::mtx_;

InputMethodManager* InputMethodManager::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (instance_ == nullptr) {
            instance_.reset(new InputMethodManager);
        }
    }
    return instance_.get();
}

void InputMethodManager::OnFocusNodeChange(const RefPtr<NG::FrameNode>& curFocusNode, FocusReason focusReason) {}

void InputMethodManager::ProcessKeyboardInWindowScene(const RefPtr<NG::FrameNode>& curFocusNode) {}

void InputMethodManager::ProcessKeyboard(const RefPtr<NG::FrameNode>& curFocusNode) {}

void InputMethodManager::SetWindowFocus(bool windowFocus) {}

bool InputMethodManager::NeedSoftKeyboard() const
{
    return false;
}

void InputMethodManager::CloseKeyboard(bool disableNeedToRequestKeyboard) {}

void InputMethodManager::CloseKeyboard(const RefPtr<NG::FrameNode>& focusNode) {}

void InputMethodManager::CloseKeyboardInPipelineDestroy() {}

void InputMethodManager::HideKeyboardAcrossProcesses() {}

void InputMethodManager::ProcessModalPageScene() {}
} // namespace OHOS::Ace