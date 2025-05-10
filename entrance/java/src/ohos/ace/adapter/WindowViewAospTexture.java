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
import android.view.View;
import android.view.MotionEvent;
import android.view.accessibility.AccessibilityNodeProvider;
import android.graphics.SurfaceTexture;

import ohos.ace.adapter.WindowViewTexture;
import ohos.ace.adapter.WindowViewAospInterface;

/**
 * WindowViewAospTexture is a class that implement WindowViewAospInterface interface.
 *
 * @since 2025-05-06
 */
public class WindowViewAospTexture extends WindowViewTexture implements WindowViewAospInterface {
    private WindowViewAospCommon windowViewAospCommon;

    /**
     * Constructor of WindowViewAospTexture
     *
     * @param context Application context
     * @param windowId Application window ID
     */
    public WindowViewAospTexture(Context context, int windowId) {
        super(context);
        windowViewAospCommon = getWindowViewAospCommon(context, windowId, this);
    }

    private WindowViewAospCommon getWindowViewAospCommon(Context context, int windowId, View view) {
        return new WindowViewAospCommon(context, windowId, view);
    }

    /**
     * Get window View.
     *
     * @return window View
     */
    @Override
    public View getView() {
        return this;
    }

    /**
     * Methods for performing auxiliary function operations。
     *
     * @param action Unobstructed operation type
     * @param arguments Parameters for auxiliary function operation
     * @return true if the action was performed, false otherwise
     */
    @Override
    public boolean superPerformAccessibilityAction(int action, Bundle arguments) {
        super.performAccessibilityAction(action, arguments);
        return false;
    }

    /**
     * Methods for performing auxiliary function operations。
     *
     * @param action Unobstructed operation type
     * @param arguments Parameters for auxiliary function operation
     * @return true if the action was performed, false otherwise
     */
    @Override
    public boolean performAccessibilityAction(int action, Bundle arguments) {
        return windowViewAospCommon.performAccessibilityAction(action, arguments);
    }

    /**
     * Accessibility node provided.
     *
     * @return AccessibilityNodeProvider object
     */
    @Override
    public AccessibilityNodeProvider getAccessibilityNodeProvider() {
        if (super.getWindowOrientation()) {
            return null;
        }
        return windowViewAospCommon.getAccessibilityNodeProvider();
    }

    /**
     * Called when the texture size changed.
     *
     * @param surface surface texture
     * @param width the width of surface
     * @param height the height of surface
     */
    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        super.onSurfaceTextureSizeChanged(surface, width, height);
        if (super.getWindowOrientation()) {
            super.setWindowOrientation(false);
            removeCallbacks(windowViewAospCommon.orientationChanged);
            postDelayed(windowViewAospCommon.orientationChanged, 100);
        }
    }

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    @Override
    public boolean superOnHoverEventAosp(MotionEvent event) {
        return super.onHoverEventAosp(event);
    }

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    @Override
    public boolean superOnHoverEvent(MotionEvent event) {
        return super.onHoverEvent(event);
    }

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    @Override
    public boolean onHoverEvent(MotionEvent event) {
        return windowViewAospCommon.onHoverEvent(event);
    }

    /**
     * Notify nativeWindow destroy.
     */
    @Override
    public void destroy() {
        super.destroy();
        windowViewAospCommon.destroyAosp();
    }

    /**
     * Release accessibilityBridge.
     */
    @Override
    public void destroyAosp() {
        windowViewAospCommon.destroyAosp();
    }
}