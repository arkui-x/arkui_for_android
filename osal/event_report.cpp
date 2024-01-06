/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

namespace OHOS::Ace {

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

void EventReport::ReportEventComplete(DataBase& data) {}

void EventReport::ReportEventJankFrame(DataBase& data) {}

void EventReport::ReportJankFrameApp(JankInfo& info) {}

void EventReport::ReportDoubleClickTitle(int32_t stateChange) {}

void EventReport::ReportClickTitleMaximizeMenu(int32_t maxMenuItem, int32_t stateChange) {}
} // namespace OHOS::Ace
