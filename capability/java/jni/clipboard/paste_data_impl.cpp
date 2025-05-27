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
#include "adapter/android/capability/java/jni/clipboard/paste_data_impl.h"

#include "adapter/android/capability/java/jni/clipboard/clipboard_utils.h"
#include "base/json/json_util.h"
#include "frameworks/base/utils/utils.h"

namespace OHOS::Ace {
namespace {
static const char *CLIPBOARD_RECORD_TYPE_TEXT = "text";
static const char *CLIPBOARD_RECORD_TYPE_TYPE = "type";
static const char *CLIPBOARD_RECORD_TYPE_DATA = "data";
static const char *CLIPBOARD_RECORD_TYPE_ISMULTI = "isMulti";
static const char *CLIPBOARD_RECORD_TYPE_RECORDS = "pasteDataRecords";
}
void PasteDataRecord::SetText(const std::string& text)
{
    text_ = text;
}

std::string PasteDataRecord::GetText() const
{
    return text_;
}

void PasteDataRecord::AddType(std::string type)
{
    type_.emplace_back(type);
}

std::string PasteDataRecord::GetType() const
{
    if (type_.size() > 0) {
        return type_.at(0);
    } else {
        return "";
    }
}

std::vector<std::string> PasteDataRecord::GetAllTypes() const
{
    return type_;
}

void PasteDataRecord::SetIsMulti(bool isMulti)
{
    isMulti_ = isMulti;
}

bool PasteDataRecord::GetIsMulti() const
{
    return isMulti_;
}

void PasteDataRecord::SetUri(const std::string& uri)
{
    uri_ = uri;
}

std::string PasteDataRecord::GetUri() const
{
    return uri_;
}

void PasteDataRecord::SetSpanString(const std::vector<uint8_t>& data)
{
    spanStringData_ = data;
}

std::vector<uint8_t> PasteDataRecord::GetSpanString() const
{
    return spanStringData_;
}

bool PasteDataRecord::IsMultiType()
{
    return type_.size() > 1 ? true : false;
}

void PasteData::AddPasteDataRecord(const std::shared_ptr<PasteDataRecord>& pasteDataRecord)
{
    pasteDataRecords_.push_back(pasteDataRecord);
}

std::string PasteData::ToJsonString()
{
    auto jsonResult = JsonUtil::Create(true);
    CHECK_NULL_RETURN(jsonResult, "{}");
    auto jsonArray = JsonUtil::CreateArray(true);
    CHECK_NULL_RETURN(jsonArray, "{}");
    for (const auto &record : pasteDataRecords_) {
        auto jsonRecord = JsonUtil::Create();
        CHECK_NULL_RETURN(jsonRecord, "{}");
        if (record->IsMultiType() && record->GetAllTypes().size() > 1) {
            jsonRecord->Put(CLIPBOARD_RECORD_TYPE_ISMULTI, true);
            jsonRecord->Put(CLIPBOARD_RECORD_TYPE_TYPE, CLIPBOARD_SPAN_STRING_TYPE.c_str());
            jsonRecord->Put(CLIPBOARD_RECORD_TYPE_TEXT, record->GetText().c_str());
            jsonRecord->Put(CLIPBOARD_RECORD_TYPE_DATA,
                ClipBoardUtils::EncodeCustomData(record->GetSpanString()).c_str());
        } else if (record->IsMultiType()) {
            jsonRecord->Put(CLIPBOARD_RECORD_TYPE_ISMULTI, true);
            if (record->GetType() == CLIPBOARD_URI_TYPE) {
                jsonRecord->Put(CLIPBOARD_RECORD_TYPE_TYPE, CLIPBOARD_URI_TYPE.c_str());
                jsonRecord->Put(CLIPBOARD_RECORD_TYPE_DATA, record->GetUri().c_str());
            }
        } else {
            jsonRecord->Put(CLIPBOARD_RECORD_TYPE_ISMULTI, false);
            if (record->GetType() == CLIPBOARD_TEXT_TYPE) {
                jsonRecord->Put(CLIPBOARD_RECORD_TYPE_TYPE, CLIPBOARD_TEXT_TYPE.c_str());
                jsonRecord->Put(CLIPBOARD_RECORD_TYPE_TEXT, record->GetText().c_str());
            } else if (record->GetType() == CLIPBOARD_SPAN_STRING_TYPE) {
                jsonRecord->Put(CLIPBOARD_RECORD_TYPE_TYPE, CLIPBOARD_SPAN_STRING_TYPE.c_str());
                jsonRecord->Put(CLIPBOARD_RECORD_TYPE_DATA,
                    ClipBoardUtils::EncodeCustomData(record->GetSpanString()).c_str());
            } else if (record->GetType() == CLIPBOARD_URI_TYPE) {
                jsonRecord->Put(CLIPBOARD_RECORD_TYPE_TYPE, CLIPBOARD_URI_TYPE.c_str());
                jsonRecord->Put(CLIPBOARD_RECORD_TYPE_DATA, record->GetUri().c_str());
            } else {
                LOGW("PasteData::ToJsonString: Unsupported data type.");
                continue;
            }
        }
        jsonArray->Put(jsonRecord);
    }
    jsonResult->Put(CLIPBOARD_RECORD_TYPE_RECORDS, jsonArray);
    return jsonResult->ToString();
}

void PasteData::FromJsonString(std::string jsonString)
{
    auto jsonValue = JsonUtil::ParseJsonString(jsonString.c_str());
    CHECK_NULL_VOID(jsonValue);
    auto jsonArray = jsonValue->GetValue(CLIPBOARD_RECORD_TYPE_RECORDS);
    CHECK_NULL_VOID(jsonArray);
    int32_t arraySize = jsonArray->GetArraySize();
    for (int i = 0; i < arraySize; i++) {
        auto pasteDataRecord = std::make_shared<PasteDataRecord>();
        CHECK_NULL_VOID(pasteDataRecord);
        auto jsonRecord = jsonArray->GetArrayItem(i);
        CHECK_NULL_VOID(jsonRecord);
        if (jsonRecord->GetBool(CLIPBOARD_RECORD_TYPE_ISMULTI)) {
            pasteDataRecord->SetIsMulti(true);
            if (jsonRecord->GetString(CLIPBOARD_RECORD_TYPE_TYPE) == CLIPBOARD_SPAN_STRING_TYPE) {
                pasteDataRecord->AddType(CLIPBOARD_SPAN_STRING_TYPE);
                pasteDataRecord->AddType(CLIPBOARD_TEXT_TYPE);
                pasteDataRecord->SetSpanString(
                    ClipBoardUtils::DecodeCustomData(jsonRecord->GetString(CLIPBOARD_RECORD_TYPE_DATA)));
                pasteDataRecord->SetText(jsonRecord->GetString(CLIPBOARD_RECORD_TYPE_TEXT));
            } else if (jsonRecord->GetString(CLIPBOARD_RECORD_TYPE_TYPE) == CLIPBOARD_URI_TYPE) {
                pasteDataRecord->AddType(CLIPBOARD_URI_TYPE);
                pasteDataRecord->SetUri(jsonRecord->GetString(CLIPBOARD_RECORD_TYPE_DATA));
            } else {
                LOGW("PasteData::FromJsonString: Unsupported data type.");
                continue;
            }
        } else {
            pasteDataRecord->SetIsMulti(false);
            if (jsonRecord->GetString(CLIPBOARD_RECORD_TYPE_TYPE) == CLIPBOARD_TEXT_TYPE) {
                pasteDataRecord->AddType(CLIPBOARD_TEXT_TYPE);
                pasteDataRecord->SetText(jsonRecord->GetString(CLIPBOARD_RECORD_TYPE_TEXT));
            } else if (jsonRecord->GetString(CLIPBOARD_RECORD_TYPE_TYPE) == CLIPBOARD_SPAN_STRING_TYPE) {
                pasteDataRecord->AddType(CLIPBOARD_SPAN_STRING_TYPE);
                pasteDataRecord->SetSpanString(
                    ClipBoardUtils::DecodeCustomData(jsonRecord->GetString(CLIPBOARD_RECORD_TYPE_DATA)));
            } else if (jsonRecord->GetString(CLIPBOARD_RECORD_TYPE_TYPE) == CLIPBOARD_URI_TYPE) {
                pasteDataRecord->AddType(CLIPBOARD_URI_TYPE);
                pasteDataRecord->SetUri(jsonRecord->GetString(CLIPBOARD_RECORD_TYPE_DATA));
            } else {
                LOGW("PasteData::FromJsonString: Unsupported data type.");
                continue;
            }
        }
        this->pasteDataRecords_.emplace_back(pasteDataRecord);
    }
}

std::vector<std::shared_ptr<PasteDataRecord>> PasteData::AllRecords() const
{
    return this->pasteDataRecords_;
}

void PasteDataImpl::SetUnifiedData(std::shared_ptr<PasteData> pasteData)
{
    pasteData_ = pasteData;
}

std::shared_ptr<PasteData> PasteDataImpl::GetPasteDataData()
{
    if (pasteData_ == nullptr) {
        pasteData_ = std::make_shared<PasteData>();
    }
    return pasteData_;
}
} // namespace OHOS::Ace