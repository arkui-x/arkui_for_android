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
#include "adapter/android/capability/java/jni/font/system_font_manager.h"

#include "adapter/android/capability/java/jni/font/system_font_jni.h"

namespace OHOS::Ace::Platform {

std::unique_ptr<FontInfoMap> SystemFontManager::fontInfoMap_ = nullptr;

void SystemFontManager::GetSystemFontList(std::vector<std::string>& fontList)
{
    GetSystemFontInfoMap();
    if (!fontInfoMap_) {
        return;
    }
    for (auto iter = fontInfoMap_->begin(); iter != fontInfoMap_->end(); iter++) {
        fontList.push_back(iter->first);
    }
}

bool SystemFontManager::GetSystemFont(const std::string& fontName, FontInfo& fontInfo)
{
    GetSystemFontInfoMap();
    if (!fontInfoMap_) {
        return false;
    }
    auto findIter = fontInfoMap_->find(fontName);
    if (fontInfoMap_->end() == findIter) {
        return false;
    }
    fontInfo.path = findIter->second.path;
    fontInfo.fullName = findIter->second.fullName;
    fontInfo.weight = findIter->second.weight;
    fontInfo.italic = findIter->second.italic;
    return true;
}

void SystemFontManager::GetSystemFontInfoMap()
{
    if (!fontInfoMap_) {
        fontInfoMap_ = std::make_unique<FontInfoMap>();
    }
    if (fontInfoMap_->empty()) {
        std::vector<FontInfoAndroid> fontInfoAndroids;
        SystemFontJni::GetSystemFontInfo(fontInfoAndroids);
        for (auto& fontInfoAndroid : fontInfoAndroids) {
            FontInfo fontInfo;
            fontInfo.path = fontInfoAndroid.path;
            fontInfo.fullName = fontInfoAndroid.name;
            fontInfo.weight = fontInfoAndroid.weight;
            fontInfo.italic = fontInfoAndroid.italic;
            fontInfoMap_->insert(std::make_pair(fontInfo.fullName, fontInfo));
        }
    }
}
} // namespace OHOS::Ace::Platform