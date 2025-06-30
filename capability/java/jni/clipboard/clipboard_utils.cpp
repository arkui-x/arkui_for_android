/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "adapter/android/capability/java/jni/clipboard/clipboard_utils.h"

#ifdef USE_M133_SKIA
#include "src/base/SkBase64.h"
#else
#include "include/utils/SkBase64.h"
#endif

namespace OHOS::Ace {
std::vector<uint8_t> ClipBoardUtils::DecodeCustomData(const std::string& encodedStr)
{
    size_t decodedLength;
    std::vector<uint8_t> decodedData;
    SkBase64::Error error = SkBase64::Decode(encodedStr.c_str(), encodedStr.size(), nullptr, &decodedLength);
    if (error != SkBase64::Error::kNoError) {
        return {};
    }
    decodedData.resize(decodedLength);
    if (SkBase64::Decode(encodedStr.c_str(), encodedStr.size(), decodedData.data(), &decodedLength) !=
        SkBase64::kNoError) {
        return {};
    }
    return decodedData;
}

std::string ClipBoardUtils::EncodeCustomData(const std::vector<uint8_t>& data)
{
    size_t encodedLength = SkBase64::Encode(data.data(), data.size(), nullptr, nullptr);
    std::string encodedData(encodedLength, '\0');
    SkBase64::Encode(data.data(), data.size(), reinterpret_cast<void*>(const_cast<char*>(encodedData.data())),
        nullptr);
    return encodedData;
}
} // namespace OHOS::Ace