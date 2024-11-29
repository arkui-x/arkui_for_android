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

#ifndef ACCESSIBILITY_ELEMENT_OPERATOR_CALLBACK_H
#define ACCESSIBILITY_ELEMENT_OPERATOR_CALLBACK_H

#include <list>

#include "accessibility_element_info.h"

namespace OHOS {
namespace Accessibility {
class AccessibilityElementOperatorCallback {
public:
    virtual ~AccessibilityElementOperatorCallback() = default;

    virtual void SetSearchElementInfoByAccessibilityIdResult(
        const std::list<AccessibilityElementInfo>& infos, const int32_t requestId) = 0;

    virtual void SetSearchElementInfoByTextResult(
        const std::list<AccessibilityElementInfo>& infos, const int32_t requestId) = 0;

    virtual void SetFindFocusedElementInfoResult(const AccessibilityElementInfo& info, const int32_t requestId) = 0;

    virtual void SetFocusMoveSearchResult(const AccessibilityElementInfo& info, const int32_t requestId) = 0;

    virtual void SetExecuteActionResult(const bool succeeded, const int32_t requestId) = 0;

    virtual void SetCursorPositionResult(const int32_t cursorPosition, const int32_t requestId) = 0;
};
} // namespace Accessibility
} // namespace OHOS
#endif // ACCESSIBILITY_ELEMENT_OPERATOR_CALLBACK_H