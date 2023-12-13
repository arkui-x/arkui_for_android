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

#include "adapter/android/osal/mouse_style_android.h"

namespace OHOS::Ace {

RefPtr<MouseStyle> MouseStyle::CreateMouseStyle()
{
    return AceType::MakeRefPtr<MouseStyleAndroid>();
}

bool MouseStyleAndroid::SetPointerStyle(int32_t windowId, MouseFormat pointerStyle) const
{
    return false;
}

int32_t MouseStyleAndroid::GetPointerStyle(int32_t windowId, int32_t& pointerStyle) const
{
    return -1;
}

bool MouseStyleAndroid::ChangePointerStyle(int32_t windowId, MouseFormat mouseFormat) const
{
    return false;
}

} // namespace OHOS::Ace