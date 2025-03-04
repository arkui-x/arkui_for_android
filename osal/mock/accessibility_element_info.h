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

#ifndef ACCESSIBILITY_ELEMENT_INFO_H
#define ACCESSIBILITY_ELEMENT_INFO_H

#include <map>
#include <set>
#include <vector>

#include "accessibility_definitions.h"

namespace OHOS {
namespace Accessibility {
/*
 * class define the action on Accessibility info
 */
class AccessibleAction {
public:
    AccessibleAction() {}

    AccessibleAction(ActionType actionType, const std::string& description);

    ActionType GetActionType() const;

    const std::string& GetDescriptionInfo() const;

protected:
    ActionType actionType_ = ACCESSIBILITY_ACTION_INVALID;
    std::string description_ = "";
};
class RangeInfo {
public:
    RangeInfo() {}

    RangeInfo(double min, double max, double current);

    double GetMin() const;

    double GetMax() const;

    double GetCurrent() const;

    void SetMin(double min);

    void SetMax(double max);

    void SetCurrent(double current);

protected:
    double min_ = 0;
    double max_ = 0;
    double current_ = 0;
};

class GridInfo {
public:
    GridInfo() {}

    GridInfo(int32_t rowCount, int32_t columnCount, int32_t mode);

    void SetGrid(int32_t rowCount, int32_t columnCount, int32_t mode);

    void SetGrid(GridInfo other);

    int32_t GetRowCount() const;

    int32_t GetColumnCount() const;

    int32_t GetSelectionMode() const;

protected:
    int32_t rowCount_ = 0;
    int32_t columnCount_ = 0;
    int32_t selectionMode_ = 0;
};

class GridItemInfo {
public:
    GridItemInfo() {}

    GridItemInfo(
        int32_t rowIndex, int32_t rowSpan, int32_t columnIndex, int32_t columnSpan, bool heading);

    void SetGridItemInfo(GridItemInfo other);

    void SetGridItemInfo(
        int32_t rowIndex, int32_t rowSpan, int32_t columnIndex, int32_t columnSpan, bool heading);
    
    void SetSelected(bool selected);

    int32_t GetColumnIndex() const;

    int32_t GetRowIndex() const;

    int32_t GetColumnSpan() const;

    int32_t GetRowSpan() const;

    bool IsHeading() const;

    bool IsSelected() const;

protected:
    bool heading_ = false;
    int32_t columnIndex_ = 0;
    int32_t rowIndex_ = 0;
    int32_t columnSpan_ = 0;
    int32_t rowSpan_ = 0;
    bool selected_ = false;
};

class ExtraElementInfo {
public:
    ExtraElementInfo() {}

    ExtraElementInfo(const std::map<std::string, std::string>& extraElementValueStr,
        const std::map<std::string, int32_t>& extraElementValueInt);

    RetError SetExtraElementInfo(const std::string& keyStr, const std::string& valueStr);

    RetError SetExtraElementInfo(const std::string keyStr, const int32_t valueInt);

    const std::map<std::string, std::string>& GetExtraElementInfoValueStr() const;

    const std::map<std::string, int32_t>& GetExtraElementInfoValueInt() const;

protected:
    std::map<std::string, std::string> extraElementValueStr_ = {};
    std::map<std::string, int32_t> extraElementValueInt_ = {};
    std::set<std::string> setOfExtraElementInfo = { "CheckboxGroupSelectedStatus", "Row", "Column",
        "SideBarContainerStates", "ListItemIndex" };
};

class Rect {
public:
    Rect() {}

    virtual ~Rect() = default;

    Rect(int32_t leftTopX, int32_t leftTopY, int32_t rightBottomX, int32_t rightBottomY)
    {
        leftTopX_ = leftTopX;
        leftTopY_ = leftTopY;
        rightBottomX_ = rightBottomX;
        rightBottomY_ = rightBottomY;
    }

    int32_t GetLeftTopXScreenPostion() const
    {
        return leftTopX_;
    }

    int32_t GetLeftTopYScreenPostion() const
    {
        return leftTopY_;
    }

    int32_t GetRightBottomXScreenPostion() const
    {
        return rightBottomX_;
    }

