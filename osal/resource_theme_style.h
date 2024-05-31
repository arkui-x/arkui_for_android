/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_RESOURCE_THEME_STYLE_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_RESOURCE_THEME_STYLE_H

#include <map>

#include "core/components/theme/theme_style.h"
#include "core/components/theme/resource_adapter.h"

namespace OHOS::Ace {
class ResourceThemeStyle : public ThemeStyle {
    DECLARE_ACE_TYPE(ResourceThemeStyle, ThemeStyle);

public:
    friend class ResourceAdapterImpl;
    friend class ResourceAdapterImplV2;
    using RawAttrMap = std::map<std::string, std::string>;
    using RawPatternMap = std::map<std::string, RawAttrMap>;

    explicit ResourceThemeStyle(RefPtr<ResourceAdapter> resAdapter) : resAdapter_(resAdapter) {};
    ~ResourceThemeStyle() override = default;

    void ParseContent() override;

protected:
    void OnParseStyle();

private:
    RawAttrMap rawAttrs_; // key and value read from global resource api.
    RawPatternMap patternAttrs_;
    RefPtr<ResourceAdapter> resAdapter_;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_RESOURCE_THEME_STYLE_H
