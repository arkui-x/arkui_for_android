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
 * Bridge method data.
 *
 * @since 10
 */
public class MethodData {
    private String methodName_;
    private Object[] parameters_;

    /**
     * Constructor of MethodData.
     *
     * @param methodName Name of method.
     * @param parameters parameters of method.
     * @return MethodData object.
     */
    public MethodData(String methodName, Object[] parameters) {
        this.methodName_ = methodName;
        this.parameters_ = parameters;
    }

    /**
     * Get Saved MethodName.
     *
     * @return Name of method.
     */
    public String getMethodName() {
        return this.methodName_;
    }

    /**
     * Get Saved parameter.
     *
     * @return Parameter of method.
     */
    public Object[] getMethodParameter() {
        return this.parameters_;
    }
}
