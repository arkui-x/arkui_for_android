/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "base/log/event_report.h"

#include <chrono>
#include "base/log/ace_trace.h"
#include "base/log/log.h"

namespace OHOS::Ace {
#define EVENT_REPORT_FORMAT_EVENT_COMPLETE "[ACE][INTERACTION_COMPLETED_LATENCY][BEHAVIOR]{" \
    "%{public}s:%{public}d, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}d, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}ld, " \
    "%{public}s:%{public}ld, " \
    "%{public}s:%{public}ld, " \
    "%{public}s:%{public}ld, " \
    "%{public}s:%{public}s}"
#define EVENT_REPORT_FORMAT_EVENT_JANK_FRAME "[ACE][INTERACTION_APP_JANK][BEHAVIOR]{" \
    "%{public}s:%{public}d, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}d, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}ld, " \
    "%{public}s:%{public}ld, " \
    "%{public}s:%{public}d, " \
    "%{public}s:%{public}d, " \
    "%{public}s:%{public}ld, " \
    "%{public}s:%{public}d, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}d}"
#define EVENT_REPORT_FORMAT_JANK_FRAME_APP "[ACE][JANK_FRAME_APP][FAULT]{" \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}d, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}ld}"
#define EVENT_REPORT_FORMAT_JANK_FRAME_FILTERED "[ACE][JANK_FRAME_FILTERED][BEHAVIOR]{" \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}d, " \
    "%{public}s:%{public}s, " \
    "%{public}s:%{public}ld}"

constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
constexpr char EVENT_KEY_STARTTIME[] = "STARTTIME";
constexpr char EVENT_KEY_VERSION_CODE[] = "VERSION_CODE";
constexpr char EVENT_KEY_VERSION_NAME[] = "VERSION_NAME";
constexpr char EVENT_KEY_BUNDLE_NAME[] = "BUNDLE_NAME";
constexpr char EVENT_KEY_ABILITY_NAME[] = "ABILITY_NAME";
constexpr char EVENT_KEY_PAGE_URL[] = "PAGE_URL";
constexpr char EVENT_KEY_APP_PID[] = "APP_PID";
constexpr char EVENT_KEY_SCENE_ID[] = "SCENE_ID";
constexpr char EVENT_KEY_INPUT_TIME[] = "INPUT_TIME";
constexpr char EVENT_KEY_ANIMATION_START_LATENCY[] = "ANIMATION_START_LATENCY";
constexpr char EVENT_KEY_ANIMATION_END_LATENCY[] = "ANIMATION_END_LATENCY";
constexpr char EVENT_KEY_E2E_LATENCY[] = "E2E_LATENCY";
constexpr char EVENT_KEY_UNIQUE_ID[] = "UNIQUE_ID";
constexpr char EVENT_KEY_MODULE_NAME[] = "MODULE_NAME";
constexpr char EVENT_KEY_DURITION[] = "DURITION";
constexpr char EVENT_KEY_TOTAL_FRAMES[] = "TOTAL_FRAMES";
constexpr char EVENT_KEY_TOTAL_MISSED_FRAMES[] = "TOTAL_MISSED_FRAMES";
constexpr char EVENT_KEY_MAX_FRAMETIME[] = "MAX_FRAMETIME";
constexpr char EVENT_KEY_MAX_SEQ_MISSED_FRAMES[] = "MAX_SEQ_MISSED_FRAMES";
constexpr char EVENT_KEY_SOURCE_TYPE[] = "SOURCE_TYPE";
constexpr char EVENT_KEY_NOTE[] = "NOTE";
constexpr char EVENT_KEY_DISPLAY_ANIMATOR[] = "DISPLAY_ANIMATOR";
constexpr char EVENT_KEY_SKIPPED_FRAME_TIME[] = "SKIPPED_FRAME_TIME";

void EventReport::SendEvent(const EventInfo& eventInfo) {}

void EventReport::SendAppStartException(AppStartExcepType type) {}

void EventReport::SendPageRouterException(PageRouterExcepType type, const std::string& pageUrl) {}

void EventReport::SendComponentException(ComponentExcepType type) {}

void EventReport::SendAPIChannelException(APIChannelExcepType type) {}

void EventReport::SendRenderException(RenderExcepType type) {}

void EventReport::SendJsException(JsExcepType type) {}

void EventReport::SendAnimationException(AnimationExcepType type) {}

void EventReport::SendEventException(EventExcepType type) {}

void EventReport::SendInternalException(InternalExcepType type) {}

void EventReport::SendAccessibilityException(AccessibilityExcepType type) {}

void EventReport::SendFormException(FormExcepType type) {}

void EventReport::JsEventReport(int32_t eventType, const std::string& jsonStr) {}

void EventReport::JsErrReport(const std::string& packageName,
    const std::string& reason, const std::string& summary) {}

void EventReport::ANRRawReport(RawEventType type, int32_t uid, const std::string& packageName,
                               const std::string& processName, const std::string& msg) {}

void EventReport::ANRShowDialog(int32_t uid, const std::string& packageName,
                                const std::string& processName, const std::string& msg) {}

void EventReport::JankFrameReport(int64_t startTime, int64_t duration, const std::vector<uint16_t>& jank,
                                  const std::string& pageUrl, uint32_t jankStatusVersion) {}

void EventReport::SendEventInner(const EventInfo& eventInfo) {}

void EventReport::ReportEventComplete(DataBase& data)
{
    // EventName: INTERACTION_COMPLETED_LATENCY
    const auto& appPid = data.baseInfo.pid;
    const auto& bundleName = data.baseInfo.bundleName;
    const auto& processName = data.baseInfo.processName;
    const auto& abilityName = data.baseInfo.abilityName;
    const auto& pageUrl = data.baseInfo.pageUrl;
    const auto& versionCode = data.baseInfo.versionCode;
    const auto& versionName = data.baseInfo.versionName;
    const auto& sceneId = data.sceneId;
    const auto& sourceType = GetSourceTypeName(data.sourceType);
    auto inputTime = data.inputTime;
    ConvertRealtimeToSystime(data.inputTime, inputTime);
    const auto& animationStartLantency = (data.beginVsyncTime - data.inputTime) / NS_TO_MS;
    const auto& animationEndLantency = (data.endVsyncTime - data.beginVsyncTime) / NS_TO_MS;
    const auto& e2eLatency = animationStartLantency + animationEndLantency;
    const auto& note = data.baseInfo.note;
    LOGE(EVENT_REPORT_FORMAT_EVENT_COMPLETE,
        EVENT_KEY_APP_PID, appPid,
        EVENT_KEY_BUNDLE_NAME, bundleName.c_str(),
        EVENT_KEY_PROCESS_NAME, processName.c_str(),
        EVENT_KEY_ABILITY_NAME, abilityName.c_str(),
        EVENT_KEY_PAGE_URL, pageUrl.c_str(),
        EVENT_KEY_VERSION_CODE, versionCode,
        EVENT_KEY_VERSION_NAME, versionName.c_str(),
        EVENT_KEY_SCENE_ID, sceneId.c_str(),
        EVENT_KEY_SOURCE_TYPE, sourceType.c_str(),
        EVENT_KEY_INPUT_TIME, static_cast<uint64_t>(inputTime),
        EVENT_KEY_ANIMATION_START_LATENCY, static_cast<uint64_t>(animationStartLantency),
        EVENT_KEY_ANIMATION_END_LATENCY, static_cast<uint64_t>(animationEndLantency),
        EVENT_KEY_E2E_LATENCY, static_cast<uint64_t>(e2eLatency),
        EVENT_KEY_NOTE, note.c_str());
    ACE_SCOPED_TRACE("INTERACTION_COMPLETED_LATENCY: sceneId =%s, inputTime=%lld(ms),"
        "e2eLatency=%lld(ms)", sceneId.c_str(),
        static_cast<long long>(inputTime), static_cast<long long>(e2eLatency));
}

void EventReport::ReportEventJankFrame(DataBase& data)
{
    // EventName: INTERACTION_APP_JANK
    const auto& uniqueId = data.beginVsyncTime / NS_TO_MS;
    const auto& sceneId = data.sceneId;
    const auto& bundleName = data.baseInfo.bundleName;
    const auto& processName = data.baseInfo.processName;
    const auto& abilityName = data.baseInfo.abilityName;
    const auto& pageUrl = data.baseInfo.pageUrl;
    const auto& versionCode = data.baseInfo.versionCode;
    const auto& versionName = data.baseInfo.versionName;
    auto startTime = data.beginVsyncTime;
    ConvertRealtimeToSystime(data.beginVsyncTime, startTime);
    const auto& durition = (data.endVsyncTime - data.beginVsyncTime) / NS_TO_MS;
    const auto& totalFrames = data.totalFrames;
    const auto& totalMissedFrames = data.totalMissed;
    const auto& maxFrameTime = data.maxFrameTime / NS_TO_MS;
    const auto& maxSeqMissedFrames = data.maxSuccessiveFrames;
    const auto& note = data.baseInfo.note;
    const auto& isDisplayAnimator = data.isDisplayAnimator;
    LOGE(EVENT_REPORT_FORMAT_EVENT_JANK_FRAME,
        EVENT_KEY_UNIQUE_ID, static_cast<int32_t>(uniqueId),
        EVENT_KEY_SCENE_ID, sceneId.c_str(),
        EVENT_KEY_PROCESS_NAME, processName.c_str(),
        EVENT_KEY_MODULE_NAME, bundleName.c_str(),
        EVENT_KEY_ABILITY_NAME, abilityName.c_str(),
        EVENT_KEY_PAGE_URL, pageUrl.c_str(),
        EVENT_KEY_VERSION_CODE, versionCode,
        EVENT_KEY_VERSION_NAME, versionName.c_str(),
        EVENT_KEY_STARTTIME, static_cast<uint64_t>(startTime),
        EVENT_KEY_DURITION, static_cast<uint64_t>(durition),
        EVENT_KEY_TOTAL_FRAMES, totalFrames,
        EVENT_KEY_TOTAL_MISSED_FRAMES, totalMissedFrames,
        EVENT_KEY_MAX_FRAMETIME, static_cast<uint64_t>(maxFrameTime),
        EVENT_KEY_MAX_SEQ_MISSED_FRAMES, maxSeqMissedFrames,
        EVENT_KEY_NOTE, note.c_str(),
        EVENT_KEY_DISPLAY_ANIMATOR, isDisplayAnimator);
    ACE_SCOPED_TRACE("INTERACTION_APP_JANK: sceneId =%s, startTime=%lld(ms),"
        "maxFrameTime=%lld(ms)", sceneId.c_str(),
        static_cast<long long>(startTime), static_cast<long long>(maxFrameTime));
}

void EventReport::ReportJankFrameApp(JankInfo& info)
{
    // EventName: JANK_FRAME_APP
    const auto& bundleName = info.baseInfo.bundleName;
    const auto& processName = info.baseInfo.processName;
    const auto& abilityName = info.baseInfo.abilityName;
    const auto& pageUrl = info.baseInfo.pageUrl;
    const auto& versionCode = info.baseInfo.versionCode;
    const auto& versionName = info.baseInfo.versionName;
    const auto& skippedFrameTime = info.skippedFrameTime;
    LOGE(EVENT_REPORT_FORMAT_JANK_FRAME_APP,
        EVENT_KEY_PROCESS_NAME, processName.c_str(),
        EVENT_KEY_MODULE_NAME, bundleName.c_str(),
        EVENT_KEY_ABILITY_NAME, abilityName.c_str(),
        EVENT_KEY_PAGE_URL, pageUrl.c_str(),
        EVENT_KEY_VERSION_CODE, versionCode,
        EVENT_KEY_VERSION_NAME, versionName.c_str(),
        EVENT_KEY_SKIPPED_FRAME_TIME, static_cast<uint64_t>(skippedFrameTime));
    ACE_SCOPED_TRACE("JANK_FRAME_APP: skipppedFrameTime=%lld(ms)",
        static_cast<long long>(skippedFrameTime / NS_TO_MS));
}

void EventReport::ReportJankFrameFiltered(JankInfo& info)
{
    // EventName: JANK_FRAME_FILTERED
    const auto& bundleName = info.baseInfo.bundleName;
    const auto& processName = info.baseInfo.processName;
    const auto& abilityName = info.baseInfo.abilityName;
    const auto& pageUrl = info.baseInfo.pageUrl;
    const auto& versionCode = info.baseInfo.versionCode;
    const auto& versionName = info.baseInfo.versionName;
    const auto& skippedFrameTime = info.skippedFrameTime;
    const auto& windowName = info.windowName;
    LOGE(EVENT_REPORT_FORMAT_JANK_FRAME_FILTERED,
        EVENT_KEY_PROCESS_NAME, processName.c_str(),
        EVENT_KEY_MODULE_NAME, bundleName.c_str(),
        EVENT_KEY_ABILITY_NAME, abilityName.c_str(),
        EVENT_KEY_PAGE_URL, pageUrl.c_str(),
        EVENT_KEY_VERSION_CODE, versionCode,
        EVENT_KEY_VERSION_NAME, versionName.c_str(),
        EVENT_KEY_SKIPPED_FRAME_TIME, static_cast<uint64_t>(skippedFrameTime));
    ACE_SCOPED_TRACE("JANK_FRAME_FILTERED: skipppedFrameTime=%lld(ms), windowName=%s",
        static_cast<long long>(skippedFrameTime / NS_TO_MS), windowName.c_str());
}

void EventReport::ReportDoubleClickTitle(int32_t stateChange) {}

void EventReport::ReportClickTitleMaximizeMenu(int32_t maxMenuItem, int32_t stateChange) {}

void EventReport::ReportPageNodeOverflow(const std::string& pageUrl, int32_t nodeCount, int32_t threshold) {}

void EventReport::ReportPageDepthOverflow(const std::string& pageUrl, int32_t depth, int32_t threshold) {}

void EventReport::ReportFunctionTimeout(const std::string& functionName, int64_t time, int32_t threshold) {}
} // namespace OHOS::Ace
