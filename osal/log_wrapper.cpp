/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "base/log/log_wrapper.h"

#include <android/log.h>

#ifdef _GNU_SOURCE
#include <dlfcn.h>
#endif

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include "log_interface_jni.h"
#include "securec.h"
#include "stage_application_delegate_jni.h"

#ifdef ACE_INSTANCE_LOG
#include "core/common/container.h"
#endif

namespace OHOS::Ace {
namespace {

constexpr uint32_t MAX_BUFFER_SIZE = 4000;
std::atomic<bool> g_logThreadRunning(false);
std::thread g_logThread;
std::queue<std::function<void()>> g_logTaskQueue;
std::mutex g_logTaskQueueMutex;
std::condition_variable g_logTaskQueueCondVar;
constexpr int LOG_LEVEL[] = { ANDROID_LOG_DEBUG, ANDROID_LOG_INFO, ANDROID_LOG_WARN, ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL };

const char* LOG_TAGS[] = {
    "Ace",
    "JSApp",
};

#ifdef ACE_INSTANCE_LOG
constexpr const char* INSTANCE_ID_GEN_REASONS[] = {
    "scope",
    "active",
    "default",
    "singleton",
    "foreground",
    "undefined",
};
#endif

} // namespace

// Initialize the static member object
LogLevel LogWrapper::level_ = LogLevel::DEBUG;

char LogWrapper::GetSeparatorCharacter()
{
    return '/';
}

void LogProcessingThread()
{
    while (g_logThreadRunning) {
        std::function<void()> task;
        std::unique_lock<std::mutex> lock(g_logTaskQueueMutex);
        g_logTaskQueueCondVar.wait(lock, [] { return !g_logTaskQueue.empty() || !g_logThreadRunning; });
        if (!g_logTaskQueue.empty()) {
            task = std::move(g_logTaskQueue.front());
            g_logTaskQueue.pop();
        }
        lock.unlock();
        if (task) {
            task();
        }
    }
}

void Platform::StartLogProcessingThread()
{
    if (!g_logThreadRunning) {
        g_logThreadRunning = true;
        g_logThread = std::thread(LogProcessingThread);
        if (g_logThread.joinable()) {
            pthread_setname_np(g_logThread.native_handle(), "LoggerThread");
        } else {
            g_logThreadRunning = false;
            __android_log_print(ANDROID_LOG_ERROR, "LogThread", "Failed to start log thread");
        }
    }
}

void Platform::StopLogProcessingThread()
{
    if (g_logThreadRunning) {
        g_logThreadRunning = false;
        g_logTaskQueueCondVar.notify_one();
        if (g_logThread.joinable()) {
            g_logThread.join();
        }
        std::lock_guard<std::mutex> lock(g_logTaskQueueMutex);
        std::queue<std::function<void()>> emptyQueue;
        std::swap(g_logTaskQueue, emptyQueue);
    }
}

void PassLogMessage(LogDomain domain, LogLevel level, std::string newFmt, va_list args)
{
    char buf[MAX_BUFFER_SIZE];
    if (vsnprintf_s(buf, sizeof(buf), sizeof(buf) - 1, newFmt.c_str(), args) < 0 && errno == EINVAL) {
        return;
    }
    std::string str(buf);
    std::unique_lock<std::mutex> lock(g_logTaskQueueMutex);
    g_logTaskQueue.emplace([=]() {
        OHOS::Ace::Platform::LogInterfaceJni::PassLogMessage(
            static_cast<int32_t>(level), std::string(LOG_TAGS[static_cast<uint32_t>(domain)]), str);
    });
    g_logTaskQueueCondVar.notify_one();
    lock.unlock();
}

void LogWrapper::PrintLog(LogDomain domain, LogLevel level, AceLogTag tag, const char* fmt, va_list args)
{
    if (!OHOS::Ace::LogWrapper::JudgeLevel(level)) {
        return;
    }
    std::string newFmt(fmt);
    StripFormatString("{public}", newFmt);
    StripFormatString("{private}", newFmt);
    
    bool uselogInterface_ = false;
    {
        std::shared_lock<std::shared_mutex> lock(OHOS::Ace::Platform::g_logInterfaceJniLock);
        uselogInterface_ = OHOS::Ace::Platform::LogInterfaceJni::logInterface_.logger &&
                           level >= OHOS::AbilityRuntime::Platform::StageApplicationDelegateJni::GetCurrentLogLevel();
    }
    
    if (uselogInterface_) {
        PassLogMessage(domain, level, newFmt, args);
    } else {
        __android_log_vprint(
            LOG_LEVEL[static_cast<int>(level)], LOG_TAGS[static_cast<uint32_t>(domain)], newFmt.c_str(), args);
    }
}

#ifdef ACE_INSTANCE_LOG
int32_t LogWrapper::GetId()
{
    return Container::CurrentId();
}

const std::string LogWrapper::GetIdWithReason()
{
    int32_t currentId = ContainerScope::CurrentId();
    std::pair<int32_t, InstanceIdGenReason> idWithReason = ContainerScope::CurrentIdWithReason();
    return std::to_string(currentId) + ":" + std::to_string(idWithReason.first) + ":" +
           INSTANCE_ID_GEN_REASONS[static_cast<uint32_t>(idWithReason.second)];
}
#endif

bool LogBacktrace(size_t maxFrameNums)
{
    static const char* (*pfnGetTrace)(size_t, size_t);
#ifdef _GNU_SOURCE
    if (!pfnGetTrace) {
        pfnGetTrace = (decltype(pfnGetTrace))dlsym(RTLD_DEFAULT, "GetTrace");
    }
#endif
    if (!pfnGetTrace) {
        return false;
    }

    static std::mutex mtx;
    std::lock_guard lock(mtx);
    size_t skipFrameNum = 2;
    LOGI("Backtrace: skipFrameNum=%{public}zu maxFrameNums=%{public}zu\n%{public}s",
        skipFrameNum, maxFrameNums, pfnGetTrace(skipFrameNum, maxFrameNums));
    return true;
}

} // namespace OHOS::Ace