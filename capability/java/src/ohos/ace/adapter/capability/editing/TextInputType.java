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

package ohos.ace.adapter.capability.editing;

/**
 * Implementation of keyboard types to meet various input type.
 *
 * @since 1
 */
public enum TextInputType {
    TEXT(0),

    // Support starting of a new line by pressing Enter.
    MULTILINE(1),
    NUMBER(2),
    PHONE(3),
    DATETIME(4),
    EMAIL_ADDRESS(5),
    URL(6),
    VISIBLE_PASSWORD(7);

    private int value = 0;

    /**
     * get TextInputType according to index
     *
     * @param index index of TextInputType
     * @return the TextInputType according to the index
     */
    public static TextInputType of(Integer index) {
        if (index == null || index < 0 || index >= TextInputType.values().length) {
            return TEXT;
        }
        return TextInputType.values()[index];
    }

    /**
     * get value of TextInputType
     *
     * @return value of TextInputType
     */
    public int getValue() {
        return value;
    }

    TextInputType(int value) {
        this.value = value;
    }
}