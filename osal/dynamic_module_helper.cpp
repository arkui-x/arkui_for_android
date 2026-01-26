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

#include "core/common/dynamic_module_helper.h"

#include <dlfcn.h>
#include <memory>

#include "base/utils/utils.h"
#include "compatible/components/component_loader.h"
#include "interfaces/inner_api/ace/utils.h"

namespace OHOS::Ace {
namespace {
const std::string DYNAMIC_MODULE_LIB_PREFIX = "libarkui_";
const std::string DYNAMIC_MODULE_LIB_POSTFIX = ".so";
const std::unordered_map<std::string, std::string> soMap = {
    {"Checkbox", "checkbox"},
    {"CheckboxGroup", "checkbox"},
    {"Gauge", "gauge"},
    {"Rating", "rating"},
    {"FlowItem", "waterflow" },
    {"WaterFlow", "waterflow" },
};
} // namespace
DynamicModuleHelper& DynamicModuleHelper::GetInstance()
{
    static DynamicModuleHelper instance;
    return instance;
}

std::unique_ptr<ComponentLoader> DynamicModuleHelper::GetLoaderByName(const char* name)
{
    return nullptr;
}

DynamicModule* DynamicModuleHelper::GetDynamicModule(const std::string& name)
{
    // Double-checked locking pattern for better performance
    {
        std::lock_guard<std::mutex> lock(moduleMapMutex_);
        auto iter = moduleMap_.find(name);
        if (iter != moduleMap_.end()) {
            return iter->second.get();
        }
    }
    auto it = soMap.find(name);
    if (it == soMap.end()) {
        LOGE("No shared library mapping found for nativeModule: %{public}s", name.c_str());
        return nullptr;
    }
    // Load module without holding the lock (dlopen/dlsym may be slow)
    auto libName = DYNAMIC_MODULE_LIB_PREFIX + it->second + DYNAMIC_MODULE_LIB_POSTFIX;
    auto* handle = dlopen(libName.c_str(), RTLD_LAZY);
    LOGI("First load %{public}s nativeModule start", name.c_str());
    if (handle == nullptr) {
        LOGE("Failed to load dynamic module library: %{public}s, error: %{public}s", name.c_str(), dlerror());
        return nullptr;
    }
    auto* createSym = reinterpret_cast<DynamicModuleCreateFunc>(dlsym(handle, (DYNAMIC_MODULE_CREATE + name).c_str()));
    if (createSym == nullptr) {
        LOGE("Failed to find symbol in library %{public}s, error: %{public}s", name.c_str(), dlerror());
        dlclose(handle);
        return nullptr;
    }
    DynamicModule* module = createSym();
    if (module == nullptr) {
        LOGE("Failed to create DynamicModule instance from library %{public}s", name.c_str());
        dlclose(handle);
        return nullptr;
    }
    LOGI("First load %{public}s nativeModule finish", name.c_str());

    // Lock again to insert into map
    {
        std::lock_guard<std::mutex> lock(moduleMapMutex_);
        // Check again in case another thread already loaded it
        auto iter = moduleMap_.find(name);
        if (iter != moduleMap_.end()) {
            // Another thread already loaded it, use that one
            delete module;
            dlclose(handle);
            return iter->second.get();
        }
        moduleMap_.emplace(name, std::unique_ptr<DynamicModule>(module));
        return module;
    }
}

} // namespace OHOS::Ace
