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

import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import ohos.ace.adapter.capability.platformview.AcePlatformViewPluginBase;
import ohos.ace.adapter.capability.web.AceWebPluginBase;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;

/**
 * WindowViewAospInterface class that extends WindowViewInterface.
 *
 * @since 2025-05-06
 */
public interface WindowViewInterface {
    /**
     * Notify nativeWindow background.
     */
    void background();

    /**
     * Notify nativeWindow backPressed.
     *
     * @return true if process BackPressed, false otherwise.
     */
    boolean backPressed();

    /**
     * Notify nativeWindow destroy.
     */
    void destroy();

    /**
     * Set whether the surfaceView is hide.
     *
     * @param isHide whether the surfaceView is hide
     */
    void setHide(boolean isHide);

    /**
     * Notify nativeWindow foreground.
     */
    void foreground();

    /**
     * Get window View.
     *
     * @return window View
     */
    View getView();

    /**
     * Get window orientation.
     *
     * @return window orientation or not.
     */
    boolean getWindowOrientation();

    /**
     * To notify keyboard height changed.
     *
     * @param height the height of keyboard
     */
    void keyboardHeightChanged(int height);

    /**
     * To notify avoid area changed.
     */
    void avoidAreaChanged();

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    boolean onHoverEventAosp(MotionEvent event);

    /**
     * Called when the windowfocus changed.
     *
     * @param hasWindowFocus has focus or not
     */
    void onWindowFocusChanged(boolean hasWindowFocus);

    /**
     * Set the input connection client.
     *
     * @param inputConnectionClient the input connection client
     */
    void setInputConnectionClient(InputConnectionClient inputConnectionClient);

    /**
     * Set instance Id.
     *
     * @param instanceId the instance id
     */
    void setInstanceId(int instanceId);

    /**
     * Set platformniew plugin.
     *
     * @param pluginBase the AcePlatformViewPluginBase object
     */
    void setPlatformViewPlugin(AcePlatformViewPluginBase pluginBase);

    /**
     * Set web plugin.
     *
     * @param pluginBase the AceWebPluginBase object
     */
    void setWebPlugin(AceWebPluginBase pluginBase);

    /**
     * Set window orientation.
     *
     * @param isOrientation is window orientation or not.
     */
    void setWindowOrientation(boolean isOrientation);

    /**
     * Create the input connection.
     *
     * @param outAttrs editor info
     * @return the input connection
     */
    InputConnection superOnCreateInputConnection(EditorInfo outAttrs) ;

    /**
     * Convert action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if hover action run success , false otherwise.
     */
    boolean superOnHoverEvent(MotionEvent event);

    /**
     * Touch action and button state to action key.
     *
     * @param event Action and button state.
     * @return true if touch action run success , false otherwise.
     */
    boolean superOnTouchEvent(MotionEvent event);

    /**
     * The action of press down key.
     *
     * @param keyCode key code.
     * @param event Action and button state.
     * @return true if press down action run success , false otherwise.
     */
    boolean superOnKeyDown(int keyCode, KeyEvent event);

    /**
     * The action of press up key.
     *
     * @param keyCode key code.
     * @param event Action and button state.
     * @return true if press up action run success , false otherwise.
     */
    boolean superOnKeyUp(int keyCode, KeyEvent event);

    /**
     * Called by native to register Window Handle.
     *
     * @param windowHandle the handle of navive window
     */
    void registerWindow(long windowHandle);

    /**
     * Called by native to unregister Window Handle.
     */
    void unRegisterWindow();
}