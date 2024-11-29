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

#include "adapter/preview/entrance/clipboard/multiType_record_impl.h"

namespace OHOS::Ace {

MultiTypeRecordImpl::MultiTypeRecordImpl() {}

void MultiTypeRecordImpl::SetPlainText(const std::string plainText)
{
    plainText_ = plainText;
}
void MultiTypeRecordImpl::SetUri(const std::string uri)
{
    uri_ = uri;
}
void MultiTypeRecordImpl::SetPixelMap(RefPtr<PixelMap> pixelMap)
{
    pixelMap_ = pixelMap;
}
const RefPtr<PixelMap> MultiTypeRecordImpl::GetPixelMap()
{
    return pixelMap_;
}
const std::string MultiTypeRecordImpl::GetPlainText()
{
    return plainText_;
}
const std::string MultiTypeRecordImpl::GetUri()
{
    return uri_;
}
std::vector<uint8_t>& MultiTypeRecordImpl::GetSpanStringBuffer()
{
    return spanStringBuffer_;
}

} // namespace OHOS::Ace
