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
#include "core/common/ai/data_detector_adapter.h"

namespace OHOS::Ace {
void DataDetectorAdapter::InitTextDetect(int32_t startPos, std::string detectText) {}
void DataDetectorAdapter::SetTextDetectTypes(const std::string& types) {}
void DataDetectorAdapter::ParseAIResult(const TextDataDetectResult& result, int32_t startPos) {}
void DataDetectorAdapter::ParseAIJson(
    const std::unique_ptr<JsonValue>& jsonValue, TextDataDetectType type, int32_t startPos) {}
void DataDetectorAdapter::StartAITask() {}

bool DataDetectorAdapter::ShowUIExtensionMenu(
    const AISpan& aiSpan, NG::RectF aiRect, const RefPtr<NG::FrameNode>& targetNode)
{
    return true;
}

void DataDetectorAdapter::ResponseBestMatchItem(const AISpan& aiSpan) {}

std::function<void(const AAFwk::WantParams&)> DataDetectorAdapter::GetOnReceive(
    const RefPtr<NG::FrameNode>& uiExtNode, NG::RectF aiRect, const RefPtr<NG::FrameNode>& targetNode)
{
    return [](const AAFwk::WantParams& wantParams) {};
}
} // namespace OHOS::Ace
