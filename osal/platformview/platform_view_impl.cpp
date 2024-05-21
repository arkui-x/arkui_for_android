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

#include "adapter/android/osal/platformview/platform_view_impl.h"
#ifdef RENDER_EXTRACT_SUPPORTED
#include "core/components_ng/render/adapter/render_texture_impl.h"
#endif
#include "base/memory/referenced.h"
#include "base/thread/task_executor.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

PlatformViewImpl::~PlatformViewImpl()
{
    if (platformViewDelegate_) {
        platformViewDelegate_->Release();
        platformViewDelegate_ = nullptr;
    }
}

void PlatformViewImpl::InitPlatformView()
{
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto uiTaskExecutor = SingleTaskExecutor::Make(container->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    auto errorCallback = [weak = WeakClaim(this), uiTaskExecutor](
                             const std::string& errorId, const std::string& param) {
        uiTaskExecutor.PostSyncTask([weak, errorId, param] {}, "ArkUI-XPlatformViewImplInitPlatformView");
    };
    platformViewDelegate_ = AceType::MakeRefPtr<PlatformViewDelegate>(container->GetPipelineContext(), errorCallback);
    platformViewDelegate_->Create(id_);
    platformViewDelegate_->SetPlatformViewReadyCallback([weak = WeakClaim(this)]() {
        auto delegate_ = weak.Upgrade();
        CHECK_NULL_VOID(delegate_);
        auto callback = delegate_->platformViewReadyCallback_;
        CHECK_NULL_VOID(callback);
        callback();
    });
}

void PlatformViewImpl::UpdatePlatformViewLayout(const NG::SizeF& drawSize, const NG::OffsetF& offset)
{
    platformViewDelegate_->UpdatePlatformViewLayout(drawSize, offset);
}

void PlatformViewImpl::HandleTouchDown(const NG::OffsetF& offset)
{
    platformViewDelegate_->HandleTouchDown(offset);
}
void PlatformViewImpl::HandleTouchUp(const NG::OffsetF& offset)
{
    platformViewDelegate_->HandleTouchUp(offset);
}
void PlatformViewImpl::HandleTouchMove(const NG::OffsetF& offset)
{
    platformViewDelegate_->HandleTouchMove(offset);
}
void PlatformViewImpl::HandleTouchCancel(const NG::OffsetF& offset)
{
    platformViewDelegate_->HandleTouchCancel(offset);
}

void PlatformViewImpl::ProcessSurfaceCreate()
{
    SetSurface();
}

void PlatformViewImpl::ProcessTextureRefresh(int32_t instanceId, int64_t textureId)
{
    if (textureRefreshCallback_) {
        textureRefreshCallback_(instanceId, textureId);
    }
}

void PlatformViewImpl::RegisterTextureEvent(TextureRefreshEvent&& textureRefreshEvent)
{
    textureRefreshCallback_ = textureRefreshEvent;
}

void PlatformViewImpl::RegisterPlatformViewReadyEvent(PlatformViewReadyEvent&& platformViewReadyEvent)
{
    platformViewReadyCallback_ = platformViewReadyEvent;
}

void PlatformViewImpl::SetRenderSurface(const RefPtr<RenderSurface>& renderSurface)
{
    renderSurface_ = renderSurface;
    auto surfaceImpl = AceType::DynamicCast<RenderTextureImpl>(renderSurface);
    surfaceImpl->SetExtSurfaceCallback(AceType::Claim(this));
}

int32_t PlatformViewImpl::SetSurface()
{
    CHECK_NULL_RETURN(platformViewDelegate_, -1);
    auto renderSurface = renderSurface_.Upgrade();
    CHECK_NULL_RETURN(renderSurface, -1);
    auto textureImpl = AceType::DynamicCast<RenderTextureImpl>(renderSurface);
    CHECK_NULL_RETURN(textureImpl, -1);
    platformViewDelegate_->RegisterPlatformViewTexture(textureImpl->GetTextureId(), id_);
    return 0;
}

void PlatformViewImpl::Dispose()
{
    CHECK_NULL_VOID(platformViewDelegate_);
    platformViewDelegate_->Dispose();
}
} // namespace OHOS::Ace::NG
