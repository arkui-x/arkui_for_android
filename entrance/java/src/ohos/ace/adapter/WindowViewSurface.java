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
import android.graphics.PixelFormat;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.os.Build;

import ohos.ace.adapter.capability.platformview.AcePlatformViewPluginBase;
import ohos.ace.adapter.capability.web.AceWebPluginBase;
import ohos.ace.adapter.WindowViewInterface;
import ohos.ace.adapter.WindowViewCommon;

/**
 * WindowViewSurface is a class that implement WindowViewInterface interface.
 *
 * @since 2025-05-06
 */
public class WindowViewSurface extends SurfaceView implements SurfaceHolder.Callback, WindowViewInterface {
    private static final String TAG = "WindowViewSurface";
    private boolean isFocused = true;
    private WindowViewCommon windowViewCommon;


    /**
     * Constructor of WindowViewSurface
     *
     * @param context Application context
     */
    public WindowViewSurface(Context context) {
        super(context);
        initView();
        windowViewCommon = getWindowViewCommon();
    }

    private void initView() {
        ALog.i(TAG, "WindowView created");
        setFocusableInTouchMode(true);
        getHolder().addCallback(this);
        setZOrderOnTop(true);
    }

    /**
     * Called by native to register Window Handle.
     *
     * @param windowHandle the handle of navive window
     */
    @Override
    public void registerWindow(long windowHandle) {
        windowViewCommon.registerWindow(windowHandle);
    }

    /**
     * Called by native to unregister Window Handle.
     */
    @Override
    public void unRegisterWindow() {
        windowViewCommon.unRegisterWindow();
    }

    /**
     * Set instance Id.
     *
     * @param instanceId the instance id
     */
    @Override
    public void setInstanceId(int instanceId) {
        setId(instanceId);
    }

    private WindowViewCommon getWindowViewCommon() {
        return new WindowViewCommon(this);
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
     * Set web plugin.
     *
     * @param pluginBase the AceWebPluginBase object
     */
    @Override
    public void setWebPlugin(AceWebPluginBase pluginBase) {
        windowViewCommon.setWebPlugin(pluginBase);
    }

    /**
     * Set platformview plugin.
     *
     * @param pluginBase the AcePlatformViewPluginBase object
     */
    @Override
    public void setPlatformViewPlugin(AcePlatformViewPluginBase pluginBase) {
        windowViewCommon.setPlatformViewPlugin(pluginBase);
    }

    /**
     * Set the input connection client.
     *
     * @param inputConnectionClient the input connection client
     */
    @Override
    public void setInputConnectionClient(InputConnectionClient inputConnectionClient) {
        windowViewCommon.setInputConnectionClient(inputConnectionClient);
    }

    /**
     * To notify keyboard height changed.
     *
     * @param height the height of keyboard
     */
    @Override
    public void keyboardHeightChanged(int height) {
        windowViewCommon.keyboardHeightChanged(height);
    }

    /**
     * To notify avoid area changed.
     */
    @Override
    public void avoidAreaChanged() {
        windowViewCommon.avoidAreaChanged();
    }

    /**
     * Called when the surface  created.
     *
     * @param holder surface holder
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        setFocusable(true);
        if (isFocused) {
            requestFocus();
        }
        holder.setFormat(PixelFormat.TRANSLUCENT);
        Surface surface = holder.getSurface();
        if (surface != null) {
            windowViewCommon.surfaceCreated(surface);
        }
    }

    /**
     * Called when the surface size changed.
     *
     * @param holder surface holder
     * @param format the format of surface
     * @param width the width of surface
     * @param height the height of surface
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        windowViewCommon.surfaceSizeChanged(width, height, getResources().getDisplayMetrics().density);
    }

    /**
     * Called when the windowfocus changed.
     *
     * @param holder surface holder
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        isFocused = hasFocus();
        windowViewCommon.surfaceDestroyed();
    }

    /**
     * Called when the windowfocus changed.
     *
     * @param hasWindowFocus has focus or not
     */
    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        windowViewCommon.onWindowFocusChanged(hasWindowFocus);
    }

    /**
     * Create the input connection.
     *
     * @param outAttrs editor info
     * @return the input connection
     */
    @Override
    public InputConnection superOnCreateInputConnection(EditorInfo outAttrs) {
        return super.onCreateInputConnection(outAttrs);
    }

    /**
     * Create the input connection.
     *
     * @param outAttrs editor info
     * @return the input connection
     */
    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        return windowViewCommon.onCreateInputConnection(outAttrs);
    }

    /**
     * Set whether the surfaceView is hide.
     *
     * @param isHide whether the surfaceView is hide
     */
    @Override
    public void setHide(boolean isHide) {
        if (Build.VERSION.SDK_INT < 30) {
            setZOrderMediaOverlay(!isHide);
        } else {
            setZOrderOnTop(!isHide);
        }
    }

    /**
     * Notify nativeWindow foreground.
     */
    @Override
    public void foreground() {
        windowViewCommon.foreground();
    }

    /**
     * Notify nativeWindow background.
     */
    @Override
    public void background() {
        windowViewCommon.background();
    }

    /**
     * Notify nativeWindow destroy.
     */
    @Override
    public void destroy() {
        windowViewCommon.destroy();
    }

    /**
     * Notify nativeWindow backPressed.
     *
     * @return true if process BackPressed, false otherwise.
     */
    @Override
    public boolean backPressed() {
        return windowViewCommon.backPressed();
    }

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    @Override
    public boolean onHoverEventAosp(MotionEvent event) {
        return windowViewCommon.onHoverEventAosp(event);
    }

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    @Override
    public boolean onHoverEvent(MotionEvent event) {
        return windowViewCommon.onHoverEvent(event);
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
     * Touch action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if touch action run success , false otherwise.
     */
    @Override
    public boolean superOnTouchEvent(MotionEvent event) {
        return super.onTouchEvent(event);
    }

    /**
     * Touch action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if touch action run success , false otherwise.
     */
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        return windowViewCommon.onTouchEvent(event);
    }

    /**
     * The action of press down key.
     *
     * @param keyCode key code.
     * @param event Action and button state.
     * @return true if press down action run success , false otherwise.
     */
    @Override
    public boolean superOnKeyDown(int keyCode, KeyEvent event) {
        return super.onKeyDown(keyCode, event);
    }

    /**
     * The action of press down key.
     *
     * @param keyCode key code.
     * @param event Action and button state.
     * @return true if press down action run success , false otherwise.
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return windowViewCommon.onKeyDown(keyCode, event);
    }

    /**
     * The action of press up key.
     *
     * @param keyCode key code.
     * @param event Action and button state.
     * @return true if press up action run success , false otherwise.
     */
    @Override
    public boolean superOnKeyUp(int keyCode, KeyEvent event) {
        return super.onKeyUp(keyCode, event);
    }

    /**
     * The action of press up key.
     *
     * @param keyCode key code.
     * @param event Action and button state.
     * @return true if press up action run success , false otherwise.
     */
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return windowViewCommon.onKeyUp(keyCode, event);
    }

    /**
     * Set window orientation.
     *
     * @param isOrientation is window orientation or not.
     */
    @Override
    public void setWindowOrientation(boolean isOrientation) {
        windowViewCommon.setWindowOrientation(isOrientation);
    }

    /**
     * Get window orientation.
     *
     * @return window orientation or not.
     */
    @Override
    public boolean getWindowOrientation() {
        return windowViewCommon.getWindowOrientation();
    }
}