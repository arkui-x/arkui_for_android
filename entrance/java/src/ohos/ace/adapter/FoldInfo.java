/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

/**
 * fold info of the device
 *
 * @since 2025-05-22
 */
public class FoldInfo {

    /**
     * fold status is unknown
     */
    public static final int FOLD_STATUS_UNKNOWN = 0;

    /**
     * fold status is expanded
     */
    public static final int FOLD_STATUS_EXPANDED = 1;

    /**
     * fold status is folded
     */
    public static final int FOLD_STATUS_FOLDED = 2; // fold status folded

    /**
     * fold status is half folded
     */
    public static final int FOLD_STATUS_HALF_FOLDED = 3;

    /**
     * is foldable
     */
    private boolean isFoldable = false;

    /**
     * fold status
     */
    private int foldStatus = FOLD_STATUS_UNKNOWN;

    /**
     * get foldable of the device
     *
     * @return foldable
     */
    public boolean isFoldable() {
        return isFoldable;
    }

    /**
     * set foldable of the device
     *
     * @param foldable is foldable
     */
    public void setFoldable(boolean foldable) {
        isFoldable = foldable;
    }

    /**
     * get fold status of the device
     *
     * @return foldStatus  fold status
     */
    public int getFoldStatus() {
        return foldStatus;
    }

    /**
     * get fold status of the device
     *
     * @param foldStatus  fold status
     */
    public void setFoldStatus(int foldStatus) {
        this.foldStatus = foldStatus;
    }
}
