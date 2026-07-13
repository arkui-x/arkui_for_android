/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

package ohos.stage.ability.adapter;

import android.app.Activity;
import android.graphics.Rect;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.PopupWindow;

import ohos.ace.adapter.ALog;
import ohos.ace.adapter.WindowViewAospInterface;
import ohos.ace.adapter.WindowViewBuilder;
import ohos.ace.adapter.AcePlatformPlugin;
import ohos.ace.adapter.WindowViewInterface;
import ohos.ace.adapter.WindowViewCommon;
import java.util.Arrays;

/**
 * The type Sub window.
 *
 * @since 2023-08-06
 */
public class SubWindow {
    private static final String TAG = "SubWindow";
    private static final int WINDOW_MODALITY = 0;

    private long nativeSubWindowPtr = 0L;

    private String name;
    private int windowId = InstanceIdGenerator.getAndIncrement();
    private int parentId;
    private int windowMode;
    private int windowType;
    private WindowParam windowParam;
    private PopupSubWindow subWindowView;
    private View contentView;
    private Activity rootActivity;
    private View rootView;
    private boolean isFocusable = true;
    private boolean hasFocus = false;
    private boolean isModal = false;
    private boolean onTop = false;
    private boolean requestedVisible = false;
    private boolean skipNativeHideCallbackForRestack = false;
    private Rect[] hotAreas;
    private boolean isFullScreen = false;
    private View mainView;
    private int mainWindowX;
    private int mainWindowY;
    private WindowViewAospInterface windowView;
    private int originWindowY;

    /**
     * The type Window param.
     */
    public static class WindowParam {
        /**
         * The Background color.
         */
        public int backgroundColor;

        /**
         * The Width.
         */
        public int width;

        /**
         * The Height.
         */
        public int height;

        /**
         * The X.
         */
        public int x;

        /**
         * The Y.
         */
        public int y;

        /**
         * Modality type of subwindow.
         */
        private int modalityType;

        /**
         * Instantiates a new Window param.
         */
        public WindowParam() {
            backgroundColor = 0;
            width = 0;
            height = 0;
            x = 0;
            y = 0;
            modalityType = WINDOW_MODALITY;
        }
    }

    /**
     * The type Popup sub window.
     */
    public class PopupSubWindow extends PopupWindow {
        private static final String TAG = "PopupSubWindow";

        public PopupSubWindow() {
            super();
        }

        @Override
        public void dismiss() {
            StackTraceElement[] stackTrace = new Exception().getStackTrace();
            if (stackTrace.length >= 2 && "onTouchEvent".equals(stackTrace[1].getMethodName())) {
                return;
            }
            super.dismiss();
            if (!skipNativeHideCallbackForRestack) {
                requestedVisible = false;
            }
            if (!skipNativeHideCallbackForRestack && nativeSubWindowPtr != 0L) {
                nativeOnSubWindowHide(nativeSubWindowPtr);
            }
        }

        /**
         * The type Window param.
         */
        public void shutdown() {
            View cView = getContentView();
            if (cView != null && cView instanceof WindowViewAospInterface) {
                ((WindowViewAospInterface) cView).destroyAosp();
                cView = null;
            }
            super.dismiss();
        }
    }

    /**
     * Instantiates a new Sub window.
     *
     * @param activity the activity
     * @param name     the name
     */
    public SubWindow(Activity activity, String name) {
        this.name = name;
        rootActivity = activity;
        rootView = rootActivity.getWindow().getDecorView();
        windowParam = new WindowParam();
    }

    /**
     * Create sub window.
     *
     * @param param   the create sub window parameter
     */
    public void createSubWindow(SubWindowManager.CreateSubWindowParam param) {
        updateWindowParam(param);
        createSubWindow(windowParam);
    }

    /**
     * Create sub window.
     *
     * @param param the param
     */
    public void createSubWindow(WindowParam param) {
        subWindowView = new PopupSubWindow();
        subWindowView.setOutsideTouchable(!isModal);
        subWindowView.setFocusable(isFocusable);
        subWindowView.setTouchable(true);

        windowView = WindowViewBuilder.makeWindowViewAosp(rootActivity, windowId, true);
        AcePlatformPlugin platformPlugin = new AcePlatformPlugin(rootActivity, windowId, windowView.getView());
        if (platformPlugin != null) {
            windowView.setInputConnectionClient(platformPlugin);
            platformPlugin.initTexturePlugin(windowId);
            platformPlugin.initSurfacePlugin(rootActivity, windowId);
        }
        setContentView(windowView.getView());
        syncContentViewFocusable(isFocusable);

        subWindowView.setTouchInterceptor(createTouchInterceptor());
        setupGlobalLayoutListener();
    }

