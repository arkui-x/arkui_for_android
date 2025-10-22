/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.web;

/**
 * AceWebNestedScrollOptionsExtObject class is used to store the nested scroll options information.
 *
 * @since 22
 */
public class AceWebNestedScrollOptionsExtObject {
    private NestedScrollMode scrollUp;
    private NestedScrollMode scrollDown;
    private NestedScrollMode scrollLeft;
    private NestedScrollMode scrollRight;

    public AceWebNestedScrollOptionsExtObject() {}

    /**
     * This is called to set nested scroll options.
     *
     * @param scrollUp An integer representing the scroll up option.
     * @param scrollDown An integer representing the scroll down option.
     * @param scrollLeft An integer representing the scroll left option.
     * @param scrollRight An integer representing the scroll right option.
     */
    public void set(int scrollUp, int scrollDown, int scrollLeft, int scrollRight) {
        this.scrollUp = fromInt(scrollUp);
        this.scrollDown = fromInt(scrollDown);
        this.scrollLeft = fromInt(scrollLeft);
        this.scrollRight = fromInt(scrollRight);
    }

    /**
     * This is called to judge if need parallel scroll.
     *
     * @param touchDeltaX An float representing the delta x.
     * @param touchDeltaY An float representing the delta y.
     * @return An boolean representing if need parallel scroll.
     */
    public boolean needParallelScroll(float touchDeltaX, float touchDeltaY) {
        if (Float.compare(Math.abs(touchDeltaX), 0.0f) == 0 &&
            Float.compare(Math.abs(touchDeltaY), 0.0f) == 0) {
                return false;
        }

        if (Math.abs(touchDeltaX) > Math.abs(touchDeltaY)) {
            if (Float.compare(touchDeltaX, 0.0f) < 0) {
                if (scrollRight == NestedScrollMode.PARALLEL) {
                    return true;
                }
            } else {
                if (scrollLeft == NestedScrollMode.PARALLEL) {
                    return true;
                }
            }
        } else {
            if (Float.compare(touchDeltaY, 0.0f) < 0) {
                if (scrollDown == NestedScrollMode.PARALLEL) {
                    return true;
                }
            } else {
                if (scrollUp == NestedScrollMode.PARALLEL) {
                    return true;
                }
            }
        }
        return false;
    }

    private NestedScrollMode fromInt(int value) {
        switch (value) {
            case 0: return NestedScrollMode.SELF_ONLY;
            case 1: return NestedScrollMode.SELF_FIRST;
            case 2: return NestedScrollMode.PARENT_FIRST;
            case 3: return NestedScrollMode.PARALLEL;
            default: return NestedScrollMode.SELF_ONLY;
        }
    }

    private enum NestedScrollMode {
        SELF_ONLY,
        SELF_FIRST,
        PARENT_FIRST,
        PARALLEL
    }
}