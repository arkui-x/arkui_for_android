/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.PopupWindow;
import ohos.ace.adapter.WindowViewAospInterface;
import ohos.ace.adapter.WindowViewBuilder;
import ohos.ace.adapter.AcePlatformPlugin;
import java.util.Arrays;

/**
 * The type Sub window.
 *
 * @since 2023-08-06
 */
public class SubWindow {
    private static final String TAG = "SubWindow";
    private long nativeSubWindowPtr = 0L;

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
         * Instantiates a new Window param.
         */
        public WindowParam() {
            backgroundColor = 0;
            width = 100;
            height = 100;
            x = 0;
            y = 0;
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
            if (nativeSubWindowPtr != 0L) {
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
     * @param type     the type
     * @param mode     the mode
     * @param tag      the tag
     * @param parentId the parent id
     * @param x        the x
     * @param y        the y
     * @param width    the width
     * @param height   the height
     */
    public void createSubWindow(int type, int mode, int tag, int parentId, int x, int y, int width, int height) {
        updateWindowParam(type, mode, tag, parentId, x, y, width, height);
        createSubWindow(windowParam);
    }

    /**
     * Create sub window.
     *
     * @param param the param
     */
    public void createSubWindow(WindowParam param) {
        Log.d(TAG, "createSubwindow called. name=" + name);
        subWindowView = new PopupSubWindow();
        subWindowView.setOutsideTouchable(true);
        subWindowView.setFocusable(isFocusable);
        subWindowView.setTouchable(true);

        WindowViewAospInterface windowView = WindowViewBuilder.makeWindowViewAosp(rootActivity, windowId, true);
        AcePlatformPlugin platformPlugin = new AcePlatformPlugin(rootActivity, windowId, windowView.getView());
        if (platformPlugin != null) {
            windowView.setInputConnectionClient(platformPlugin);
            platformPlugin.initTexturePlugin(windowId);
            platformPlugin.initSurfacePlugin(rootActivity, windowId);
        }
        setContentView(windowView.getView());

        subWindowView.setTouchInterceptor(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int x = (int) event.getX();
                int y = (int) event.getY();
                Log.d(TAG, "subWindowView onTouch. event.getAction()=" + event.getAction() + ", x=" + x + ", y=" + y);
                if ((event.getAction() == MotionEvent.ACTION_OUTSIDE) || ((event.getAction() == MotionEvent.ACTION_DOWN)
                        && (x < 0 || x > subWindowView.getWidth() || y < 0 || y > subWindowView.getHeight()))) {
                    Log.d(TAG, "touch outside");
                    if (nativeSubWindowPtr != 0L) {
                        nativeOnWindowTouchOutside(nativeSubWindowPtr);
                    }
                    releaseFocus();
                } else {
                    if (hotAreas != null) {
                        boolean inHotAreas = false;
                        for (Rect rect : hotAreas) {
                            if (rect.contains(x, y)) {
                                inHotAreas = true;
                                break;
                            }
                        }
                        if (!inHotAreas) {
                            if ((event.getAction() == MotionEvent.ACTION_DOWN) && (nativeSubWindowPtr != 0L)) {
                                nativeOnWindowTouchOutside(nativeSubWindowPtr);
                            }
                            return dispatchTouchEventToMainWindow(event);
                        }
                    }

                    if (event.getAction() == MotionEvent.ACTION_DOWN) {
                        getFocus();
                    }
                }

                return false;
            }
        });

        ViewTreeObserver.OnGlobalLayoutListener listener = new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                int[] location = new int[2];
                mainView.getLocationOnScreen(location);
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
     * @param type     the type
     * @param mode     the mode
     * @param tag      the tag
     * @param parentId the parent id
     * @param x        the x
     * @param y        the y
     * @param width    the width
     * @param height   the height
     */
    public void updateWindowParam(int type, int mode, int tag, int parentId, int x, int y, int width, int height) {
        this.windowType = type;
        this.windowMode = mode;
        this.windowTag = tag;
        this.parentId = parentId;
        this.windowParam.x = x;
        this.windowParam.y = y;

        if (width > 0) {
            this.windowParam.width = width;
        }

        if (height > 0) {
            this.windowParam.height = height;
        }

        mainView = rootView.findViewById(android.R.id.content);
        int[] location = new int[2];
        mainView.getLocationOnScreen(location);
        mainWindowX = location[0];
        mainWindowY = location[1];
    }

    /**
     * Show window.
     */
    public void showWindow() {
        subWindowView.setWidth(windowParam.width);
        subWindowView.setHeight(windowParam.height);
        subWindowView.setElevation(0);
        subWindowView.setContentView(contentView);

        int screenX = windowParam.x + mainWindowX;
        int screenY = windowParam.y + mainWindowY;
        Log.d(TAG, "showWindow called. x=" + windowParam.x + ", y=" + windowParam.y + ", screenX=" + screenX +
            ", screenY=" + screenY + ", width=" + windowParam.width + ", height=" + windowParam.height);
        subWindowView.showAtLocation(rootView, Gravity.TOP | Gravity.START, screenX, screenY);
        getFocus();
    }

    /**
     * Update window.
     */
    public void updateWindow() {
        int screenX = windowParam.x + mainWindowX;
        int screenY = windowParam.y + mainWindowY;

        Log.d(TAG, "updateWindow called. x=" + windowParam.x + ", y=" + windowParam.y + ", screenX=" + screenX +
            ", screenY=" + screenY + ", width=" + windowParam.width + ", height=" + windowParam.height);
        subWindowView.update(screenX, screenY, windowParam.width, windowParam.height, true);
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
     * Set window focusable.
     *
     * @param isFocusable the is focusable
     */
    public void setFocusable(boolean isFocusable) {
        subWindowView.setFocusable(isFocusable);
        this.isFocusable = isFocusable;
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
            Log.e(TAG, "not showing.");
            return false;
        }

        if (!this.isFocusable) {
            Log.e(TAG, "not focusable.");
            return false;
        }

        getFocus();
        return true;
    }

    /**
     * release focus.
     */
    public void getFocus() {
        Log.d(TAG, "getFocus(), isFocusable=" + this.isFocusable);
        if (this.isFocusable) {
            subWindowView.setFocusable(true);
            subWindowView.update();
        }
    }

    /**
     * release focus.
     */
    public void releaseFocus() {
        Log.d(TAG, "releaseFocus(), isFocusable=" + this.isFocusable);
        if (this.isFocusable) {
            subWindowView.setFocusable(false);
            subWindowView.update();
        }
    }

    /**
     * set window touch hot area.
     *
     * @param rectArray the rect array
     */
    public void setTouchHotArea(Rect[] rectArray) {
        Log.d(TAG, "setTouchHotArea(), rectArray:");
        for (Rect rect : rectArray) {
            Log.d(TAG, "left=" + rect.left + ", top=" + rect.top + ", right=" + rect.right + ", bottom=" + rect.bottom);
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
    }

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
    private Rect[] hotAreas;
    private boolean isFullScreen = false;
    private View mainView;
    private int mainWindowX;
    private int mainWindowY;

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
}
