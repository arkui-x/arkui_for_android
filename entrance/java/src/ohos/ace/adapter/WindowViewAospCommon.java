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
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.accessibility.AccessibilityManager;
import android.view.accessibility.AccessibilityNodeProvider;
import android.view.View;

import ohos.ace.adapter.WindowViewAospInterface;

/**
 * WindowViewAospCommon is a class that implement WindowViewAosp common interface.
 *
 * @since 2025-05-06
 */
public class WindowViewAospCommon {
    private static final String TAG = "WindowViewAospCommon";

    private int windowId;

    private Context context;

    private AccessibilityCrossPlatformBridge accessibilityBridge;

    private WindowViewAospInterface windowViewAospInterface;

    Runnable orientationChanged = () -> {
        accessibilityCrossPlatformBridgeInit();
        if (accessibilityBridge != null && accessibilityBridge.isTouchExplorationEnabled()) {
            accessibilityBridge.sendAccessibilityFocusInvalidate(
                AccessibilityCrossPlatformBridge.INVALID_VIRTUAL_VIEW_ID);
        }
    };

    /**
     * Constructor of WindowViewAospCommon
     *
     * @param context Application context
     * @param windowId Application window ID
     * @param view The window view
     */
    public WindowViewAospCommon(Context context, int windowId, View view) {
        this.windowId = windowId;
        this.context = context;
        if (view instanceof WindowViewAospInterface) {
            this.windowViewAospInterface = (WindowViewAospInterface) view;
        } else {
            this.windowViewAospInterface = null;
        }
        accessibilityCrossPlatformBridgeInit();
    }

    private void accessibilityCrossPlatformBridgeInit() {
        if (accessibilityBridge == null && context != null && windowViewAospInterface instanceof View) {
            accessibilityBridge = new AccessibilityCrossPlatformBridge(
            (AccessibilityManager) context.getSystemService(Context.ACCESSIBILITY_SERVICE),
            context.getContentResolver(), (View) windowViewAospInterface, windowId);
        }
    }

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    public boolean onHoverEvent(MotionEvent event) {
        if (windowViewAospInterface == null || event == null) {
            return false;
        }
        accessibilityCrossPlatformBridgeInit();
        if (accessibilityBridge != null && accessibilityBridge.isEnabled()) {
            return windowViewAospInterface.superOnHoverEventAosp(event);
        }
        return windowViewAospInterface.superOnHoverEvent(event);
    }

    /**
     * Accessibility node provided.
     *
     * @return AccessibilityNodeProvider object
     */
    public AccessibilityNodeProvider getAccessibilityNodeProvider() {
        accessibilityCrossPlatformBridgeInit();
        if (accessibilityBridge != null && accessibilityBridge.isAccessibilityEnabled()) {
            return accessibilityBridge;
        }
        return null;
    }

    /**
     * Methods for performing auxiliary function operations。
     *
     * @param action Unobstructed operation type
     * @param arguments Parameters for auxiliary function operation
     * @return true if the action was performed, false otherwise
     */
    public boolean performAccessibilityAction(int action, Bundle arguments) {
        if (windowViewAospInterface == null) {
            return false;
        }
        return windowViewAospInterface.superPerformAccessibilityAction(action, arguments);
    }

    /**
     * Release accessibilityBridge.
     */
    public void destroyAosp() {
        if (accessibilityBridge != null) {
            accessibilityBridge.release();
        }
    }
}