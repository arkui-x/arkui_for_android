/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_OHOS_OSAL_MOUSE_STYLE_ANDROID_H
#define FOUNDATION_ACE_ADAPTER_OHOS_OSAL_MOUSE_STYLE_ANDROID_H

#include "base/mousestyle/mouse_style.h"

namespace OHOS::Ace {

class MouseStyleAndroid : public MouseStyle {
    DECLARE_ACE_TYPE(MouseStyleAndroid, MouseStyle)

public:
    MouseStyleAndroid() = default;
    ~MouseStyleAndroid() = default;
    bool SetPointerStyle(int32_t windowId, MouseFormat pointerStyle) const override;
    int32_t GetPointerStyle(int32_t windowId, int32_t& pointerStyle) const override;
    bool ChangePointerStyle(int32_t windowId, MouseFormat mouseFormat) const override;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_OHOS_OSAL_MOUSE_STYLE_ANDROID_H