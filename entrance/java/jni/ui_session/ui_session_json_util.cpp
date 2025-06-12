/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "interfaces/inner_api/ui_session/ui_session_json_util.h"

namespace OHOS::Ace {

InspectorJsonValue::InspectorJsonValue(JsonObject* object) : object_(object) {}

InspectorJsonValue::InspectorJsonValue(JsonObject* object, bool isRoot) : object_(object), isRoot_(isRoot) {}

InspectorJsonValue::~InspectorJsonValue()
{
    if (object_ && isRoot_) {
    }
}

bool InspectorJsonValue::Contains(const std::string& key) const
{
    return false;
}

bool InspectorJsonValue::IsObject() const
{
    return false;
}

bool InspectorJsonValue::Put(double value)
{
    return false;
}

bool InspectorJsonValue::Put(const char* key, const char* value)
{
    return false;
}

bool InspectorJsonValue::Put(const char* key, const std::unique_ptr<InspectorJsonValue>& value)
{
    return false;
}

// add item to array
bool InspectorJsonValue::Put(const std::unique_ptr<InspectorJsonValue>& value)
{
    return false;
}

bool InspectorJsonValue::Put(const char* key, size_t value)
{
    return false;
}

bool InspectorJsonValue::Put(const char* key, int32_t value)
{
    return false;
}

bool InspectorJsonValue::Put(const char* key, int64_t value)
{
    return false;
}

bool InspectorJsonValue::Put(const char* key, double value)
{
    return false;
}

bool InspectorJsonValue::Put(const char* key, bool value)
{
    return false;
}

bool InspectorJsonValue::Replace(const char* key, const char* value)
{
    return false;
}

const JsonObject* InspectorJsonValue::GetJsonObject() const
{
    return nullptr;
}

std::string InspectorJsonValue::ToString()
{
    return "";
}

std::string InspectorJsonValue::GetString() const
{
    return "";
}

std::string InspectorJsonValue::GetString(const std::string& key, const std::string& defaultVal) const
{
    return "";
}

std::unique_ptr<InspectorJsonValue> InspectorJsonValue::GetValue(const std::string& key) const
{
    return nullptr;
}

int32_t InspectorJsonValue::GetInt() const
{
    return 0;
}

int32_t InspectorJsonValue::GetInt(const std::string& key, int32_t defaultVal) const
{
    return 0;
}

bool InspectorJsonValue::IsNull() const
{
    return false;
}

bool InspectorJsonValue::IsString() const
{
    return false;
}

bool InspectorJsonValue::IsNumber() const
{
    return false;
}

std::shared_ptr<InspectorJsonValue> InspectorJsonUtil::Create(bool isRoot)
{
    return nullptr;
}

std::unique_ptr<InspectorJsonValue> InspectorJsonUtil::CreateArray(bool isRoot)
{
    return nullptr;
}

std::unique_ptr<InspectorJsonValue> InspectorJsonUtil::CreateObject(bool isRoot)
{
    return nullptr;
}

std::unique_ptr<InspectorJsonValue> InspectorJsonUtil::ParseJsonData(const char* data, const char** parseEnd)
{
    return nullptr;
}

std::unique_ptr<InspectorJsonValue> InspectorJsonUtil::ParseJsonString(
    const std::string& content, const char** parseEnd)
{
    return nullptr;
}

} // namespace OHOS::Ace