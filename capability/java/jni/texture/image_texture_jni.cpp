/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "image_texture_jni.h"

#include <android/native_window_jni.h>
#include <sstream>

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Platform {
namespace {
static std::mutex g_textureMapMutex;
static std::map<ImageTextureJni::ImageTextureId, std::shared_ptr<ImageTextureJni>> g_textureMap;
static ImageTextureJni::ImageTextureId g_textureLastId = 0;
} // namespace

ImageTextureJni::ImageTextureJni()
{
    std::lock_guard<std::mutex> lock(g_textureMapMutex);
    textureId_ = ++g_textureLastId;
}

ImageTextureJni::~ImageTextureJni()
{
    std::lock_guard<std::mutex> lock(imageReaderMutex_);
    if (imageReader_ != nullptr) {
        AImageReader_setImageListener(imageReader_, nullptr);
        AImageReader_delete(imageReader_);
        imageReader_ = nullptr;
    }

    if (oldNativeWindow_ != nullptr) {
        ANativeWindow_release(oldNativeWindow_);
        oldNativeWindow_ = nullptr;
    }

    if (nativeWindow_ != nullptr) {
        ANativeWindow_release(nativeWindow_);
        nativeWindow_ = nullptr;
    }
}

int32_t ImageTextureJni::Create(int32_t width, int32_t height, int32_t format, int64_t usageFlags, int32_t maxImages)
{
    if (imageReader_ != nullptr) {
        AImageReader_setImageListener(imageReader_, nullptr);
        AImageReader_delete(imageReader_);
        imageReader_ = nullptr;
    }
    if (oldNativeWindow_ != nullptr) {
        ANativeWindow_release(oldNativeWindow_);
        oldNativeWindow_ = nullptr;
    }
    if (nativeWindow_ != nullptr) {
        oldNativeWindow_ = nativeWindow_;
        nativeWindow_ = nullptr;
    }
    media_status_t status = AImageReader_newWithUsage(width, height, format, usageFlags, maxImages, &imageReader_);
    if (status != AMEDIA_OK || imageReader_ == nullptr) {
        status = AImageReader_new(width, height, format, maxImages, &imageReader_);
        if (status != AMEDIA_OK || imageReader_ == nullptr) {
            LOGE("AImageReader_new failed, status=%{public}d", status);
            return status;
        }
    }
    status = AImageReader_getWindow(imageReader_, &nativeWindow_);
    if (status != AMEDIA_OK || nativeWindow_ == nullptr) {
        LOGE("AImageReader_getWindow failed, status=%{public}d", status);
        AImageReader_delete(imageReader_);
        imageReader_ = nullptr;
        return status;
    }

    listener_ = { .context = this, .onImageAvailable = OnImageAvailable };
    status = AImageReader_setImageListener(imageReader_, &listener_);
    if (status != AMEDIA_OK) {
        LOGE("AImageReader_setImageListener failed, status=%{public}d", status);
        AImageReader_delete(imageReader_);
        imageReader_ = nullptr;
        ANativeWindow_release(nativeWindow_);
        nativeWindow_ = nullptr;
        return status;
    }
    return AMEDIA_OK;
}

void ImageTextureJni::OnImageAvailable(void* context, AImageReader* reader)
{
    ImageTextureJni* texture = static_cast<ImageTextureJni*>(context);
    if (texture != nullptr) {
        texture->NotifyImageAvailable();
    }
}

void ImageTextureJni::NotifyImageAvailable()
{
    std::lock_guard<std::mutex> lock(listenerMutex_);
    for (auto& [id, listener] : imageAvailableListeners_) {
        if (listener != nullptr) {
            listener->OnImageAvailable(textureId_);
        }
    }
}

ImageTextureJni::ImageListenerId ImageTextureJni::SetImageAvailableListener(
    const std::shared_ptr<ListenerContent>& listenerContent)
{
    std::lock_guard<std::mutex> lock(listenerMutex_);
    uint32_t listenerId = ++currentListenerId_;
    imageAvailableListeners_[listenerId] = listenerContent;
    return listenerId;
}

void ImageTextureJni::RemoveImageAvailableListener(const ImageListenerId& listener)
{
    std::lock_guard<std::mutex> lock(listenerMutex_);
    auto it = imageAvailableListeners_.find(listener);
    if (it != imageAvailableListeners_.end()) {
        imageAvailableListeners_.erase(it);
    }
}

bool ImageTextureJni::IsValidHardwareBuffer(const AHardwareBuffer* buffer) const
{
    AHardwareBuffer_Desc desc;
    AHardwareBuffer_describe(buffer, &desc);

    if (desc.width <= 0 || desc.height <= 0) {
        LOGE("IsValidHardwareBuffer: Invalid buffer size %{public}ux%{public}u", desc.width, desc.height);
        return false;
    }
    return true;
}

ImageTextureJni::ImageTextureId ImageTextureJni::CreateImageTexture()
{
    auto texture = std::make_shared<ImageTextureJni>();
    CHECK_NULL_RETURN(texture, 0);
    {
        std::lock_guard<std::mutex> lock(g_textureMapMutex);
        g_textureMap[texture->GetTextureId()] = texture;
    }
    return texture->GetTextureId();
}

std::shared_ptr<ImageTextureJni> ImageTextureJni::GetImageTexture(ImageTextureId id)
{
    std::lock_guard<std::mutex> lock(g_textureMapMutex);
    auto it = g_textureMap.find(id);
    if (it != g_textureMap.end()) {
        return it->second;
    }
    return nullptr;
}

static JNINativeMethod g_methods[] = {
    {
        .name = "nativeCreateImageTexture",
        .signature = "()J",
        .fnPtr = reinterpret_cast<void*>(&ImageTextureJni::JniCreateImageTexture),
    },
    {
        .name = "nativeGetImageSurface",
        .signature = "(JIIIJI)Landroid/view/Surface;",
        .fnPtr = reinterpret_cast<void*>(&ImageTextureJni::JniGetImageSurface),
    },
    {
        .name = "nativeDeleteImageTexture",
        .signature = "(J)V",
        .fnPtr = reinterpret_cast<void*>(&ImageTextureJni::JniDeleteImageTexture),
    },
    {
        .name = "nativeSetImageAvailableListener",
        .signature = "(JLohos/ace/adapter/capability/texture/AceImageTexture$OnImageAvailableListener;)J",
        .fnPtr = reinterpret_cast<void*>(&ImageTextureJni::JniSetImageAvailableListener),
    },
    {
        .name = "nativeGetId",
        .signature = "(JJJ)V",
        .fnPtr = reinterpret_cast<void*>(&ImageTextureJni::JninativeGetId),
    },
    {
        .name = "nativeRemoveImageAvailableListener",
        .signature = "(JJ)V",
        .fnPtr = reinterpret_cast<void*>(&ImageTextureJni::JniRemoveImageAvailableListener),
    }
};

bool ImageTextureJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    CHECK_NULL_RETURN(env, false);
    jclass clazz = env->FindClass("ohos/ace/adapter/capability/texture/AceImageTexture");
    CHECK_NULL_RETURN(clazz, false);

    if (env->RegisterNatives(clazz, g_methods, sizeof(g_methods) / sizeof(g_methods[0])) != JNI_OK) {
        LOGE("AndroidSurfaceTexture: registered failed");
        env->DeleteLocalRef(clazz);
        return false;
    }

    env->DeleteLocalRef(clazz);
    return true;
}

