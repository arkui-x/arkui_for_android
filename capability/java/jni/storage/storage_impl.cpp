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

#include "adapter/android/capability/java/jni/storage/storage_impl.h"

#include "adapter/android/capability/java/jni/storage/storage_jni.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::Platform {

StorageImpl::StorageImpl() : Storage()
{
    auto pipelineContext = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    RefPtr<TaskExecutor> taskExecutor = pipelineContext->GetTaskExecutor();
    taskExecutor_ = taskExecutor;
}

void StorageImpl::SetString(const std::string& key, const std::string& value)
{
    if (taskExecutor_) {
        taskExecutor_->PostSyncTask(
            [key, value] { StorageJni::Set(key, value); }, TaskExecutor::TaskType::JS, "ArkUI-XStotageSetString");
    }
}

std::string StorageImpl::GetString(const std::string& key)
{
    std::string result;
    if (taskExecutor_) {
        taskExecutor_->PostSyncTask(
            [key, &result] { result = StorageJni::Get(key); }, TaskExecutor::TaskType::JS, "ArkUI-XStotageGetString");
    }
    return result;
}

void StorageImpl::SetDouble(const std::string& key, const double value) {}

bool StorageImpl::GetDouble(const std::string& key, double& value)
{
    return false;
}

void StorageImpl::SetBoolean(const std::string& key, const bool value) {}

bool StorageImpl::GetBoolean(const std::string& key, bool& value)
{
    return false;
}

void StorageImpl::Clear()
{
    if (taskExecutor_) {
        taskExecutor_->PostSyncTask([] { StorageJni::Clear(); }, TaskExecutor::TaskType::JS, "ArkUI-XStotageClear");
    }
}

void StorageImpl::Delete(const std::string& key)
{
    if (taskExecutor_) {
        taskExecutor_->PostSyncTask(
            [key] { StorageJni::Delete(key); }, TaskExecutor::TaskType::JS, "ArkUI-XStotageDelete");
    }
}

} // namespace OHOS::Ace::Platform
