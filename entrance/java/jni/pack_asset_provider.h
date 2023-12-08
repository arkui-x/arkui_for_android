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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_PACK_ASSET_PROVIDER_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_PACK_ASSET_PROVIDER_H

#include <android/asset_manager_jni.h>
#include <jni.h>

#include "base/log/log.h"
#include "core/common/asset_mapping.h"
#include "jni_environment.h"

namespace OHOS::Ace {
class PackAssetProvider {
public:
    PackAssetProvider(JNIEnv* env, jobject assetManager, std::string dir);
    ~PackAssetProvider() = default;

    std::unique_ptr<AssetMapping> GetAsMapping(const std::string& name) const;
    bool IsValid()
    {
        return true;
    }

private:
    class PackAssetMapping : public AssetMapping {
    public:
        PackAssetMapping(AAsset* asset) : asset_(asset) {}
        ~PackAssetMapping() override
        {
            AAsset_close(asset_);
        }
        size_t GetSize() const override
        {
            return AAsset_getLength(asset_);
        }
        const uint8_t* GetAsset() const override
        {
            return reinterpret_cast<const uint8_t*>(AAsset_getBuffer(asset_));
        }

    private:
        AAsset* const asset_;
    };

    Ace::Platform::JniEnvironment::JavaGlobalRef javaAssetManager_;
    const std::string dir_;
    AAssetManager* manager_;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_PACK_ASSET_PROVIDER_H
