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

package ohos.ace.adapter.capability.environment;

/**
 * EnvironmentBase which config to store environment.
 *
 */
public abstract class EnvironmentBase {
    /**
     * get the accessibility is enable or not
     *
     * @return accessibility is enable or not
     */
    public abstract String getAccessibilityEnabled();

    /**
     * native func for Init.
     */
    protected native void nativeInit();
}
