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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_PLATFORM_VIEW_PLATFORM_VIEW_IMPL_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_PLATFORM_VIEW_PLATFORM_VIEW_IMPL_H

#include "core/common/platformview/platform_view_interface.h"
#include "adapter/android/osal/platformview/resource/platform_view_delegate.h"
#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"

#include "core/components_ng/render/ext_surface_callback_interface.h"

namespace OHOS::Ace::NG {
class PlatformViewImpl : public PlatformViewInterface, public ExtSurfaceCallbackInterface {
    DECLARE_ACE_TYPE(PlatformViewImpl, NG::PlatformViewInterface)
public:
    PlatformViewImpl() = default;
    explicit PlatformViewImpl(const std::string& id)
        : id_(id)
    {}
    ~PlatformViewImpl() override;

    void InitPlatformView() override;

    void UpdatePlatformViewLayout(const NG::SizeF& drawSize, const NG::OffsetF& offset) override;
    void HandleTouchDown(const NG::OffsetF& offset) override;
    void HandleTouchUp(const NG::OffsetF& offset) override;
    void HandleTouchMove(const NG::OffsetF& offset) override;
    void HandleTouchCancel(const NG::OffsetF& offset) override;

    void ProcessSurfaceCreate() override;
    void ProcessSurfaceChange(int32_t width, int32_t height) override {}
    void ProcessSurfaceDestroy() override {}
    void ProcessTextureRefresh(int32_t instanceId, int64_t textureId) override;
    void RegisterTextureEvent(TextureRefreshEvent&& textureRefreshEvent) override;
    void RegisterPlatformViewReadyEvent(PlatformViewReadyEvent&& platformViewReadyEvent) override;
    void SetRenderSurface(const RefPtr<RenderSurface>& renderSurface) override;

    int32_t SetSurface() override;
    void Dispose() override;

private:
    std::string id_;
    WeakPtr<RenderSurface> renderSurface_;
    RefPtr<PlatformViewDelegate> platformViewDelegate_;

    TextureRefreshEvent textureRefreshCallback_;
    PlatformViewReadyEvent platformViewReadyCallback_;

    ACE_DISALLOW_COPY_AND_MOVE(PlatformViewImpl);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_OSAL_PLATFORM_VIEW_PLATFORM_VIEW_IMPL_H
