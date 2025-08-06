/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "adapter/android/entrance/java/jni/udmf/udmf_impl.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <string>

#include "application_defined_record.h"
#include "error_code.h"
#include "js_native_api_types.h"
#include "native_engine/native_engine.h"
#include "plain_text.h"
#include "summary_napi.h"
#include "unified_data_napi.h"

#include "base/utils/utils.h"
#include "bridge/common/utils/engine_helper.h"
#include "bridge/common/utils/utils.h"
#include "bridge/js_frontend/engine/common/js_engine.h"
#include "core/common/udmf/unified_data.h"

const std::string UNIFIED_KEY_SCHEMA = "udmf://";
const std::string UNIFIED_INTENTION = "drag";
namespace OHOS::Ace {
std::mutex UdmfClientImpl::unifiedDataLock_;
std::map<std::string, RefPtr<UnifiedData>> UdmfClientImpl::unifiedDataMap_;
int32_t UdmfClientImpl::groupId_ = 0;

UdmfClient* UdmfClient::GetInstance()
{
    static UdmfClientImpl instance;
    return &instance;
}

RefPtr<UnifiedData> UdmfClientImpl::TransformUnifiedData(napi_value napiValue)
{
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, nullptr);
    NativeEngine* nativeEngine = engine->GetNativeEngine();
    napi_env env = reinterpret_cast<napi_env>(nativeEngine);
    void* native = nullptr;
    napi_unwrap(env, napiValue, &native);
    auto* unifiedData = reinterpret_cast<UDMF::UnifiedDataNapi*>(native);
    CHECK_NULL_RETURN(unifiedData, nullptr);
    CHECK_NULL_RETURN(unifiedData->value_, nullptr);
    auto udData = AceType::MakeRefPtr<UnifiedDataImpl>();
    udData->SetUnifiedData(unifiedData->value_);
    return udData;
}

RefPtr<UnifiedData> UdmfClientImpl::TransformUnifiedDataForNative(void* rawData)
{
    return nullptr;
}

RefPtr<DataLoadParams> UdmfClientImpl::TransformDataLoadParamsForNative(void* rawData)
{
    return nullptr;
}

void* UdmfClientImpl::TransformUnifiedDataPtr(RefPtr<UnifiedData>& unifiedData)
{
    return nullptr;
}

RefPtr<DataLoadParams> UdmfClientImpl::TransformDataLoadParams(napi_env env, napi_value napiValue)
{
    return nullptr;
}

int32_t UdmfClientImpl::SetDelayInfo(RefPtr<DataLoadParams> dataLoadParams, std::string& key)
{
    return -1;
}

napi_value UdmfClientImpl::TransformUdmfUnifiedData(RefPtr<UnifiedData>& UnifiedData)
{
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, nullptr);
    NativeEngine* nativeEngine = engine->GetNativeEngine();
    napi_env env = reinterpret_cast<napi_env>(nativeEngine);
    auto unifiedData = AceType::DynamicCast<UnifiedDataImpl>(UnifiedData)->GetUnifiedData();
    CHECK_NULL_RETURN(unifiedData, nullptr);
    napi_value dataVal = nullptr;
    UDMF::UnifiedDataNapi::NewInstance(env, unifiedData, dataVal);
    CHECK_NULL_RETURN(dataVal, nullptr);
    return dataVal;
}

napi_value UdmfClientImpl::TransformSummary(std::map<std::string, int64_t>& summary)
{
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, nullptr);
    NativeEngine* nativeEngine = engine->GetNativeEngine();
    napi_env env = reinterpret_cast<napi_env>(nativeEngine);
    std::shared_ptr<UDMF::Summary> udmfSummary = std::make_shared<UDMF::Summary>();
    CHECK_NULL_RETURN(udmfSummary, nullptr);
    udmfSummary->totalSize = 0;
    for (auto element : summary) {
        udmfSummary->totalSize += element.second;
    }
    udmfSummary->summary = std::move(summary);
    napi_value dataVal = nullptr;
    UDMF::SummaryNapi::NewInstance(env, udmfSummary, dataVal);
    CHECK_NULL_RETURN(dataVal, nullptr);
    return dataVal;
}

RefPtr<UnifiedData> UdmfClientImpl::CreateUnifiedData()
{
    return AceType::DynamicCast<UnifiedData>(AceType::MakeRefPtr<UnifiedDataImpl>());
}

int32_t UdmfClientImpl::SetData(const RefPtr<UnifiedData>& unifiedData, std::string& key)
{
    CHECK_NULL_RETURN(unifiedData, UDMF::E_INVALID_PARAMETERS);
    std::lock_guard<std::mutex> lock(unifiedDataLock_);
    std::string groupIdStr = std::to_string(getGroupId());
    std::string* keyRef_ = (std::string*)&key;
    *keyRef_ = UNIFIED_KEY_SCHEMA + UNIFIED_INTENTION + groupIdStr;
    RefPtr<UnifiedData>& udRef_ = const_cast<RefPtr<UnifiedData>&>(unifiedData);
    unifiedDataMap_[*keyRef_] = udRef_;
    return UDMF::E_OK;
}

