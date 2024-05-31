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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_PAGE_URL_CHECKER_ANDROID_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_PAGE_URL_CHECKER_ANDROID_H

#include "interfaces/inner_api/ace/ui_content.h"

#include "frameworks/core/common/page_url_checker.h"

namespace OHOS::Ace {
class PageUrlCheckerAndroid : public PageUrlChecker {
    DECLARE_ACE_TYPE(PageUrlCheckerAndroid, PageUrlChecker)

public:
    explicit PageUrlCheckerAndroid() = default;
    ~PageUrlCheckerAndroid() = default;
    void LoadPageUrl(const std::string& url, const std::function<void()>& callback,
        const std::function<void(int32_t, const std::string&)>& silentInstallErrorCallBack) override;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_STAGE_PAGE_URL_CHECKER_ANDROID_H