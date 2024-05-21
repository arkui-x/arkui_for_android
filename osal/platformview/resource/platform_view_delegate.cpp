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

#include "adapter/android/osal/platformview/resource/platform_view_delegate.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace {
const char REGISTER_PLATFORM_VIEW[] = "registerPlatformView";
const char PLATFORM_VIEW_READY[] = "platformViewReady";
const char UPDATE_PLATFORM_VIEW_LAYOUT[] = "updateLayout";
const char PLATFORM_VIEW_TOUCH_DOWN[] = "touchDown";
const char PLATFORM_VIEW_TOUCH_UP[] = "touchUp";
const char PLATFORM_VIEW_TOUCH_MOVE[] = "touchMove";
const char PLATFORM_VIEW_TOUCH_CANCEL[] = "touchCancel";
const char DISPOSE_PLATFORM_VIEW[] = "dispose";
const char PLATFORM_VIEW_HEIGHT[] = "platformViewHeight";
const char PLATFORM_VIEW_WIDTH[] = "platformViewWidth";
const char PLATFORM_VIEW_TOP[] = "platformViewTop";
const char PLATFORM_VIEW_LEFT[] = "platformViewLeft";
const char PLATFORM_VIEW_TOUCH_POINT_OFFSET_X[] = "platformViewTouchPointOffsetX";
const char PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y[] = "platformViewTouchPointOffsetY";
const char VIEW_TAG[] = "viewTag";
const char TEXTURE_ID[] = "textureId";
const char INSTANCE_ID[] = "instanceId";
const char PLATFORM_VIEW_ERROR_CODE_CREATEFAIL[] = "error_platform_view_000001";
const char PLATFORM_VIEW_ERROR_MSG_CREATEFAIL[] = "Create platformView failed.";

PlatformViewDelegate::~PlatformViewDelegate()
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto resRegister = context->GetPlatformResRegister();
    CHECK_NULL_VOID(resRegister);
    auto platformTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::PLATFORM);
    if (platformTaskExecutor.IsRunOnCurrentThread()) {
        resRegister->UnregisterEvent(MakeEventHash(PLATFORM_VIEW_READY));
    } else {
        WeakPtr<PlatformResRegister> weak = resRegister;
        platformTaskExecutor.PostTask([eventHash = MakeEventHash(PLATFORM_VIEW_READY), weak] {
            auto resRegister = weak.Upgrade();
            CHECK_NULL_VOID(resRegister);
            resRegister->UnregisterEvent(eventHash);
        }, "ArkUI-XPlatformViewDelegatePlatformViewDelegate");
    }
}

void PlatformViewDelegate::Create(const std::string& viewTag)
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);

    auto platformTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::PLATFORM);
    platformTaskExecutor.PostSyncTask([weak = WeakClaim(this), viewTag] {
        auto platformView = weak.Upgrade();
        if (platformView) {
            platformView->CreatePlatformView(viewTag);
        }
    }, "ArkUI-XPlatformViewDelegateCreate");
}

void PlatformViewDelegate::CreatePlatformView(const std::string& viewTag)
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto resRegister = context->GetPlatformResRegister();
    CHECK_NULL_VOID(resRegister);
    std::stringstream paramStream;
    paramStream << VIEW_TAG << PLATFORM_VIEW_PARAM_EQUALS << viewTag;
    std::string param = paramStream.str();
    id_ = resRegister->CreateResource(type_, param);
    if (id_ == PLATFORM_VIEW_INVALID_ID) {
        onError_(PLATFORM_VIEW_ERROR_CODE_CREATEFAIL, PLATFORM_VIEW_ERROR_MSG_CREATEFAIL);
        return;
    }
    hash_ = MakeResourceHash();

    resRegister->RegisterEvent(
        MakeEventHash(PLATFORM_VIEW_READY), [weak = WeakClaim(this)](const std::string& param) {
            auto platformView = weak.Upgrade();
            if (platformView) {
                if (platformView->onPlatformViewReady_) {
                    platformView->onPlatformViewReady_();
                }
            }
        });
}

void PlatformViewDelegate::RegisterPlatformViewTexture(int64_t textureId, std::string& viewTag)
{
    std::stringstream paramStream;
    paramStream << TEXTURE_ID << PLATFORM_VIEW_PARAM_EQUALS << textureId << PLATFORM_VIEW_PARAM_AND << VIEW_TAG << PLATFORM_VIEW_PARAM_EQUALS << viewTag;
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(REGISTER_PLATFORM_VIEW), param);
}

void PlatformViewDelegate::UpdatePlatformViewLayout(const NG::SizeF& drawSize, const NG::OffsetF& offset)
{    
    std::stringstream paramStream;
    paramStream << PLATFORM_VIEW_WIDTH << PLATFORM_VIEW_PARAM_EQUALS
                << drawSize.Width() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_HEIGHT << PLATFORM_VIEW_PARAM_EQUALS
                << drawSize.Height() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_LEFT << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetX() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_TOP << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetY();
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(UPDATE_PLATFORM_VIEW_LAYOUT), param);
}

void PlatformViewDelegate::HandleTouchDown(const NG::OffsetF& offset)
{
    std::stringstream paramStream;
    paramStream << PLATFORM_VIEW_TOUCH_POINT_OFFSET_X << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetX() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetY();
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(PLATFORM_VIEW_TOUCH_DOWN), param);
}

void PlatformViewDelegate::HandleTouchUp(const NG::OffsetF& offset)
{
    std::stringstream paramStream;
    paramStream << PLATFORM_VIEW_TOUCH_POINT_OFFSET_X << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetX() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetY();
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(PLATFORM_VIEW_TOUCH_UP), param);
}

void PlatformViewDelegate::HandleTouchMove(const NG::OffsetF& offset)
{
    std::stringstream paramStream;
    paramStream << PLATFORM_VIEW_TOUCH_POINT_OFFSET_X << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetX() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetY();
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(PLATFORM_VIEW_TOUCH_MOVE), param);
}

void PlatformViewDelegate::HandleTouchCancel(const NG::OffsetF& offset)
{
    std::stringstream paramStream;
    paramStream << PLATFORM_VIEW_TOUCH_POINT_OFFSET_X << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetX() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetY();
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(PLATFORM_VIEW_TOUCH_CANCEL), param);
}

void PlatformViewDelegate::Dispose()
{
    CallResRegisterMethod(MakeMethodHash(DISPOSE_PLATFORM_VIEW), "");
}
} // namespace OHOS::Ace