jlong ImageTextureJni::JniCreateImageTexture(JNIEnv* env, jobject myObject)
{
    ImageTextureId id = CreateImageTexture();
    if (id == 0) {
        return 0;
    }
    return static_cast<jlong>(id);
}

jobject ImageTextureJni::JniGetImageSurface(JNIEnv* env, jobject myObject, jlong imageTextureId,
    jint width, jint height, jint format, jlong usageFlags, jint maxImages)
{
    auto texture = GetImageTexture(imageTextureId);
    CHECK_NULL_RETURN(texture, nullptr);

    std::lock_guard<std::mutex> lock(texture->imageReaderMutex_);
    int32_t ret = texture->Create(width, height, format, usageFlags, maxImages);
    if (ret != AMEDIA_OK) {
        LOGE("Failed to create ImageReader for id %{public}lld", static_cast<ImageTextureId>(imageTextureId));
        return nullptr;
    }
    texture->SetUpdateState(true);
    jobject surface = ANativeWindow_toSurface(env, texture->GetNativeWindow());
    CHECK_NULL_RETURN(surface, nullptr);
    return surface;
}

void ImageTextureJni::SetUpdateState(bool state)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    updateState_ = state;
}

bool ImageTextureJni::GetUpdateState()
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return updateState_;
}

