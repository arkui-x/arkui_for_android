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

#include "display_info.h"

#include <new>
#include <parcel.h>
#include "base/log/log.h"
#include "base/utils/utils.h"

using namespace OHOS::Ace;
using namespace OHOS::Ace::Platform;

namespace OHOS::Rosen {

DisplayInfoJni DisplayInfo::displayInfoJni_;

bool DisplayInfo::Register(const std::shared_ptr<JNIEnv>& env)
{
    static const JNINativeMethod methods[] = { {
        .name = "nativeSetupDisplayInfo",
        .signature = "()V",
        .fnPtr = reinterpret_cast<void*>(&SetupDisplayInfo),
    } };

    if (!env) {
        LOGE("JNI Window: null java env");
        return false;
    }

    const jclass clazz = env->FindClass("ohos/ace/adapter/DisplayInfo");
    if (clazz == nullptr) {
        LOGE("JNI: can't find java class Window");
        return false;
    }
    bool ret = env->RegisterNatives(clazz, methods, ArraySize(methods)) == 0;
    env->DeleteLocalRef(clazz);
    return ret;
}

void DisplayInfo::SetupDisplayInfo(JNIEnv* env, jobject obj)
{
    LOGI("DisplayInfo::SetupDisplayInfo called");

    jclass clazz = env->GetObjectClass(obj);
    displayInfoJni_.object = env->NewGlobalRef(obj);
    displayInfoJni_.clazz = (jclass)env->NewGlobalRef(clazz);
    displayInfoJni_.getDisplayIdMethod = env->GetMethodID(clazz, "getDisplayId", "()I");
    displayInfoJni_.getOrentationMethod = env->GetMethodID(clazz, "getOrentation", "()I");
    displayInfoJni_.getWidthMethod = env->GetMethodID(clazz, "getWidth", "()I");
    displayInfoJni_.getHeightMethod = env->GetMethodID(clazz, "getHeight", "()I");
    displayInfoJni_.getRefreshRateMethod = env->GetMethodID(clazz, "getRefreshRate", "()F");
}


DisplayInfo::DisplayInfo()
{
    JNIEnv* env = JniEnvironment::GetInstance().GetJniEnv().get();
    if (env == nullptr) {
        LOGE("DisplayInfo:: env is NULL");
        return;
    }
    jint displayId = env->CallIntMethod(displayInfoJni_.object, displayInfoJni_.getDisplayIdMethod);
    jint orentation = env->CallIntMethod(displayInfoJni_.object, displayInfoJni_.getOrentationMethod);
    jint width = env->CallIntMethod(displayInfoJni_.object, displayInfoJni_.getWidthMethod);
    jint height = env->CallIntMethod(displayInfoJni_.object, displayInfoJni_.getHeightMethod);
    jfloat refreshRate = env->CallFloatMethod(displayInfoJni_.object, displayInfoJni_.getRefreshRateMethod);

    LOGI("DisplayInfo:: displayId=%d orentation=%d with=%d height=%d refreshRate=%.3f", displayId, orentation, width, height, refreshRate);

    SetDisplayId(displayId);
    SetWidth((int32_t)width);
    SetHeight((int32_t)height);
    SetRefreshRate(refreshRate);

    Orientation orientation2 = Orientation::UNSPECIFIED;
    switch((int)orentation) {
        case 0: { // ROTATION_0
            orientation2 = Orientation::HORIZONTAL;
            break;
        }
        case 1: { // ROTATION_90
            orientation2 = Orientation::VERTICAL;
            break;
        }
        case 2: { // ROTATION_180
            orientation2 = Orientation::REVERSE_HORIZONTAL;
            break;
        }
        case 3: { // ROTATION_270
            orientation2 = Orientation::REVERSE_VERTICAL;
            break;
        }
        default:
            break;
    }
    SetOrientation(orientation2);
}

DisplayInfo::~DisplayInfo()
{

}

DisplayId DisplayInfo::GetDisplayId() const
{
    return id_;
}
int32_t DisplayInfo::GetWidth() const
{
    return width_;
}
int32_t DisplayInfo::GetHeight() const
{
    return height_;
}
Orientation DisplayInfo::GetOrientation() const
{
    return orientation_;
}
DisplayOrientation DisplayInfo::GetDisplayOrientation() const
{
    return displayOrientation_;
}

float DisplayInfo::GetRefreshRate() const {
    return refreshRate_;
}

void DisplayInfo::SetDisplayId(DisplayId displayId)
{
    id_ = displayId;
}
void DisplayInfo::SetWidth(int32_t width)
{
    width_ = width;
}
void DisplayInfo::SetHeight(int32_t height)
{
    height_ = height;
}
void DisplayInfo::SetOrientation(Orientation orientation)
{
    orientation_ = orientation;
}
void DisplayInfo::SetDisplayOrientation(DisplayOrientation displayOrientation)
{
    displayOrientation_ = displayOrientation;
}

void DisplayInfo::SetRefreshRate(float refreshRate) {
    refreshRate_ = refreshRate;
}

} // namespace OHOS::Rosen