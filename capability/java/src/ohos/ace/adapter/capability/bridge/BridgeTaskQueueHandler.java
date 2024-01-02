/**
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

package ohos.ace.adapter.capability.bridge;

import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import ohos.ace.adapter.ALog;

/**
 * The TaskQueue handler of bridge.
 *
 * @since 11
 */
public class BridgeTaskQueueHandler implements BridgeTaskQueue {
    private static final String LOG_TAG = "BridgeTaskQueueHandler";

    private final ExecutorService executorService_;

    private ConcurrentLinkedQueue<Runnable> queue_;

    private AtomicBoolean isExecute_;

    private TaskTag taskTag_;

    private TaskOption taskOption_;

    /**
     * Constructor of BridgeTaskQueueHandler.
     *
     * @param executorService context of the application.
     * @param tag name of bridge.
     * @return BridgePlugin object.
     */
    public BridgeTaskQueueHandler(ExecutorService executorService, TaskTag tag, TaskOption taskOption) {
        this.taskTag_ = tag;
        this.taskOption_ = taskOption;
        this.executorService_ = executorService;
        this.isExecute_ = new AtomicBoolean(false);
        this.queue_ = new ConcurrentLinkedQueue<>();
    }

    /**
     * Get tag of BridgeTaskQueueHandler.
     *
     * @return The tag of BridgeTaskQueueHandler.
     */
    @Override
    public TaskTag getTag() {
        return this.taskTag_;
    }

    /**
     * Dispatch BridgeTask.
     *
     * @param runnable Runnable BridgeTask.
     */
    @Override
    public void dispatch(Runnable runnable) {
        if (taskOption_.getTaskOption()) {
            this.queue_.add(runnable);
            this.executorService_.execute(() -> {
                loopRefresh();
            });
        } else {
            executorService_.execute(runnable);
        }
    }

    private void loopRefresh() {
        if (this.isExecute_.compareAndSet(false, true)) {
            try {
                Runnable runnable = this.queue_.poll();
                if (runnable != null) {
                    runnable.run();
                }
            } finally {
                this.isExecute_.set(false);
                if (!this.queue_.isEmpty()) {
                    this.executorService_.execute(() -> {
                        loopRefresh();
                    });
                }
            }
        }
    }

    /**
     * Dump Information of BridgeTaskQueueHandler.
     *
     */
    public void dump() {
        ALog.i(LOG_TAG, "BridgeTaskQueueHandler dump called");
        ALog.i(LOG_TAG, "TaskOption: " + String.valueOf(taskOption_.getTaskOption()));
        ALog.i(LOG_TAG, "Queue size: " + String.valueOf(queue_.size()));
        for (Runnable task : queue_) {
            ALog.i(LOG_TAG, "Task type: " + task.getClass().getSimpleName());
            ALog.i(LOG_TAG, "Task status: " + task.toString());
        }
    }
}
