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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_EDITING_TEXT_INPUT_CLIENT_HANDLER_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_EDITING_TEXT_INPUT_CLIENT_HANDLER_H

#include "base/memory/referenced.h"
#include "base/utils/singleton.h"

#include "core/common/ime/text_input_connection.h"

namespace OHOS::Ace::Platform {

class TextInputClientHandler : public Singleton<TextInputClientHandler> {
    DECLARE_SINGLETON(TextInputClientHandler);

public:
    void SetCurrentConnection(const RefPtr<TextInputConnection>& currentConnection)
    {
        currentConnection_ = currentConnection;
    }

    bool ConnectionIsCurrent(const TextInputConnection* connection) const
    {
        // RefPtr has already overload operator == with raw pointer.
        return currentConnection_ == connection;
    }

    void UpdateEditingValue(
        const int32_t clientId, const std::shared_ptr<TextEditingValue>& value, bool needFireChangeEvent = true);
    void PerformAction(const int32_t clientId, const TextInputAction action);
    void UpdateInputFilterErrorText(const int32_t clientId, const std::string& errorText);

private:
    // All operations must be called on Ace UI thread.
    RefPtr<TextInputConnection> currentConnection_;
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_EDITING_TEXT_INPUT_CLIENT_HANDLER_H