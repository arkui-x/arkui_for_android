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

#include "accessibility_element_info.h"

#include <cinttypes>

namespace OHOS {
namespace Accessibility {
void AccessibilityElementInfo::SetComponentId(const int64_t componentId)
{
    elementId_ = componentId;
}

int64_t AccessibilityElementInfo::GetChildId(const int32_t index) const
{
    if (index >= childCount_ || index < 0 || index >= static_cast<int32_t>(childNodeIds_.size())) {
        return -1;
    }
    return childNodeIds_[index];
}

int32_t AccessibilityElementInfo::GetChildCount() const
{
    return childCount_;
}

const std::vector<int64_t>& AccessibilityElementInfo::GetChildIds() const
{
    return childNodeIds_;
}

void AccessibilityElementInfo::AddChild(const int64_t childId)
{
    for (int32_t i = 0; i < childCount_; i++) {
        if (childNodeIds_[i] == childId) {
            return;
        }
    }
    childCount_++;
    childNodeIds_.push_back(childId);
}

bool AccessibilityElementInfo::RemoveChild(const int64_t childId)
{
    for (auto iter = childNodeIds_.begin(); iter != childNodeIds_.end(); iter++) {
        if (*iter == childId) {
            iter = childNodeIds_.erase(iter);
            childCount_--;
            return true;
        }
    }
    return false;
}

const std::vector<AccessibleAction>& AccessibilityElementInfo::GetActionList() const
{
    return operations_;
}

void AccessibilityElementInfo::AddAction(AccessibleAction& action)
{
    operations_.push_back(action);
}

void AccessibilityElementInfo::DeleteAction(AccessibleAction& action)
{
    for (auto iter = operations_.begin(); iter != operations_.end(); iter++) {
        if (iter->GetActionType() == action.GetActionType()) {
            iter = operations_.erase(iter);
            return;
        }
    }
}

bool AccessibilityElementInfo::DeleteAction(ActionType& actionType)
{
    for (auto iter = operations_.begin(); iter != operations_.end(); iter++) {
        if (iter->GetActionType() == actionType) {
            iter = operations_.erase(iter);
            return true;
        }
    }
    return false;
}

void AccessibilityElementInfo::DeleteAllActions()
{
    operations_.clear();
}

void AccessibilityElementInfo::SetTextLengthLimit(const int32_t max)
{
    textLengthLimit_ = max;
}

int32_t AccessibilityElementInfo::GetTextLengthLimit() const
{
    return textLengthLimit_;
}

int32_t AccessibilityElementInfo::GetWindowId() const
{
    return windowId_;
}

void AccessibilityElementInfo::SetWindowId(const int32_t windowId)
{
    windowId_ = windowId;
}

int64_t AccessibilityElementInfo::GetParentNodeId() const
{
    return parentId_;
}

void AccessibilityElementInfo::SetParent(const int64_t parentId)
{
    parentId_ = parentId;
}

const Rect& AccessibilityElementInfo::GetRectInScreen() const
{
    return bounds_;
}

void AccessibilityElementInfo::SetRectInScreen(Rect& bounds)
{
    bounds_.SetLeftTopScreenPostion(bounds.GetLeftTopXScreenPostion(), bounds.GetLeftTopYScreenPostion());
    bounds_.SetRightBottomScreenPostion(bounds.GetRightBottomXScreenPostion(), bounds.GetRightBottomYScreenPostion());
}

bool AccessibilityElementInfo::IsCheckable() const
{
    return checkable_;
}

void AccessibilityElementInfo::SetCheckable(const bool checkable)
{
    checkable_ = checkable;
}

bool AccessibilityElementInfo::IsChecked() const
{
    return checked_;
}

void AccessibilityElementInfo::SetChecked(const bool checked)
{
    checked_ = checked;
}

bool AccessibilityElementInfo::IsFocusable() const
{
    return focusable_;
}

void AccessibilityElementInfo::SetFocusable(const bool focusable)
{
    focusable_ = focusable;
}

bool AccessibilityElementInfo::IsFocused() const
{
    return focused_;
}

void AccessibilityElementInfo::SetFocused(const bool focused)
{
    focused_ = focused;
}

bool AccessibilityElementInfo::IsVisible() const
{
    return visible_;
}

void AccessibilityElementInfo::SetVisible(const bool visible)
{
    visible_ = visible;
}

bool AccessibilityElementInfo::HasAccessibilityFocus() const
{
    return accessibilityFocused_;
}

void AccessibilityElementInfo::SetAccessibilityFocus(const bool focused)
{
    accessibilityFocused_ = focused;
}

bool AccessibilityElementInfo::IsSelected() const
{
    return selected_;
}

void AccessibilityElementInfo::SetSelected(const bool selected)
{
    selected_ = selected;
}

bool AccessibilityElementInfo::IsClickable() const
{
    return clickable_;
}

void AccessibilityElementInfo::SetClickable(const bool clickable)
{
    clickable_ = clickable;
}

bool AccessibilityElementInfo::IsLongClickable() const
{
    return longClickable_;
}

void AccessibilityElementInfo::SetLongClickable(const bool longClickable)
{
    longClickable_ = longClickable;
}

bool AccessibilityElementInfo::IsEnabled() const
{
    return enable_;
}

void AccessibilityElementInfo::SetEnabled(const bool enabled)
{
    enable_ = enabled;
}

bool AccessibilityElementInfo::IsPassword() const
{
    return isPassword_;
}

void AccessibilityElementInfo::SetPassword(const bool type)
{
    isPassword_ = type;
}

bool AccessibilityElementInfo::IsScrollable() const
{
    return scrollable_;
}

void AccessibilityElementInfo::SetScrollable(const bool scrollable)
{
    scrollable_ = scrollable;
}

int32_t AccessibilityElementInfo::GetCurrentIndex() const
{
    return currentIndex_;
}

void AccessibilityElementInfo::SetCurrentIndex(const int32_t index)
{
    currentIndex_ = index;
}

int32_t AccessibilityElementInfo::GetBeginIndex() const
{
    return beginIndex_;
}

void AccessibilityElementInfo::SetBeginIndex(const int32_t index)
{
    beginIndex_ = index;
}

int32_t AccessibilityElementInfo::GetEndIndex() const
{
    return endIndex_;
}

void AccessibilityElementInfo::SetEndIndex(const int32_t index)
{
    endIndex_ = index;
}

int32_t AccessibilityElementInfo::GetInputType() const
{
    return inputType_;
}

void AccessibilityElementInfo::SetInputType(const int32_t inputType)
{
    inputType_ = inputType;
}

void AccessibilityElementInfo::SetValidElement(const bool valid)
{
    validElement_ = valid;
}

void AccessibilityElementInfo::SetInspectorKey(const std::string& key)
{
    inspectorKey_ = key;
}

const std::string& AccessibilityElementInfo::GetInspectorKey() const
{
    return inspectorKey_;
}

void AccessibilityElementInfo::SetPagePath(const std::string& path)
{
    pagePath_ = path;
}

const std::string& AccessibilityElementInfo::GetPagePath() const
{
    return pagePath_;
}

bool AccessibilityElementInfo::IsValidElement() const
{
    return validElement_;
}

bool AccessibilityElementInfo::IsEditable() const
{
    return editable_;
}

void AccessibilityElementInfo::SetEditable(const bool editable)
{
    editable_ = editable;
}

bool AccessibilityElementInfo::IsPluraLineSupported() const
{
    return multiLine_;
}

void AccessibilityElementInfo::SetPluraLineSupported(const bool multiLine)
{
    multiLine_ = multiLine;
}

bool AccessibilityElementInfo::IsPopupSupported() const
{
    return popupSupported_;
}

void AccessibilityElementInfo::SetPopupSupported(const bool supportPopup)
{
    popupSupported_ = supportPopup;
}

bool AccessibilityElementInfo::IsDeletable() const
{
    return deletable_;
}

void AccessibilityElementInfo::SetDeletable(const bool deletable)
{
    deletable_ = deletable;
}

bool AccessibilityElementInfo::IsEssential() const
{
    return isEssential_;
}

void AccessibilityElementInfo::SetEssential(const bool essential)
{
    isEssential_ = essential;
}

bool AccessibilityElementInfo::IsGivingHint() const
{
    return hint_;
}
void AccessibilityElementInfo::SetHinting(const bool hinting)
{
    hint_ = hinting;
}

const std::string& AccessibilityElementInfo::GetBundleName() const
{
    return bundleName_;
}

void AccessibilityElementInfo::SetBundleName(const std::string& bundleName)
{
    bundleName_ = bundleName;
}

const std::string& AccessibilityElementInfo::GetComponentType() const
{
    return componentType_;
}

void AccessibilityElementInfo::SetComponentType(const std::string& className)
{
    componentType_ = className;
}

const std::string& AccessibilityElementInfo::GetContent() const
{
    return text_;
}

void AccessibilityElementInfo::SetContent(const std::string& text)
{
    text_ = text;
}

void AccessibilityElementInfo::SetSelectedBegin(const int32_t start)
{
    beginSelected_ = start;
}

int32_t AccessibilityElementInfo::GetSelectedBegin() const
{
    return beginSelected_;
}

void AccessibilityElementInfo::SetSelectedEnd(const int32_t end)
{
    endSelected_ = end;
}

int32_t AccessibilityElementInfo::GetSelectedEnd() const
{
    return endSelected_;
}

const std::string& AccessibilityElementInfo::GetHint() const
{
    return hintText_;
}

void AccessibilityElementInfo::SetHint(const std::string& hintText)
{
    hintText_ = hintText;
}

const std::string& AccessibilityElementInfo::GetDescriptionInfo() const
{
    return contentDescription_;
}

void AccessibilityElementInfo::SetDescriptionInfo(const std::string& contentDescription)
{
    contentDescription_ = contentDescription;
}

void AccessibilityElementInfo::SetComponentResourceId(const std::string& viewIdResName)
{
    resourceName_ = viewIdResName;
}

const std::string& AccessibilityElementInfo::GetComponentResourceId() const
{
    return resourceName_;
}

void AccessibilityElementInfo::SetLiveRegion(const int32_t liveRegion)
{
    liveRegion_ = liveRegion;
}

int32_t AccessibilityElementInfo::GetLiveRegion() const
{
    return liveRegion_;
}

void AccessibilityElementInfo::SetContentInvalid(const bool contentInvalid)
{
    contentInvalid_ = contentInvalid;
}

bool AccessibilityElementInfo::GetContentInvalid() const
{
    return contentInvalid_;
}

void AccessibilityElementInfo::SetError(const std::string& error)
{
    error_ = error;
}

const std::string& AccessibilityElementInfo::GetError() const
{
    return error_;
}

void AccessibilityElementInfo::SetLabeled(const int64_t componentId)
{
    labeled_ = componentId;
}

int64_t AccessibilityElementInfo::GetLabeledAccessibilityId() const
{
    return labeled_;
}

void AccessibilityElementInfo::SetAccessibilityId(const int64_t componentId)
{
    elementId_ = componentId;
}

int64_t AccessibilityElementInfo::GetAccessibilityId() const
{
    return elementId_;
}

const RangeInfo& AccessibilityElementInfo::GetRange() const
{
    return rangeInfo_;
}

void AccessibilityElementInfo::SetRange(RangeInfo& rangeInfo)
{
    rangeInfo_.SetMax(rangeInfo.GetMax());
    rangeInfo_.SetMin(rangeInfo.GetMin());
    rangeInfo_.SetCurrent(rangeInfo.GetCurrent());
}

const GridInfo& AccessibilityElementInfo::GetGrid() const
{
    return grid_;
}

void AccessibilityElementInfo::SetGrid(const GridInfo& grid)
{
    grid_ = grid;
}

const GridItemInfo& AccessibilityElementInfo::GetGridItem() const
{
    return gridItem_;
}

void AccessibilityElementInfo::SetGridItem(const GridItemInfo& gridItem)
{
    gridItem_ = gridItem;
}

const std::string& AccessibilityElementInfo::GetAccessibilityText() const
{
    return accessibilityText_;
}

void AccessibilityElementInfo::SetAccessibilityText(const std::string& accessibilityText)
{
    accessibilityText_ = accessibilityText;
}

void AccessibilityElementInfo::SetTextType(const std::string& textType)
{
    textType_ = textType;
}

const std::string& AccessibilityElementInfo::GetTextType() const
{
    return textType_;
}

void AccessibilityElementInfo::SetOffset(const float offset)
{
    offset_ = offset;
}

float AccessibilityElementInfo::GetOffset() const
{
    return offset_;
}

AccessibilityElementInfo::AccessibilityElementInfo() {}

AccessibleAction::AccessibleAction(ActionType actionType, const std::string& description)
    : actionType_(actionType), description_(description) {}

ActionType AccessibleAction::GetActionType() const
{
    return actionType_;
}

const std::string& AccessibleAction::GetDescriptionInfo() const
{
    return description_;
}

RangeInfo::RangeInfo(double min, double max, double current)
{
    min_ = min;
    max_ = max;
    current_ = current;
}

double RangeInfo::GetMin() const
{
    return min_;
}

double RangeInfo::GetMax() const
{
    return max_;
}

double RangeInfo::GetCurrent() const
{
    return current_;
}

void RangeInfo::SetMin(double min)
{
    min_ = min;
}

void RangeInfo::SetMax(double max)
{
    max_ = max;
}

void RangeInfo::SetCurrent(double current)
{
    current_ = current;
}

GridInfo::GridInfo(int32_t rowCount, int32_t columnCount, int32_t mode)
{
    rowCount_ = rowCount;
    columnCount_ = columnCount;
    selectionMode_ = mode;
}

void GridInfo::SetGrid(int32_t rowCount, int32_t columnCount, int32_t mode)
{
    rowCount_ = rowCount;
    columnCount_ = columnCount;
    selectionMode_ = mode;
}

void GridInfo::SetGrid(GridInfo other)
{
    rowCount_ = other.rowCount_;
    columnCount_ = other.columnCount_;
    selectionMode_ = other.selectionMode_;
}

int32_t GridInfo::GetRowCount() const
{
    return rowCount_;
}

int32_t GridInfo::GetColumnCount() const
{
    return columnCount_;
}

int32_t GridInfo::GetSelectionMode() const
{
    return selectionMode_;
}

GridItemInfo::GridItemInfo(
    int32_t rowIndex, int32_t rowSpan, int32_t columnIndex, int32_t columnSpan, bool heading)
{
    rowIndex_ = rowIndex;
    rowSpan_ = rowSpan;
    columnIndex_ = columnIndex;
    columnSpan_ = columnSpan;
    heading_ = heading;
}

void GridItemInfo::SetGridItemInfo(GridItemInfo other)
{
    rowIndex_ = other.rowIndex_;
    rowSpan_ = other.rowSpan_;
    columnIndex_ = other.columnIndex_;
    columnSpan_ = other.columnSpan_;
    heading_ = other.heading_;
    selected_ = other.selected_;
}

void GridItemInfo::SetGridItemInfo(
    int32_t rowIndex, int32_t rowSpan, int32_t columnIndex, int32_t columnSpan, bool heading)
{
    rowIndex_ = rowIndex;
    rowSpan_ = rowSpan;
    columnIndex_ = columnIndex;
    columnSpan_ = columnSpan;
    heading_ = heading;
}

void GridItemInfo::SetSelected(bool selected)
{
    selected_ = selected;
}

int32_t GridItemInfo::GetColumnIndex() const
{
    return columnIndex_;
}

int32_t GridItemInfo::GetRowIndex() const
{
    return rowIndex_;
}

int32_t GridItemInfo::GetColumnSpan() const
{
    return columnSpan_;
}

int32_t GridItemInfo::GetRowSpan() const
{
    return rowSpan_;
}

bool GridItemInfo::IsHeading() const
{
    return heading_;
}

bool GridItemInfo::IsSelected() const
{
    return selected_;
}

int32_t AccessibilityElementInfo::GetPageId() const
{
    return pageId_;
}

void AccessibilityElementInfo::SetPageId(const int32_t pageId)
{
    pageId_ = pageId;
}

void AccessibilityElementInfo::SetTextMovementStep(const TextMoveUnit granularity)
{
    textMoveStep_ = granularity;
}

TextMoveUnit AccessibilityElementInfo::GetTextMovementStep() const
{
    return textMoveStep_;
}

void AccessibilityElementInfo::SetItemCounts(const int32_t itemCounts)
{
    itemCounts_ = itemCounts;
}

int32_t AccessibilityElementInfo::GetItemCounts() const
{
    return itemCounts_;
}

void AccessibilityElementInfo::SetTriggerAction(const ActionType action)
{
    triggerAction_ = action;
}

ActionType AccessibilityElementInfo::GetTriggerAction() const
{
    return triggerAction_;
}

void AccessibilityElementInfo::SetContentList(const std::vector<std::string>& contentList)
{
    contentList_.clear();
    contentList_.resize(contentList.size());
    std::copy(contentList.begin(), contentList.end(), contentList_.begin());
}

void AccessibilityElementInfo::GetContentList(std::vector<std::string>& contentList) const
{
    contentList.clear();
    contentList.resize(contentList_.size());
    std::copy(contentList_.begin(), contentList_.end(), contentList.begin());
}

void AccessibilityElementInfo::SetLatestContent(const std::string& content)
{
    latestContent_ = content;
}

const std::string& AccessibilityElementInfo::GetLatestContent() const
{
    return latestContent_;
}

void AccessibilityElementInfo::SetChildTreeIdAndWinId(const int32_t iChildTreeId, const int32_t iChildWindowId)
{
    childTreeId_ = iChildTreeId;
    childWindowId_ = iChildWindowId;
}

int32_t AccessibilityElementInfo::GetChildTreeId() const
{
    return childTreeId_;
}

int32_t AccessibilityElementInfo::GetChildWindowId() const
{
    return childWindowId_;
}

void AccessibilityElementInfo::SetBelongTreeId(const int32_t iBelongTreeId)
{
    belongTreeId_ = iBelongTreeId;
}

int32_t AccessibilityElementInfo::GetBelongTreeId() const
{
    return belongTreeId_;
}

void AccessibilityElementInfo::SetParentWindowId(const int32_t iParentWindowId)
{
    parentWindowId_ = iParentWindowId;
}

int32_t AccessibilityElementInfo::GetParentWindowId() const
{
    return parentWindowId_;
}

ExtraElementInfo::ExtraElementInfo(const std::map<std::string, std::string>& extraElementValueStr,
    const std::map<std::string, int32_t>& extraElementValueInt)
    : extraElementValueStr_(extraElementValueStr),
      extraElementValueInt_(extraElementValueInt) {}

RetError ExtraElementInfo::SetExtraElementInfo(const std::string& keyStr, const std::string& valueStr)
{
    auto extraElementInfoIter = setOfExtraElementInfo.find(keyStr);
    if (extraElementInfoIter != setOfExtraElementInfo.end()) {
        extraElementValueStr_[keyStr] = valueStr;
    } else {
        return RET_ERR_FAILED;
    }
    return RET_OK;
}

RetError ExtraElementInfo::SetExtraElementInfo(const std::string keyStr, const int32_t valueInt)
{
    auto extraElementInfoIter = setOfExtraElementInfo.find(keyStr);
    if (extraElementInfoIter != setOfExtraElementInfo.end()) {
        extraElementValueInt_[keyStr] = valueInt;
    } else {
        return RET_ERR_FAILED;
    }
    return RET_OK;
}

const std::map<std::string, std::string>& ExtraElementInfo::GetExtraElementInfoValueStr() const
{
    return extraElementValueStr_;
}

const std::map<std::string, int32_t>& ExtraElementInfo::GetExtraElementInfoValueInt() const
{
    return extraElementValueInt_;
}

void AccessibilityElementInfo::SetExtraElement(const ExtraElementInfo& extraElementInfo)
{
    extraElementInfo_ = extraElementInfo;
}

const ExtraElementInfo& AccessibilityElementInfo::GetExtraElement() const
{
    return extraElementInfo_;
}
const std::string& AccessibilityElementInfo::GetAccessibilityLevel() const
{
    return accessibilityLevel_;
}

bool AccessibilityElementInfo::GetAccessibilityGroup() const
{
    return accessibilityGroup_;
}

void AccessibilityElementInfo::SetAccessibilityGroup(const bool accessibilityGroup)
{
    accessibilityGroup_ = accessibilityGroup;
}

void AccessibilityElementInfo::SetAccessibilityLevel(const std::string accessibilityLevel)
{
    accessibilityLevel_ = accessibilityLevel;
}

void AccessibilityElementInfo::SetZIndex(const int32_t zIndex)
{
    zIndex_ = zIndex;
}

int32_t AccessibilityElementInfo::GetZIndex() const
{
    return zIndex_;
}

void AccessibilityElementInfo::SetOpacity(const float opacity)
{
    opacity_ = opacity;
}

float AccessibilityElementInfo::GetOpacity() const
{
    return opacity_;
}

void AccessibilityElementInfo::SetBackgroundColor(const std::string& backgroundColor)
{
    backgroundColor_ = backgroundColor;
}

const std::string& AccessibilityElementInfo::GetBackgroundColor() const
{
    return backgroundColor_;
}

void AccessibilityElementInfo::SetBackgroundImage(const std::string& backgroundImage)
{
    backgroundImage_ = backgroundImage;
}

const std::string& AccessibilityElementInfo::GetBackgroundImage() const
{
    return backgroundImage_;
}

void AccessibilityElementInfo::SetBlur(const std::string& blur)
{
    blur_ = blur;
}

const std::string& AccessibilityElementInfo::GetBlur() const
{
    return blur_;
}

void AccessibilityElementInfo::SetHitTestBehavior(const std::string& hitTestBehavior)
{
    hitTestBehavior_ = hitTestBehavior;
}

const std::string& AccessibilityElementInfo::GetHitTestBehavior() const
{
    return hitTestBehavior_;
}

void AccessibilityElementInfo::SetNavDestinationId(const int64_t navDestinationId)
{
    navDestinationId_ = navDestinationId;
}

int64_t AccessibilityElementInfo::GetNavDestinationId() const
{
    return navDestinationId_;
}

void AccessibilityElementInfo::AddSpan(const SpanInfo& span)
{
    spanList_.push_back(span);
    for (auto array : spanList_) {
    }
}

void AccessibilityElementInfo::SetSpanList(const std::vector<SpanInfo>& spanList)
{
    spanList_.clear();
    spanList_.resize(spanList.size());
    std::copy(spanList.begin(), spanList.end(), spanList_.begin());
}

const std::vector<SpanInfo>& AccessibilityElementInfo::GetSpanList() const
{
    return spanList_;
}

SpanInfo::SpanInfo(const int32_t& spanId, const std::string& spanText, const std::string& accessibilityText,
    const std::string& accessibilityDescription, const std::string& accessibilityLevel)
    : spanId_(spanId), spanText_(spanText), accessibilityText_(accessibilityText),
      accessibilityDescription_(accessibilityDescription), accessibilityLevel_(accessibilityLevel) {}

void SpanInfo::SetSpanId(const int32_t spanId)
{
    spanId_ = spanId;
}

void SpanInfo::SetSpanText(const std::string spanText)
{
    spanText_ = spanText;
}

void SpanInfo::SetAccessibilityText(const std::string& accessibilityText)
{
    accessibilityText_ = accessibilityText;
}

void SpanInfo::SetAccessibilityDescription(const std::string& accessibilityDescription)
{
    accessibilityDescription_ = accessibilityDescription;
}

void SpanInfo::SetAccessibilityLevel(const std::string& accessibilityLevel)
{
    accessibilityLevel_ = accessibilityLevel;
}
int32_t SpanInfo::GetSpanId() const
{
    return spanId_;
}

const std::string& SpanInfo::GetSpanText() const
{
    return spanText_;
}

const std::string& SpanInfo::GetAccessibilityText() const
{
    return accessibilityText_;
}

const std::string& SpanInfo::GetAccessibilityDescription() const
{
    return accessibilityDescription_;
}

const std::string& SpanInfo::GetAccessibilityLevel() const
{
    return accessibilityLevel_;
}
} // namespace Accessibility
} // namespace OHOS