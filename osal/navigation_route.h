/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_NAVIGATION_ROUTE_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_NAVIGATION_ROUTE_H
#include <cstdint>

#include "bundle_container.h"
#include "navigation/router_map_helper.h"

#include "frameworks/core/components_ng/pattern/navigation/navigation_route.h"

namespace OHOS::Ace {

class ACE_EXPORT NavigationRoute : public NG::NavigationRoute {
    DECLARE_ACE_TYPE(NavigationRoute, NG::NavigationRoute);

public:
    explicit NavigationRoute(const std::string& bundleName)
    {
        InitRouteMap();
    }
    ~NavigationRoute() = default;

    bool GetRouteItem(const std::string& name, NG::RouteItem& info) override;

    bool HasLoaded(const std::string& name) override
    {
        return std::find(names_.begin(), names_.end(), name) != names_.end();
    }

    int32_t LoadPage(const std::string& name) override;

    bool IsNavigationItemExits(const std::string& name) override;

    void InitRouteMap() override;

private:
    bool GetRouteItemFromBundle(const std::string& name, AppExecFwk::RouterItem& routeItem);
    int32_t LoadPageFromHapModule(const std::string& name);
    AppExecFwk::RouterItem GetRouteItem(const std::string name);
    std::vector<AppExecFwk::RouterItem> allRouteItems_;
    std::vector<AppExecFwk::HapModuleInfo> moduleInfos_;
    std::vector<std::string> names_;
};
} // namespace OHOS::Ace
#endif