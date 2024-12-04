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

#ifndef FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_MMI_EVENT_CONVERTOR_H
#define FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_MMI_EVENT_CONVERTOR_H

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/vector.h"
#include "base/log/log.h"
#include "base/utils/macros.h"
#include "core/event/axis_event.h"
#include "core/event/key_event.h"
#include "core/event/mouse_event.h"
#include "core/event/pointer_event.h"
#include "core/event/touch_event.h"
#include "pointer_event.h"

namespace OHOS::Ace::Platform {
namespace {
const std::unordered_map<TouchType, int32_t> TOUCH_TYPE_MAP = {
    { TouchType::CANCEL, MMI::PointerEvent::POINTER_ACTION_CANCEL },
    { TouchType::DOWN, MMI::PointerEvent::POINTER_ACTION_DOWN },
    { TouchType::MOVE, MMI::PointerEvent::POINTER_ACTION_MOVE },
    { TouchType::UP, MMI::PointerEvent::POINTER_ACTION_UP },
    { TouchType::PULL_DOWN, MMI::PointerEvent::POINTER_ACTION_PULL_DOWN },
    { TouchType::PULL_MOVE, MMI::PointerEvent::POINTER_ACTION_PULL_MOVE },
    { TouchType::PULL_UP, MMI::PointerEvent::POINTER_ACTION_PULL_UP },
    { TouchType::PULL_IN_WINDOW, MMI::PointerEvent::POINTER_ACTION_PULL_IN_WINDOW },
    { TouchType::PULL_OUT_WINDOW, MMI::PointerEvent::POINTER_ACTION_PULL_OUT_WINDOW },
};

const size_t DATA_ALIGNAS = 8;
} // namespace

struct alignas(DATA_ALIGNAS) AceActionData {
    enum class ActionType : int64_t {
        UNKNOWN = -1,
        CANCEL = 0,
        ADD,
        REMOVE,
        HOVER,
        DOWN,
        MOVE,
        UP,
        HOVER_ENTER,
        HOVER_MOVE,
        HOVER_EXIT,
    };

    int64_t pointerId_ { -1 };
    ActionType actionType = ActionType::UNKNOWN;
    double windowX_ { 0.0 };
    double windowY_ { 0.0 };
    double width_ { 0.0 };
    double height_ { 0.0 };
    double pressure_ { 0.0 };
    int64_t deviceId_ {};
    int64_t downTime_ {};
    int64_t actionTime_ {};
    int64_t sourceType_ {};
    int64_t toolType_ {};
    int8_t actionPoint = 0;
};

struct alignas(DATA_ALIGNAS) AceMouseData {
    enum class Action : int64_t {
        NONE,
        PRESS,
        RELEASE,
        MOVE,
        HOVER_ENTER,
        HOVER_MOVE,
        HOVER_EXIT,
    };

    enum class ActionButton : int64_t {
        NONE_BUTTON = 0,
        LEFT_BUTTON = 1,
        RIGHT_BUTTON = 2,
        MIDDLE_BUTTON = 4,
        BACK_BUTTON = 8,
        FORWARD_BUTTON = 16,
    };

    double physicalX;
    double physicalY;
    double physicalZ;
    double deltaX;
    double deltaY;
    double deltaZ;
    double scrollDeltaX;
    double scrollDeltaY;
    double scrollDeltaZ;
    Action action;
    ActionButton actionButton;
    int64_t pressedButtons;
    int64_t timeStamp;
    int64_t deviceId;
    int64_t deviceType;
};

void ConvertPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, DragPointerEvent& event);
TouchEvent ConvertTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
void CreatePointerEventsFromBytes(
    std::vector<std::shared_ptr<MMI::PointerEvent>>& pointerEvent, const std::vector<uint8_t>& data);
void SetTouchEventType(int32_t orgAction, TouchEvent& event);
void UpdateTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, TouchEvent& touchEvent);
TouchPoint ConvertTouchPoint(const MMI::PointerEvent::PointerItem& pointerItem);
void SetPointerEventAction(AceActionData::ActionType actionType, std::shared_ptr<MMI::PointerEvent>& pointerEvent);
void SetPointerItemPressed(AceActionData::ActionType actionType, MMI::PointerEvent::PointerItem& pointerItem);
void ConvertMouseEvent(const std::vector<uint8_t>& data, MouseEvent& events);
void SetMouseEventAction(AceMouseData::Action action, MouseEvent& event);
void SetMouseEventActionButton(AceMouseData::ActionButton actionButton, MouseEvent& event);
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_MMI_EVENT_CONVERTOR_H
