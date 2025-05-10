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

import android.os.Bundle;
import android.view.MotionEvent;
import android.view.accessibility.AccessibilityNodeProvider;

import ohos.ace.adapter.WindowViewInterface;

/**
 * WindowViewAospInterface is a class that extends WindowViewInterface.
 *
 * @since 2025-05-06
 */
public interface WindowViewAospInterface extends WindowViewInterface {
    /**
     * Notify nativeWindow destroy.
     */
    void destroy();

    /**
     * Release accessibilityBridge.
     */
    void destroyAosp();

    /**
     * Accessibility node provided.
     *
     * @return AccessibilityNodeProvider object
     */
    AccessibilityNodeProvider getAccessibilityNodeProvider();

    /**
     * Methods for performing auxiliary function operations。
     *
     * @param action Unobstructed operation type
     * @param arguments Parameters for auxiliary function operation
     * @return true if the action was performed, false otherwise
     */
    boolean performAccessibilityAction(int action, Bundle arguments);

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    boolean superOnHoverEvent(MotionEvent event);

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    boolean superOnHoverEventAosp(MotionEvent event);

    /**
     * Methods for performing auxiliary function operations。
     *
     * @param action Unobstructed operation type
     * @param arguments Parameters for auxiliary function operation
     * @return true if the action was performed, false otherwise
     */
    boolean superPerformAccessibilityAction(int action, Bundle arguments);
}