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
package ohos.ace.adapter;

import android.content.Context;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.accessibility.AccessibilityManager;
import android.view.accessibility.AccessibilityNodeProvider;
import android.view.SurfaceHolder;

import ohos.ace.adapter.WindowView;

public class WindowViewAosp extends WindowView {
    public static String TAG = WindowViewAosp.class.getSimpleName();
    AccessibilityCrossPlatformBridge accessibilityBridge;

    Runnable orientationChanged = () -> {
        if (accessibilityBridge != null && accessibilityBridge.isTouchExplorationEnabled()) {
            accessibilityBridge.sendAccessibilityFocusInvalidate(-1);
        }
    };

    public WindowViewAosp(Context context, int windowId) {
        super(context);
        accessibilityBridge = new AccessibilityCrossPlatformBridge(
                    (AccessibilityManager) context.getSystemService(Context.ACCESSIBILITY_SERVICE),
                    context.getContentResolver(),this, windowId);
    }
    
    public boolean performAccessibilityAction(int action,  Bundle arguments) {
        super.performAccessibilityAction(action, arguments);
        return false;
    }
    
    @Override
    public AccessibilityNodeProvider getAccessibilityNodeProvider() {
        if (super.getWindowOrientation()) {
            return null;
        }

        if (accessibilityBridge != null && accessibilityBridge.isAccessibilityEnabled()) {
            return accessibilityBridge;
        } else {
            return null;
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        super.surfaceChanged(holder, format, width, height);

        if (super.getWindowOrientation()) {
            super.setWindowOrientation(false);
            removeCallbacks(orientationChanged);
            postDelayed(orientationChanged, 100);
        }
    }

    @Override
    public boolean onHoverEvent(MotionEvent event) {
        if (accessibilityBridge != null && accessibilityBridge.isEnabled()) {
            return super.onHoverEventAosp(event);
        }
        return super.onHoverEvent(event);
    }
    
    @Override
    public void destroy() {
        super.destroy();
        destroyAosp();
    }

    public void destroyAosp() {
        if (accessibilityBridge != null) {
            accessibilityBridge.release();
        }
    }
}
