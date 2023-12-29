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

#include "adapter/android/osal/resource_adapter_impl.h"

#include <dirent.h>

#include "adapter/android/osal/resource_convertor.h"
#include "adapter/android/osal/resource_theme_style.h"
#include "adapter/android/entrance/java/jni/ace_application_info_impl.h"
#include "base/utils/system_properties.h"
#include "core/components/theme/theme_attributes.h"

namespace OHOS::Ace {

namespace {
constexpr char DELIMITER[] = "/";
constexpr uint32_t THEME_ID_LIGHT = 117440515;
constexpr uint32_t THEME_ID_DARK = 117440516;
constexpr uint32_t OHOS_THEME_ID = 125829872; // ohos_theme

void CheckThemeId(int32_t& themeId)
{
    if (themeId >= 0) {
        return;
    }
    auto deviceType = SystemProperties::GetDeviceType();
    themeId = (deviceType == DeviceType::PHONE || deviceType == DeviceType::UNKNOWN || deviceType == DeviceType::CAR)
                  ? THEME_ID_LIGHT
                  : THEME_ID_DARK;
}

const char* PATTERN_MAP[] = {
    THEME_PATTERN_BUTTON,
    THEME_PATTERN_CHECKBOX,
    THEME_PATTERN_DATA_PANEL,
    THEME_PATTERN_RADIO,
    THEME_PATTERN_SWIPER,
    THEME_PATTERN_SWITCH,
    THEME_PATTERN_TOOLBAR,
    THEME_PATTERN_TOGGLE,
    THEME_PATTERN_TOAST,
    THEME_PATTERN_DIALOG,
    THEME_PATTERN_DRAG_BAR,
    THEME_PATTERN_SEMI_MODAL,
    // append
    THEME_PATTERN_BADGE,
    THEME_PATTERN_CALENDAR,
    THEME_PATTERN_CAMERA,
    THEME_PATTERN_CLOCK,
    THEME_PATTERN_COUNTER,
    THEME_PATTERN_DIVIDER,
    THEME_PATTERN_FOCUS_ANIMATION,
    THEME_PATTERN_GRID,
    THEME_PATTERN_IMAGE,
    THEME_PATTERN_LIST,
    THEME_PATTERN_LIST_ITEM,
    THEME_PATTERN_MARQUEE,
    THEME_PATTERN_NAVIGATION_BAR,
    THEME_PATTERN_PICKER,
    THEME_PATTERN_PIECE,
    THEME_PATTERN_POPUP,
    THEME_PATTERN_PROGRESS,
    THEME_PATTERN_QRCODE,
    THEME_PATTERN_RATING,
    THEME_PATTERN_REFRESH,
    THEME_PATTERN_SCROLL_BAR,
    THEME_PATTERN_SEARCH,
    THEME_PATTERN_SELECT,
    THEME_PATTERN_SLIDER,
    THEME_PATTERN_STEPPER,
    THEME_PATTERN_TAB,
    THEME_PATTERN_TEXT,
    THEME_PATTERN_TEXTFIELD,
    THEME_PATTERN_TEXT_OVERLAY,
    THEME_PATTERN_VIDEO,
    THEME_PATTERN_ICON,
    THEME_PATTERN_INDEXER,
    THEME_PATTERN_APP_BAR,
    THEME_PATTERN_ADVANCED_PATTERN,
    THEME_PATTERN_SECURITY_COMPONENT,
    THEME_PATTERN_SIDE_BAR,
    THEME_PATTERN_PATTERN_LOCK
};

bool IsDirExist(const std::string& path)
{
    char realPath[PATH_MAX] = { 0x00 };
    CHECK_NULL_RETURN(realpath(path.c_str(), realPath), false);
    DIR* dir = opendir(realPath);
    CHECK_NULL_RETURN(dir, false);
    closedir(dir);
    return true;
}

DimensionUnit ParseDimensionUnit(const std::string& unit)
{
    if (unit == "px") {
        return DimensionUnit::PX;
    } else if (unit == "fp") {
        return DimensionUnit::FP;
    } else if (unit == "lpx") {
        return DimensionUnit::LPX;
    } else if (unit == "%") {
        return DimensionUnit::PERCENT;
    } else {
        return DimensionUnit::VP;
    }
};

} // namespace

RefPtr<ResourceAdapter> ResourceAdapter::Create()
{
    return AceType::MakeRefPtr<ResourceAdapterImpl>();
}

void ResourceAdapterImpl::Init(const ResourceInfo& resourceInfo)
{
    std::string packagePath = resourceInfo.GetPackagePath();
    resourcePathStr_ = packagePath;
    std::string sysResIndexPath = packagePath + DELIMITER + "systemres" + DELIMITER + "resources.index";
    auto resConfig = ConvertConfigToGlobal(resourceInfo.GetResourceConfiguration());
    resConfig_ = resConfig;
    auto hapPath = resourceInfo.GetHapPath();
    if (hapPath.empty()) {
        LOGI("sysResIndexPath: %s", sysResIndexPath.c_str());
        std::shared_ptr<Global::Resource::ResourceManager> newResMgr(Global::Resource::CreateResourceManager());
        auto sysResRet = newResMgr->AddResource(sysResIndexPath.c_str());
        auto configRet = newResMgr->UpdateResConfig(*resConfig);
        LOGI("AddSysRes result=%{public}d, UpdateResConfig result=%{public}d,"
             "ori=%{public}d, dpi=%{public}d, device=%{public}d, colorMode=%{public}d,",
            sysResRet, configRet, resConfig->GetDirection(), resConfig->GetScreenDensity(),
            resConfig->GetDeviceType(), resConfig->GetColorMode());
        resourceManager_ = newResMgr;
        packagePathStr_ = "";
    } else {
        std::istringstream iss(hapPath);
        std::string token;
        while (std::getline(iss, token, ':')) {
            std::shared_ptr<Global::Resource::ResourceManager> newResMgr(Global::Resource::CreateResourceManager());
            if (!newResMgr) {
                LOGE("create resource manager from Global::Resource::CreateResourceManager() failed!");
            }
            std::string appResIndexPath = token + DELIMITER + "resources.index";
            auto appResRet = newResMgr->AddResource(appResIndexPath.c_str());
            LOGI("sysResIndexPath: %s", sysResIndexPath.c_str());
            auto sysResRet = newResMgr->AddResource(sysResIndexPath.c_str());

            auto configRet = newResMgr->UpdateResConfig(*resConfig);
            LOGI("AddAppRes result=%{public}d, AddSysRes result=%{public}d,  UpdateResConfig result=%{public}d,"
                 "ori=%{public}d, dpi=%{public}d, device=%{public}d, colorMode=%{public}d,",
                appResRet, sysResRet, configRet, resConfig->GetDirection(), resConfig->GetScreenDensity(),
                resConfig->GetDeviceType(), resConfig->GetColorMode());
            resourceManager_ = newResMgr;
            resourceManagers_[token.substr(token.rfind(DELIMITER) + 1)] = resourceManager_;
            packagePathStr_ = (IsDirExist(token) ? token : std::string());
            rawFilePaths_[token.substr(token.rfind(DELIMITER) + 1)] = packagePathStr_;
        }
    }
    Platform::AceApplicationInfoImpl::GetInstance().SetResourceManager(resourceManager_);
}

void ResourceAdapterImpl::UpdateConfig(const ResourceConfiguration& config, bool themeFlag)
{
    LOGI("UpdateConfig ori=%{public}d, dpi=%{public}d, device=%{public}d, "
        "colorMode=%{public}d,",
        config.GetOrientation(), config.GetDensity(), config.GetDeviceType(),
        config.GetColorMode());
    auto resConfig = ConvertConfigToGlobal(config);
    LOGI("UpdateConfig ori=%{public}d, dpi=%{public}d, device=%{public}d, "
        "colorMode=%{public}d, inputDevice=%{public}d",
        resConfig->GetDirection(), resConfig->GetScreenDensity(), resConfig->GetDeviceType(),
        resConfig->GetColorMode(), resConfig->GetInputDevice());
    resourceManager_->UpdateResConfig(*resConfig, themeFlag);
}

RefPtr<ThemeStyle> ResourceAdapterImpl::GetTheme(int32_t themeId)
{
    CheckThemeId(themeId);
    auto theme = AceType::MakeRefPtr<ResourceThemeStyle>(AceType::Claim(this));
    auto ret = resourceManager_->GetThemeById(themeId, theme->rawAttrs_);

    LOGI("theme themeId=%{public}d, ret=%{public}d, attr size=%{public}zu",
        themeId, ret, theme->rawAttrs_.size());
    if (!ret) {
        ret = resourceManager_->GetThemeById(OHOS_THEME_ID, theme->rawAttrs_);
    }
    std::string OHFlag = "ohos_"; // fit with resource/base/theme.json and pattern.json
    for (uint64_t i = 0; i < sizeof(PATTERN_MAP) / sizeof(PATTERN_MAP[0]); i++) {
        ResourceThemeStyle::RawAttrMap attrMap;
        std::string patternTag = PATTERN_MAP[i];
        std::string patternName =  OHFlag + PATTERN_MAP[i];
        ret = resourceManager_->GetPatternByName(patternName.c_str(), attrMap);
        LOGI("theme pattern[%{public}s, %{public}s], attr size=%{public}zu",
            patternTag.c_str(), patternName.c_str(), attrMap.size());
        if (attrMap.empty()) {
            continue;
        }
        theme->patternAttrs_[patternTag] = attrMap;
    }
    LOGI("theme themeId=%{public}d, ret=%{public}d, attr size=%{public}zu, pattern size=%{public}zu",
        themeId, ret, theme->rawAttrs_.size(), theme->patternAttrs_.size());
    if (theme->patternAttrs_.empty() && theme->rawAttrs_.empty()) {
        LOGW("Failed to get the theme resource. The default theme config is used.");
        return nullptr;
    }

    theme->ParseContent();
    theme->patternAttrs_.clear();

    auto& attrMap = theme->rawAttrs_;
    auto iter = attrMap.find(THEME_ATTR_BG_COLOR);
    if (iter != attrMap.end()) {
        auto& attribute = iter->second;
        if (!attribute.empty()) {
            Color bgColor = Color::FromString(attribute);
            theme->SetAttr(THEME_ATTR_BG_COLOR, { .type = ThemeConstantsType::COLOR, .value = bgColor });
        }
    }

    return theme;
}

Color ResourceAdapterImpl::GetColor(uint32_t resId)
{
    uint32_t result = 0;
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, Color(result));
    auto state = resourceManager_->GetColorById(resId, result);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetColor error, id=%{public}u", resId);
    }
    return Color(result);
}

