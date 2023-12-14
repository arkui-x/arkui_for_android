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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JNI_ENVIRONMENT_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JNI_ENVIRONMENT_H

#include <cstdint>
#include <memory>

#include "jni.h"

#include "base/utils/macros.h"
#include "base/utils/noncopyable.h"
#include "base/utils/singleton.h"

namespace OHOS::Ace::Platform {

// Help to convert jlong to pointer.
template<typename T>
inline const T* JavaLongToConstPointer(jlong value)
{
    return reinterpret_cast<const T*>(static_cast<uintptr_t>(value));
}

template<typename T>
inline T* JavaLongToPointer(jlong value)
{
    return reinterpret_cast<T*>(static_cast<uintptr_t>(value));
}

// Help to convert pointer to jlong.
template<typename T>
inline jlong ConstPointerToJavaLong(const T* ptr)
{
    return static_cast<jlong>(reinterpret_cast<uintptr_t>(ptr));
}

template<typename T>
inline jlong PointerToJavaLong(T* ptr)
{
    return static_cast<jlong>(reinterpret_cast<uintptr_t>(ptr));
}

// Help to hold shared pointer
template<typename T>
class SharedPtrHolder {
public:
    SharedPtrHolder() = delete;

    static inline jlong Save(const std::shared_ptr<T>& sharedPtr)
    {
        return sharedPtr ? PointerToJavaLong(new SharedPtrHolder(sharedPtr)) : 0;
    }

    static inline std::shared_ptr<T> Load(jlong sharedPtreId)
    {
        return sharedPtreId != 0 ? JavaLongToPointer<SharedPtrHolder>(sharedPtreId)->Get() : nullptr;
    }

    static inline void Destroy(jlong sharedPtreId)
    {
        if (sharedPtreId != 0) {
            delete JavaLongToPointer<SharedPtrHolder>(sharedPtreId);
        }
    }

private:
    explicit SharedPtrHolder(const std::shared_ptr<T>& sharedPtr) : sharedPtr_(sharedPtr) {}
    ~SharedPtrHolder() = default;

    inline const std::shared_ptr<T>& Get() const
    {
        return sharedPtr_;
    }

    std::shared_ptr<T> sharedPtr_;

    ACE_DISALLOW_COPY_AND_MOVE(SharedPtrHolder);
};

class ACE_EXPORT JniEnvironment : public NonCopyable {
    DECLARE_SINGLETON(JniEnvironment);

public:
    // Help to get basic tyoe of a pointer, this function should NEVER be called.
    template<typename T>
    static T GetOriginalType(T* ptr)
    {
        return T();
    }

    static JniEnvironment& GetInstance();

    // Defination of Java object and Java weak reference.
    using JavaObject = decltype(GetOriginalType(jobject()));
    using JavaWeak = decltype(GetOriginalType(jweak()));
    using JavaGlobalRefDeleter = void (*)(jobject);
    using JavaGlobalRef = std::unique_ptr<JavaObject, JavaGlobalRefDeleter>;
    using JavaLocalRefDeleter = void (*)(jobject);
    using JavaLocalRef = std::unique_ptr<JavaObject, JavaLocalRefDeleter>;
    using JavaWeakRefDeleter = void (*)(jweak);
    using JavaWeakRef = std::unique_ptr<JavaWeak, JavaWeakRefDeleter>;

    // Defination of method information of Java native methods.
    // Use this to avoid some warnings while compiling.
    struct JniMethod {
        const char* name;
        const char* signaure;
        void* function;
    };

    inline static const JNINativeMethod* ToJniNativeMethod(const JniMethod* methods)
    {
        return reinterpret_cast<const JNINativeMethod*>(methods);
    }

    // Initialize of JNI environment with Java VM
    bool Initialize(const std::shared_ptr<JavaVM>& javaVm);

    // Get JNI environment for current thread, if current thread is not attatched to Java VM, attch it.
    std::shared_ptr<JNIEnv> GetJniEnv(JNIEnv* jniEnv = nullptr, bool isDetach = true) const;

    std::shared_ptr<JavaVM> GetVM() const
    {
        return javaVm_;
    }

    // Get version of current Java VM.
    inline jint Version() const
    {
        return version_;
    }

    // Delete Java Object reference.
    static void DeleteJavaGlobalRef(jobject object);
    static void DeleteJavaLocalRef(jobject object);
    static void DeleteJavaWeakRef(jobject object);

    // Make Java Object reference.
    static JavaGlobalRef MakeJavaGlobalRef(
        const std::shared_ptr<JNIEnv>& jniEnvIn, jobject object, JavaGlobalRefDeleter deleter = DeleteJavaGlobalRef);
    static JavaLocalRef MakeJavaLocalRef(
        const std::shared_ptr<JNIEnv>& jniEnvIn, jobject object, JavaLocalRefDeleter deleter = DeleteJavaLocalRef);
    static JavaWeakRef MakeJavaWeakRef(
        const std::shared_ptr<JNIEnv>& jniEnvIn, jobject object, JavaWeakRefDeleter deleter = DeleteJavaWeakRef);
private:
    static JniEnvironment instance_;
    std::shared_ptr<JavaVM> javaVm_;
    jint version_ { JNI_VERSION_1_6 };
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_ENTRANCE_JAVA_JNI_JNI_ENVIRONMENT_H