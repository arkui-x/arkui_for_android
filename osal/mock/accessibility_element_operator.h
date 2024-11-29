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

#ifndef ACCESSIBILITY_ELEMENT_OPERATOR_H
#define ACCESSIBILITY_ELEMENT_OPERATOR_H

#include <map>

#include "accessibility_element_operator_callback.h"

namespace OHOS {
namespace Accessibility {
class AccessibilityElementOperator {
public:
    virtual ~AccessibilityElementOperator() = default;

    virtual void SearchElementInfoByAccessibilityId(const int64_t elementId, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback, const int32_t mode) = 0;

    virtual void SearchElementInfosByText(const int64_t elementId, const std::string& text, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback) = 0;

    virtual void FindFocusedElementInfo(const int64_t elementId, const int32_t focusType, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback) = 0;

    virtual void FocusMoveSearch(const int64_t elementId, const int32_t direction, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback) = 0;

    virtual void ExecuteAction(const int64_t elementId, const int32_t action,
        const std::map<std::string, std::string>& actionArguments, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback) = 0;

    virtual void GetCursorPosition(
        const int64_t elementId, const int32_t requestId, AccessibilityElementOperatorCallback& callback) = 0;

    virtual void ClearFocus() = 0;
    virtual void OutsideTouch() = 0;

    virtual void SetChildTreeIdAndWinId(const int64_t elementId, const int32_t treeId, const int32_t childWindowId)
    {
        (void)elementId;
        (void)treeId;
        (void)childWindowId;
    }

    virtual void SetBelongTreeId(const int32_t treeId)
    {
        (void)treeId;
    }

    virtual void SetParentWindowId(const int32_t parentWindowId)
    {
        (void)parentWindowId;
    }
};
} // namespace Accessibility
} // namespace OHOS
#endif // ACCESSIBILITY_ELEMENT_OPERATOR_H