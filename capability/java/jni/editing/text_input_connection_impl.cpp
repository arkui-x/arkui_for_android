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

#include "adapter/android/capability/java/jni/editing/text_input_connection_impl.h"

#include "adapter/android/capability/java/jni/editing/text_input_client_handler.h"
#include "adapter/android/capability/java/jni/editing/text_input_jni.h"
#include "base/log/event_report.h"
#include "base/log/log.h"

namespace OHOS::Ace::Platform {

constexpr uint32_t KEYBOARD_SHOW_DELAY_TIME = 50;

TextInputConnectionImpl::TextInputConnectionImpl(
    const WeakPtr<TextInputClient>& client, const RefPtr<TaskExecutor>& taskExecutor)
    : TextInputConnection(client, taskExecutor)
{}

void TextInputConnectionImpl::Show(bool isFocusViewChanged, int32_t instanceId)
{
    if (taskExecutor_ && Attached()) {
        taskExecutor_->PostDelayedTask(
            [instanceId, isFocusViewChanged] {
                TextInputJni::ShowTextInput(isFocusViewChanged, instanceId);
            },
            TaskExecutor::TaskType::PLATFORM, KEYBOARD_SHOW_DELAY_TIME);
    }
}

void TextInputConnectionImpl::SetEditingState(
    const TextEditingValue& value, int32_t instanceId, bool needFireChangeEvent)
{
    if (taskExecutor_ && Attached()) {
        taskExecutor_->PostTask(
            [value, instanceId, needFireChangeEvent] {
                TextInputJni::SetEditingState(value, instanceId, needFireChangeEvent);
            },
            TaskExecutor::TaskType::PLATFORM);
    }
}

void TextInputConnectionImpl::Close(int32_t instanceId)
{
    if (taskExecutor_ && Attached()) {
        taskExecutor_->PostSyncTask(
            [instanceId] {
                TextInputJni::ClearClient(instanceId);
                TextInputJni::HideTextInput(instanceId);
            },
            TaskExecutor::TaskType::PLATFORM);
    }
    TextInputClientHandler::GetInstance().SetCurrentConnection(nullptr);

    if (Attached()) {
        LOGE("text input connection close failed");
        EventReport::SendComponentException(ComponentExcepType::TEXT_INPUT_CONNECTION_CLOSE_ERR);
    }
}

bool TextInputConnectionImpl::Attached()
{
    // Should be called on AceUI thread.
    return TextInputClientHandler::GetInstance().ConnectionIsCurrent(this);
}

} // namespace OHOS::Ace::Platform