    int32_t GetRightBottomYScreenPostion() const
    {
        return rightBottomY_;
    }

    void SetLeftTopScreenPostion(int32_t leftTopX, int32_t leftTopY)
    {
        leftTopY_ = leftTopY;
        leftTopX_ = leftTopX;
    }

    void SetRightBottomScreenPostion(int32_t rightBottomX, int32_t rightBottomY)
    {
        rightBottomY_ = rightBottomY;
        rightBottomX_ = rightBottomX;
    }

protected:
    int32_t leftTopX_ = 0;
    int32_t leftTopY_ = 0;
    int32_t rightBottomX_ = 0;
    int32_t rightBottomY_ = 0;
};

class SpanInfo {
public:
    SpanInfo() {}

    SpanInfo(const int32_t& spanId, const std::string& spanText, const std::string& accessibilityText,
        const std::string& accessibilityDescription, const std::string& accessibilityLevel);

    void SetSpanId(const int32_t spanId);

    void SetSpanText(const std::string spanText);

    void SetAccessibilityText(const std::string& accessibilityText);

    void SetAccessibilityDescription(const std::string& accessibilityDescription);

    void SetAccessibilityLevel(const std::string& accessibilityLevel);

    int32_t GetSpanId() const;

    const std::string& GetSpanText() const;

    const std::string& GetAccessibilityText() const;

    const std::string& GetAccessibilityDescription() const;

    const std::string& GetAccessibilityLevel() const;

protected:
    int32_t spanId_;
    std::string spanText_;
    std::string accessibilityText_;
    std::string accessibilityDescription_;
    std::string accessibilityLevel_;
};

class AccessibilityElementInfo {
public:
    static constexpr int64_t UNDEFINED_ACCESSIBILITY_ID = -1;
    static constexpr int32_t UNDEFINED_TREE_ID = -1;
    static constexpr int32_t UNDEFINED_WINID_ID = -1;
    static constexpr int32_t MAX_SIZE = 50;
    static constexpr int64_t ROOT_PARENT_ID = -2100000;

    AccessibilityElementInfo();

    void SetComponentId(const int64_t componentId);

    int64_t GetChildId(const int32_t index) const;

    int32_t GetChildCount() const;

    const std::vector<int64_t>& GetChildIds() const;

    void AddChild(const int64_t childId);

    bool RemoveChild(const int64_t childId);

    const std::vector<AccessibleAction>& GetActionList() const;

    void AddAction(AccessibleAction& action);

    void DeleteAction(AccessibleAction& action);

    bool DeleteAction(ActionType& actionType);

    void DeleteAllActions();

    void SetTextLengthLimit(const int32_t max);

    int32_t GetTextLengthLimit() const;

    int32_t GetWindowId() const;

    void SetWindowId(const int32_t windowId);

    int64_t GetParentNodeId() const;

    void SetParent(const int64_t parentId);

    const Rect& GetRectInScreen() const;

    void SetRectInScreen(Rect& bounds);

    bool IsCheckable() const;

    void SetCheckable(const bool checkable);

    bool IsChecked() const;

    void SetChecked(const bool checked);

    bool IsFocusable() const;

    void SetFocusable(const bool focusable);

    bool IsFocused() const;

    void SetFocused(const bool focused);

    bool IsVisible() const;

    void SetVisible(const bool visible);

    bool HasAccessibilityFocus() const;

    void SetAccessibilityFocus(const bool focused);

    bool IsSelected() const;

    void SetSelected(const bool selected);

    bool IsClickable() const;

    void SetClickable(const bool clickable);

    bool IsLongClickable() const;

    void SetLongClickable(const bool longClickable);

    bool IsEnabled() const;

    void SetEnabled(const bool enabled);

    bool IsPassword() const;

    void SetPassword(const bool type);

    bool IsScrollable() const;

    void SetScrollable(const bool scrollable);

    bool IsEditable() const;

    void SetEditable(const bool editable);

    bool IsPluraLineSupported() const;

    void SetPluraLineSupported(const bool multiLine);

    bool IsPopupSupported() const;