Color ResourceAdapterImpl::GetColorByName(const std::string& resName)
{
    uint32_t result = 0;
    auto actualResName = GetActualResourceName(resName);
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, Color(result));
    auto state = resourceManager_->GetColorByName(actualResName.c_str(), result);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetColor error, resName=%{public}s", resName.c_str());
    }
    return Color(result);
}

Dimension ResourceAdapterImpl::GetDimension(uint32_t resId)
{
    float dimensionFloat = 0.0f;
    std::string unit;
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    if (resourceManager_) {
        auto state = resourceManager_->GetFloatById(resId, dimensionFloat, unit);
        if (state != Global::Resource::SUCCESS) {
            LOGE("GetDimension error, id=%{public}u", resId);
        }
        return Dimension(static_cast<double>(dimensionFloat), ParseDimensionUnit(unit));
    }
    return Dimension(static_cast<double>(dimensionFloat));
}

Dimension ResourceAdapterImpl::GetDimensionByName(const std::string& resName)
{
    float dimensionFloat = 0.0f;
    auto actualResName = GetActualResourceName(resName);
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, Dimension());
    std::string unit;
    auto state = resourceManager_->GetFloatByName(actualResName.c_str(), dimensionFloat, unit);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetDimension error, resName=%{public}s", resName.c_str());
    }
    return Dimension(static_cast<double>(dimensionFloat), ParseDimensionUnit(unit));
}