int32_t UdmfClientImpl::GetData(const RefPtr<UnifiedData>& unifiedData, const std::string& key)
{
    if (key.empty()) {
        LOGE("GetData Failed, Invalid key");
        return UDMF::E_INVALID_PARAMETERS;
    }

    std::lock_guard<std::mutex> lock(unifiedDataLock_);
    RefPtr<UnifiedData>& udRef = const_cast<RefPtr<UnifiedData>&>(unifiedData);
    auto iter = unifiedDataMap_.find(key);
    if (iter == unifiedDataMap_.end()) {
        LOGE("GetData Failed, Data is empty");
        return UDMF::E_NOT_FOUND;
    }
    udRef = iter->second;
    unifiedDataMap_.erase(iter);
    return UDMF::E_OK;
}

int32_t UdmfClientImpl::GetSummary(std::string& key, DragSummaryInfo& dragSummaryInfo)
{
    if (key.empty()) {
        LOGE("GetSummary Failed, Invalid key");
        return UDMF::E_INVALID_PARAMETERS;
    }

    std::lock_guard<std::mutex> lock(unifiedDataLock_);
    auto iter = unifiedDataMap_.find(key);
    if (iter == unifiedDataMap_.end()) {
        LOGE("GetSummary Failed, Data is empty");
        return UDMF::E_NOT_FOUND;
    }

    RefPtr<UnifiedData>& unifiedData = iter->second;
    UDMF::Summary summary;
    for (const auto& record : AceType::DynamicCast<UnifiedDataImpl>(unifiedData)->GetUnifiedData()->GetRecords()) {
        int64_t recordSize = record->GetSize();
        auto udType = UDMF::UtdUtils::GetUtdIdFromUtdEnum(record->GetType());
        auto it = summary.summary.find(udType);
        if (it == summary.summary.end()) {
            summary.summary[udType] = recordSize;
        } else {
            summary.summary[udType] += recordSize;
        }
        summary.totalSize += recordSize;
    }
    dragSummaryInfo.summary = summary.summary;
    dragSummaryInfo.detailedSummary = summary.specificSummary;
    dragSummaryInfo.summaryFormat = summary.summaryFormat;
    dragSummaryInfo.version = summary.version;
    dragSummaryInfo.totalSize = summary.totalSize;
    return UDMF::E_OK;
}

bool UdmfClientImpl::GetRemoteStatus(std::string& key)
{
    return false;
}

void UdmfClientImpl::AddFormRecord(
    const RefPtr<UnifiedData>& unifiedData, int32_t formId, const RequestFormInfo& cardInfo)
{}

void UdmfClientImpl::AddLinkRecord(
    const RefPtr<UnifiedData>& unifiedData, const std::string& url, const std::string& description)
{}

void UdmfClientImpl::GetLinkRecord(const RefPtr<UnifiedData>& unifiedData, std::string& url, std::string& description)
{}

bool UdmfClientImpl::GetFileUriRecord(const RefPtr<UnifiedData>& unifiedData, std::vector<std::string>& uri)
{
    return false;
}

bool UdmfClientImpl::AddFileUriRecord(const RefPtr<UnifiedData>& unifiedData, std::vector<std::string>& uri)
{
    return false;
}

void UdmfClientImpl::AddHtmlRecord(
    const RefPtr<UnifiedData>& unifiedData, const std::string& htmlContent, const std::string& plainContent)
{}

void UdmfClientImpl::GetHtmlRecord(
    const RefPtr<UnifiedData>& unifiedData, std::string& htmlContent, std::string& plainContent)
{}

void UdmfClientImpl::AddPixelMapRecord(
    const RefPtr<UnifiedData>& unifiedData, std::vector<uint8_t>& data, PixelMapRecordDetails& details)
{}

void UdmfClientImpl::AddImageRecord(const RefPtr<UnifiedData>& unifiedData, const std::string& uri) {}

void UdmfClientImpl::AddPlainTextRecord(const RefPtr<UnifiedData>& unifiedData, const std::string& selectedStr)
{
    auto record = std::make_shared<UDMF::PlainText>(selectedStr, "");

    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_VOID(udData);
    CHECK_NULL_VOID(udData->GetUnifiedData());
    udData->GetUnifiedData()->AddRecord(record);
}

std::string UdmfClientImpl::GetSinglePlainTextRecord(const RefPtr<UnifiedData>& unifiedData)
{
    return "";
}

