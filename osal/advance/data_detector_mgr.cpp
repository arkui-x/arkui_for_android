/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "core/common/ai/data_detector_mgr.h"

#include "core/common/ai/data_detector_default.h"
namespace OHOS::Ace {
DataDetectorMgr& DataDetectorMgr::GetInstance()
{
    static DataDetectorMgr instance;
    return instance;
}

DataDetectorMgr::DataDetectorMgr() {}

bool DataDetectorMgr::IsDataDetectorSupported()
{
    return false;
}

void DataDetectorMgr::GetAIEntityMenu(TextDataDetectResult& textDataDetectResult) {}

void DataDetectorMgr::DataDetect(const TextDataDetectInfo& info, const TextDetectResultFunc& resultFunc) {}

void DataDetectorMgr::AdjustCursorPosition(
    int32_t& caretPos, const std::string& content, TimeStamp& lastAiPosTimeStamp, const TimeStamp& lastClickTimeStamp)
{
    caretPos = GetCursorPosition(content, caretPos);
}

void DataDetectorMgr::AdjustWordSelection(int32_t& caretPos, const std::string& content, int32_t& start, int32_t& end)
{
    auto ret = GetWordSelection(content, caretPos);
    start = ret[0];
    end = ret[1];
}

std::vector<int8_t> DataDetectorMgr::GetWordSelection(const std::string& text, int8_t offset)
{
    return std::vector<int8_t> { -1, -1 };
}

int8_t DataDetectorMgr::GetCursorPosition(const std::string& text, int8_t offset)
{
    return -1;
}
} // namespace OHOS::Ace
