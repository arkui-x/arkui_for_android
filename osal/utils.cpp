/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "base/utils/utils.h"
#include "stage_asset_provider.h"

namespace OHOS::Ace::NG {
std::string GetSystemPath(const std::string& fileName)
{
    std::string dataDirPath = AbilityRuntime::Platform::StageAssetProvider::GetInstance()->GetFilesDir();
    LOGI("GetSystemResAbcPath: Got path from StageAssetProvider: %{public}s", dataDirPath.c_str());
    if (!dataDirPath.empty()) {
        const std::string filesWithSlash = "/files";
        auto pos = dataDirPath.rfind(filesWithSlash);
        if (pos != std::string::npos && pos + filesWithSlash.length() == dataDirPath.length()) {
            dataDirPath = dataDirPath.substr(0, pos);
        }
        return dataDirPath + "/arkui-x/systemres/abc/" + fileName;
    }
    return "/arkui-x/systemres/abc/" + fileName;
}
} // namespace OHOS::Ace::NG
