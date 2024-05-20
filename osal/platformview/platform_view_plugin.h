/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_PLATFORMVIEW_PLATFORM_VIEW_PLUGIN_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_PLATFORMVIEW_PLATFORM_VIEW_PLUGIN_H

#include "base/utils/noncopyable.h"

#include "core/common/platformview/platform_view.h"

namespace OHOS::Ace::Platform {

class PlatformViewPlugin final : public PlatformView {
public:
    PlatformViewPlugin() = default;
    ~PlatformViewPlugin() = default;

    RefPtr<NG::PlatformViewInterface> Attach(const std::string& id) override;

private:
    ACE_DISALLOW_COPY_AND_MOVE(PlatformViewPlugin);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_PLATFORMVIEW_PLATFORM_VIEW_PLUGIN_H