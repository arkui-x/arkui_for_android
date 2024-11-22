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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_NATIVE_WINDOW_SURFACE_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_NATIVE_WINDOW_SURFACE_H

#include "android/native_window.h"
#include "android/native_window_jni.h"
#include "jni.h"

#include "base/utils/macros.h"

namespace OHOS::Ace {

class ACE_EXPORT NativeWindowFromSurface {
public:
    static ANativeWindow* GetWindow(JNIEnv* env, jobject jsurface)
    {
        return ANativeWindow_fromSurface(env, jsurface);
    }
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_NATIVE_WINDOW_SURFACE_H