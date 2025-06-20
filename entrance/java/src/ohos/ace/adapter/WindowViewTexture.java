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
import android.graphics.SurfaceTexture;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;
import android.view.TextureView.SurfaceTextureListener;
import android.view.TextureView;

import ohos.ace.adapter.capability.platformview.AcePlatformViewPluginBase;
import ohos.ace.adapter.capability.web.AceWebPluginBase;
import ohos.ace.adapter.WindowViewInterface;
import ohos.ace.adapter.WindowViewCommon;

/**
 * WindowViewTexture is a class that implement WindowViewInterface interface.
 *
 * @since 2025-05-06
 */
public class WindowViewTexture extends TextureView implements TextureView.SurfaceTextureListener, WindowViewInterface {
    private static final String TAG = "WindowViewTexture";

    private boolean isFocused = true;

    private WindowViewCommon windowViewCommon;

    /**
     * Constructor of Window View Texture
     *
     * @param context Application context
     */
    public WindowViewTexture(Context context) {
        super(context);
        initView();
        windowViewCommon = getWindowViewCommon();
    }

    private void initView() {
        ALog.i(TAG, "WindowView created");
        setSurfaceTextureListener(this);
        setFocusableInTouchMode(true);
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
     * Called when surface texture is available.
     *
     * @param surface surface texture
     * @param width the width of texture
     * @param height the height of texture
     */
    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        setFocusable(true);
        if (isFocused) {
            requestFocus();
        }
        if (surface != null) {
            Surface s = new Surface(surface);
            windowViewCommon.surfaceCreated(s);
            windowViewCommon.surfaceSizeChanged(width, height, getResources().getDisplayMetrics().density);
        }
    }

    /**
     * Called when the texture size changed.
     *
     * @param surface surface texture
     * @param width the width of texture
     * @param height the height of texture
     */
    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        ALog.i(TAG, "onSurfaceTextureSizeChanged");
        windowViewCommon.surfaceSizeChanged(width, height, getResources().getDisplayMetrics().density);
    }

    /**
     * Called when the texture size destroyed.
     *
     * @param surface surface texture
     * @return if success or not
     */
    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        isFocused = hasFocus();
        windowViewCommon.surfaceDestroyed();
        return true;
    }

    /**
     * Called when the texture updated.
     *
     * @param surface surface texture
     */
    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
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