std::string ResourceAdapterImpl::GetString(uint32_t resId)
{
    std::string strResult = "";
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, strResult);
    auto state = resourceManager_->GetStringById(resId, strResult);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetString error, id=%{public}u", resId);
    }
    return strResult;
}

std::string ResourceAdapterImpl::GetStringByName(const std::string& resName)
{
    std::string strResult = "";
    auto actualResName = GetActualResourceName(resName);
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, strResult);
    auto state = resourceManager_->GetStringByName(actualResName.c_str(), strResult);
    if (state != Global::Resource::SUCCESS) {
        LOGD("GetString error, resName=%{public}s", resName.c_str());
    }
    return strResult;
}

std::string ResourceAdapterImpl::GetPluralString(uint32_t resId, int quantity)
{
    std::string strResult = "";
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, strResult);
    auto state = resourceManager_->GetPluralStringById(resId, quantity, strResult);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetPluralString error, id=%{public}u", resId);
    }
    return strResult;
}

std::string ResourceAdapterImpl::GetPluralStringByName(const std::string& resName, int quantity)
{
    std::string strResult = "";
    auto actualResName = GetActualResourceName(resName);
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, strResult);
    auto state = resourceManager_->GetPluralStringByName(actualResName.c_str(), quantity, strResult);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetPluralString error, resName=%{public}s", resName.c_str());
    }
    return strResult;
}

