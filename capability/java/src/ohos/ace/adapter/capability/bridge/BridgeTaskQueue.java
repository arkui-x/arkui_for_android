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
/**
 * The TaskQueue interface of bridge.
 *
 * @since 11
 */
public interface BridgeTaskQueue {

    /**
     * Get tag of BridgeTaskQueue.
     *
     * @return The tag of BridgeTaskQueue.
     */
    TaskTag getTag();

    /**
     * Dispatch BridgeTask.
     *
     * @param runnable Runnable BridgeTask.
     */
    void dispatch(Runnable runnable);

    /**
     * The tag of BridgeTaskQueue.
     *
     * @since 11
     */
    public enum TaskTag {
        INPUT,
        OUTPUT;
    }
}
