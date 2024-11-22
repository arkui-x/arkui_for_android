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

#include "adapter/android/capability/java/jni/font/font_platform_impl.h"

#include "adapter/android/capability/java/jni/font/system_font_manager.h"

namespace OHOS::Ace::Platform {

void FontPlatformImpl::GetSystemFontList(std::vector<std::string>& fontList)
{
    SystemFontManager systemFontManager;
    systemFontManager.GetSystemFontList(fontList);
}

bool FontPlatformImpl::GetSystemFont(const std::string& fontName, FontInfo& fontInfo)
{
    SystemFontManager systemFontManager;
    return systemFontManager.GetSystemFont(fontName, fontInfo);
}

} // namespace OHOS::Ace::Platform