std::vector<std::string> ResourceAdapterImpl::GetStringArray(uint32_t resId) const
{
    std::vector<std::string> strResults;
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, strResults);
    auto state = resourceManager_->GetStringArrayById(resId, strResults);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetStringArray error, id=%{public}u", resId);
    }
    return strResults;
}

std::vector<std::string> ResourceAdapterImpl::GetStringArrayByName(const std::string& resName) const
{
    std::vector<std::string> strResults;
    auto actualResName = GetActualResourceName(resName);
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, strResults);
    auto state = resourceManager_->GetStringArrayByName(actualResName.c_str(), strResults);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetStringArray error, resName=%{public}s", resName.c_str());
    }
    return strResults;
}

double ResourceAdapterImpl::GetDouble(uint32_t resId)
{
    float result = 0.0f;
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, static_cast<double>(result));
    auto state = resourceManager_->GetFloatById(resId, result);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetDouble error, id=%{public}u", resId);
    }
    return static_cast<double>(result);
}

double ResourceAdapterImpl::GetDoubleByName(const std::string& resName)
{
    float result = 0.0f;
    auto actualResName = GetActualResourceName(resName);
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, static_cast<double>(result));
    auto state = resourceManager_->GetFloatByName(actualResName.c_str(), result);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetDouble error, resName=%{public}s", resName.c_str());
    }
    return static_cast<double>(result);
}

int32_t ResourceAdapterImpl::GetInt(uint32_t resId)
{
    int32_t result = 0;
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, result);
    auto state = resourceManager_->GetIntegerById(resId, result);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetInt error, id=%{public}u", resId);
    }
    return result;
}

int32_t ResourceAdapterImpl::GetIntByName(const std::string& resName)
{
    int32_t result = 0;
    auto actualResName = GetActualResourceName(resName);
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, result);
    auto state = resourceManager_->GetIntegerByName(actualResName.c_str(), result);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetInt error, resName=%{public}s", resName.c_str());
    }
    return result;
}

std::vector<uint32_t> ResourceAdapterImpl::GetIntArray(uint32_t resId) const
{
    std::vector<int> intVectorResult;
    {
        std::shared_lock<std::shared_mutex> lock(resourceMutex_);
        if (resourceManager_) {
            auto state = resourceManager_->GetIntArrayById(resId, intVectorResult);
            if (state != Global::Resource::SUCCESS) {
                LOGE("GetIntArray error, id=%{public}u", resId);
            }
        }
    }

    std::vector<uint32_t> result;
    std::transform(
        intVectorResult.begin(), intVectorResult.end(), result.begin(), [](int x) { return static_cast<uint32_t>(x); });
    return result;
}

std::vector<uint32_t> ResourceAdapterImpl::GetIntArrayByName(const std::string& resName) const
{
    std::vector<int> intVectorResult;
    auto actualResName = GetActualResourceName(resName);
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, {});
    auto state = resourceManager_->GetIntArrayByName(actualResName.c_str(), intVectorResult);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetIntArray error, resName=%{public}s", resName.c_str());
    }

    std::vector<uint32_t> result;
    std::transform(
        intVectorResult.begin(), intVectorResult.end(), result.begin(), [](int x) { return static_cast<uint32_t>(x); });
    return result;
}

bool ResourceAdapterImpl::GetBoolean(uint32_t resId) const
{
    bool result = false;
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, result);
    auto state = resourceManager_->GetBooleanById(resId, result);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetBoolean error, id=%{public}u", resId);
    }
    return result;
}

bool ResourceAdapterImpl::GetBooleanByName(const std::string& resName) const
{
    bool result = false;
    auto actualResName = GetActualResourceName(resName);
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, result);
    auto state = resourceManager_->GetBooleanByName(actualResName.c_str(), result);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetBoolean error, resName=%{public}s", resName.c_str());
    }
    return result;
}

