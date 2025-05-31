/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_CLIPBOARD_UTILS_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_CLIPBOARD_UTILS_H
#include <string>
#include <vector>
namespace OHOS::Ace {
class ClipBoardUtils {
public:
    static std::string EncodeCustomData(const std::vector<uint8_t>& buf);
    static std::vector<uint8_t> DecodeCustomData(const std::string& encodedString);
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_PASTE_DATA_IMPL_H