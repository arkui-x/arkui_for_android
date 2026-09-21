/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_DMSERVER_DISPLAY_MANAGER_AGENT_H
#define FOUNDATION_DMSERVER_DISPLAY_MANAGER_AGENT_H

#include <cstdint>
#include <parcel.h>
#include <mutex>
#include <memory>
#include <vector>
#include "nocopyable.h"
#include "base/utils/macros.h"

namespace OHOS::Rosen {

class ACE_EXPORT DisplayManagerAgent : public virtual RefBase {
public:
    DisplayManagerAgent();
    ~DisplayManagerAgent();
    DISALLOW_COPY_AND_MOVE(DisplayManagerAgent);
    static std::shared_ptr<DisplayManagerAgent> GetInstance();
    bool IsFoldable() const;
    uint32_t GetFoldStatus() const;
    int32_t GetFoldCreaseDirection() const;
    std::vector<uint64_t> GetAllDisplayIds() const;
    std::vector<int32_t> GetLiveCreaseRegion() const;
    void RegisterDisplayListener();
    void UnregisterDisplayListener();
    std::string RegisterFoldStatusListener();
    void UnRegisterFoldStatusListener();

private:
    static std::shared_ptr<DisplayManagerAgent> instance_;
    static std::mutex mutex_;
};
} // namespace OHOS::Rosen
#endif // FOUNDATION_DMSERVER_DISPLAY_MANAGER_AGENT_H