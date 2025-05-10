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

import android.content.Context;

import ohos.ace.adapter.WindowViewAospInterface;
import ohos.ace.adapter.WindowViewInterface;

/**
 * WindowViewBuilder is a class that create an instance of WindowView class.
 *
 * @since 2025-05-06
 */
public class WindowViewBuilder {
    /**
     * Create an instance of WindowViewAospInterface object.
     *
     * @param context Application context
     * @param windowId Application window ID
     * @param useSurfaceView if true create surface view else texture view
     * @return if useSurfaceView is true return WindowViewAospSurface else return WindowViewAospTexture
     */
    public static WindowViewAospInterface makeWindowViewAosp(Context context, int windowId, boolean useSurfaceView) {
        if (useSurfaceView) {
            return new WindowViewAospSurface(context, windowId);
        } else {
            return new WindowViewAospTexture(context, windowId);
        }
    }

    /**
     * Create an instance of WindowViewInterface object.
     *
     * @param context Application context
     * @param useSurfaceView if true create surface view else texture view
     * @return if useSurfaceView is true return WindowViewSurface else return WindowViewTexture
     */
    public static WindowViewInterface makeWindowView(Context context, boolean useSurfaceView) {
        if (useSurfaceView) {
            return new WindowViewSurface(context);
        } else {
            return new WindowViewTexture(context);
        }
    }
}
