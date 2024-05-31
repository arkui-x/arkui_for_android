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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_EDITING_TEXT_INPUT_PLUGIN_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_EDITING_TEXT_INPUT_PLUGIN_H

#include "base/utils/noncopyable.h"

#include "core/common/ime/text_input.h"

namespace OHOS::Ace::Platform {

class TextInputPlugin final : public TextInput {
public:
    TextInputPlugin() = default;
    ~TextInputPlugin() = default;

    // Implement TextInput
    RefPtr<TextInputConnection> Attach(const WeakPtr<TextInputClient>& client,
        const TextInputConfiguration& config, const RefPtr<TaskExecutor>& taskExecutor, int32_t instanceId) override;

private:
    ACE_DISALLOW_COPY_AND_MOVE(TextInputPlugin);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_EDITING_TEXT_INPUT_PLUGIN_H