std::string ResourceAdapterImpl::GetMediaPath(uint32_t resId)
{
    CHECK_NULL_RETURN(resourceManager_, "");
    std::string mediaPath = "";
    {
        std::shared_lock<std::shared_mutex> lock(resourceMutex_);
        auto state = resourceManager_->GetMediaById(resId, mediaPath);
        if (state != Global::Resource::SUCCESS) {
            LOGE("GetMediaPath error, id=%{public}u", resId);
            return "";
        }
    }
    return "file:///" + mediaPath;
}

std::string ResourceAdapterImpl::GetMediaPathByName(const std::string& resName)
{
    std::string mediaPath = "";
    auto actualResName = GetActualResourceName(resName);
    {
        std::shared_lock<std::shared_mutex> lock(resourceMutex_);
        CHECK_NULL_RETURN(resourceManager_, "");
        auto state = resourceManager_->GetMediaByName(actualResName.c_str(), mediaPath);
        if (state != Global::Resource::SUCCESS) {
            LOGE("GetMediaPathByName error, resName=%{public}s, errorCode=%{public}u", resName.c_str(), state);
            return "";
        }
    }
    return "file:///" + mediaPath;
}

std::string ResourceAdapterImpl::GetRawfile(const std::string& fileName)
{
    return "file:///" + packagePathStr_ + "/resources/rawfile/" + fileName;
}

bool ResourceAdapterImpl::GetRawFileData(const std::string& rawFile, size_t& len, std::unique_ptr<uint8_t[]>& dest)
{
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetRawFileFromHap(rawFile, len, dest);
    if (state != Global::Resource::SUCCESS || !dest) {
        LOGW("GetRawFileFromHap error, raw filename:%{public}s, error:%{public}u", rawFile.c_str(), state);
        return false;
    }
    return true;
}

bool ResourceAdapterImpl::GetRawFileData(const std::string& rawFile, size_t& len, std::unique_ptr<uint8_t[]>& dest,
    const std::string& bundleName, const std::string& moduleName)
{
    auto manager = GetResourceManager(bundleName, moduleName);
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetRawFileFromHap(rawFile, len, dest);
    if (state != Global::Resource::SUCCESS || !dest) {
        LOGE("GetRawFileFromHap error, raw filename:%{public}s, error:%{public}u", rawFile.c_str(), state);
        LOGW("GetRawFileFromHap error, raw filename:%{public}s, bundleName:%{public}s, moduleName:%{public}s, "
             "error:%{public}u",
            rawFile.c_str(), bundleName.c_str(), moduleName.c_str(), state);
        return false;
    }
    return true;
}

bool ResourceAdapterImpl::GetMediaData(uint32_t resId, size_t& len, std::unique_ptr<uint8_t[]>& dest)
{
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetMediaDataById(resId, len, dest);
    if (state != Global::Resource::SUCCESS) {
        LOGW("GetMediaDataById error, id=%{public}u, error:%{public}u", resId, state);
        return false;
    }
    return true;
}

bool ResourceAdapterImpl::GetMediaData(uint32_t resId, size_t& len, std::unique_ptr<uint8_t[]>& dest,
    const std::string& bundleName, const std::string& moduleName)
{
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    auto manager = GetResourceManager(bundleName, moduleName);
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetMediaDataById(resId, len, dest);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetMediaDataById error, id=%{public}u, error:%{public}u", resId, state);
        LOGW("GetMediaDataById error, id=%{public}u, bundleName:%{public}s, moduleName:%{public}s, error:%{public}u",
            resId, bundleName.c_str(), moduleName.c_str(), state);
        return false;
    }
    return true;
}

bool ResourceAdapterImpl::GetMediaData(const std::string& resName, size_t& len, std::unique_ptr<uint8_t[]>& dest)
{
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, false);
    auto state = resourceManager_->GetMediaDataByName(resName.c_str(), len, dest);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetMediaDataByName error, res=%{public}s, error:%{public}u", resName.c_str(), state);
        return false;
    }
    return true;
}