    private View.OnTouchListener createTouchInterceptor() {
        return new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int x = (int) event.getX();
                int y = (int) event.getY();
                ALog.d(TAG, "subWindowView onTouch. event.getAction()=" + event.getAction() + ", x=" + x + ", y=" + y);

                if (isTouchOutsideWindow(event, x, y)) {
                    return handleTouchOutside();
                }

                Boolean handledByHotArea = handleHotAreaIfNeeded(event, x, y);
                if (handledByHotArea != null) {
                    return handledByHotArea;
                }

                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    getFocus();
                }
                return false;
            }
        };
    }

    private boolean isTouchOutsideWindow(MotionEvent event, int x, int y) {
        boolean isOutsideBounds = (event.getAction() == MotionEvent.ACTION_DOWN)
                && (x < 0 || x > subWindowView.getWidth() || y < 0 || y > subWindowView.getHeight());
        return event.getAction() == MotionEvent.ACTION_OUTSIDE || isOutsideBounds;
    }

    private boolean handleTouchOutside() {
        ALog.d(TAG, "touch outside");
        if (nativeSubWindowPtr != 0L) {
            nativeOnWindowTouchOutside(nativeSubWindowPtr);
        }
        if (!isModal) {
            releaseFocus();
            return false;
        }
        return true;
    }

    private boolean isPointInHotAreas(int x, int y) {
        for (Rect rect : hotAreas) {
            if (rect.contains(x, y)) {
                return true;
            }
        }
        return false;
    }

    private Boolean handleHotAreaIfNeeded(MotionEvent event, int x, int y) {
        if (hotAreas == null) {
            return null;
        }
        if (isPointInHotAreas(x, y)) {
            return null;
        }
        if (event.getAction() == MotionEvent.ACTION_DOWN && nativeSubWindowPtr != 0L) {
            nativeOnWindowTouchOutside(nativeSubWindowPtr);
        }
        if (isModal) {
            return true;
        }
        return dispatchTouchEventToMainWindow(event);
    }

    private void setupGlobalLayoutListener() {
        ViewTreeObserver.OnGlobalLayoutListener listener = new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                int[] location = new int[2];
                mainView.getLocationInWindow(location);
                mainWindowX = location[0];
                mainWindowY = location[1];
                if (isFullScreen) {
                    windowParam.x = 0;
                    windowParam.y = 0;
                    windowParam.width = mainView.getWidth();
                    windowParam.height = mainView.getHeight();
                }
                updateWindow();
            }
        };
        mainView.getViewTreeObserver().addOnGlobalLayoutListener(listener);
    }

    /**
     * Dispatch touch event to main window.
     */
    public boolean dispatchTouchEventToMainWindow(MotionEvent event) {
        int touchX = (int) event.getX() + windowParam.x;
        int touchY = (int) event.getY() + windowParam.y;
        MotionEvent newEvent = MotionEvent.obtain(event.getDownTime(), event.getEventTime(), event.getAction(),
            touchX, touchY, event.getMetaState());
        mainView.dispatchTouchEvent(newEvent);
        newEvent.recycle();
        return true;
    }

    /**
     * Update window param.
     *
     * @param param   the create sub window parameter
     */
    private void updateWindowParam(SubWindowManager.CreateSubWindowParam param) {
        this.windowType = param.type;
        this.windowMode = param.mode;
        this.windowTag = param.tag;
        this.parentId = param.parentId;
        this.isModal = param.isModal;
        this.windowParam.modalityType = param.modalityType;
        this.windowParam.x = param.x;
        this.windowParam.y = param.y;
        this.windowParam.width = param.width;
        this.windowParam.height = param.height;


        mainView = rootView.findViewById(android.R.id.content);
        int[] location = new int[2];
        mainView.getLocationInWindow(location);
        mainWindowX = location[0];
        mainWindowY = location[1];
    }

    /**
     * Show window.
     */
    public void showWindow() {
        showWindow(true);
    }

    /**
     * Show window.
     *
     * @param requestFocus whether to request focus after showing
     */
    public void showWindow(boolean requestFocus) {
        if (subWindowView.isShowing()) {
            return;
        }
        requestedVisible = true;
        if (windowParam.width == 0 || windowParam.height == 0) {
            if (mainView != null) {
                windowParam.width = mainView.getWidth();
                windowParam.height = mainView.getHeight();
            }
        }
        subWindowView.setWidth(windowParam.width);
        subWindowView.setHeight(windowParam.height);
        subWindowView.setElevation(0);
        subWindowView.setContentView(contentView);

        int screenX = windowParam.x + mainWindowX;
        int screenY = windowParam.y + mainWindowY;
        ALog.d(TAG, "showWindow called. x=" + windowParam.x + ", y=" + windowParam.y + ", screenX=" + screenX +
            ", screenY=" + screenY + ", width=" + windowParam.width + ", height=" + windowParam.height);
        subWindowView.showAtLocation(rootView, Gravity.TOP | Gravity.START, screenX, screenY);
        if (requestFocus) {
            getFocus();
        }
    }

    /**
     * Update window.
     */
    public void updateWindow() {
        int screenX = windowParam.x + mainWindowX;
        int screenY = windowParam.y + mainWindowY;

        ALog.d(TAG, "updateWindow called. x=" + windowParam.x + ", y=" + windowParam.y + ", screenX=" + screenX +
            ", screenY=" + screenY + ", width=" + windowParam.width + ", height=" + windowParam.height);
        subWindowView.update(screenX, screenY, windowParam.width, windowParam.height, true);
    }

    /**
     * Notify keyboard height changed.
     */
    public void notifyKeyboardHeightChanged() {
        if (!hasFocus) {
            ALog.d(TAG, "[SubwindowKbd] notifyKeyboardHeightChanged: window is not focused, restore and skip");
            restoreWindowPosition();
            return;
        }
        int keyboardHeight = WindowViewCommon.getSubwindowKeyboardAvoidHeight();
        int avoidHeight = calculateKeyboardAvoidHeight(keyboardHeight);
        if (windowView != null) {
            ALog.d(TAG, "[SubwindowKbd] notifyKeyboardAvoidHeight: avoidHeight=" + avoidHeight);
            windowView.keyboardHeightChanged(avoidHeight);
        }
    }

    private void restoreWindowPosition() {
        if (originWindowY > 0) {
            windowParam.y = originWindowY;
            originWindowY = 0;
        }
    }

    private int calculateKeyboardAvoidHeight(int keyboardHeight) {
        if (keyboardHeight == 0) {
            ALog.d(TAG, "[SubwindowKbd] calculateKeyboardAvoidHeight: restore the window position");
            restoreWindowPosition();
            return 0;
        }
        int screenHeight = getRootViewHeight();
        if (screenHeight <= 0) {
            ALog.d(TAG, "[SubwindowKbd] calculateKeyboardAvoidHeight: screenHeight <= 0");
            return 0;
        }
        int keyboardTop = screenHeight - keyboardHeight;
        // Upper limit of window movement
        int maxWindowPosY = originWindowY == 0 ? windowParam.y : originWindowY;
        int minWindowPosY = keyboardTop - windowParam.height - mainWindowY;
        // Final Y clamped between [0, maxWindowPosY, minWindowPosY]:
        //  - 0: cannot go above the screen top
        //  - maxWindowPosY: cannot move downward past original position
        //  - minWindowPosY: target to stay above the keyboard
        int finalTargetY = Math.max(0, Math.min(maxWindowPosY, minWindowPosY));
        // movedWindowBottom = movedWindowY + currentWindowHeight + statusBarHeight
        int movedWindowBottom = finalTargetY + windowParam.height + mainWindowY;
        // The part of the window still covered by the keyboard after max upward movement
        int reportHeight = Math.max(0, movedWindowBottom - keyboardTop);
        // Clamp to keyboard height: the occupied area cannot exceed the keyboard itself
        reportHeight = Math.min(reportHeight, keyboardHeight);
        if (finalTargetY != windowParam.y) {
            if (originWindowY == 0) {
                originWindowY = windowParam.y;
            }
            windowParam.y = finalTargetY;
        }
        return reportHeight;
    }

    private int getRootViewHeight() {
        View view = rootView;
        if (rootActivity != null && rootActivity.getWindow() != null) {
            view = rootActivity.getWindow().getDecorView();
        }
        if (view != null && view.getRootView() != null && view.getRootView().getHeight() > 0) {
            return view.getRootView().getHeight();
        }
        if (view != null && view.getResources() != null) {
            return view.getResources().getDisplayMetrics().heightPixels;
        }
        return 0;
    }

    /**
     * Show window.
     *
     * @param anchor the anchor
     */
    public void showWindow(View anchor) {
        subWindowView.showAsDropDown(anchor);
    }

    /**
     * Show window.
     *
     * @param anchor  the anchor
     * @param xoffset the xoffset
     * @param yoffset the yoffset
     */
    public void showWindow(View anchor, int xoffset, int yoffset) {
        subWindowView.showAsDropDown(anchor, xoffset, yoffset);
    }

    /**
     * Resize.
     *
     * @param width  the width
     * @param height the height
     */
    public void resize(int width, int height) {
        if (isFullScreen) {
            return;
        }
        windowParam.width = width;
        windowParam.height = height;
        updateWindow();
    }

    /**
     * Move window to.
     *
     * @param x the x
     * @param y the y
     */
    public void moveWindowTo(int x, int y) {
        if (isFullScreen) {
            return;
        }
        windowParam.x = x;
        windowParam.y = y;
        updateWindow();
    }

    /**
     * Destroy window.
     */
    public void destroyWindow() {
        subWindowView.shutdown();
    }

    /**
     * Dismiss window for restack without notifying native hide callback.
     */
    public void dismissForRestack() {
        if (subWindowView == null || !subWindowView.isShowing()) {
            return;
        }
        skipNativeHideCallbackForRestack = true;
        subWindowView.dismiss();
        skipNativeHideCallbackForRestack = false;
    }

    /**
     * Set window focusable.
     *
     * @param isFocusable the is focusable
     */
    public void setFocusable(boolean isFocusable) {
        syncContentViewFocusable(isFocusable);
        subWindowView.setFocusable(isFocusable);
        this.isFocusable = isFocusable;
        if (subWindowView.isShowing()) {
            subWindowView.update();
        }
    }

    /**
     * Set window touchable.
     *
     * @param isTouchable the is touchable
     */
    public void setTouchable(boolean isTouchable) {
        subWindowView.setTouchable(isTouchable);
    }

    /**
     * request focus.
     *
     * @return the boolean
     */
    public boolean requestFocus() {
        if (!subWindowView.isShowing()) {
            ALog.e(TAG, "not showing.");
            return false;
        }

        if (!this.isFocusable) {
            ALog.e(TAG, "not focusable.");
            return false;
        }

        getFocus();
        return true;
    }

    /**
     * release focus.
     */
    public void getFocus() {
        ALog.d(TAG, "getFocus(), isFocusable=" + this.isFocusable);
        if (this.isFocusable) {
            hasFocus = true;
            syncContentViewFocusable(true);
            subWindowView.setFocusable(true);
            subWindowView.update();
        }
    }

    /**
     * release focus.
     */
    public void releaseFocus() {
        ALog.d(TAG, "releaseFocus(), isFocusable=" + this.isFocusable);
        if (this.isFocusable) {
            hasFocus = false;
            syncContentViewFocusable(false);
            subWindowView.setFocusable(false);
            subWindowView.update();
        }
    }

    private void syncContentViewFocusable(boolean isFocusable) {
        if (contentView instanceof WindowViewInterface) {
            ((WindowViewInterface) contentView).setWindowFocusable(isFocusable);
        }
    }

    /**
     * set window touch hot area.
     *
     * @param rectArray the rect array
     */
    public void setTouchHotArea(Rect[] rectArray) {
        ALog.d(TAG, "setTouchHotArea(), rectArray:");
        for (Rect rect : rectArray) {
            ALog.d(TAG,
                "left=" + rect.left + ", top=" + rect.top + ", right=" + rect.right + ", bottom=" + rect.bottom);
        }
        hotAreas = Arrays.copyOf(rectArray, rectArray.length);
    }

    /**
     * set FullScreen.
     *
     * @param status the status
     */
    public boolean setFullScreen(boolean status) {
        if (status) {
            windowParam.x = 0;
            windowParam.y = 0;
            windowParam.width = mainView.getWidth();
            windowParam.height = mainView.getHeight();
            isFullScreen = true;
        } else {
            isFullScreen = false;
        }
        updateWindow();
        return true;
    }

    /**
     * Set subwindow on top.
     *
     * @param status the status
     */
    public void setOnTop(boolean status) {
        if (status) {
            subWindowView.setWindowLayoutType(WindowManager.LayoutParams.TYPE_APPLICATION_SUB_PANEL);
        } else {
            subWindowView.setWindowLayoutType(WindowManager.LayoutParams.TYPE_APPLICATION_PANEL);
        }
        onTop = status;
    }

    /**
     * Get subwindow on top.
     *
     * @return true if the subwindow is on top
     */
    public boolean getOnTop() {
        return onTop;
    }

    /**
     * Check whether this window is modal.
     *
     * @return true if modal
     */
    public boolean isModalWindow() {
        return isModal;
    }

    /**
     * Set requested visibility for manager restack.
     *
     * @param requestedVisible whether the manager wants this subwindow showing
     */
    public void setRequestedVisible(boolean requestedVisible) {
        this.requestedVisible = requestedVisible;
    }

    /**
     * Get requested visibility.
     *
     * @return true if manager expects this window to be showing
     */
    public boolean isRequestedVisible() {
        return requestedVisible;
    }

    /**
     * Gets window tag.
     *
     * @return the window tag
     */
    public int getWindowTag() {
        return windowTag;
    }

    /**
     * Sets window tag.
     *
     * @param windowTag the window tag
     */
    public void setWindowTag(int windowTag) {
        this.windowTag = windowTag;
    }

    private int windowTag;

    /**
     * Gets sub window view.
     *
     * @return the sub window view
     */
    public PopupSubWindow getSubWindowView() {
        return subWindowView;
    }

    /**
     * Sets sub window view.
     *
     * @param subWindowView the sub window view
     */
    public void setSubWindowView(PopupSubWindow subWindowView) {
        this.subWindowView = subWindowView;
    }

    /**
     * Gets name.
     *
     * @return the name
     */
    public String getName() {
        return name;
    }

    /**
     * Sets name.
     *
     * @param name the name
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * Gets window id.
     *
     * @return the window id
     */
    public int getWindowId() {
        return windowId;
    }

    /**
     * Sets window id.
     *
     * @param windowId the window id
     */
    public void setWindowId(int windowId) {
        this.windowId = windowId;
    }

    /**
     * Gets parent id.
     *
     * @return the parent id
     */
    public int getParentId() {
        return parentId;
    }

    /**
     * Sets parent id.
     *
     * @param parentId the parent id
     */
    public void setParentId(int parentId) {
        this.parentId = parentId;
    }

    /**
     * Gets window mode.
     *
     * @return the window mode
     */
    public int getWindowMode() {
        return windowMode;
    }

    /**
     * Sets window mode.
     *
     * @param windowMode the window mode
     */
    public void setWindowMode(int windowMode) {
        this.windowMode = windowMode;
    }

    /**
     * Gets window type.
     *
     * @return the window type
     */
    public int getWindowType() {
        return windowType;
    }

    /**
     * Sets window type.
     *
     * @param windowType the window type
     */
    public void setWindowType(int windowType) {
        this.windowType = windowType;
    }

    /**
     * Gets content view.
     *
     * @return the content view
     */
    public View getContentView() {
        return contentView;
    }

    /**
     * Sets content view.
     *
     * @param contentView the content view
     */
    public void setContentView(View contentView) {
        this.contentView = contentView;
    }

    /**
     * Gets window param.
     *
     * @return the window param
     */
    public WindowParam getWindowParam() {
        return windowParam;
    }

    /**
     * Sets window param.
     *
     * @param param the param
     */
    public void setWindowParam(WindowParam param) {
        this.windowParam = param;
    }

    /**
     * Gets root activity.
     *
     * @return the root activity
     */
    public Activity getRootActivity() {
        return rootActivity;
    }

    /**
     * Sets root activity.
     *
     * @param rootActivity the root activity
     */
    public void setRootActivity(Activity rootActivity) {
        this.rootActivity = rootActivity;
    }

    /**
     * Gets root view.
     *
     * @return the root view
     */
    public View getRootView() {
        return rootView;
    }

    /**
     * Sets root view.
     *
     * @param rootView the root view
     */
    public void setRootView(View rootView) {
        this.rootView = rootView;
    }

    /**
     * Called by native to register Window Handle.
     *
     * @param windowHandle the handle of navive window
     */
    public void registerSubWindow(long subWindowHandle) {
        nativeSubWindowPtr = subWindowHandle;
    }

    /**
     * Called by native to unregister Window Handle.
     */
    public void unregisterSubWindow() {
        nativeSubWindowPtr = 0L;
    }

    private native void nativeOnWindowTouchOutside(long windowPtr);
    private native void nativeOnSubWindowHide(long windowPtr);
    private native void nativeOnSubWindowClose(long windowPtr);
}