    void SetPopupSupported(const bool supportPopup);

    bool IsDeletable() const;

    void SetDeletable(const bool deletable);

    bool IsEssential() const;

    void SetEssential(const bool essential);

    bool IsGivingHint() const;

    void SetHinting(const bool hinting);

    const std::string& GetBundleName() const;

    void SetBundleName(const std::string& bundleName);

    const std::string& GetComponentType() const;

    void SetComponentType(const std::string& className);

    const std::string& GetContent() const;

    void SetContent(const std::string& text);

    const std::string& GetHint() const;

    void SetHint(const std::string& hintText);

    const std::string& GetDescriptionInfo() const;

    void SetDescriptionInfo(const std::string& contentDescription);

    void SetComponentResourceId(const std::string& viewIdResName);

    const std::string& GetComponentResourceId() const;

    void SetLiveRegion(const int32_t liveRegion);

    int32_t GetLiveRegion() const;

    void SetContentInvalid(const bool contentInvalid);

    bool GetContentInvalid() const;

    void SetError(const std::string& error);

    const std::string& GetError() const;

    void SetLabeled(const int64_t componentId);

    int64_t GetLabeledAccessibilityId() const;

    void SetAccessibilityId(const int64_t componentId);

    int64_t GetAccessibilityId() const;

    const RangeInfo& GetRange() const;

    void SetRange(RangeInfo& rangeInfo);

    void SetSelectedBegin(const int32_t start);

    int32_t GetSelectedBegin() const;

    void SetSelectedEnd(const int32_t end);

    int32_t GetSelectedEnd() const;

    const GridInfo& GetGrid() const;

    void SetGrid(const GridInfo& grid);

    const GridItemInfo& GetGridItem() const;

    void SetGridItem(const GridItemInfo& gridItem);

    int32_t GetCurrentIndex() const;

    void SetCurrentIndex(const int32_t index);

    int32_t GetBeginIndex() const;

    void SetBeginIndex(const int32_t index);

    int32_t GetEndIndex() const;

    void SetEndIndex(const int32_t index);

    int32_t GetInputType() const;

    void SetInputType(const int32_t inputType);

    bool IsValidElement() const;

    void SetValidElement(const bool valid);

    void SetInspectorKey(const std::string& key);

    const std::string& GetInspectorKey() const;

    void SetPagePath(const std::string& path);

    const std::string& GetPagePath() const;

    void SetPageId(const int32_t pageId);

    int32_t GetPageId() const;

    void SetTextMovementStep(const TextMoveUnit granularity);

    TextMoveUnit GetTextMovementStep() const;

    void SetItemCounts(const int32_t itemCounts);

    int32_t GetItemCounts() const;

    void SetTriggerAction(const ActionType action);

    ActionType GetTriggerAction() const;

    void SetContentList(const std::vector<std::string>& contentList);

    void GetContentList(std::vector<std::string>& contentList) const;

    void SetLatestContent(const std::string& content);

    const std::string& GetLatestContent() const;

    void SetAccessibilityText(const std::string& accessibilityText);

    const std::string& GetAccessibilityText() const;

    void SetTextType(const std::string& textType);

    const std::string& GetTextType() const;

    void SetOffset(const float offset);

    float GetOffset() const;

    void SetChildTreeIdAndWinId(const int32_t iChildTreeId, const int32_t iChildWindowId);

    int32_t GetChildTreeId() const;

    int32_t GetChildWindowId() const;

    void SetBelongTreeId(const int32_t iBelongTreeId);

    int32_t GetBelongTreeId() const;

    int32_t GetParentWindowId() const;

    void SetParentWindowId(const int32_t iParentWindowId);

    void SetExtraElement(const ExtraElementInfo& extraElementInfo);

    const ExtraElementInfo& GetExtraElement() const;

    bool GetAccessibilityGroup() const;

    void SetAccessibilityGroup(const bool accessibilityGroup);

    void SetAccessibilityLevel(const std::string accessibilityLevel);

    const std::string& GetAccessibilityLevel() const;

    void SetZIndex(const int32_t zIndex);

    int32_t GetZIndex() const;

    void SetOpacity(const float opacity);

