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

#include "base/perfmonitor/perf_interfaces.h"

namespace OHOS::Ace {

void PerfInterfaces::SetScrollState(bool state)
{
}

void PerfInterfaces::RecordInputEvent(PerfActionType type, PerfSourceType sourceType, int64_t time)
{
}

int64_t PerfInterfaces::GetInputTime(const std::string& sceneId, PerfActionType type, const std::string& note)
{
    return 0;
}

void PerfInterfaces::NotifyAppJankStatsBegin()
{
}

void PerfInterfaces::NotifyAppJankStatsEnd()
{
}

void PerfInterfaces::SetPageUrl(const std::string& pageUrl)
{
}

std::string PerfInterfaces::GetPageUrl()
{
    return {};
}

void PerfInterfaces::SetPageName(const std::string& pageName)
{
}

std::string PerfInterfaces::GetPageName()
{
    return {};
}

void PerfInterfaces::SetAppForeground(bool isShow)
{
}

void PerfInterfaces::SetAppStartStatus()
{
}

bool PerfInterfaces::IsScrollJank(const std::string& sceneId)
{
    return false;
}

void PerfInterfaces::Start(const std::string& sceneId, PerfActionType type, const std::string& note)
{
}

void PerfInterfaces::End(const std::string& sceneId, bool isRsRender)
{
}

void PerfInterfaces::StartCommercial(const std::string& sceneId, PerfActionType type, const std::string& note)
{
}

void PerfInterfaces::EndCommercial(const std::string& sceneId, bool isRsRender)
{
}

void PerfInterfaces::SetFrameTime(int64_t vsyncTime, int64_t duration, double jank, const std::string& windowName)
{
}

void PerfInterfaces::ReportJankFrameApp(double jank, int32_t jankThreshold)
{
}

void PerfInterfaces::ReportPageShowMsg(const std::string& pageUrl, const std::string& bundleName,
    const std::string& pageName)
{
}

} // namespace OHOS::Ace
