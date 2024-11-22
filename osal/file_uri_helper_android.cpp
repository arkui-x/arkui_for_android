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

#include "base/image/file_uri_helper.h"

namespace OHOS::Ace {
std::string FileUriHelper::GetRealPath(std::string fileUriStr)
{
    const std::string fileHead = "file://";
    size_t pos = fileUriStr.find(fileHead);
    if (pos != std::string::npos) {
        return fileUriStr.substr(pos + fileHead.length());
    }
    return {};
}
} // namespace OHOS::Ace
