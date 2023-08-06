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
 * See C++ TextInputAction
 *
 * @since 1
 */
public enum TextInputAction {
    UNSPECIFIED(0),
    NONE(1),
    GO(2),
    SEARCH(3),
    SEND(4),
    NEXT(5),
    DONE(6),
    PREVIOUS(7);

    private int value = 0;

    /**
     * constructor of TextInputAction
     *
     * @param value action value
     */
    TextInputAction(int value) {
        this.value = value;
    }

    /**
     * Get text input action according to index.
     *
     * @param index index of TextInputAction
     * @return the TextInputAction according to the index
     */
    public static TextInputAction of(Integer index) {
        if (index == null || index < 0 || index >= TextInputAction.values().length) {
            return UNSPECIFIED;
        }
        return TextInputAction.values()[index];
    }

    /**
     * Get value of TextInputAction.
     *
     * @return value of TextInputAction
     */
    public int getValue() {
        return value;
    }
}