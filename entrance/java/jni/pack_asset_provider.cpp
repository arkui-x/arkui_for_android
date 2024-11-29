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
#include "adapter/android/entrance/java/jni/pack_asset_provider.h"

#include <algorithm>
#include <sstream>
#include <unistd.h>
namespace OHOS::Ace {
PackAssetProvider::PackAssetProvider(JNIEnv* env, jobject jassetManager, std::string dir)
    : javaAssetManager_(Ace::Platform::JniEnvironment::MakeJavaGlobalRef(
          Ace::Platform::JniEnvironment::GetInstance().GetJniEnv(), jassetManager)),
      dir_(std::move(dir))
{
    manager_ = AAssetManager_fromJava(env, jassetManager);
}

std::unique_ptr<AssetMapping> PackAssetProvider::GetAsMapping(const std::string& name) const
{
    std::stringstream ss;
    ss << dir_.c_str() << "/" << name;
    AAsset* asset = AAssetManager_open(manager_, ss.str().c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        return nullptr;
    }
    
    return std::make_unique<PackAssetMapping>(asset);
}
} // namespace OHOS::Ace
