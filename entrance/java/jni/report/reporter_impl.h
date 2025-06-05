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

#ifndef FOUNDATION_ACE_ADAPTER_PREVIEW_ENTRANCE_REPORTER_IMPL_H
#define FOUNDATION_ACE_ADAPTER_PREVIEW_ENTRANCE_REPORTER_IMPL_H

#include "frameworks/core/common/reporter/reporter.h"
#include "frameworks/core/components_ng/manager/event/json_report.h"

namespace OHOS::Ace::NG {
class ReporterImpl : public Reporter {
    DECLARE_ACE_TYPE(ReporterImpl, Reporter);

public:
    void HandleUISessionReporting(const JsonReport& report) const override;
};
} // namespace OHOS::Ace::NG
#endif
