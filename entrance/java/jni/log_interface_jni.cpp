/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "log_interface_jni.h"

#include <algorithm>
#include <mutex>

#include "securec.h"
#include "utils.h"

namespace OHOS::Ace::Platform {
namespace {
constexpr uint32_t DEBUG = 0;
constexpr uint32_t INFO = 1;
constexpr uint32_t WARN = 2;
constexpr uint32_t ERROR = 3;
constexpr uint32_t FATAL = 4;

static const char START_D[] = "d";
static const char START_D_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;)V";
static const char START_I[] = "i";
static const char START_I_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;)V";
static const char START_W[] = "w";
static const char START_W_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;)V";
static const char START_E[] = "e";
static const char START_E_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;)V";
static const char START_F[] = "f";
static const char START_F_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;)V";

} // namespace
std::shared_mutex g_logInterfaceJniLock;
LogInterface LogInterfaceJni::logInterface_;

void LogInterfaceJni::SetLogger(JNIEnv* env, jobject jobjLogger)
{
    CHECK_NULL_VOID(env);
    std::unique_lock<std::shared_mutex> lock(g_logInterfaceJniLock);
    if (jobjLogger != nullptr) {
        logInterface_.logger = env->NewGlobalRef(jobjLogger);
        CHECK_NULL_VOID(logInterface_.logger);
        StartLogProcessingThread();
    } else {
        StopLogProcessingThread();
        if (logInterface_.logger != nullptr) {
            env->DeleteGlobalRef(logInterface_.logger);
            logInterface_.logger = nullptr;
        }
        return;
    }
    jclass cls = env->GetObjectClass(jobjLogger);
    CHECK_NULL_VOID(cls);
    logInterface_.d = env->GetMethodID(cls, START_D, START_D_PARAM);
    CHECK_NULL_VOID(logInterface_.d);
    logInterface_.i = env->GetMethodID(cls, START_I, START_I_PARAM);
    CHECK_NULL_VOID(logInterface_.i);
    logInterface_.w = env->GetMethodID(cls, START_W, START_W_PARAM);
    CHECK_NULL_VOID(logInterface_.w);
    logInterface_.e = env->GetMethodID(cls, START_E, START_E_PARAM);
    CHECK_NULL_VOID(logInterface_.e);
    logInterface_.f = env->GetMethodID(cls, START_F, START_F_PARAM);
    CHECK_NULL_VOID(logInterface_.f);
    env->DeleteLocalRef(cls);

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void LogInterfaceJni::PassLogMessage(const int32_t level, const std::string& domain, const std::string& newFmt)
{
    auto env = Platform::JniEnvironment::GetInstance().GetJniEnv();
    CHECK_NULL_VOID(env);
    std::shared_lock<std::shared_mutex> lock(g_logInterfaceJniLock);
    CHECK_NULL_VOID(logInterface_.logger);
    jstring jDomain = env->NewStringUTF(domain.c_str());
    jstring jNewFmt = env->NewStringUTF(newFmt.c_str());
    switch (level) {
        case DEBUG:
            CHECK_NULL_VOID(logInterface_.d);
            env->CallVoidMethod(logInterface_.logger, logInterface_.d, jDomain, jNewFmt);
            break;
        case INFO:
            CHECK_NULL_VOID(logInterface_.i);
            env->CallVoidMethod(logInterface_.logger, logInterface_.i, jDomain, jNewFmt);
            break;
        case WARN:
            CHECK_NULL_VOID(logInterface_.w);
            env->CallVoidMethod(logInterface_.logger, logInterface_.w, jDomain, jNewFmt);
            break;
        case ERROR:
            CHECK_NULL_VOID(logInterface_.e);
            env->CallVoidMethod(logInterface_.logger, logInterface_.e, jDomain, jNewFmt);
            break;
        case FATAL:
            CHECK_NULL_VOID(logInterface_.f);
            env->CallVoidMethod(logInterface_.logger, logInterface_.f, jDomain, jNewFmt);
            break;
        default:
            break;
    }
    env->DeleteLocalRef(jDomain);
    env->DeleteLocalRef(jNewFmt);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
} // namespace OHOS::Ace::Platform