    float GetOpacity() const;

    void SetBackgroundColor(const std::string& backgroundColor);

    const std::string& GetBackgroundColor() const;

    void SetBackgroundImage(const std::string& backgroundImage);

    const std::string& GetBackgroundImage() const;

    void SetBlur(const std::string& blur);

    const std::string& GetBlur() const;

    void SetHitTestBehavior(const std::string& hitTestBehavior);

    const std::string& GetHitTestBehavior() const;

    void SetNavDestinationId(const int64_t navDestinationId);

    int64_t GetNavDestinationId() const;

    void AddSpan(const SpanInfo& span);

    void SetSpanList(const std::vector<SpanInfo>& spanList);

    const std::vector<SpanInfo>& GetSpanList() const;

    void SetImportantForAccessibility(bool important)
    {
        isImportant_ = important;
    }
    bool GetImportantForAccessibility()
    {
        return isImportant_;
    }

protected:
    int32_t pageId_ = -1;
    int32_t windowId_ = -1;
    int64_t elementId_ = UNDEFINED_ACCESSIBILITY_ID;
    int64_t parentId_ = UNDEFINED_ACCESSIBILITY_ID;

    int32_t belongTreeId_ = UNDEFINED_TREE_ID;
    int32_t childTreeId_ = UNDEFINED_TREE_ID;
    int32_t childWindowId_ = UNDEFINED_WINID_ID;
    int32_t parentWindowId_ = UNDEFINED_WINID_ID;

    std::string bundleName_ = "";
    std::string componentType_ = "";
    std::string text_ = "";
    std::string hintText_ = "";
    std::string accessibilityText_ = "";
    std::string contentDescription_ = "";
    std::string resourceName_ = "";
    std::string inspectorKey_ = "";
    std::string pagePath_ = "";
    std::vector<int64_t> childNodeIds_;
    int32_t childCount_ = 0;
    std::vector<AccessibleAction> operations_;
    int32_t textLengthLimit_ = -1;
    Rect bounds_ {};
    bool checkable_ = false;
    bool checked_ = false;
    bool focusable_ = false;
    bool focused_ = false;
    bool visible_ = false;
    bool accessibilityFocused_ = false;
    bool selected_ = false;
    bool clickable_ = false;
    bool longClickable_ = false;
    bool enable_ = false;
    bool isPassword_ = false;
    bool scrollable_ = false;
    bool editable_ = false;
    bool popupSupported_ = false;
    bool multiLine_ = false;
    bool deletable_ = false;
    bool hint_ = false;
    bool isEssential_ = false;
    int32_t currentIndex_ = 0;
    int32_t beginIndex_ = 0;
    int32_t endIndex_ = 0;
    RangeInfo rangeInfo_ {};
    GridInfo grid_ {};
    GridItemInfo gridItem_ {};
    int32_t liveRegion_ = 0;
    bool contentInvalid_ = true;
    std::string error_ = "";
    int64_t labeled_ = 0;
    int32_t beginSelected_ = 0;
    int32_t endSelected_ = 0;
    int32_t inputType_ = 0; // text input type added
    bool validElement_ = true;
    TextMoveUnit textMoveStep_ = STEP_CHARACTER;
    int32_t itemCounts_ = 0;
    ActionType triggerAction_ = ACCESSIBILITY_ACTION_INVALID;
    std::vector<std::string> contentList_ {};
    std::string latestContent_ = "";
    std::string textType_ = "";
    float offset_ = 0.0f;
    ExtraElementInfo extraElementInfo_ {};
    bool accessibilityGroup_ = true;
    std::string accessibilityLevel_ = "auto";
    int32_t zIndex_ = 0;
    float opacity_ = 0.0f;
    std::string backgroundColor_ = "";
    std::string backgroundImage_ = "";
    std::string blur_ = "";
    std::string hitTestBehavior_ = "";
    int64_t navDestinationId_ = -1;
    std::vector<SpanInfo> spanList_ {};
    bool isImportant_ = false;
};
} // namespace Accessibility
} // namespace OHOS
#endif // ACCESSIBILITY_ELEMENT_INFO_H