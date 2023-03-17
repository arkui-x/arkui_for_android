/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_WINDOW_H
#define OHOS_ROSEN_WINDOW_H

#include <memory>

namespace OHOS {
namespace Rosen {
constexpr uint32_t INVALID_WINDOW_ID = 0;

enum class WindowMode : uint32_t {
    WINDOW_MODE_UNDEFINED = 0,
    WINDOW_MODE_FULLSCREEN = 1,
    WINDOW_MODE_SPLIT_PRIMARY = 100,
    WINDOW_MODE_SPLIT_SECONDARY,
    WINDOW_MODE_FLOATING,
    WINDOW_MODE_PIP
};

class WindowOption {
public:
    WindowOption() = default;
    virtual ~WindowOption() = default;
};
class Window {
public:
    explicit Window(const std::shared_ptr<WindowOption>& option);
    ~Window() = default;

    uint32_t GetWindowId() const;
    bool IsFocused() const
    {
        return false;
    };
    bool IsDecorEnable() const
    {
        return false;
    };
};
}
}
#endif // OHOS_ROSEN_WINDOW_H
