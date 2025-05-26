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
#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_PASTE_DATA_IMPL_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_PASTE_DATA_IMPL_H

#include <memory>
#include <string>
#include <vector>

#include "core/common/clipboard/paste_data.h"

namespace OHOS::Ace {
static const std::string CLIPBOARD_TEXT_TYPE = "0";
static const std::string CLIPBOARD_IMAGE_TYPE = "1";
static const std::string CLIPBOARD_URI_TYPE = "2";
static const std::string CLIPBOARD_SPAN_STRING_TYPE = "3";
class PasteDataRecord {
public:
    PasteDataRecord() = default;
    ~PasteDataRecord() = default;

    void SetText(const std::string& text);
    std::string GetText() const;
    void AddType(std::string type);
    std::string GetType() const;
    std::vector<std::string> GetAllTypes() const;
    void SetIsMulti(bool isMulti);
    bool GetIsMulti() const;
    void SetUri(const std::string& uri);
    std::string GetUri() const;
    void SetSpanString(const std::vector<uint8_t>& data);
    std::vector<uint8_t> GetSpanString() const;
    bool IsMultiType();

private:
    std::string text_;
    std::vector<std::string> type_;
    bool isMulti_ = false;
    std::string uri_;
    std::vector<uint8_t> spanStringData_;
};
class PasteData {
public:
    PasteData() = default;
    ~PasteData() = default;

    void AddPasteDataRecord(const std::shared_ptr<PasteDataRecord>& pasteDataRecord);
    std::vector<std::shared_ptr<PasteDataRecord>> AllRecords() const;
    std::string ToJsonString();
    void FromJsonString(std::string);
private:
    std::vector<std::shared_ptr<PasteDataRecord>> pasteDataRecords_;
};

class PasteDataImpl : public PasteDataMix {
    DECLARE_ACE_TYPE(PasteDataImpl, PasteDataMix);

public:
    PasteDataImpl() = default;
    ~PasteDataImpl() = default;

    std::shared_ptr<PasteData> GetPasteDataData();
    void SetUnifiedData(std::shared_ptr<PasteData> pasteData);

private:
    std::shared_ptr<PasteData> pasteData_;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_PASTE_DATA_IMPL_H