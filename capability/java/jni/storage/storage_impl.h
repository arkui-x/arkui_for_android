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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_STORAGE_STORAGE_IMPL_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_STORAGE_STORAGE_IMPL_H

#include "core/common/storage/storage.h"

namespace OHOS::Ace::Platform {

class StorageImpl : public Storage {
public:
    explicit StorageImpl(const RefPtr<TaskExecutor>& taskExecutor);
    ~StorageImpl() override = default;

    void SetString(const std::string& key, const std::string& value) override;
    std::string GetString(const std::string& key) override;
    void SetDouble(const std::string& key, const double value) override;
    bool GetDouble(const std::string& key, double& value) override;
    void SetBoolean(const std::string& key, const bool value) override;
    bool GetBoolean(const std::string& key, bool& value) override;
    void Clear() override;
    void Delete(const std::string& key) override;
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_STORAGE_STORAGE_IMPL_H
