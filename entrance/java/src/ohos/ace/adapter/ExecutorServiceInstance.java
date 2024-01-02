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

package ohos.ace.adapter;

import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;

/**
 * Single instance object of ExecutorService.
 *
 * @since 11
 */
public class ExecutorServiceInstance {
    private static final Object INSTANCE_LOCK = new Object();

    private static volatile ExecutorServiceInstance INSTANCE = null;

    private static ExecutorService executor = null;

    private ExecutorServiceInstance() {
        executor = Executors.newWorkStealingPool();
    }

    /**
     * Construct a single instance object of ExecutorService.
     *
     * @return ExecutorServiceInstance object.
     */
    public static ExecutorServiceInstance getInstance() {
        if (INSTANCE != null) {
            return INSTANCE;
        }
        synchronized (INSTANCE_LOCK) {
            if (INSTANCE != null) {
                return INSTANCE;
            }
            INSTANCE = new ExecutorServiceInstance();
            return INSTANCE;
        }
    }

    /**
     * get object of ExecutorService.
     *
     * @return object of ExecutorService.
     */
    public static ExecutorService getExecutorService() {
        if (executor == null) {
            getInstance();
        }
        return executor;
    }
}