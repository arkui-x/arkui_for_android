/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "core/common/ai/ai_write_adapter.h"

namespace OHOS::Ace {
bool AIWriteAdapter::IsSentenceBoundary(const char16_t value) { return false; }
uint32_t AIWriteAdapter::GetSelectLengthOnlyText(const std::u16string& content) { return 0; }
void AIWriteAdapter::CloseModalUIExtension() {}
void AIWriteAdapter::ShowModalUIExtension(const AIWriteInfo& info,
    std::function<void(std::vector<uint8_t>&)> resultCallback) {}
std::vector<uint8_t> GetBufferParam(const std::string& key, const AAFwk::WantParams& wantParams) { return {}; }
bool AIWriteAdapter::GetBoolParam(const std::string& key, const AAFwk::WantParams& wantParams) { return false; }

} // namespace OHOS::Ace