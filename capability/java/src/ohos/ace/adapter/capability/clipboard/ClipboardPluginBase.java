/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.clipboard;

/**
 * base interface of clipboard plugin
 *
 * @since 1
 */
public abstract class ClipboardPluginBase {
    /**
     * Get data in clipboard
     *
     * @return text in clipboard
     */
    public abstract String getData();

    /**
     * Set data to clipboard
     *
     * @param data data to set
     */
    public abstract void setData(String data);

    /**
     * Check if clipboard has data
     *
     * @return true if clipboard has data
     */
    public abstract boolean hasData();

    /**
     * Clear data in clipboard
     */
    public abstract void clear();

    protected native void nativeInit();
}
