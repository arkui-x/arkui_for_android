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

#include "base/window/drag_window.h"

namespace OHOS::Ace {

RefPtr<DragWindow> DragWindow::CreateDragWindow(const std::string& windowName, int32_t x, int32_t y, uint32_t width,
    uint32_t height)
{
    return nullptr;
}

RefPtr<DragWindow> DragWindow::CreateTextDragWindow(
    const std::string& windowName, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    return nullptr;
}

void DragWindow::MoveTo(int32_t x, int32_t y) const {}

void DragWindow::Destroy() const {}

void DragWindow::DrawPixelMap(const RefPtr<PixelMap>& pixelmap) {}

} // namespace OHOS::Ace