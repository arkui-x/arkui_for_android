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
 * Bridge method result listener.
 *
 * @since 10
 */
public interface IMethodResult {
    /**
     * Call other platform method successfully and return trigger.
     *
     * @param resultValue the instance id of current view.
     */
    void onSuccess(Object resultValue);

    /**
     * Call other platform method failed and return trigger.
     *
     * @param methodName The method name.
     * @param errorCode The error id.
     * @param errorMessage The error message.
     */
    void onError(String methodName, int errorCode, String errorMessage);

    /**
     * Call other platform method successfully and return trigger.
     *
     * @param methodName The method name.
     */
    void onMethodCancel(String methodName);
}
