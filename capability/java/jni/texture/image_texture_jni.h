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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_IMAGE_TEXTURE_JNI_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_IMAGE_TEXTURE_JNI_H

#include <android/hardware_buffer_jni.h>
#include <android/native_window.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <media/NdkImageReader.h>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

namespace OHOS::Ace::Platform {
struct AcquiredFrame {
    AImage* image = nullptr;
    AHardwareBuffer* buffer = nullptr;
    uint32_t bufferWidth = 0;
    uint32_t bufferHeight = 0;

    ~AcquiredFrame()
    {
        if (image != nullptr) {
            AImage_delete(image);
            image = nullptr;
        }
        if (buffer != nullptr) {
            buffer = nullptr;
        }
    }
};

class ImageTextureJni {
public:
    using ImageTextureOnAvailableCallback = std::function<void(ImageTextureJni* imageTexture)>;
    using ImageListenerId = uint32_t;
    using ImageTextureId = int64_t;

    class ListenerContent {
    public:
        ListenerContent() = default;
        virtual ~ListenerContent() = default;
        virtual void OnImageAvailable(ImageTextureId textureId) = 0;
    };
    ImageTextureJni();
    ~ImageTextureJni();
    static ImageTextureId CreateImageTexture();
    static int32_t CreateImageReader(ImageTextureId id);
    static std::shared_ptr<ImageTextureJni> GetImageTexture(ImageTextureId id);

    bool IsValid() const
    {
        return imageReader_ != nullptr && nativeWindow_ != nullptr;
    }

    int64_t GetTextureId() const
    {
        return textureId_;
    }

    int32_t GetRequestedWidth() const
    {
        return width_;
    }

    int32_t GetRequestedHeight() const
    {
        return height_;
    }

    void SetinstanceId(jlong instanceId)
    {
        instanceId_ = instanceId;
    }

    void SetId(jlong id)
    {
        id_ = id;
    }

    void SetUpdateState(bool state);
    bool GetUpdateState();
    std::shared_ptr<AcquiredFrame> AcquireLatestHardwareBuffer();
    static bool Register(const std::shared_ptr<JNIEnv>& env);
    static jlong JniCreateImageTexture(JNIEnv* env, jobject myObject);
    static jobject JniGetImageSurface(JNIEnv* env, jobject myObject, jlong imageTextureId,
        jint width, jint height, jint format, jlong usageFlags, jint maxImages);
    static jlong JniGetImageReader(JNIEnv* env, jobject myObject, jlong imageTextureId);
    static void JniDeleteImageTexture(JNIEnv* env, jobject myObject, jlong imageTextureId);
    static jlong JniSetImageAvailableListener(JNIEnv* env, jobject myObject, jlong imageTextureId, jobject listener);
    static void JniRemoveImageAvailableListener(JNIEnv* env, jobject myObject, jlong imageTextureId, jlong listenerId);
    static void JninativeGetId(
        JNIEnv* env, jobject myObject, jlong imageTextureId, jlong instanceId, jlong id);

private:
    ImageTextureJni(const ImageTextureJni&) = delete;
    ImageTextureJni& operator=(const ImageTextureJni&) = delete;
    int32_t Create(int32_t width, int32_t height, int32_t format, int64_t usageFlags, int32_t maxImages);
    ImageListenerId SetImageAvailableListener(const std::shared_ptr<ListenerContent>& listenerContent);
    void RemoveImageAvailableListener(const ImageListenerId& listener);
    ANativeWindow* GetNativeWindow() const
    {
        return nativeWindow_;
    }

    AImageReader* GetImageReader() const
    {
        return imageReader_;
    }

    void NotifyImageAvailable();
    bool IsValidHardwareBuffer(const AHardwareBuffer* buffer) const;
    static void OnImageAvailable(void* context, AImageReader* reader);

    int32_t width_;
    int32_t height_;
    int32_t format_;
    int32_t maxImages_;
    int64_t usageFlags_;
    bool updateState_ = false;
    AImageReader* imageReader_ = nullptr;
    ANativeWindow* nativeWindow_ = nullptr;
    ANativeWindow* oldNativeWindow_ = nullptr;
    AImageReader_ImageListener listener_;
    uint32_t currentListenerId_ = 0;
    std::mutex imageReaderMutex_;
    std::mutex listenerMutex_;
    std::mutex stateMutex_;
    std::map<int32_t, std::shared_ptr<ListenerContent>> imageAvailableListeners_;
    ImageTextureId textureId_ = 0;
    jlong instanceId_;
    jlong id_;
    AHardwareBuffer* lastHardwareBuffer_ = nullptr;
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_IMAGE_TEXTURE_JNI_H
