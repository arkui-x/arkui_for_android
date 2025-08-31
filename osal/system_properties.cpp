/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "base/utils/system_properties.h"

#include "base/log/log.h"

namespace OHOS::Ace {
namespace {

// Device type, same as w/ java in AceView
constexpr int32_t ORIENTATION_PORTRAIT = 1;
constexpr int32_t ORIENTATION_LANDSCAPE = 2;
constexpr char UNDEFINED_PARAM[] = "undefined parameter";
constexpr int32_t DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD = 20;

constexpr int32_t DEFAULT_VELOCITY_TRACKER_POINTNUMBER_VALUE = 20;

} // namespace

bool SystemProperties::isRound_ = false;
bool SystemProperties::isDeviceAccess_ = false;
bool SystemProperties::developerModeOn_ = false;
int32_t SystemProperties::deviceWidth_ = 0;
int32_t SystemProperties::deviceHeight_ = 0;
int32_t SystemProperties::devicePhysicalWidth_ = 0;
int32_t SystemProperties::devicePhysicalHeight_ = 0;
double SystemProperties::resolution_ = 1.0;
DeviceType SystemProperties::deviceType_ { DeviceType::PHONE };
DeviceOrientation SystemProperties::orientation_ { DeviceOrientation::PORTRAIT };
std::string SystemProperties::brand_ = INVALID_PARAM;
std::string SystemProperties::manufacturer_ = INVALID_PARAM;
std::string SystemProperties::model_ = INVALID_PARAM;
std::string SystemProperties::product_ = INVALID_PARAM;
std::string SystemProperties::apiVersion_ = INVALID_PARAM;
std::string SystemProperties::releaseType_ = INVALID_PARAM;
std::string SystemProperties::paramDeviceType_ = INVALID_PARAM;
int32_t SystemProperties::mcc_ = MCC_UNDEFINED;
int32_t SystemProperties::mnc_ = MNC_UNDEFINED;
ScreenShape SystemProperties::screenShape_ { ScreenShape::NOT_ROUND };
LongScreenType SystemProperties::LongScreen_ { LongScreenType::NOT_LONG };
bool SystemProperties::unZipHap_ = true;
bool SystemProperties::svgTraceEnable_ = false;
bool SystemProperties::rosenBackendEnabled_ = true;
bool SystemProperties::downloadByNetworkEnabled_ = false;
bool SystemProperties::recycleImageEnabled_ = false;
bool SystemProperties::isHookModeEnabled_ = false;
bool SystemProperties::syncDebugTraceEnable_ = false;
bool SystemProperties::measureDebugTraceEnable_ = false;
bool SystemProperties::safeAreaDebugTraceEnable_ = false;
bool SystemProperties::pixelRoundEnable_ = true;
bool SystemProperties::textTraceEnable_ = false;
bool SystemProperties::syntaxTraceEnable_ = false;
bool SystemProperties::accessibilityEnabled_ = false;
bool SystemProperties::windowAnimationEnabled_ = false;
bool SystemProperties::debugEnabled_ = false;
DebugFlags SystemProperties::debugFlags_ = 0;
std::atomic<bool> SystemProperties::debugBoundaryEnabled_(false);
bool SystemProperties::debugAutoUIEnabled_ = false;
bool SystemProperties::debugOffsetLogEnabled_ = false;
bool SystemProperties::extSurfaceEnabled_ = true;
uint32_t SystemProperties::dumpFrameCount_ = 0;
std::atomic<bool> SystemProperties::layoutTraceEnable_(false);
bool SystemProperties::buildTraceEnable_ = false;
bool SystemProperties::dynamicDetectionTraceEnable_ = false;
bool SystemProperties::enableScrollableItemPool_ = false;
bool SystemProperties::navigationBlurEnabled_ = true;
bool SystemProperties::forceSplitIgnoreOrientationEnabled_ = false;
std::optional<bool> SystemProperties::arkUIHookEnabled_;
bool SystemProperties::gridCacheEnabled_ = false;
bool SystemProperties::sideBarContainerBlurEnable_ = false;
std::atomic<bool> SystemProperties::acePerformanceMonitorEnable_(false);
std::atomic<bool> SystemProperties::focusCanBeActive_(true);
bool SystemProperties::aceCommercialLogEnable_ = false;
bool SystemProperties::imageFileCacheConvertAstc_ = false;
int32_t SystemProperties::imageFileCacheConvertAstcThreshold_ = 2;
std::atomic<bool> SystemProperties::traceInputEventEnable_(false);
bool SystemProperties::imageFrameworkEnable_ = true;
float SystemProperties::pageCount_ = 0.0f;
float SystemProperties::dragStartDampingRatio_ = 0.2f;
float SystemProperties::dragStartPanDisThreshold_ = 10.0f;
bool SystemProperties::accessTraceEnable_ = true;
uint32_t SystemProperties::canvasDebugMode_ = 0;
float SystemProperties::fontScale_ = 1.0f;

std::pair<float, float> SystemProperties::brightUpPercent_ = {};

bool SystemProperties::taskPriorityAdjustmentEnable_ = false;

int32_t SystemProperties::dragDropFrameworkStatus_ = 0;

bool SystemProperties::pageTransitionFrzEnabled_ = false;
bool SystemProperties::softPagetransition_ = false;

ACE_WEAK_SYM float SystemProperties::GetFontScale()
{
    return fontScale_;
}

int32_t SystemProperties::formSharedImageCacheThreshold_ = DEFAULT_FORM_SHARED_IMAGE_CACHE_THRESHOLD;
WidthLayoutBreakPoint SystemProperties::widthLayoutBreakpoints_ = WidthLayoutBreakPoint();
HeightLayoutBreakPoint SystemProperties::heightLayoutBreakpoints_ = HeightLayoutBreakPoint();
bool SystemProperties::syncLoadEnabled_ = false;

int32_t SystemProperties::velocityTrackerPointNumber_ = DEFAULT_VELOCITY_TRACKER_POINTNUMBER_VALUE;
bool SystemProperties::isVelocityWithinTimeWindow_ = true;
bool SystemProperties::isVelocityWithoutUpPoint_ = true;
bool SystemProperties::prebuildInMultiFrameEnabled_ = false;

bool SystemProperties::IsOpIncEnable()
{
    return false;
}

void SystemProperties::InitDeviceType(DeviceType type)
{
    deviceType_ = type;
}

DeviceType SystemProperties::GetDeviceType()
{
    return deviceType_;
}
void SystemProperties::InitDeviceInfo(
    int32_t deviceWidth, int32_t deviceHeight, int32_t orientation, double resolution, bool isRound)
{
    // SetDeviceOrientation should be earlier than deviceWidth/Height's initialization
    SetDeviceOrientation(orientation);

    isRound_ = isRound;
    resolution_ = resolution;
    deviceWidth_ = deviceWidth;
    deviceHeight_ = deviceHeight;
    if (isRound_) {
        screenShape_ = ScreenShape::ROUND;
    } else {
        screenShape_ = ScreenShape::NOT_ROUND;
    }
}

void SystemProperties::SetDeviceOrientation(int32_t orientation)
{
    if (orientation == ORIENTATION_PORTRAIT && orientation_ != DeviceOrientation::PORTRAIT) {
        std::swap(deviceWidth_, deviceHeight_);
        orientation_ = DeviceOrientation::PORTRAIT;
    } else if (orientation == ORIENTATION_LANDSCAPE && orientation_ != DeviceOrientation::LANDSCAPE) {
        std::swap(deviceWidth_, deviceHeight_);
        orientation_ = DeviceOrientation::LANDSCAPE;
    } else {
        LOGW("SetDeviceOrientation, undefined orientation or current orientation is same as the orientation to set");
    }
}

void SystemProperties::InitDeviceTypeBySystemProperty() {}

float SystemProperties::GetFontWeightScale()
{
    return 1.0f;
}

void SystemProperties::InitMccMnc(int32_t mcc, int32_t mnc)
{
    mcc_ = mcc;
    mnc_ = mnc;
}

bool SystemProperties::IsScoringEnabled(const std::string& name)
{
    return false;
}

bool SystemProperties::GetDebugEnabled()
{
#ifdef ACE_DEBUG_LOG
    return true;
#else
    return false;
#endif
}

bool SystemProperties::GetLayoutDetectEnabled()
{
    return false;
}

bool SystemProperties::IsSyscapExist(const char* cap)
{
    return false;
}

bool SystemProperties::IsApiVersionGreaterOrEqual(int majorVersion, int minorVersion, int patchVersion)
{
    return false;
}

std::string SystemProperties::GetLanguage()
{
    return UNDEFINED_PARAM;
}

std::string SystemProperties::GetRegion()
{
    return UNDEFINED_PARAM;
}

std::string SystemProperties::GetPartialUpdatePkg()
{
    return "";
}

int32_t SystemProperties::GetSvgMode()
{
    return 1;
}

bool SystemProperties::GetIsUseMemoryMonitor()
{
    return false;
}

bool SystemProperties::IsFormAnimationLimited()
{
    return false;
}

bool SystemProperties::GetDebugPixelMapSaveEnabled()
{
    return false;
}

bool SystemProperties::GetResourceDecoupling()
{
    return false;
}

bool SystemProperties::IsPCMode()
{
    return false;
}

int32_t SystemProperties::GetJankFrameThreshold()
{
    return 0;
}

bool SystemProperties::GetTitleStyleEnabled()
{
    return false;
}

std::string SystemProperties::GetCustomTitleFilePath()
{
    return UNDEFINED_PARAM;
}

bool SystemProperties::Is24HourClock()
{
    return false;
}

bool SystemProperties::GetDisplaySyncSkipEnabled()
{
    return true;
}

bool SystemProperties::GetNavigationBlurEnabled()
{
    return navigationBlurEnabled_;
}

bool SystemProperties::GetCacheNavigationNodeEnable()
{
    return false;
}

bool SystemProperties::GetForceSplitIgnoreOrientationEnabled()
{
    return forceSplitIgnoreOrientationEnabled_;
}

std::optional<bool> SystemProperties::GetArkUIHookEnabled()
{
    return arkUIHookEnabled_;
}

bool SystemProperties::GetGridCacheEnabled()
{
    return gridCacheEnabled_;
}

bool SystemProperties::GetGridIrregularLayoutEnabled()
{
    return false;
}

bool SystemProperties::GetSideBarContainerBlurEnable()
{
    return sideBarContainerBlurEnable_;
}

bool SystemProperties::WaterFlowUseSegmentedLayout()
{
    return false;
}

float SystemProperties::GetDefaultResolution()
{
    return 1.0f;
}

std::string SystemProperties::GetAtomicServiceBundleName()
{
    return UNDEFINED_PARAM;
}

float SystemProperties::GetDragStartDampingRatio()
{
    return dragStartDampingRatio_;
}

float SystemProperties::GetDragStartPanDistanceThreshold()
{
    return dragStartPanDisThreshold_;
}

int32_t SystemProperties::GetVelocityTrackerPointNumber()
{
    return velocityTrackerPointNumber_;
}

bool SystemProperties::IsVelocityWithinTimeWindow()
{
    return isVelocityWithinTimeWindow_;
}

bool SystemProperties::IsVelocityWithoutUpPoint()
{
    return isVelocityWithoutUpPoint_;
}

bool SystemProperties::IsSmallFoldProduct()
{
    return false;
}

std::string SystemProperties::GetWebDebugRenderMode()
{
    return UNDEFINED_PARAM;
}

std::string SystemProperties::GetDebugInspectorId()
{
    return UNDEFINED_PARAM;
}

double SystemProperties::GetSrollableVelocityScale()
{
    return 0.0;
}

double SystemProperties::GetSrollableFriction()
{
    return 0.0;
}

double SystemProperties::GetScrollableDistance()
{
    return 0.0;
}

bool SystemProperties::IsNeedResampleTouchPoints()
{
    return false;
}

bool SystemProperties::IsNeedSymbol()
{
    return true;
}

int32_t SystemProperties::GetDragDropFrameworkStatus()
{
    return dragDropFrameworkStatus_;
}

bool SystemProperties::IsSuperFoldDisplayDevice()
{
    return false;
}

bool SystemProperties::GetContainerDeleteFlag()
{
    return false;
}

bool SystemProperties::IsPageTransitionFreeze()
{
    return pageTransitionFrzEnabled_;
}

bool SystemProperties::IsSoftPageTransition()
{
    return softPagetransition_;
}

bool SystemProperties::GetMultiInstanceEnabled()
{
    return false;
}

bool SystemProperties::ConfigChangePerform()
{
    return false;
}

int32_t SystemProperties::getFormSharedImageCacheThreshold()
{
    return formSharedImageCacheThreshold_;
}

bool SystemProperties::IsWhiteBlockEnabled()
{
    return false;
}

bool SystemProperties::IsWhiteBlockIdleChange()
{
    return false;
}

int32_t SystemProperties::GetWhiteBlockIndexValue()
{
    return 0;
}

int32_t SystemProperties::GetWhiteBlockCacheCountValue()
{
    return 0;
}

int32_t SystemProperties::GetPreviewStatus()
{
    return -1;
}
} // namespace OHOS::Ace
