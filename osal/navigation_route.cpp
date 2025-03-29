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
#include "adapter/android/osal/navigation_route.h"

#include "application_context.h"

#include "base/error/error_code.h"
#include "base/log/log.h"

namespace OHOS::Ace {

void NavigationRoute::InitRouteMap()
{
    auto applicationContext = AbilityRuntime::Platform::ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        TAG_LOGE(AceLogTag::ACE_NAVIGATION, "applicationContext is nullptr");
        return;
    }
    auto bundleContainer = applicationContext->GetBundleContainer();
    if (!bundleContainer) {
        TAG_LOGE(AceLogTag::ACE_NAVIGATION, "get bundle container failed");
        return;
    }
    AppExecFwk::BundleInfo bundleInfo;
    if (bundleContainer->GetBundleInfoForSelf(
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ROUTER_MAP),
        bundleInfo) != 0) {
        TAG_LOGE(AceLogTag::ACE_NAVIGATION, "get bundle info failed");
        return;
    }
    allRouteItems_ = bundleInfo.routerArray;
    moduleInfos_ = bundleInfo.hapModuleInfos;
}

bool NavigationRoute::GetRouteItem(const std::string& name, NG::RouteItem& info)
{
    AppExecFwk::RouterItem routeItem;
    if (!GetRouteItemFromBundle(name, routeItem)) {
        TAG_LOGE(AceLogTag::ACE_NAVIGATION, "get route info %{public}s failed", name.c_str());
        return false;
    }
    info.name = routeItem.name;
    info.bundleName = routeItem.bundleName;
    info.moduleName = routeItem.moduleName;
    info.pageSourceFile = routeItem.pageSourceFile;
    info.data = routeItem.data;
    return true;
}

bool NavigationRoute::GetRouteItemFromBundle(const std::string& name, AppExecFwk::RouterItem& routeItem)
{
    for (auto moduleIter = allRouteItems_.begin(); moduleIter != allRouteItems_.end(); moduleIter++) {
        if (moduleIter->name == name) {
            routeItem = *moduleIter;
            return true;
        }
    }
    TAG_LOGE(AceLogTag::ACE_NAVIGATION, "can't find name in config file: %{public}s", name.c_str());
    return false;
}

int32_t NavigationRoute::LoadPage(const std::string& name)
{
    AppExecFwk::RouterItem item;
    if (!GetRouteItemFromBundle(name, item)) {
        TAG_LOGE(AceLogTag::ACE_NAVIGATION, "get route name failed");
        return ERROR_CODE_BUILDER_FUNCTION_NOT_REGISTERED;
    }
    if (callback_ == nullptr) {
        return -1;
    }
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "load navdestination %{public}s, ohmurl: %{public}s",
        item.bundleName.c_str(), item.moduleName.c_str());
    int32_t res = callback_(item.bundleName, item.moduleName, item.ohmurl, false);
    if (res == 0) {
        names_.emplace_back(name);
    }
    return LoadPageFromHapModule(name);
}

bool NavigationRoute::IsNavigationItemExits(const std::string& name)
{
    if (HasLoaded(name)) {
        return true;
    }
    AppExecFwk::RouterItem item;
    if (GetRouteItemFromBundle(name, item)) {
        return true;
    }
    return false;
}

int32_t NavigationRoute::LoadPageFromHapModule(const std::string& name)
{
    int32_t res = -1;
    if (!callback_) {
        return res;
    }
    for (auto hapIter = moduleInfos_.begin(); hapIter != moduleInfos_.end(); hapIter++) {
        auto routerInfo = hapIter->routerArray;
        for (auto routerIter = routerInfo.begin(); routerIter != routerInfo.end(); routerIter++) {
            if (routerIter->name != name) {
                continue;
            }
            res = callback_(routerIter->bundleName, routerIter->moduleName, routerIter->ohmurl, false);
            TAG_LOGD(AceLogTag::ACE_NAVIGATION, "load current destination name: %{public}s, ohmurl: %{public}s",
                name.c_str(), routerIter->ohmurl.c_str());
            if (res == 0) {
                return 0;
            }
            break;
        }
    }
    return res;
}
} // namespace OHOS::Ace