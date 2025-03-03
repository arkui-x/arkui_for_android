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
#include "core/event/statusbar/statusbar_event_proxy.h"

namespace OHOS::Ace {
std::unique_ptr<StatusBarEventProxy> StatusBarEventProxy::instance_;
std::mutex StatusBarEventProxy::mutex_;

StatusBarEventProxy* StatusBarEventProxy::GetInstance()
{
    return nullptr;
}
} // namespace OHOS::Ace