std::shared_ptr<AcquiredFrame> ImageTextureJni::AcquireLatestHardwareBuffer()
{
    std::lock_guard<std::mutex> lock(imageReaderMutex_);
    CHECK_NULL_RETURN(imageReader_, nullptr);

    AImage* image = nullptr;
    media_status_t status = AImageReader_acquireLatestImage(imageReader_, &image);
    if (status != AMEDIA_OK || image == nullptr) {
        return nullptr;
    }
    AHardwareBuffer* lastHardwareBuffer = nullptr;
    status = AImage_getHardwareBuffer(image, &lastHardwareBuffer);
    if (status != AMEDIA_OK) {
        LOGE("AImage_getHardwareBuffer failed, status=%{public}d", status);
        AImage_delete(image);
        return nullptr;
    }
    AHardwareBuffer_Desc desc;
    AHardwareBuffer_describe(lastHardwareBuffer, &desc);
    if (desc.width <= 0 || desc.height <= 0) {
        LOGE("Invalid hardware buffer dimensions: width=%{public}d, height=%{public}d", desc.width, desc.height);
        AImage_delete(image);
        return nullptr;
    }

    auto frame = std::make_shared<AcquiredFrame>();
    frame->image = image;
    frame->buffer = lastHardwareBuffer;
    frame->bufferWidth = desc.width;
    frame->bufferHeight = desc.height;
    return frame;
}

void ImageTextureJni::JniDeleteImageTexture(JNIEnv* env, jobject myObject, jlong imageTextureId)
{
    std::lock_guard<std::mutex> lock(g_textureMapMutex);
    auto it = g_textureMap.find(imageTextureId);
    if (it != g_textureMap.end()) {
        g_textureMap.erase(it);
    }
}

class ListenerContentImp : public ImageTextureJni::ListenerContent {
public:
    explicit ListenerContentImp(jobject listener)
        : imageAvailableListener_(
            JniEnvironment::MakeJavaGlobalRef(JniEnvironment::GetInstance().GetJniEnv(), listener)) {}

    bool Init()
    {
        auto env = JniEnvironment::GetInstance().GetJniEnv();
        const jclass clazz = env->GetObjectClass(imageAvailableListener_.get());
        CHECK_NULL_RETURN(clazz, false);
        onImageAvailableMethod_ = env->GetMethodID(clazz, "onImageAvailableListener", "(J)V");
        CHECK_NULL_RETURN(onImageAvailableMethod_, false);
        env->DeleteLocalRef(clazz);
        return true;
    }

    void OnImageAvailable(ImageTextureJni::ImageTextureId textureId) override
    {
        auto env = JniEnvironment::GetInstance().GetJniEnv();
        CHECK_NULL_VOID(env);
        env->CallVoidMethod(imageAvailableListener_.get(), onImageAvailableMethod_, textureId);
        if (env->ExceptionCheck()) {
            LOGE("ImageListenerId exception occurred");
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }

private:
    JniEnvironment::JavaGlobalRef imageAvailableListener_;
    jmethodID onImageAvailableMethod_ = nullptr;
};

void ImageTextureJni::JninativeGetId(
    JNIEnv* env, jobject myObject, jlong imageTextureId, jlong instanceId, jlong id)
{
    auto texture = GetImageTexture(imageTextureId);
    CHECK_NULL_VOID(texture);
    texture->SetinstanceId(instanceId);
    texture->SetId(id);
}

jlong ImageTextureJni::JniSetImageAvailableListener(
    JNIEnv* env, jobject myObject, jlong imageTextureId, jobject listener)
{
    auto texture = GetImageTexture(imageTextureId);
    CHECK_NULL_RETURN(texture, 0);

    auto listenerContent = std::make_shared<ListenerContentImp>(listener);
    CHECK_NULL_RETURN(listenerContent, 0);
    if (!listenerContent->Init()) {
        return 0;
    }
    return static_cast<jlong>(texture->SetImageAvailableListener(listenerContent));
}

void ImageTextureJni::JniRemoveImageAvailableListener(
    JNIEnv* env, jobject myObject, jlong imageTextureId, jlong listenerId)
{
    auto texture = GetImageTexture(imageTextureId);
    CHECK_NULL_VOID(texture);

    texture->RemoveImageAvailableListener(listenerId);
}
} // namespace OHOS::Ace::Platform
