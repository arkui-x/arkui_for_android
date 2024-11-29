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

#include "accessibility_constants.h"

namespace OHOS {
namespace Accessibility {
// text move step
const std::string MOVE_UNIT_CHARACTER = "char";
const std::string MOVE_UNIT_WORD = "word";
const std::string MOVE_UNIT_LINE = "line";
const std::string MOVE_UNIT_PAGE = "page";
const std::string MOVE_UNIT_PARAGRAPH = "paragraph";

// Operation Arguments Type
const std::string ACTION_ARGU_INVALID = "invalid";
const std::string ACTION_ARGU_SELECT_TEXT_START = "selectTextBegin";
const std::string ACTION_ARGU_SELECT_TEXT_END = "selectTextEnd";
const std::string ACTION_ARGU_SELECT_TEXT_INFORWARD = "selectTextInForWard";
const std::string ACTION_ARGU_SET_OFFSET = "offset";
const std::string ACTION_ARGU_HTML_ELEMENT = "htmlItem";
const std::string ACTION_ARGU_SET_TEXT = "setText";
const std::string ACTION_ARGU_MOVE_UNIT = "textMoveUnit";
const std::string ACTION_ARGU_SPAN_ID = "spanId";

// HtmlItemType
const std::string HTML_ITEM_INVALID = "invalid";
const std::string HTML_ITEM_LINK = "link";
const std::string HTML_ITEM_CONTROL = "control";
const std::string HTML_ITEM_GRAPHIC = "graphic";
const std::string HTML_ITEM_LIST_ITEM = "listItem";
const std::string HTML_ITEM_LIST = "list";
const std::string HTML_ITEM_TABLE = "table";
const std::string HTML_ITEM_COMBOX = "combox";
const std::string HTML_ITEM_HEADING = "heading";
const std::string HTML_ITEM_BUTTON = "button";
const std::string HTML_ITEM_CHECKBOX = "checkBox";
const std::string HTML_ITEM_LANDMARK = "landmark";
const std::string HTML_ITEM_TEXT_FIELD = "textField";
const std::string HTML_ITEM_FOCUSABLE = "focusable";
const std::string HTML_ITEM_H1 = "h1";
const std::string HTML_ITEM_H2 = "h2";
const std::string HTML_ITEM_H3 = "h3";
const std::string HTML_ITEM_H4 = "h4";
const std::string HTML_ITEM_H5 = "h5";
const std::string HTML_ITEM_H6 = "h6";
const std::string HTML_ITEM_UNKOWN = "unknown";

const int32_t PREFETCH_PREDECESSORS = 1 << 0;
const int32_t PREFETCH_SIBLINGS = 1 << 1;
const int32_t PREFETCH_CHILDREN = 1 << 2;
const int32_t PREFETCH_RECURSIVE_CHILDREN = 1 << 3;
const int32_t GET_SOURCE_MODE = PREFETCH_RECURSIVE_CHILDREN | PREFETCH_PREDECESSORS;
const int32_t UNDEFINED = -1;
const int32_t GET_SOURCE_PREFETCH_MODE = PREFETCH_PREDECESSORS | PREFETCH_SIBLINGS | PREFETCH_CHILDREN;
const int32_t UNDEFINED_SELECTION_INDEX = -1;
const int32_t UNDEFINED_ITEM_ID = 0x1FFFFFF;
const int32_t ROOT_ITEM_ID = -1;
const int32_t MAX_TEXT_LENGTH = 500;
const int32_t HOST_VIEW_ID = -1;
const int64_t ROOT_NODE_ID = -1;

// Focus types
const int32_t FOCUS_TYPE_INVALID = -1;
const int32_t FOCUS_TYPE_INPUT = 1 << 0;
const int32_t FOCUS_TYPE_ACCESSIBILITY = 1 << 1;

// grid mode
const int32_t SELECTION_MODE_NONE = 0;
const int32_t SELECTION_MODE_SINGLE = 1;
const int32_t SELECTION_MODE_MULTIPLE = 2;

const int32_t INVALID_WINDOW_ID = -1;
const int32_t INVALID_TREE_ID = -1;
const int32_t ANY_WINDOW_ID = -2;
const int32_t ACTIVE_WINDOW_ID = 0x7FFFFFFF;

const uint32_t STATE_ACCESSIBILITY_ENABLED = 0x00000001;
const uint32_t STATE_EXPLORATION_ENABLED = 0x00000002;
const uint32_t STATE_CAPTION_ENABLED = 0x00000004;
const uint32_t STATE_KEYEVENT_ENABLED = 0x00000008;
const uint32_t STATE_GESTURE_ENABLED = 0x00000010;
const uint32_t STATE_SCREENMAGNIFIER_ENABLED = 0x00000020;
const uint32_t STATE_MOUSEKEY_ENABLED = 0x00000040;
const uint32_t STATE_SHORTKEY_ENABLED = 0x00000080;
const uint32_t STATE_HIGHCONTRAST_ENABLED = 0x00000100;
const uint32_t STATE_INVETRTCOLOR_ENABLED = 0x00000200;
const uint32_t STATE_ANIMATIONOFF_ENABLED = 0x00000400;
const uint32_t STATE_AUDIOMONO_ENABLED = 0x00000800;
const uint32_t STATE_DALTONIZATION_STATE_ENABLED = 0x00001000;
const uint32_t STATE_IGNORE_REPEAT_CLICK_ENABLED = 0x00002000;

const int32_t INVALID_CHANNEL_ID = 0xFFFFFFFF;
} // namespace Accessibility
} // namespace OHOS