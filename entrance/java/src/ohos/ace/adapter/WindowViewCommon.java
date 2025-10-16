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

import android.app.Activity;
import android.content.Context;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;

import java.nio.ByteBuffer;
import java.util.Map;

import ohos.ace.adapter.capability.platformview.AcePlatformViewPluginBase;
import ohos.ace.adapter.capability.platformview.AcePlatformViewBase;
import ohos.ace.adapter.capability.web.AceWebPluginBase;
import ohos.ace.adapter.capability.web.AceWebBase;
import ohos.ace.adapter.WindowViewInterface;

/**
 * WindowViewCommon is a class that implement WindowView common interface.
 *
 * @since 2025-05-06
 */
public class WindowViewCommon {
    private static final String TAG = "WindowViewCommon";

    private long nativeWindowPtr = 0L;
    private int surfaceWidth = 0;
    private int surfaceHeight = 0;
    private int lastMouseButtonState = 0;
    private int lastMouseActionKey = 0;
    private float density = 0f;
    private float lastMouseX = 0f;
    private float lastMouseY = 0f;
    private boolean delayNotifySurfaceChanged = false;
    private boolean delayNotifySurfaceDestroyed = false;
    private boolean enterPressed = false;
    private boolean numpadEnterPressed = false;
    private boolean isWindowOrientationChanging = false;
    private boolean keyboardVisible = false;
    private AcePlatformViewPluginBase acePlatformViewPluginBase;
    private AceWebPluginBase aceWebPluginBase;
    private InputConnectionClient inputClient = null;
    private Surface delayNotifyCreateSurface = null;
    private WindowViewInterface windowViewInterface = null;

    /**
     * Constructor of WindowViewCommon
     *
     * @param windowViewInterface WindowViewInterface object
     */
    public WindowViewCommon(WindowViewInterface windowViewInterface) {
        this.windowViewInterface = windowViewInterface;
    }

    /**
     * Set the input connection client.
     *
     * @param inputConnectionClient the input connection client
     */
    public void setInputConnectionClient(InputConnectionClient inputConnectionClient) {
        inputClient = inputConnectionClient;
    }

    /**
     * Called by native to register Window Handle.
     *
     * @param windowHandle the handle of navive window
     */
    public void registerWindow(long windowHandle) {
        nativeWindowPtr = windowHandle;
        delayNotifyIfNeeded();
    }

    /**
     * Called by native to unregister Window Handle.
     */
    public void unRegisterWindow() {
        nativeWindowPtr = 0L;
    }

    /**
     * Set web plugin.
     *
     * @param pluginBase the AceWebPluginBase object
     */
    public void setWebPlugin(AceWebPluginBase pluginBase) {
        aceWebPluginBase = pluginBase;
    }

    /**
     * Set platform view plugin.
     *
     * @param pluginBase the AcePlatformViewPluginBase object
     */
    public void setPlatformViewPlugin(AcePlatformViewPluginBase pluginBase) {
        acePlatformViewPluginBase = pluginBase;
    }

