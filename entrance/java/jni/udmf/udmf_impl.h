/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_PREVIEW_ENTRANCE_UDMF_IMPL_H
#define FOUNDATION_ACE_ADAPTER_PREVIEW_ENTRANCE_UDMF_IMPL_H

#include <memory>

#include "base/memory/ace_type.h"
#include "core/common/udmf/udmf_client.h"

namespace OHOS::Ace {
class UnifiedDataImpl;
class UdmfClientImpl : public UdmfClient {
    DECLARE_ACE_TYPE(UdmfClientImpl, UdmfClient);

public:
    RefPtr<UnifiedData> TransformUnifiedData(napi_value napiValue) override;
    napi_value TransformUdmfUnifiedData(RefPtr<UnifiedData>& UnifiedData) override;
    napi_value TransformSummary(std::map<std::string, int64_t>& summary) override;
    RefPtr<UnifiedData> CreateUnifiedData() override;
    int32_t SetData(const RefPtr<UnifiedData>& unifiedData, std::string& key) override;
    int32_t GetData(const RefPtr<UnifiedData>& unifiedData, const std::string& key) override;
    int32_t GetSummary(std::string& key, std::map<std::string, int64_t>& summaryMap) override;
    void AddFormRecord(
        const RefPtr<UnifiedData>& unifiedData, int32_t formId, const RequestFormInfo& cardInfo) override;
    void AddLinkRecord(
        const RefPtr<UnifiedData>& unifiedData, const std::string& url, const std::string& description) override;
    void GetLinkRecord(const RefPtr<UnifiedData>& unifiedData, std::string& url, std::string& description) override;
    bool GetFileUriRecord(const RefPtr<UnifiedData>& unifiedData, std::vector<std::string>& uri) override;
    bool AddFileUriRecord(const RefPtr<UnifiedData>& unifiedData, std::vector<std::string>& uri) override;
    void AddHtmlRecord(const RefPtr<UnifiedData>& unifiedData, const std::string& htmlContent,
        const std::string& plainContent) override;
    void GetHtmlRecord(
        const RefPtr<UnifiedData>& unifiedData, std::string& htmlContent, std::string& plainContent) override;
    void AddPixelMapRecord(
        const RefPtr<UnifiedData>& unifiedData, std::vector<uint8_t>& data, PixelMapRecordDetails& details) override;
    void AddImageRecord(const RefPtr<UnifiedData>& unifiedData, const std::string& uri) override;
    void AddPlainTextRecord(const RefPtr<UnifiedData>& unifiedData, const std::string& selectedStr) override;
    std::string GetSinglePlainTextRecord(const RefPtr<UnifiedData>& unifiedData) override;
    std::vector<std::string> GetPlainTextRecords(const RefPtr<UnifiedData>& unifiedData) override;
    int32_t GetVideoRecordUri(const RefPtr<UnifiedData>& unifiedData, std::string& uri) override;
    std::pair<int32_t, std::string> GetErrorInfo(int32_t errorCode) override;
};

class UnifiedDataImpl : public UnifiedData {
    DECLARE_ACE_TYPE(UnifiedDataImpl, UnifiedData);

public:
    UnifiedDataImpl() = default;
    ~UnifiedDataImpl() = default;

    int64_t GetSize() override;
    std::shared_ptr<UnifiedData> GetUnifiedData();
    void SetUnifiedData(std::shared_ptr<UnifiedData> unifiedData);
};

} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_ACE_ENGINE_ADAPTER_OHOS_CAPABILITY_UDMF_IMPL_H