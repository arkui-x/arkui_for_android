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

package ohos.ace.adapter.capability.storage;

/**
 * PersistentStorageBase which use perference to store data.
 *
 * @since 1
 */
public abstract class PersistentStorageBase {
    /**
     * Set up date storage base.
     *
     * @param filedb to which file the data can be stored
     */
    public abstract void initializeStorage(String filedb);

    /**
     * store the data in key, value pair into preference
     *
     * @param key key value
     * @param value value value
     */
    public abstract void set(String key, String value);

    /**
     * get the value from the preference
     *
     * @param key key value
     * @return value from the preference
     */
    public abstract String get(String key);

    /**
     * clear all the data in the preference.
     *
     * @param key key value
     */
    public abstract void clear();

    /**
     * delete the data will specific key
     *
     * @param key key value
     */
    public abstract void delete(String key);

    /**
     * native func for Init.
     */
    protected native void nativeInit();
}