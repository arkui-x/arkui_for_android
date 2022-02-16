/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "adapter/android/entrance/java/jni/dump_helper_jni.h"

#include <unistd.h>

#include "adapter/android/entrance/java/jni/jni_environment.h"
#include "base/log/dump_log.h"
#include "base/log/log.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"

namespace OHOS::Ace::Platform {

bool DumpHelperJni::Register(const std::shared_ptr<JNIEnv>& env)
{
    static const JNINativeMethod methods[] = {
        {
            .name = "nativeDump",
            .signature = "(Ljava/lang/String;Ljava/io/FileDescriptor;[Ljava/lang/String;)V",
            .fnPtr = reinterpret_cast<void*>(&Dump),
        },
    };
    if (!env) {
        LOGE("JNIEnv is null when registering dumpHelper jni!");
        return false;
    }

    const jclass myClass = env->FindClass("ohos/ace/adapter/DumpHelper");
    if (myClass == nullptr) {
        LOGE("Failed ot find dumphelper class");
        return false;
    }

    bool ret = env->RegisterNatives(myClass, methods, ArraySize(methods)) == 0;
    env->DeleteLocalRef(myClass);
    return ret;
}

void DumpHelperJni::Dump(JNIEnv* env, jclass myClass, jstring prefix, jobject fileDescriptor, jobjectArray args)
{
    if (!env) {
        LOGE("JNIEnv is null when call dump!");
        return;
    }

    jclass classFileDesc = env->FindClass("java/io/FileDescriptor");
    if (classFileDesc == nullptr) {
        LOGE("Failed to find class FileDescriptor");
        return;
    }

    auto descriptorField = env->GetFieldID(classFileDesc, "descriptor", "I");
    auto fd = env->GetIntField(fileDescriptor, descriptorField);
    jsize size = env->GetArrayLength(args);
    std::vector<std::string> params;
    for (jsize i = 0; i < size; i++) {
        jstring arg = (jstring)env->GetObjectArrayElement(args, i);
        const char* param = env->GetStringUTFChars(arg, nullptr);
        if (param != nullptr) {
            params.push_back(param);
            env->ReleaseStringUTFChars(arg, param);
        }
    }

    if (!params.empty()) {
        DumpLog::DumpFile file(nullptr, &fclose);
        int32_t newFd = dup(fd);
        if (newFd >= 0) {
            file.reset(fdopen(newFd, "w"));
        }

        if (file) {
            DumpLog::GetInstance().SetDumpFile(std::move(file));
            AceEngine::Get().Dump(params);
            DumpLog::GetInstance().Reset();
        } else {
            LOGE("Failed to open fd as file");
        }

        env->DeleteLocalRef(classFileDesc);
    } else {
        LOGE("Get String failed");
    }
}

} // namespace OHOS::Ace::Platform