std::vector<std::string> UdmfClientImpl::GetPlainTextRecords(const RefPtr<UnifiedData>& unifiedData)
{
    std::vector<std::string> textRecords;
    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_RETURN(udData, textRecords);
    CHECK_NULL_RETURN(udData->GetUnifiedData(), textRecords);
    auto records = udData->GetUnifiedData()->GetRecords();
    for (const auto& record : records) {
        UDMF::UDType type = record->GetType();
        if (type == UDMF::UDType::PLAIN_TEXT) {
            UDMF::PlainText* plainText = reinterpret_cast<UDMF::PlainText*>(record.get());
            std::string str = plainText->GetContent();
            textRecords.emplace_back(str);
        }
    }
    return textRecords;
}

int32_t UdmfClientImpl::GetVideoRecordUri(const RefPtr<UnifiedData>& unifiedData, std::string& uri)
{
    return -1;
}

std::pair<int32_t, std::string> UdmfClientImpl::GetErrorInfo(int32_t errorCode)
{
    switch (errorCode) {
        case UDMF::E_NOT_FOUND:
            return { ERROR_CODE_DRAG_DATA_NOT_FOUND, "GetData failed, data not found." };
        default:
            return { ERROR_CODE_DRAG_DATA_ERROR, "GetData failed, data error." };
    }
}

std::shared_ptr<UDMF::UnifiedData> UnifiedDataImpl::GetUnifiedData()
{
    if (unifiedData_ == nullptr) {
        unifiedData_ = std::make_shared<UDMF::UnifiedData>();
    }
    return unifiedData_;
}

void UnifiedDataImpl::SetUnifiedData(std::shared_ptr<UDMF::UnifiedData> unifiedData)
{
    unifiedData_ = unifiedData;
}

int64_t UnifiedDataImpl::GetSize()
{
    CHECK_NULL_RETURN(unifiedData_, 0);
    return unifiedData_->GetRecords().size();
}

int32_t UdmfClientImpl::getGroupId()
{
    return groupId_++;
}
void UdmfClientImpl::AddSpanStringRecord(
    const RefPtr<UnifiedData>& unifiedData, std::vector<uint8_t>& data)
{
    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_VOID(udData);
    CHECK_NULL_VOID(udData->GetUnifiedData());
    auto record = std::make_shared<UDMF::ApplicationDefinedRecord>("OPENHARMONY_STYLED_STRING_UDMF", data);
    udData->GetUnifiedData()->AddRecord(record);
}

std::vector<uint8_t> UdmfClientImpl::GetSpanStringRecord(const RefPtr<UnifiedData>& unifiedData)
{
    std::vector<uint8_t> arr;
    auto udData = AceType::DynamicCast<UnifiedDataImpl>(unifiedData);
    CHECK_NULL_RETURN(udData, arr);
    CHECK_NULL_RETURN(udData->GetUnifiedData(), arr);
    auto records = udData->GetUnifiedData()->GetRecords();
    for (auto record: records) {
        UDMF::UDType type = record->GetType();
        if (type == UDMF::UDType::APPLICATION_DEFINED_RECORD) {
            UDMF::ApplicationDefinedRecord* app = reinterpret_cast<UDMF::ApplicationDefinedRecord*>(record.get());
            if (app->GetApplicationDefinedType() == "OPENHARMONY_STYLED_STRING_UDMF") {
                arr = app->GetRawData();
                return arr;
            }
        }
    }
    return arr;
}

int32_t UdmfClientImpl::StartAsyncDataRetrieval(napi_env env, napi_value napiValue, const std::string& key)
{
    return -1;
}

int32_t UdmfClientImpl::Cancel(const std::string& key)
{
    return -1;
}

void UdmfClientImpl::SetTagProperty(const RefPtr<UnifiedData>& unifiedData, const std::string& tag) {}

std::string UdmfClientImpl::GetPlainTextEntry(const RefPtr<UnifiedData>& unifiedData)
{
    return "";
}

void UdmfClientImpl::GetHtmlEntry(
    const RefPtr<UnifiedData>& unifiedData, std::string& htmlContent, std::string& plainContent)
{}

void UdmfClientImpl::GetLinkEntry(const RefPtr<UnifiedData>& unifiedData, std::string& url, std::string& description) {}

bool UdmfClientImpl::GetFileUriEntry(const RefPtr<UnifiedData>& unifiedData, std::vector<std::string>& uri)
{
    return false;
}

std::vector<uint8_t> UdmfClientImpl::GetSpanStringEntry(const RefPtr<UnifiedData>& unifiedData)
{
    return {};
}

bool UdmfClientImpl::IsAppropriateType(DragSummaryInfo& dragSummaryInfo, const std::set<std::string>& allowTypes)
{
    return false;
}

} // namespace OHOS::Ace