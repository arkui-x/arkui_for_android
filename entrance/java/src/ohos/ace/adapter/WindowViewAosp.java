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
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.accessibility.AccessibilityManager;
import android.view.accessibility.AccessibilityNodeProvider;
import android.view.SurfaceHolder;

import ohos.ace.adapter.WindowView;

public class WindowViewAosp extends WindowView {
    public static String TAG = WindowViewAosp.class.getSimpleName();
    AccessibilityCrossPlatformBridge accessibilityBridge;

    boolean isWindowOrientationChanging = false;

    Runnable orientationChanged = ()->{
        if (accessibilityBridge != null && accessibilityBridge.isTouchExplorationEnabled()
            && isWindowOrientationChanging) {
            Log.d(TAG,"surfaceChanged aosp");
            isWindowOrientationChanging = false;
            accessibilityBridge.sendAccessibilityFocusInvalidate(-1);
        }
    };

    public static String ARKUI_ORIENTAION_ACTION = "intent.arkui.ORIENTAION_ACTION";
    BroadcastReceiver orientationReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (ARKUI_ORIENTAION_ACTION.equals(intent.getAction())) {
                isWindowOrientationChanging = true;
                Log.d(TAG, "onReceive : " + ARKUI_ORIENTAION_ACTION);
            }
        }
    };

    public void setWindowOrientationChanging(boolean windowOrientationChanging) {
        isWindowOrientationChanging = windowOrientationChanging;
    }

    public WindowViewAosp(Context context, int windowId) {
        super(context);
        accessibilityBridge = new AccessibilityCrossPlatformBridge(
                    (AccessibilityManager) context.getSystemService(Context.ACCESSIBILITY_SERVICE),
                    context.getContentResolver(),this, windowId);
        IntentFilter filter = new IntentFilter(ARKUI_ORIENTAION_ACTION);
        context.registerReceiver(orientationReceiver,filter);

    }
    public boolean performAccessibilityAction(int action,  Bundle arguments) {
        super.performAccessibilityAction(action, arguments);
        return false;
    }
    
    @Override
    public AccessibilityNodeProvider getAccessibilityNodeProvider() {
        if (accessibilityBridge != null && accessibilityBridge.isAccessibilityEnabled()) {
            return accessibilityBridge;
        } else {
            return null;
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        super.surfaceChanged(holder, format, width, height);
        removeCallbacks(orientationChanged);
        postDelayed(orientationChanged, 100);
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
        try{
            if (orientationReceiver != null) {
                Log.d(TAG, "destroyAosp unregisterReceiver");
                getContext().unregisterReceiver(orientationReceiver);
                orientationReceiver = null;
            }
        } catch (Exception e) {
            Log.e(TAG, "destroyAosp:" + e.toString());
        }
        if (accessibilityBridge != null) {
            accessibilityBridge.release();
        }
    }
}