bool ResourceAdapterImpl::GetMediaData(const std::string& resName, size_t& len, std::unique_ptr<uint8_t[]>& dest,
    const std::string& bundleName, const std::string& moduleName)
{
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    auto manager = GetResourceManager(bundleName, moduleName);
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetMediaDataByName(resName.c_str(), len, dest);
    if (state != Global::Resource::SUCCESS) {
        LOGW("GetMediaDataByName error, res=%{public}s, bundleName:%{public}s, moduleName:%{public}s, error:%{public}u",
            resName.c_str(), bundleName.c_str(), moduleName.c_str(), state);
        return false;
    }
    return true;
}

std::shared_ptr<Global::Resource::ResourceManager> ResourceAdapterImpl::GetResourceManager(
    const std::string& bundleName, const std::string& moduleName)
{
    auto it = resourceManagers_.find(moduleName);
    if (it != resourceManagers_.end()) {
        packagePathStr_ = rawFilePaths_[moduleName];
        resourceManager_ = it->second;
    } else if (!moduleName.empty()) {
        AddResourceManagerByModuleName(moduleName);
    }
    return resourceManager_;
}

void ResourceAdapterImpl::UpdateResourceManager(const std::string& bundleName, const std::string& moduleName)
{
    auto it = resourceManagers_.find(moduleName);
    if (it != resourceManagers_.end()) {
        packagePathStr_ = rawFilePaths_[moduleName];
        resourceManager_ = it->second;
    } else if (!moduleName.empty()) {
        AddResourceManagerByModuleName(moduleName);
    }
}

bool ResourceAdapterImpl::GetRawFileDescription(
    const std::string& rawfileName, RawfileDescription& rawfileDescription) const
{
    OHOS::Global::Resource::ResourceManager::RawFileDescriptor descriptor;
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, false);
    auto state = resourceManager_->GetRawFileDescriptorFromHap(rawfileName, descriptor);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetRawfileDescription error, rawfileName=%{public}s, error:%{public}u", rawfileName.c_str(), state);
        return false;
    }
    rawfileDescription.fd = descriptor.fd;
    rawfileDescription.offset = descriptor.offset;
    rawfileDescription.length = descriptor.length;
    return true;
}

bool ResourceAdapterImpl::GetMediaById(const int32_t& resId, std::string& mediaPath) const
{
    std::shared_lock<std::shared_mutex> lock(resourceMutex_);
    CHECK_NULL_RETURN(resourceManager_, false);
    auto state = resourceManager_->GetMediaById(resId, mediaPath);
    if (state != Global::Resource::SUCCESS) {
        LOGE("GetMediaById error, resId=%{public}d, error:%{public}u", resId, state);
        return false;
    }
    return true;
}

std::string ResourceAdapterImpl::GetActualResourceName(const std::string& resName) const
{
    auto index = resName.find_last_of('.');
    if (index == std::string::npos) {
        LOGE("GetActualResourceName error, incorrect resName format.");
        return {};
    }
    return resName.substr(index + 1, resName.length() - index - 1);
}

void ResourceAdapterImpl::AddResourceManagerByModuleName(const std::string moduleName)
{
    std::shared_ptr<Global::Resource::ResourceManager> newResMgr(Global::Resource::CreateResourceManager());
    if (!newResMgr) {
        LOGE("create resource manager from Global::Resource::CreateResourceManager() failed!");
    }
    std::string appResIndexPath = resourcePathStr_ + DELIMITER + moduleName + DELIMITER + "resources.index";
    auto appResRet = newResMgr->AddResource(appResIndexPath.c_str());
    std::string sysResIndexPath = resourcePathStr_ + DELIMITER + "systemres" + DELIMITER + "resources.index";
    LOGI("sysResIndexPath: %s", sysResIndexPath.c_str());
    auto sysResRet = newResMgr->AddResource(sysResIndexPath.c_str());

    auto configRet = newResMgr->UpdateResConfig(*resConfig_);
    LOGI("AddAppRes result=%{public}d, AddSysRes result=%{public}d,  UpdateResConfig result=%{public}d,"
         "ori=%{public}d, dpi=%{public}d, device=%{public}d, colorMode=%{public}d,",
        appResRet, sysResRet, configRet, resConfig_->GetDirection(), resConfig_->GetScreenDensity(),
        resConfig_->GetDeviceType(), resConfig_->GetColorMode());
    resourceManager_ = newResMgr;
    resourceManagers_[moduleName] = resourceManager_;
    packagePathStr_ = resourcePathStr_ + DELIMITER + moduleName;
    rawFilePaths_[moduleName] = packagePathStr_;
}
} // namespace OHOS::Ace
