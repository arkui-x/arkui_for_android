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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_EDITING_TEXT_INPUT_CONNECTION_IMPL_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_EDITING_TEXT_INPUT_CONNECTION_IMPL_H

#include "core/common/ime/text_input_client.h"
#include "core/common/ime/text_input_connection.h"

namespace OHOS::Ace::Platform {

class TextInputConnectionImpl : public TextInputConnection {
public:
    explicit TextInputConnectionImpl(const WeakPtr<TextInputClient>& client, const RefPtr<TaskExecutor>& taskExecutor);
    ~TextInputConnectionImpl() override = default;

    // Implement TextInputConnection
    void Show(bool isFocusViewChanged, int32_t instanceId) override;
    void SetEditingState(const TextEditingValue& value, int32_t instanceId, bool needFireChangeEvent = true) override;
    void Close(int32_t instanceId) override;

private:
    TextInputConnectionImpl() = delete;
    bool Attached();
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_EDITING_TEXT_INPUT_CONNECTION_IMPL_H