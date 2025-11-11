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

package ohos.ace.adapter;

import android.graphics.Rect;

/**
 * The avoid Area class.
 *
 * @since 2025-10-29
 */
public class AvoidArea {

    /**
     * The left avoid Area.
     */
    public final Rect leftRect;

    /**
     * The top avoid Area.
     */
    public final Rect topRect;

    /**
     * The right avoid Area.
     */
    public final Rect rightRect;

    /**
     * The bottom avoid Area.
     */
    public final Rect bottomRect;

    /**
     * AvoidArea constructor with specific rectangles.
     *
     * @param topRect the top avoid area rectangle
     * @param rightRect the right avoid area rectangle
     * @param bottomRect the bottom avoid area rectangle
     * @param leftRect the left avoid area rectangle
     */
    public AvoidArea(Rect leftRect, Rect topRect, Rect rightRect, Rect bottomRect) {
        this.topRect = topRect != null ? topRect : new Rect();
        this.rightRect = rightRect != null ? rightRect : new Rect();
        this.bottomRect = bottomRect != null ? bottomRect : new Rect();
        this.leftRect = leftRect != null ? leftRect : new Rect();
    }

    /**
     * Check if this AvoidArea is equal to another AvoidArea.
     *
     * @param rect the other AvoidArea to compare with
     * @return true if all four rectangles (top, right, bottom, left) are equal, false otherwise
     */
    public boolean isEqualTo(AvoidArea rect) {
        if (rect == null) {
            return false;
        }
        return topRect.equals(rect.topRect) && rightRect.equals(rect.rightRect) &&
                bottomRect.equals(rect.bottomRect) && leftRect.equals(rect.leftRect);
    }
}