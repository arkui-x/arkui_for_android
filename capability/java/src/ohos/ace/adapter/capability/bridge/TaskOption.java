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
 * Task option of Bridge.
 *
 * @since 11
 */
public class TaskOption {

    private boolean isSerial_ = true;

    /**
     * Constructor of TaskOption.
     *
     * @return TaskOption object.
     */
    public TaskOption() {}

    /**
     * Constructor of modifiable TaskOption.
     *
     * @param isSerial option of task.
     * @return TaskOption object.
     */
    public TaskOption(boolean isSerial) {
        this.isSerial_ = isSerial;
    }

    /**
     * get option of task.
     *
     * @return option of task.
     */
    public boolean getTaskOption() {
        return this.isSerial_;
    }
}
