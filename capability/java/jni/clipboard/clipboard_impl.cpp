/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "adapter/android/capability/java/jni/clipboard/clipboard_impl.h"

#include "adapter/android/capability/java/jni/clipboard/clipboard_jni.h"
#include "adapter/android/capability/java/jni/clipboard/multiType_record_impl.h"
#include "adapter/android/capability/java/jni/clipboard/paste_data_impl.h"
#include "frameworks/base/utils/utils.h"

namespace OHOS::Ace::Platform {
void ClipboardImpl::AddPixelMapRecord(const RefPtr<PasteDataMix>& pasteData, const RefPtr<PixelMap>& pixmap) {}
void ClipboardImpl::AddImageRecord(const RefPtr<PasteDataMix>& pasteData, const std::string& uri) {}
void ClipboardImpl::AddTextRecord(const RefPtr<PasteDataMix>& pasteData, const std::string& selectedStr)
{
    auto peData = AceType::DynamicCast<PasteDataImpl>(pasteData);
    CHECK_NULL_VOID(peData);
    auto peDataRecord = std::make_shared<PasteDataRecord>();
    CHECK_NULL_VOID(peDataRecord);
    peDataRecord->AddType(CLIPBOARD_TEXT_TYPE);
    peDataRecord->SetText(selectedStr);
    peData->GetPasteDataData()->AddPasteDataRecord(peDataRecord);
}

void ClipboardImpl::AddSpanStringRecord(const RefPtr<PasteDataMix>& pasteData, std::vector<uint8_t>& data)
{
    auto peData = AceType::DynamicCast<PasteDataImpl>(pasteData);
    CHECK_NULL_VOID(peData);
    auto peDataRecord = std::make_shared<PasteDataRecord>();
    CHECK_NULL_VOID(peDataRecord);
    peDataRecord->AddType(CLIPBOARD_SPAN_STRING_TYPE);
    peDataRecord->SetSpanString(data);
    peData->GetPasteDataData()->AddPasteDataRecord(peDataRecord);
}

void ClipboardImpl::AddMultiTypeRecord(const RefPtr<PasteDataMix>& pasteData,
    const RefPtr<MultiTypeRecordMix>& multiTypeRecord)
{
    auto peData = AceType::DynamicCast<PasteDataImpl>(pasteData);
    CHECK_NULL_VOID(peData);
    auto multiTypeRecordImpl = AceType::DynamicCast<MultiTypeRecordImpl>(multiTypeRecord);
    CHECK_NULL_VOID(multiTypeRecordImpl);
    auto peDataRecord = std::make_shared<PasteDataRecord>();
    CHECK_NULL_VOID(peDataRecord);
    peDataRecord->SetIsMulti(true);
    if (!multiTypeRecordImpl->GetPlainText().empty()) {
        peDataRecord->AddType(CLIPBOARD_TEXT_TYPE);
        peDataRecord->SetText(multiTypeRecordImpl->GetPlainText());
    }
    if (!multiTypeRecordImpl->GetUri().empty()) {
        peDataRecord->AddType(CLIPBOARD_URI_TYPE);
        peDataRecord->SetUri(multiTypeRecordImpl->GetUri());
    }
    if (!multiTypeRecordImpl->GetSpanStringBuffer().empty()) {
        peDataRecord->AddType(CLIPBOARD_SPAN_STRING_TYPE);
        peDataRecord->SetSpanString(multiTypeRecordImpl->GetSpanStringBuffer());
    }
    if (!multiTypeRecordImpl->GetPixelMap()) {
        peData->GetPasteDataData()->AddPasteDataRecord(peDataRecord);
    }
}

void ClipboardImpl::SetData(const RefPtr<PasteDataMix>& pasteData, CopyOptions copyOption)
{
    CHECK_NULL_VOID(taskExecutor_);
    auto peData = AceType::DynamicCast<PasteDataImpl>(pasteData);
    CHECK_NULL_VOID(peData);
    taskExecutor_->PostTask([peData] { ClipboardJni::SetMultiTypeData(peData->GetPasteDataData()->ToJsonString()); },
        TaskExecutor::TaskType::PLATFORM, "ArkUI-XClipboardImplSetData");
}

void ClipboardImpl::GetData(const std::function<void(const std::string&, bool isLastRecord)>& textCallback,
    const std::function<void(const RefPtr<PixelMap>&, bool isLastRecord)>& pixelMapCallback,
    const std::function<void(const std::string&, bool isLastRecord)>& urlCallback, bool syncMode)
{}

void ClipboardImpl::GetSpanStringData(
    const std::function<void(std::vector<std::vector<uint8_t>>&, const std::string&, bool&)>& callback,
    bool syncMode)
{
    if (!taskExecutor_ || !callback) {
        return;
    }

    GetSpanStringDataHelper(callback, syncMode);
}

void ClipboardImpl::GetSpanStringDataHelper(
    const std::function<void(std::vector<std::vector<uint8_t>>&, const std::string&, bool&)>& callback,
    bool syncMode)
{
    auto task = [callback, weakExecutor = WeakClaim(RawPtr(taskExecutor_)), weak = WeakClaim(this)]() {
        auto clip = weak.Upgrade();
        CHECK_NULL_VOID(clip);
        auto taskExecutor = weakExecutor.Upgrade();
        CHECK_NULL_VOID(taskExecutor);
        auto hasData = ClipboardJni::HasPasteData();
        CHECK_NULL_VOID(hasData);
        std::vector<std::vector<uint8_t>> arrays;
        std::string text;
        bool isMultiTypeRecord = false;
        auto IsMultiType = ClipboardJni::IsMultiTypeData();
        if (IsMultiType) {
            auto result = ClipboardJni::GetMultiTypeData();
            auto pasteData = std::make_shared<PasteData>();
            CHECK_NULL_VOID(pasteData);
            pasteData->FromJsonString(result);
            clip->ProcessSpanStringData(arrays, *pasteData, text, isMultiTypeRecord);
        } else {
            text = ClipboardJni::GetData();
        }
        taskExecutor->PostTask(
            [callback, arrays, text, isMultiTypeRecord]() mutable { callback(arrays, text, isMultiTypeRecord); },
            TaskExecutor::TaskType::UI, "ArkUIClipboardGetSpanStringDataCallback", PriorityType::IMMEDIATE);
    };
    if (syncMode) {
        taskExecutor_->PostSyncTask(task, TaskExecutor::TaskType::BACKGROUND, "ArkUIClipboardGetSpanStringDataSync");
    } else {
        taskExecutor_->PostTask(task, TaskExecutor::TaskType::BACKGROUND, "ArkUIClipboardGetSpanStringDataAsync",
            PriorityType::IMMEDIATE);
    }
}

void ClipboardImpl::ProcessSpanStringData(std::vector<std::vector<uint8_t>>& arrays, const PasteData& pasteData,
    std::string& text, bool& isMultiTypeRecord)
{
    auto allRecords = pasteData.AllRecords();
    for (auto pasteDataRecordTmp = allRecords.rbegin(); pasteDataRecordTmp != allRecords.rend(); ++pasteDataRecordTmp) {
        auto pasteDataRecord = *pasteDataRecordTmp;
        if (pasteDataRecord == nullptr) {
            continue;
        }

        if (pasteDataRecord->IsMultiType() && pasteDataRecord->GetIsMulti()) {
            isMultiTypeRecord = true;
        }

        if (pasteDataRecord->IsMultiType() && pasteDataRecord->GetType() == CLIPBOARD_SPAN_STRING_TYPE) {
            auto spanString = pasteDataRecord->GetSpanString();
            if (!spanString.empty()) {
                arrays.emplace_back(spanString);
            }
            text.append(pasteDataRecord->GetText());
        } else if (!pasteDataRecord->IsMultiType() && pasteDataRecord->GetType() == CLIPBOARD_SPAN_STRING_TYPE) {
            auto spanString = pasteDataRecord->GetSpanString();
            if (!spanString.empty()) {
                arrays.emplace_back(spanString);
            }
        } else if (!pasteDataRecord->IsMultiType() && pasteDataRecord->GetType() == CLIPBOARD_TEXT_TYPE) {
            auto recordText = pasteDataRecord->GetText();
            if (!recordText.empty()) {
                text.append(recordText);
            }
        } else {
            text.append("");
            LOGW("ClipboardImpl::ProcessSpanStringData: Unsupported data type.");
        }
    }
}

RefPtr<PasteDataMix> ClipboardImpl::CreatePasteDataMix()
{
    return AceType::MakeRefPtr<PasteDataImpl>();
}

void ClipboardImpl::SetData(const std::string& data, CopyOptions copyOption, bool isDragData)
{
    CHECK_NULL_VOID(taskExecutor_);
    taskExecutor_->PostTask(
        [data] { ClipboardJni::SetData(data); }, TaskExecutor::TaskType::PLATFORM, "ArkUI-XClipboardImplSetData");
}

void ClipboardImpl::GetData(const std::function<void(const std::string&)>& callback, bool syncMode)
{
    if (taskExecutor_) {
        taskExecutor_->PostTask([callback, taskExecutor = WeakClaim(RawPtr(
                                               taskExecutor_))] { ClipboardJni::GetData(callback, taskExecutor); },
            TaskExecutor::TaskType::PLATFORM, "ArkUI-XClipboardImplGetData");
    }
}

void ClipboardImpl::HasData(const std::function<void(bool hasData)>& callback)
{
    if (taskExecutor_) {
        auto task = [callback, taskExecutor = WeakClaim(RawPtr(taskExecutor_))] {
            ClipboardJni::HasData(callback, taskExecutor);
        };
        taskExecutor_->PostTask(task, TaskExecutor::TaskType::PLATFORM, "ArkUI-XClipboardImplHasData");
    }
}

void ClipboardImpl::HasDataType(
    const std::function<void(bool hasData)>& callback, const std::vector<std::string>& mimeTypes)
{
    HasData(callback);
}

void ClipboardImpl::SetPixelMapData(const RefPtr<PixelMap>& pixmap, CopyOptions copyOption)
{
    if (!taskExecutor_ || !callbackSetClipboardPixmapData_) {
        LOGE("Failed to set the pixmap data to clipboard.");
        return;
    }
    taskExecutor_->PostTask([callbackSetClipboardPixmapData = callbackSetClipboardPixmapData_,
                                pixmap] { callbackSetClipboardPixmapData(pixmap); },
        TaskExecutor::TaskType::UI, "ArkUI-XClipboardImplSetPixelMapData");
}

void ClipboardImpl::GetPixelMapData(const std::function<void(const RefPtr<PixelMap>&)>& callback, bool syncMode)
{
    if (!taskExecutor_ || !callbackGetClipboardPixmapData_ || !callback) {
        LOGE("Failed to get the pixmap data from clipboard.");
        return;
    }
    taskExecutor_->PostTask([callbackGetClipboardPixmapData = callbackGetClipboardPixmapData_,
                                callback] { callback(callbackGetClipboardPixmapData()); },
        TaskExecutor::TaskType::UI, "ArkUI-XClipboardImplGetPixelMapData");
}

void ClipboardImpl::Clear() {}

void ClipboardImpl::RegisterCallbackSetClipboardPixmapData(CallbackSetClipboardPixmapData callback)
{
    callbackSetClipboardPixmapData_ = callback;
}

void ClipboardImpl::RegisterCallbackGetClipboardPixmapData(CallbackGetClipboardPixmapData callback)
{
    callbackGetClipboardPixmapData_ = callback;
}
} // namespace OHOS::Ace::Platform