    /**
     * Create the input connection.
     *
     * @param outAttrs editor info
     * @return the input connection
     */
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        if (inputClient != null) {
            if (windowViewInterface instanceof WindowViewSurface) {
                return inputClient.onCreateInputConnection((WindowViewSurface) windowViewInterface, outAttrs);
            } else if (windowViewInterface instanceof WindowViewTexture) {
                return inputClient.onCreateInputConnection((WindowViewTexture) windowViewInterface, outAttrs);
            } else {
                return null;
            }
        }
        ALog.i(TAG, "onCreateInputConnection");
        return windowViewInterface.superOnCreateInputConnection(outAttrs);
    }

    /**
     * Called when the surface  created.
     *
     * @param surface surface object
     */
    public void surfaceCreated(Surface surface) {
        if (nativeWindowPtr == 0L) {
            ALog.w(TAG, "surfaceCreated nativeWindow not ready, delay notify");
            delayNotifyCreateSurface = surface;
        } else {
            ALog.i(TAG, "surfaceCreated");
            nativeSurfaceCreated(nativeWindowPtr, surface);
        }
    }

    /**
     * Called when the surface size changed.
     *
     * @param width the width of surface
     * @param height the height of surface
     * @param density the density of surface
     */
    public void surfaceSizeChanged(int width, int height, float density) {
        this.surfaceWidth = width;
        this.surfaceHeight = height;
        this.density = density;
        if (this.nativeWindowPtr == 0L) {
            ALog.w(TAG, "surfaceChanged nativeWindow not ready, delay notify");
            this.delayNotifySurfaceChanged = true;
        } else {
            ALog.i(TAG, "surfaceChanged w=" + this.surfaceWidth + " h=" + this.surfaceHeight);
            this.nativeSurfaceChanged(
                this.nativeWindowPtr, this.surfaceWidth, this.surfaceHeight, this.density);
        }
    }

    /**
     * Called when the windowfocus changed.
     *
     * @param holder surface holder
     */
    public void surfaceDestroyed() {
        if (this.nativeWindowPtr == 0L) {
            ALog.w(TAG, "surfaceDestroyed nativeWindow not ready, delay notify");
            delayNotifySurfaceDestroyed = true;
        } else {
            ALog.i(TAG, "surfaceDestroyed");
            nativeSurfaceDestroyed(this.nativeWindowPtr);
        }
    }

    /**
     * To notify keyboard height changed.
     *
     * @param height the height of keyboard
     */
    public void keyboardHeightChanged(int height) {
        ALog.i(TAG, "keyboardHeightChanged call");
        keyboardVisible = height > 0;
        if (nativeWindowPtr == 0L) {
            ALog.w(TAG, "keyboardHeightChanged nativeWindow not ready.");
        } else {
            nativeKeyboardHeightChanged(nativeWindowPtr, height);
        }
    }

    /**
     * To notify avoid area changed.
     */
    public void avoidAreaChanged() {
        ALog.i(TAG, "avoidAreaChanged call");
        if (nativeWindowPtr == 0L) {
            ALog.w(TAG, "avoidAreaChanged nativeWindow not ready.");
        } else {
            nativeAvoidAreaChanged(nativeWindowPtr);
        }
    }

    /**
     * Called when the windowfocus changed.
     *
     * @param hasWindowFocus has focus or not
     */
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        if (nativeWindowPtr == 0L) {
            ALog.w(TAG, "onWindowFocusChanged: nativeWindow is null");
            return;
        }
        ALog.i(TAG, "onWindowFocusChanged call");
        nativeOnWindowFocusChanged(nativeWindowPtr, hasWindowFocus);
    }

    /**
     * Notify nativeWindow foreground.
     */
    public void foreground() {
        if (nativeWindowPtr == 0L) {
            ALog.w(TAG, "foreground: nativeWindow is null");
            return;
        }
        ALog.i(TAG, "foreground call");
        nativeForeground(nativeWindowPtr);
    }

    /**
     * Notify nativeWindow background.
     */
    public void background() {
        if (nativeWindowPtr == 0L) {
            ALog.w(TAG, "background: nativeWindow is null");
            return;
        }
        ALog.i(TAG, "background call");
        nativeBackground(nativeWindowPtr);
    }

    /**
     * Notify nativeWindow destroy.
     */
    public void destroy() {
        if (nativeWindowPtr == 0L) {
            ALog.w(TAG, "destroy: nativeWindow is null");
            return;
        }
        ALog.i(TAG, "destroy call");
        nativeDestroy(nativeWindowPtr);
    }

    /**
     * Notify nativeWindow backPressed.
     *
     * @return true if process BackPressed, false otherwise.
     */
    public boolean backPressed() {
        if (nativeWindowPtr == 0L) {
            ALog.w(TAG, "backPressed: nativeWindow is null");
            return false;
        }
        ALog.i(TAG, "backPressed call");
        return nativeBackPressed(nativeWindowPtr);
    }

    /**
     * Set window orientation.
     *
     * @param isOrientation is window orientation or not.
     */
    public void setWindowOrientation(boolean isOrientation) {
        isWindowOrientationChanging = isOrientation;
    }

    /**
     * Get window orientation.
     *
     * @return window orientation or not.
     */
    public boolean getWindowOrientation() {
        return isWindowOrientationChanging;
    }

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    public boolean onHoverEventAosp(MotionEvent event) {
        if (event == null) {
            return false;
        }
        if (nativeWindowPtr == 0L) {
            return windowViewInterface.superOnTouchEvent(event);
        }
        this.setWebTouchEvent(event);
        this.setPlatformViewTouchEvent(event);

        try {
            int source = event.getSource();
            if (source == InputDevice.SOURCE_MOUSE) {
                lastMouseX = event.getX(event.getActionIndex());
                lastMouseY = event.getY(event.getActionIndex());
                int actionMasked = event.getActionMasked();
                int buttonState = event.getButtonState();

                int actionKey = getActionKey(actionMasked, buttonState);
                ByteBuffer mousePacket = AceEventProcessorAosp.processMouseEvent(event, actionKey, lastMouseX,
                        lastMouseY);
                nativeDispatchMouseDataPacket(nativeWindowPtr, mousePacket, mousePacket.position());
            }
            ByteBuffer packet = AceEventProcessorAosp.processHoverTouchEvent(event);
            nativeDispatchPointerDataPacket(nativeWindowPtr, packet, packet.position());
            return true;
        } catch (AssertionError error) {
            ALog.e(TAG, "process touch event failed: " + error.getMessage());
            return false;
        }
    }

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    public boolean onHoverEvent(MotionEvent event) {
        if (event == null) {
            return false;
        }
        if (nativeWindowPtr == 0L) {
            return windowViewInterface.superOnHoverEvent(event);
        }
        try {
            lastMouseX = event.getX(event.getActionIndex());
            lastMouseY = event.getY(event.getActionIndex());

            int actionMasked = event.getActionMasked();
            int buttonState = event.getButtonState();

            int actionKey = getActionKey(actionMasked, buttonState);
            ByteBuffer packet = AceEventProcessorAosp.processMouseEvent(event, actionKey, lastMouseX, lastMouseY);
            nativeDispatchMouseDataPacket(nativeWindowPtr, packet, packet.position());
            return true;
        } catch (AssertionError error) {
            ALog.e(TAG, "process hover event failed: " + error.getMessage());
            return false;
        }
    }

    /**
     * Touch action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if touch action run success , false otherwise.
     */
    public boolean onTouchEvent(MotionEvent event) {
        if (event == null) {
            return false;
        }
        if (nativeWindowPtr == 0L) {
            return windowViewInterface.superOnTouchEvent(event);
        }
        this.setWebTouchEvent(event);
        this.setPlatformViewTouchEvent(event);

        try {
            int source = event.getSource();
            if (source == InputDevice.SOURCE_MOUSE) {
                lastMouseX = event.getX(event.getActionIndex());
                lastMouseY = event.getY(event.getActionIndex());
                int actionMasked = event.getActionMasked();
                int buttonState = event.getButtonState();

                int actionKey = getActionKey(actionMasked, buttonState);
                ByteBuffer mousePacket = AceEventProcessorAosp.processMouseEvent(event, actionKey, lastMouseX,
                        lastMouseY);
                nativeDispatchMouseDataPacket(nativeWindowPtr, mousePacket, mousePacket.position());
            }
            ByteBuffer packet = AceEventProcessorAosp.processTouchEvent(event);
            nativeDispatchPointerDataPacket(nativeWindowPtr, packet, packet.position());
            return true;
        } catch (AssertionError error) {
            ALog.e(TAG, "process touch event failed: " + error.getMessage());
            return false;
        }
    }

    /**
     * The action of press down key.
     *
     * @param keyCode key code.
     * @param event Action and button state.
     * @return true if press down action run success , false otherwise.
     */
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (event == null) {
            return false;
        }
        if (this.nativeWindowPtr == 0L) {
            return windowViewInterface.superOnKeyDown(keyCode, event);
        }

        if (filterNumpadKeyCode(keyCode, event.getAction())) {
            return windowViewInterface.superOnKeyDown(keyCode, event);
        }

        int eventSouce = event.getSource();

        if (eventSouce == InputDevice.SOURCE_MOUSE) {
            if (keyCode == KeyEvent.KEYCODE_FORWARD || keyCode == KeyEvent.KEYCODE_BACK) {
                ByteBuffer packet = AceEventProcessorAosp.processMouseEvent(event, lastMouseX, lastMouseY);
                nativeDispatchMouseDataPacket(this.nativeWindowPtr, packet, packet.position());
                return true;
            }
        }
        markdownNumpadKeyStatus(keyCode, event.getAction());
        int modifierKeys = AceEventProcessorAosp.getModifierKeys(event);
        int source = AceEventProcessorAosp.eventSourceTransKeySource(eventSouce);
        int deviceId = event.getDeviceId();
        if (nativeDispatchKeyEvent(this.nativeWindowPtr, event.getKeyCode(), event.getAction(), event.getRepeatCount(),
                event.getEventTime(), event.getDownTime(), source, deviceId, modifierKeys)) {
            return true;
        }
        return windowViewInterface.superOnKeyDown(keyCode, event);
    }

    /**
     * The action of press up key.
     *
     * @param keyCode key code.
     * @param event Action and button state.
     * @return true if press up action run success , false otherwise.
     */
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (event == null) {
            return false;
        }
        if (this.nativeWindowPtr == 0L) {
            return windowViewInterface.superOnKeyUp(keyCode, event);
        }
        if (filterNumpadKeyCode(keyCode, event.getAction())) {
            return windowViewInterface.superOnKeyUp(keyCode, event);
        }

        int eventSouce = event.getSource();
        if (eventSouce == InputDevice.SOURCE_MOUSE) {
            if (keyCode == KeyEvent.KEYCODE_FORWARD || keyCode == KeyEvent.KEYCODE_BACK) {
                ByteBuffer packet = AceEventProcessorAosp.processMouseEvent(event, lastMouseX, lastMouseY);
                nativeDispatchMouseDataPacket(this.nativeWindowPtr, packet, packet.position());
                return true;
            }
        }
        markdownNumpadKeyStatus(keyCode, event.getAction());
        int modifierKeys = AceEventProcessorAosp.getModifierKeys(event);
        int source = AceEventProcessorAosp.eventSourceTransKeySource(eventSouce);
        int deviceId = event.getDeviceId();
        if (nativeDispatchKeyEvent(this.nativeWindowPtr, event.getKeyCode(), event.getAction(), event.getRepeatCount(),
                event.getEventTime(), event.getDownTime(), source, deviceId, modifierKeys)) {
            return true;
        }
        return windowViewInterface.superOnKeyUp(keyCode, event);
    }

    /**
     * The action of pre ime key.
     *
     * @param keyCode key code.
     * @param event Action and button state.
     * @param context context of the application.
     * @return true if pre ime action run success , false otherwise.
     */
    public boolean onKeyPreIme(int keyCode, KeyEvent event, Context context) {
        if (keyCode == KeyEvent.KEYCODE_BACK && event.getAction() == KeyEvent.ACTION_UP && keyboardVisible
                && !inputClient.isStopBackPress()) {
            if (!(context instanceof Activity)) {
                ALog.e(TAG, "cannot get activity.");
                return windowViewInterface.superOnKeyPreIme(keyCode, event);
            }
            Activity activity = (Activity) context;
            activity.onBackPressed();
            return true;
        }
        return windowViewInterface.superOnKeyPreIme(keyCode, event);
    }

    private void delayNotifyIfNeeded() {
        if (nativeWindowPtr == 0L) {
            ALog.e(TAG, "delay notify, nativeWindow is invalid!");
            return;
        }
        if (delayNotifySurfaceDestroyed) {
            ALog.i(TAG, "delay notify surfaceDestroyed");
            nativeSurfaceDestroyed(nativeWindowPtr);
            delayNotifySurfaceDestroyed = false;
        }

        if (delayNotifyCreateSurface != null) {
            ALog.i(TAG, "delay notify surfaceCreated");
            nativeSurfaceCreated(nativeWindowPtr, delayNotifyCreateSurface);
            delayNotifyCreateSurface = null;
        }

        if (delayNotifySurfaceChanged) {
            ALog.i(TAG, "delay notify surface changed w=" + surfaceWidth + " h=" + surfaceHeight);
            nativeSurfaceChanged(nativeWindowPtr, surfaceWidth, surfaceHeight, density);
            delayNotifySurfaceChanged = false;
        }
    }

    private void setWebTouchEvent(MotionEvent event) {
        if (aceWebPluginBase == null) {
            return;
        }
        Map<Long, AceWebBase> webObjectMap = aceWebPluginBase.getObjectMap();
        if (webObjectMap != null) {
            for (Map.Entry<Long, AceWebBase> entry : webObjectMap.entrySet()) {
                entry.getValue().setTouchEvent(event);
            }
        }
    }

    private void setPlatformViewTouchEvent(MotionEvent event) {
        if (acePlatformViewPluginBase == null) {
            return;
        }
        Map<Long, AcePlatformViewBase> platformViewObjectMap = acePlatformViewPluginBase.getObjectMap();
        if (platformViewObjectMap != null) {
            for (Map.Entry<Long, AcePlatformViewBase> entry : platformViewObjectMap.entrySet()) {
                entry.getValue().setTouchEvent(event);
            }
        }
    }

    private void markdownNumpadKeyStatus(int keyCode, int action) {
        if (keyCode == KeyEvent.KEYCODE_NUMPAD_ENTER) {
            if (action == KeyEvent.ACTION_DOWN) {
                numpadEnterPressed = true;
            } else {
                numpadEnterPressed = false;
            }
        } else if (keyCode == KeyEvent.KEYCODE_ENTER) {
            if (action == KeyEvent.ACTION_DOWN) {
                enterPressed = true;
            } else {
                enterPressed = false;
            }
        }
    }

    private boolean filterNumpadKeyCode(int keyCode, int action) {
        if (keyCode == KeyEvent.KEYCODE_ENTER) {
            if (action == KeyEvent.ACTION_DOWN) {
                if (numpadEnterPressed) {
                    return true;
                }
            } else {
                if (!enterPressed) {
                    return true;
                }
            }
        }
        return false;
    }

    private int getActionKey(int actionMasked, int buttonState) {
        int actionKey = lastMouseButtonState ^ buttonState;
        int action = actionMasked;
        if (actionMasked == MotionEvent.ACTION_DOWN) {
            lastMouseButtonState = buttonState;
            lastMouseActionKey = actionKey;
        } else if (actionMasked == MotionEvent.ACTION_UP) {
            lastMouseButtonState = lastMouseButtonState ^ actionKey;
        } else if (actionMasked == MotionEvent.ACTION_MOVE) {
            if (buttonState < lastMouseButtonState) {
                lastMouseButtonState = lastMouseButtonState ^ actionKey;
                action = MotionEvent.ACTION_UP;
            } else if (buttonState > lastMouseButtonState) {
                lastMouseButtonState = buttonState;
                action = MotionEvent.ACTION_DOWN;
                lastMouseActionKey = actionKey;
            } else {
                actionKey = lastMouseActionKey;
            }
        }
        return ((actionKey << 4) + action);
    }

    private native void nativeSurfaceCreated(long windowPtr, Surface surface);

    private native void nativeSurfaceChanged(long windowPtr, int width, int height, float density);

    private native void nativeSurfaceDestroyed(long windowPtr);

    private native void nativeKeyboardHeightChanged(long windowPtr, int height);

    private native void nativeWindowSizeChanged(long windowPtr, int width, int height);

    private native void nativeOnWindowFocusChanged(long windowPtr, boolean hasWindowFocus);

    private native void nativeForeground(long windowPtr);

    private native void nativeBackground(long windowPtr);

    private native void nativeDestroy(long windowPtr);

    private native boolean nativeBackPressed(long windowPtr);

    private native boolean nativeDispatchPointerDataPacket(long windowPtr, ByteBuffer buffer, int position);

    private native boolean nativeDispatchMouseDataPacket(long windowPtr, ByteBuffer buffer, int position);

    private native boolean nativeDispatchKeyEvent(long windowPtr, int keyCode, int action, int repeatTime,
            long timeStamp, long timeStampStart, int source, int deviceId, int metaKey);

    private native void nativeAvoidAreaChanged(long windowPtr);
}