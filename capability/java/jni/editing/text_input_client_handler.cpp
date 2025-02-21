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

#include "adapter/android/capability/java/jni/editing/text_input_client_handler.h"

namespace OHOS::Ace::Platform {

TextInputClientHandler::TextInputClientHandler() = default;

TextInputClientHandler::~TextInputClientHandler() = default;

void TextInputClientHandler::UpdateEditingValue(
    const int32_t clientId, const std::shared_ptr<TextEditingValue>& value, bool needFireChangeEvent) const
{
    TextInputConnection* connection = GetConnectionByClientId(clientId);
    if (connection) {
        auto weak = AceType::WeakClaim(connection);
        connection->GetTaskExecutor()->PostTask(
            [weak, value, needFireChangeEvent]() {
                auto currentConnection = weak.Upgrade();
                if (currentConnection == nullptr) {
                    LOGE("currentConnection is nullptr");
                    return;
                }
                auto client = currentConnection->GetClient();
                if (client) {
                    client->UpdateEditingValue(value, needFireChangeEvent);
                }
            },
            TaskExecutor::TaskType::UI, "ArkUI-XTextInputClientHandlerUpdateEditingValue");
    }
}

void TextInputClientHandler::PerformAction(const int32_t clientId, const TextInputAction action)
{
    TextInputConnection* connection = GetConnectionByClientId(clientId);
    if (connection) {
        auto weak = AceType::WeakClaim(connection);
        connection->GetTaskExecutor()->PostTask(
            [weak, action] {
                auto currentConnection = weak.Upgrade();
                if (currentConnection == nullptr) {
                    LOGE("currentConnection is nullptr");
                    return;
                }
                auto client = currentConnection->GetClient();
                if (client) {
                    client->PerformAction(action, true);
                }
            },
            TaskExecutor::TaskType::UI, "ArkUI-XTextInputClientHandlerPerformAction");
    }
}

void TextInputClientHandler::UpdateInputFilterErrorText(const int32_t clientId, const std::string& errorText)
{
    TextInputConnection* connection = GetConnectionByClientId(clientId);
    if (connection) {
        auto weak = AceType::WeakClaim(connection);
        connection->GetTaskExecutor()->PostTask(
            [weak, errorText] {
                auto currentConnection = weak.Upgrade();
                if (currentConnection == nullptr) {
                    LOGE("currentConnection is nullptr");
                    return;
                }
                auto client = currentConnection->GetClient();
                if (client) {
                    client->UpdateInputFilterErrorText(errorText);
                }
            },
            TaskExecutor::TaskType::UI, "ArkUI-XTextInputClientHandlerUpdateInputFilterErrorText");
    }
}

void TextInputClientHandler::NotifyKeyboardClosedByUser(const int32_t clientId)
{
    TextInputConnection* connection = GetConnectionByClientId(clientId);
    if (connection) {
        auto weak = AceType::WeakClaim(connection);
        connection->GetTaskExecutor()->PostTask(
            [weak] {
                auto currentConnection = weak.Upgrade();
                if (currentConnection == nullptr) {
                    LOGE("currentConnection is nullptr");
                    return;
                }
                auto client = currentConnection->GetClient();
                if (client) {
                    client->NotifyKeyboardClosedByUser();
                }
            },
            TaskExecutor::TaskType::UI, "ArkUI-XTextInputClientHandlerNotifyKeyboardClosedByUser");
    }
}
} // namespace OHOS::Ace::Platform
