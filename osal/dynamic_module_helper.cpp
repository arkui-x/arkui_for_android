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

#include "core/common/dynamic_module_helper.h"

#include <memory>

#include "compatible/components/component_loader.h"
#include "interfaces/inner_api/ace/utils.h"

#include "base/utils/utils.h"
#include "core/common/dynamic_module.h"

namespace OHOS::Ace {
namespace {
#ifdef WINDOWS_PLATFORM
#ifdef UNICODE
const std::wstring COMPATIABLE_LIB = L"libace_compatible_components.dll";
const std::wstring DYNAMIC_MODULE_LIB_PREFIX = L"libarkui_";
const std::wstring DYNAMIC_MODULE_LIB_POSTFIX = L".dll";
#else
const std::string COMPATIABLE_LIB = "libace_compatible_components.dll";
const std::string DYNAMIC_MODULE_LIB_PREFIX = "libarkui_";
const std::string DYNAMIC_MODULE_LIB_POSTFIX = ".dll";
#endif
#elif MAC_PLATFORM
const std::string COMPATIABLE_LIB = "libace_compatible_components.dylib";
const std::string DYNAMIC_MODULE_LIB_PREFIX = "libarkui_";
const std::string DYNAMIC_MODULE_LIB_POSTFIX = ".dylib";
#else
const std::string COMPATIABLE_LIB = "libace_compatible_components.z.so";
const std::string DYNAMIC_MODULE_LIB_PREFIX = "libarkui_";
const std::string DYNAMIC_MODULE_LIB_POSTFIX = ".z.so";
#endif
} // namespace
DynamicModuleHelper& DynamicModuleHelper::GetInstance()
{
    static DynamicModuleHelper instance;
    return instance;
}

std::unique_ptr<ComponentLoader> DynamicModuleHelper::GetLoaderByName(const char* name)
{
    if (compatibleLoaderFunc_) {
        return std::unique_ptr<ComponentLoader>(compatibleLoaderFunc_(name));
    }
    LIBHANDLE handle = LOADLIB(COMPATIABLE_LIB.c_str());
#if defined(WINDOWS_PLATFORM) && defined(UNICODE)
    LOGI("Load compatible lib %{public}ls", COMPATIABLE_LIB.c_str());
#else
    LOGI("Load compatible lib %{public}s", COMPATIABLE_LIB.c_str());
#endif
    auto* createSym = reinterpret_cast<ComponentLoaderFunc>(LOADSYM(handle, COMPATIABLE_COMPONENT_LOADER));
    CHECK_NULL_RETURN(createSym, nullptr);
    compatibleLoaderFunc_ = createSym;
    return std::unique_ptr<ComponentLoader>(compatibleLoaderFunc_(name));
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

    // Load module without holding the lock (LOADLIB/LOADSYM may be slow)
#ifdef WINDOWS_PLATFORM
    std::wstring nameW = std::wstring(name.begin(), name.end());
    auto libName = DYNAMIC_MODULE_LIB_PREFIX + nameW + DYNAMIC_MODULE_LIB_POSTFIX;
#else
    auto libName = DYNAMIC_MODULE_LIB_PREFIX + name + DYNAMIC_MODULE_LIB_POSTFIX;
#endif
    LIBHANDLE handle = LOADLIB(libName.c_str());
    CHECK_NULL_RETURN(handle, nullptr);
    auto* createSym = reinterpret_cast<DynamicModuleCreateFunc>(LOADSYM(handle, DYNAMIC_MODULE_CREATE));
    CHECK_NULL_RETURN(createSym, nullptr);
    DynamicModule* module = createSym();
    CHECK_NULL_RETURN(module, nullptr);
    LOGI("First load %{public}s nativeModule finish", name.c_str());

    // Lock again to insert into map
    {
        std::lock_guard<std::mutex> lock(moduleMapMutex_);
        // Check again in case another thread already loaded it
        auto iter = moduleMap_.find(name);
        if (iter != moduleMap_.end()) {
            // Another thread already loaded it, use that one
            delete module;
            return iter->second.get();
        }
        moduleMap_.emplace(name, std::unique_ptr<DynamicModule>(module));
        return module;
    }
}

} // namespace OHOS::Ace
