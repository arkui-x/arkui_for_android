/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ACE_ENGINE_ADAPTER_PREVIEW_ENTRANCE_MULTITYPE_RECORD_IMPL_H
#define FOUNDATION_ACE_ACE_ENGINE_ADAPTER_PREVIEW_ENTRANCE_MULTITYPE_RECORD_IMPL_H

#include "core/common/clipboard/clipboard_interface.h"

namespace OHOS::Ace {

class MultiTypeRecordImpl : public MultiTypeRecordMix {
    DECLARE_ACE_TYPE(MultiTypeRecordImpl, MultiTypeRecordMix);

public:
    MultiTypeRecordImpl();
    ~MultiTypeRecordImpl() = default;

    void SetPlainText(const std::string plainText);
    void SetUri(const std::string uri);
    void SetPixelMap(RefPtr<PixelMap> pixelMap);
    const RefPtr<PixelMap> GetPixelMap();
    const std::string GetPlainText();
    const std::string GetUri();
    std::vector<uint8_t>& GetSpanStringBuffer();

private:
    RefPtr<PixelMap> pixelMap_;
    std::string plainText_;
    std::string uri_;
    std::vector<uint8_t> spanStringBuffer_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ACE_ENGINE_ADAPTER_PREVIEW_ENTRANCE_MULTITYPE_RECORD_IMPL_H
