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

#ifndef ACCESSIBILITY_CONSTANTS_H
#define ACCESSIBILITY_CONSTANTS_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace Accessibility {
// text move step
extern const std::string MOVE_UNIT_CHARACTER;
extern const std::string MOVE_UNIT_WORD;
extern const std::string MOVE_UNIT_LINE;
extern const std::string MOVE_UNIT_PAGE;
extern const std::string MOVE_UNIT_PARAGRAPH;

// Operation Arguments Type
extern const std::string ACTION_ARGU_INVALID;
extern const std::string ACTION_ARGU_SELECT_TEXT_START;
extern const std::string ACTION_ARGU_SELECT_TEXT_END;
extern const std::string ACTION_ARGU_SELECT_TEXT_INFORWARD;
extern const std::string ACTION_ARGU_SET_OFFSET;
extern const std::string ACTION_ARGU_HTML_ELEMENT;
extern const std::string ACTION_ARGU_SET_TEXT;
extern const std::string ACTION_ARGU_MOVE_UNIT;
extern const std::string ACTION_ARGU_SPAN_ID;

// HtmlItemType
extern const std::string HTML_ITEM_INVALID;
extern const std::string HTML_ITEM_LINK;
extern const std::string HTML_ITEM_CONTROL;
extern const std::string HTML_ITEM_GRAPHIC;
extern const std::string HTML_ITEM_LIST_ITEM;
extern const std::string HTML_ITEM_LIST;
extern const std::string HTML_ITEM_TABLE;
extern const std::string HTML_ITEM_COMBOX;
extern const std::string HTML_ITEM_HEADING;
extern const std::string HTML_ITEM_BUTTON;
extern const std::string HTML_ITEM_CHECKBOX;
extern const std::string HTML_ITEM_LANDMARK;
extern const std::string HTML_ITEM_TEXT_FIELD;
extern const std::string HTML_ITEM_FOCUSABLE;
extern const std::string HTML_ITEM_H1;
extern const std::string HTML_ITEM_H2;
extern const std::string HTML_ITEM_H3;
extern const std::string HTML_ITEM_H4;
extern const std::string HTML_ITEM_H5;
extern const std::string HTML_ITEM_H6;
extern const std::string HTML_ITEM_UNKOWN;

extern const int32_t PREFETCH_PREDECESSORS;
extern const int32_t PREFETCH_SIBLINGS;
extern const int32_t PREFETCH_CHILDREN;
extern const int32_t PREFETCH_RECURSIVE_CHILDREN;
extern const int32_t GET_SOURCE_MODE;
extern const int32_t UNDEFINED;
extern const int32_t GET_SOURCE_PREFETCH_MODE;
extern const int32_t UNDEFINED_SELECTION_INDEX;
extern const int32_t UNDEFINED_ITEM_ID;
extern const int32_t ROOT_ITEM_ID;
extern const int32_t MAX_TEXT_LENGTH;
extern const int32_t HOST_VIEW_ID;
extern const int64_t ROOT_NODE_ID;

// Focus types
extern const int32_t FOCUS_TYPE_INVALID;
extern const int32_t FOCUS_TYPE_INPUT;
extern const int32_t FOCUS_TYPE_ACCESSIBILITY;

// grid mode
extern const int32_t SELECTION_MODE_NONE;
extern const int32_t SELECTION_MODE_SINGLE;
extern const int32_t SELECTION_MODE_MULTIPLE;

extern const int32_t INVALID_WINDOW_ID;
extern const int32_t INVALID_TREE_ID;
extern const int32_t ANY_WINDOW_ID;
extern const int32_t ACTIVE_WINDOW_ID;

extern const uint32_t STATE_ACCESSIBILITY_ENABLED;
extern const uint32_t STATE_EXPLORATION_ENABLED;
extern const uint32_t STATE_CAPTION_ENABLED;
extern const uint32_t STATE_KEYEVENT_ENABLED;
extern const uint32_t STATE_GESTURE_ENABLED;
extern const uint32_t STATE_SCREENMAGNIFIER_ENABLED;
extern const uint32_t STATE_MOUSEKEY_ENABLED;
extern const uint32_t STATE_SHORTKEY_ENABLED;
extern const uint32_t STATE_HIGHCONTRAST_ENABLED;
extern const uint32_t STATE_INVETRTCOLOR_ENABLED;
extern const uint32_t STATE_ANIMATIONOFF_ENABLED;
extern const uint32_t STATE_DALTONIZATION_STATE_ENABLED;
extern const uint32_t STATE_AUDIOMONO_ENABLED;
extern const uint32_t STATE_IGNORE_REPEAT_CLICK_ENABLED;
extern const int32_t INVALID_CHANNEL_ID;
} // namespace Accessibility
} // namespace OHOS
#endif // ACCESSIBILITY_CONSTANTS_H