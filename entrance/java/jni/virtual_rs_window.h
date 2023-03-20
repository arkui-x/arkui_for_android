/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_VIRTUAL_RS_WINDOW_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_VIRTUAL_RS_WINDOW_H

#include <memory>

#include "flutter/shell/common/vsync_waiter.h"
#include "refbase.h"
#include "render_service_client/core/ui/rs_surface_node.h"

#include "base/utils/noncopyable.h"

namespace OHOS {
namespace Rosen {
constexpr uint32_t INVALID_WINDOW_ID = 0;
using OnCallback = std::function<void(int64_t)>;
struct VsyncCallback {
    OnCallback onCallback;
};

class Window : public RefBase {
public:
    explicit Window(const flutter::TaskRunners& taskRunners);
    ~Window() override = default;

    virtual void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback);

    void CreateSurfaceNode(void* nativeWindow);

    std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const
    {
        return surfaceNode_;
    }

private:
    std::shared_ptr<RSSurfaceNode> surfaceNode_;
    std::shared_ptr<flutter::VsyncWaiter> vsyncWaiter_;

    DISALLOW_COPY_AND_MOVE(Window);
};

} // namespace Rosen
} // namespace OHOS
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_VIRTUAL_RS_WINDOW_H
