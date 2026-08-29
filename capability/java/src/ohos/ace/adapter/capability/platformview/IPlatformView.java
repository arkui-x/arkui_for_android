/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.platformview;

import android.view.View;

/**
 * The class for interface IPlatformView on Android platform.
 *
 * @since 1
 */
public interface IPlatformView {
    /**
     * This is Android view.
     *
     * @return View
     */
    View getView();

    /**
     * This is dispose resource.
     *
     */
    void onDispose();

    /**
     * This is tag of PlatformView.
     *
     * @return String
     */
    String getPlatformViewID();

    /**
     * Called by ArkUI before a pan gesture is accepted, to ask the native view
     * whether it wants to consume the gesture (so ArkUI can let the parent List
     * scroll or not).
     *
     * @param offsetX cumulative pan offset on X axis
     * @param offsetY cumulative pan offset on Y axis
     * @return true if the native view consumes the gesture; false to let the
     *         parent List scroll.
     */
    default boolean onGestureJudge(float offsetX, float offsetY) {
        return true;
    }
}