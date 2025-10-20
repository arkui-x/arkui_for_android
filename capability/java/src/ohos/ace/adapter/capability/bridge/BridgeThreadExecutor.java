/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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

package ohos.ace.adapter.capability.bridge;

import android.os.Handler;
import android.os.Looper;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.RejectedExecutionHandler;

import ohos.ace.adapter.ALog;

/**
 * Bridge thread executor.
 *
 * @since 22
 */
public class BridgeThreadExecutor {
    private static final String LOG_TAG = "BridgeThreadExecutor";

    private static final int CPU_COUNT = Runtime.getRuntime().availableProcessors();

    private static final int CORE_POOL_SIZE = Math.max(1, CPU_COUNT);

    private static final int MAX_POOL_SIZE = Math.max(CORE_POOL_SIZE, CPU_COUNT * 2);

    private static final long KEEP_ALIVE_TIME = 60_000L;

    private static final int QUEUE_CAPACITY = 256;

    private static final RejectedExecutionHandler REJECTED_HANDLER = (r, executor) -> {
        if (!executor.isShutdown()) {
            r.run();
        }
    };

    private static final Object INSTANCE_LOCK = new Object();

    private static volatile BridgeThreadExecutor instance = null;

    private final ExecutorService executor;

    private final Handler mainHandler = new Handler(Looper.getMainLooper());

    private BridgeThreadExecutor() {
        this.executor = new ThreadPoolExecutor(
            CORE_POOL_SIZE,
            MAX_POOL_SIZE,
            KEEP_ALIVE_TIME,
            TimeUnit.MILLISECONDS,
            new LinkedBlockingQueue<>(QUEUE_CAPACITY),
            r -> {
                Thread thread = new Thread(r, "BridgeThread");
                thread.setDaemon(false);
                return thread;
            },
            REJECTED_HANDLER
        );
    }

    /**
     * Get singleton instance of BridgeThreadExecutor.
     *
     * @return BridgeThreadExecutor singleton instance.
     */
    public static BridgeThreadExecutor getInstance() {
        if (instance != null) {
            return instance;
        }

        synchronized (INSTANCE_LOCK) {
            if (instance == null) {
                instance = new BridgeThreadExecutor();
            }
            return instance;
        }
    }

    /**
     * The task is executed on the bridge thread.
     *
     * @param task Tasks to be executed.
     */
    public void execute(Runnable task) {
        if (task == null) {
            ALog.e(LOG_TAG, "Task is null, BridgeThread execute exits.");
            return;
        }
        executor.execute(task);
    }

    /**
     * Determine whether the current thread is the main thread.
     *
     * @return True is in the main thread, while false is not.
     */
    public boolean isOnMainThread() {
        return Looper.myLooper() == Looper.getMainLooper();
    }

    /**
     * The task is executed on the main thread.
     *
     * @param task Tasks to be executed.
     */
    public void postToMainThreadSync(Runnable task) {
        if (task == null) {
            ALog.e(LOG_TAG, "Task is null, postToMainThreadSync exits.");
            return;
        }
        if (isOnMainThread()) {
            task.run();
        } else {
            mainHandler.post(task);
        }
    }
}