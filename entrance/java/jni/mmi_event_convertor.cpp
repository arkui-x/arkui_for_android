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

#include "mmi_event_convertor.h"

#include "base/log/log.h"
#include "base/utils/time_util.h"
#include "base/utils/utils.h"
#include "core/event/ace_events.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::Platform {
namespace {
constexpr double SIZE_DIVIDE = 2.0;
constexpr int8_t ACTION_POINT = 1;
} // namespace

static std::unordered_map<int, int> actionPointMap;

TouchPoint ConvertTouchPoint(const MMI::PointerEvent::PointerItem& pointerItem)
{
    TouchPoint touchPoint;
    // just get the max of width and height
    touchPoint.size = std::max(pointerItem.GetWidth(), pointerItem.GetHeight()) / SIZE_DIVIDE;
    touchPoint.id = pointerItem.GetPointerId();
    touchPoint.downTime = TimeStamp(std::chrono::microseconds(pointerItem.GetDownTime()));
    touchPoint.x = pointerItem.GetWindowX();
    touchPoint.y = pointerItem.GetWindowY();
    touchPoint.screenX = pointerItem.GetDisplayX();
    touchPoint.screenY = pointerItem.GetDisplayY();
    touchPoint.isPressed = pointerItem.IsPressed();
    touchPoint.force = static_cast<float>(pointerItem.GetPressure());
    touchPoint.tiltX = pointerItem.GetTiltX();
    touchPoint.tiltY = pointerItem.GetTiltY();
    touchPoint.sourceTool = static_cast<SourceTool>(pointerItem.GetToolType());
    return touchPoint;
}

void UpdateTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, TouchEvent& touchEvent)
{
    auto ids = pointerEvent->GetPointerIds();
    for (auto&& id : ids) {
        MMI::PointerEvent::PointerItem item;
        bool ret = pointerEvent->GetPointerItem(id, item);
        if (!ret) {
            LOGE("get pointer item failed.");
            continue;
        }
        auto touchPoint = ConvertTouchPoint(item);
        touchEvent.pointers.emplace_back(std::move(touchPoint));
    }
    touchEvent.CovertId();
}

void ConvertTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, std::vector<TouchEvent>& events)
{
    auto items = pointerEvent->GetAllPointerItems();
    for (auto& item : items) {
        auto touchPoint = ConvertTouchPoint(item);
        if (actionPointMap[item.GetPointerId()] == ACTION_POINT) {
            std::chrono::microseconds microseconds(pointerEvent->GetActionTime());
            TimeStamp time(microseconds);
            TouchEvent event;
            event.SetId(touchPoint.id)
                .SetX(touchPoint.x)
                .SetY(touchPoint.y)
                .SetScreenX(touchPoint.screenX)
                .SetScreenY(touchPoint.screenY)
                .SetType(TouchType::UNKNOWN)
                .SetPullType(TouchType::UNKNOWN)
                .SetTime(time)
                .SetSize(touchPoint.size)
                .SetForce(touchPoint.force)
                .SetTiltX(touchPoint.tiltX)
                .SetTiltY(touchPoint.tiltY)
                .SetDeviceId(pointerEvent->GetDeviceId())
                .SetTargetDisplayId(pointerEvent->GetTargetDisplayId())
                .SetSourceType(static_cast<SourceType>(pointerEvent->GetSourceType()))
                .SetSourceTool(touchPoint.sourceTool)
                .SetTouchEventId(pointerEvent->GetId());
            event.pointerEvent = pointerEvent;
            event.originalId = item.GetOriginPointerId();
            int32_t orgAction = pointerEvent->GetPointerAction();
            SetTouchEventType(orgAction, event);
            UpdateTouchEvent(pointerEvent, event);
            events.emplace_back(event);
        }
    }
}

void SetTouchEventType(int32_t orgAction, TouchEvent& event)
{
    switch (orgAction) {
        case OHOS::MMI::PointerEvent::POINTER_ACTION_CANCEL:
            event.type = TouchType::CANCEL;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_DOWN:
            event.type = TouchType::DOWN;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE:
            event.type = TouchType::MOVE;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_UP:
            event.type = TouchType::UP;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_DOWN:
            event.type = TouchType::PULL_DOWN;
            event.pullType = TouchType::PULL_DOWN;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_MOVE:
            event.type = TouchType::PULL_MOVE;
            event.pullType = TouchType::PULL_MOVE;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_UP:
            event.type = TouchType::PULL_UP;
            event.pullType = TouchType::PULL_UP;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_IN_WINDOW:
            event.type = TouchType::PULL_IN_WINDOW;
            event.pullType = TouchType::PULL_IN_WINDOW;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_PULL_OUT_WINDOW:
            event.type = TouchType::PULL_OUT_WINDOW;
            event.pullType = TouchType::PULL_OUT_WINDOW;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER:
            event.type = TouchType::HOVER_ENTER;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_HOVER_MOVE:
            event.type = TouchType::HOVER_MOVE;
            return;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_HOVER_EXIT:
            event.type = TouchType::HOVER_EXIT;
            return;
        default:
            LOGW("unknown type");
            return;
    }
}

void ConvertPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, PointerEvent& event)
{
    event.rawPointerEvent = pointerEvent;
    event.pointerId = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    event.pressed = pointerItem.IsPressed();
    event.windowX = pointerItem.GetWindowX();
    event.windowY = pointerItem.GetWindowY();
    event.displayX = pointerItem.GetDisplayX();
    event.displayY = pointerItem.GetDisplayY();
    event.size = std::max(pointerItem.GetWidth(), pointerItem.GetHeight()) / SIZE_DIVIDE;
    event.force = static_cast<float>(pointerItem.GetPressure());
    event.deviceId = pointerItem.GetDeviceId();
    event.downTime = TimeStamp(std::chrono::microseconds(pointerItem.GetDownTime()));
    event.time = TimeStamp(std::chrono::microseconds(pointerEvent->GetActionTime()));
    event.sourceTool = static_cast<SourceTool>(pointerItem.GetToolType());
    event.targetWindowId = pointerItem.GetTargetWindowId();
}

void CreatePointerEventFromBytes(std::shared_ptr<MMI::PointerEvent>& pointerEvent, const std::vector<uint8_t>& data)
{
    const auto* origin = reinterpret_cast<const AceActionData*>(data.data());
    size_t size = data.size() / sizeof(AceActionData);
    auto current = const_cast<AceActionData*>(origin);
    auto end = current + size;
    pointerEvent->SetDeviceId(static_cast<int32_t>(current->deviceId_));
    pointerEvent->SetSourceType(static_cast<int32_t>(current->sourceType_));
    pointerEvent->SetActionTime(current->actionTime_);
    pointerEvent->SetTargetDisplayId(0);
    SetPointerEventAction(current->actionType, pointerEvent);
    while (current < end) {
        if (current == origin) {
            pointerEvent->SetPointerId(current->pointerId_);
        }
        OHOS::MMI::PointerEvent::PointerItem pointerItem;
        pointerItem.SetPointerId(static_cast<int32_t>(current->pointerId_));
        pointerItem.SetDownTime(current->downTime_);
        pointerItem.SetWindowX(static_cast<int32_t>(current->windowX_));
        pointerItem.SetWindowY(static_cast<int32_t>(current->windowY_));
        pointerItem.SetDisplayX(static_cast<int32_t>(current->windowX_));
        pointerItem.SetDisplayY(static_cast<int32_t>(current->windowY_));
        pointerItem.SetWidth(static_cast<int32_t>(current->width_));
        pointerItem.SetHeight(static_cast<int32_t>(current->height_));
        pointerItem.SetPressure(current->pressure_);
        pointerItem.SetToolType(static_cast<int32_t>(current->toolType_));
        pointerItem.SetOriginPointerId(static_cast<int32_t>(current->pointerId_));
        SetPointerItemPressed(current->actionType, pointerItem);
        pointerEvent->AddPointerItem(pointerItem);
        actionPointMap[current->pointerId_] = current->actionPoint;
        current++;
    }
}

void SetPointerEventAction(AceActionData::ActionType actionType, std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    switch (actionType) {
        case AceActionData::ActionType::CANCEL:
            pointerEvent->SetPointerAction(OHOS::MMI::PointerEvent::POINTER_ACTION_CANCEL);
        case AceActionData::ActionType::DOWN:
            pointerEvent->SetPointerAction(OHOS::MMI::PointerEvent::POINTER_ACTION_DOWN);
            return;
        case AceActionData::ActionType::MOVE:
            pointerEvent->SetPointerAction(OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE);
            return;
        case AceActionData::ActionType::UP:
            pointerEvent->SetPointerAction(OHOS::MMI::PointerEvent::POINTER_ACTION_UP);
            return;
        default:
            return;
    }
}

void SetPointerItemPressed(AceActionData::ActionType actionType, MMI::PointerEvent::PointerItem& pointerItem)
{
    switch (actionType) {
        case AceActionData::ActionType::CANCEL:
            pointerItem.SetPressed(false);
            return;
        case AceActionData::ActionType::DOWN:
            pointerItem.SetPressed(true);
            return;
        case AceActionData::ActionType::MOVE:
            pointerItem.SetPressed(true);
            return;
        case AceActionData::ActionType::UP:
            pointerItem.SetPressed(false);
            return;
        default:
            return;
    }
}
} // namespace OHOS::Ace::Platform
