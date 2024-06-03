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

#include "core/common/stylus/stylus_detector_mgr.h"

namespace OHOS::Ace {
void StylusDetectorMgr::StylusDetectorCallBack::RequestFocus(int32_t nodeId) {}

void StylusDetectorMgr::StylusDetectorCallBack::SetText(int32_t nodeId, std::string args) {}

std::string StylusDetectorMgr::StylusDetectorCallBack::GetText(int32_t nodeId)
{
    return "";
}

void StylusDetectorMgr::StylusDetectorCallBack::OnDetector(
    const CommandType& command, std::string args, std::shared_ptr<IAceStylusCallback> callback)
{}

StylusDetectorMgr* StylusDetectorMgr::GetInstance()
{
    static StylusDetectorMgr instance;
    return &instance;
}

bool StylusDetectorMgr::IsEnable()
{
    return false;
}

bool StylusDetectorMgr::RegisterStylusInteractionListener(const std::shared_ptr<IStylusDetectorCallback>& callback)
{
    isRegistered_ = true;
    return false;
}
void StylusDetectorMgr::UnRegisterStylusInteractionListener() {}

bool StylusDetectorMgr::Notify(const NotifyInfo& notifyInfo)
{
    return false;
}

bool StylusDetectorMgr::IsNeedInterceptedTouchEvent(const TouchEvent& touchEvent)
{
    return false;
}

void StylusDetectorMgr::AddTextFieldFrameNode(const RefPtr<NG::FrameNode>& frameNode) {}
void StylusDetectorMgr::RemoveTextFieldFrameNode(const int32_t id) {}

StylusDetectorMgr::StylusDetectorMgr() : engine_(nullptr), isRegistered_(false) {}

bool StylusDetectorMgr::CheckTextEditable(const RefPtr<NG::FrameNode> parentNode)
{
    return false;
}

RefPtr<NG::FrameNode> StylusDetectorMgr::FindTargetInChildNodes(
    const RefPtr<NG::UINode> parentNode, const FrameNodeSet& hitFrameNodes)
{
    return nullptr;
}

RefPtr<NG::FrameNode> StylusDetectorMgr::FindTextInputFrameNodeByPosition(float globalX, float globalY)
{
    return nullptr;
}
bool StylusDetectorMgr::IsStylusTouchEvent(const TouchEvent& touchEvent) const
{
    return false;
}
} // namespace OHOS::Ace