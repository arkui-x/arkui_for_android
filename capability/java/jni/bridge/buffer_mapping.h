/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BUFFER_MAPPING_H
#define FOUNDATION_ACE_ADAPTER_CAPABILITY_JAVA_JNI_BUFFER_MAPPING_H

#include <memory>
#include <string>
#include <vector>

#include "base/log/log.h"
#include "base/utils/macros.h"
#include "securec.h"

namespace OHOS::Ace::Platform {
class ACE_EXPORT BufferMapping {
public:
    BufferMapping() : data_(nullptr), size_(0) {}

    BufferMapping(uint8_t* data, size_t size) : data_(data), size_(size) {}

    BufferMapping(BufferMapping&& mapping) : data_(mapping.data_), size_(mapping.size_)
    {
        mapping.data_ = nullptr;
        mapping.size_ = 0;
    }

    BufferMapping(const BufferMapping&) = delete;
    BufferMapping& operator=(const BufferMapping&) = delete;

    ~BufferMapping()
    {
        if (data_) {
            free(data_);
            data_ = nullptr;
            LOGI("free buffer.");
        }
    }

    size_t GetSize() const
    {
        return size_;
    }

    const uint8_t* GetMapping() const
    {
        return data_;
    }

    static BufferMapping Copy(const void* begin, size_t length)
    {
        auto result = BufferMapping(reinterpret_cast<uint8_t*>(malloc(length)), length);
        if (result.GetMapping() != nullptr) {
            if (memcpy_s((void*)result.GetMapping(), length, begin, length) != EOK) {
                LOGE("Copy buffer error.");
            }
        }
        return result;
    }

    // Removes ownership of the data buffer
    [[nodiscard]] uint8_t* Release()
    {
        uint8_t* result = data_;
        data_ = nullptr;
        size_ = 0;
        return result;
    }

private:
    uint8_t* data_;
    size_t size_;
};
} // namespace OHOS::Ace::Platform
#endif