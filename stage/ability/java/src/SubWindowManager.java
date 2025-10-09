/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

import android.app.ActionBar;
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.provider.Settings;
import android.view.Display;
import android.view.View;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;
import android.widget.PopupWindow;
import android.util.Log;
import android.os.Build;
import android.graphics.Rect;

import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

/**
 * The type Sub window manager.
 *
 * @since 2023-08-06
 */
public class SubWindowManager {
    /**
     * Gets ui options.
     */
    public static int uiOptionsStatic = View.SYSTEM_UI_FLAG_VISIBLE | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;

    private static final String TAG = "SubWindowManager";
    private static final int NO_HEIGHT = 0;
    private static final int LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES = 1;
    private static final int LOCATION_X = 0;
    private static final int LOCATION_Y = 1;
    private static final int LOCATION_SIZE = 2;
    private static final int API_28 = 28;
    private static final int API_29 = 29;
    private static final int API_30 = 30;

    /*
     ** copy from native wm_common.h: enum class Orientation
     */
    private static final int UNSPECIFIED = 0;
    private static final int VERTICAL = 1;
    private static final int HORIZONTAL = 2;
    private static final int REVERSE_VERTICAL = 3;
    private static final int REVERSE_HORIZONTAL = 4;
    private static final int UI_DARK_COLOR = 0x66000000;
    private static final int UI_LIGHT_COLOR = 0xE5FFFFFF;
    private static SubWindowManager sinstance;

    private Activity mRootActivity;
    private String mInstanceName;
    private Map<String, SubWindow> mSubWindowMap = new HashMap<>();
    private int uiOptions_ = View.SYSTEM_UI_FLAG_VISIBLE | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;

    private SubWindowManager() {
        Log.d(TAG, "SubWindowManager created.");
        nativeSetupSubWindowManager();
    }

    /**
     * Gets instance.
     *
     * @return the instance
     */
    public static final SubWindowManager getInstance() {
        if (sinstance == null) {
            synchronized (SubWindowManager.class) {
                if (sinstance == null) {
                    sinstance = new SubWindowManager();
                }
            }
        }

        return sinstance;
    }

    /**
     * Sets activity.
     *
     * @param activity      the activity
     * @param instanceName  the activity instance name.
     */
    public void setActivity(Activity activity, String instanceName) {
        Log.d(TAG, "setActivity called, instanceName: " + instanceName);
        mRootActivity = activity;
        mInstanceName = instanceName;
        if (mRootActivity != null) {
            Window window = mRootActivity.getWindow();
            View decorView = window.getDecorView();
            int option = decorView.getSystemUiVisibility();
            uiOptions_ = uiOptions_ | option;
            uiOptionsStatic = uiOptionsStatic | option;
        }
    }

    /**
     * Release activity.
     *
     * @param instanceName the activity instance name.
     */
    public void releaseActivity(String instanceName) {
        Log.d(TAG, "releaseActivity called, instanceName: " + instanceName + ", mInstanceName: " + mInstanceName);
        if (instanceName == mInstanceName) {
            mRootActivity = null;
        }
        return;
    }

    /**
     * Create sub window.
     *
     * @param name     the name
     * @param type     the type
     * @param mode     the mode
     * @param tag      the tag
     * @param parentId the parent id
     * @param x        the x
     * @param y        the y
     * @param width    the width
     * @param height   the height
     * @return the boolean
     */
    public boolean createSubWindow(String name, int type, int mode, int tag, int parentId, int x, int y, int width,
        int height) {
        Log.d(TAG,
            "createSubWindow called: " + String.format(Locale.ENGLISH,
                "name=%s type=%d mode=%d tag=%d parentId=%d x=%d y=%d width=%d height=%d", name, type, mode, tag,
                parentId, x, y, width, height));
        if (mRootActivity == null) {
            Log.e(TAG, "createSubWindow failed mRootActivity is invalid");
            return false;
        }
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow == null) {
            subWindow = new SubWindow(mRootActivity, name);
            subWindow.createSubWindow(type, mode, tag, parentId, x, y, width, height);
            mSubWindowMap.put(name, subWindow);
            return true;
        } else {
            Log.e(TAG, "SubWindow " + name + " already created.");
            return false;
        }
    }

    /**
     * Gets content view.
     *
     * @param name the name
     * @return the content view
     */
    public View getContentView(String name) {
        Log.d(TAG, "getContentView called. name=" + name);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow != null) {
            return subWindow.getContentView();
        }
        return null;
    }

    /**
     * Gets window id.
     *
     * @param name the name
     * @return the window id
     */
    public int getWindowId(String name) {
        Log.d(TAG, "getWindowId called. name=" + name);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow != null) {
            return subWindow.getWindowId();
        }
        return -1;
    }

    /**
     * Gets top window.
     *
     * @return the top window
     */
    public View getTopWindow() {
        Log.d(TAG, "getTopWindow called. ");
        if (mRootActivity != null) {
            View rootView = mRootActivity.getWindow().getDecorView();
            View topView = rootView.findFocus();
            if (topView instanceof SurfaceView) {
                return topView;
            }
        }
        return null;
    }

    /**
     * Show window boolean.
     *
     * @param name the name
     * @return the boolean
     */
    public boolean showWindow(String name) {
        Log.d(TAG, "showWindow called. name=" + name);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow != null) {
            subWindow.showWindow();
            if (subWindow.getSubWindowView().isShowing()) {
                return true;
            } else {
                Log.e(TAG, "showWindow failed due to not shown.");
                return false;
            }
        }
        Log.e(TAG, "not found SubWindow: " + name);
        return false;
    }

    /**
     * whether the sub-window is showing or not
     *
     * @param name the window name
     * @return the popup window is showing or not
     */
    public boolean isShowing(String name) {
        Log.d(TAG, "isShowing called. name=" + name);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow != null) {
            if (subWindow.getSubWindowView().isShowing()) {
                Log.e(TAG, "sub window is shown.");
                return true;
            } else {
                Log.e(TAG, "sub window is not shown.");
                return false;
            }
        }
        Log.e(TAG, "not found SubWindow: " + name);
        return false;
    }

    /**
     * Resize boolean.
     *
     * @param name   the name
     * @param width  the width
     * @param height the height
     * @return the boolean
     */
    public boolean resize(String name, int width, int height) {
        Log.d(TAG, "resize called. name=" + name + ", width=" + width + ", height=" + height);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow != null) {
            subWindow.resize(width, height);
            return true;
        }
        Log.e(TAG, "not found SubWindow: " + name);
        return false;
    }

    /**
     * Move window to boolean.
     *
     * @param name the name
     * @param x    the x
     * @param y    the y
     * @return the boolean
     */
    public boolean moveWindowTo(String name, int x, int y) {
        Log.d(TAG, "moveWindowTo called. name=" + name + ", x=" + x + ", y=" + y);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow != null) {
            subWindow.moveWindowTo(x, y);
            return true;
        }
        Log.e(TAG, "not found SubWindow: " + name);
        return false;
    }

    /**
     * Destroy window boolean.
     *
     * @param name the name
     * @return the boolean
     */
    public boolean destroyWindow(String name) {
        Log.d(TAG, "destroyWindow called. name=" + name);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow != null) {
            subWindow.destroyWindow();
            mSubWindowMap.remove(name);
            return true;
        }
        Log.e(TAG, "not found SubWindow: " + name);
        return false;
    }

    /**
     * Sets background color.
     *
     * @param color the color
     * @return the background color
     */
    public boolean setBackgroundColor(int color) {
        Log.d(TAG, "setBackgroundColor called: color=" + color);

        if (mRootActivity != null) {
            mRootActivity.getCurrentFocus().setBackgroundColor(color);
            return true;
        }
        return false;
    }

    /**
     * Sets app screen brightness.
     *
     * @param birghtessValue the birghtess value
     * @return the app screen brightness
     */
    public boolean setAppScreenBrightness(float birghtessValue) {
        Log.d(TAG, "setAppScreenBrightness called: birghtessValue=" + birghtessValue);

        if (mRootActivity != null) {
            Window window = mRootActivity.getWindow();
            WindowManager.LayoutParams lp = window.getAttributes();
            lp.screenBrightness = birghtessValue;
            window.setAttributes(lp);
            return true;
        }
        return false;
    }

    /**
     * get app screen brightness.
     *
     * @return the app screen brightness
     */
    public float getAppScreenBrightness() {
        if (mRootActivity == null) {
            Log.e(TAG, "getAppScreenBrightness failed, mRootActivity is null");
            return 0.0f;
        }
        Window window = mRootActivity.getWindow();
        WindowManager.LayoutParams wlp = window.getAttributes();

        if (wlp != null) {
            if (wlp.screenBrightness != WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_NONE) {
                Log.d(TAG, "getAppScreenBrightness called: brightnessValue=" + wlp.screenBrightness);
                return wlp.screenBrightness;
            } else {
                Log.e(TAG, "getAppScreenBrightness is not set, use system brightness");
            }
        }
        int systemBrightness = 0;
        try {
            systemBrightness =
                Settings.System.getInt(mRootActivity.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS);
        } catch (Settings.SettingNotFoundException exception) {
            Log.e(TAG, "getAppScreenBrightness is not set, query system brightness exception");
        }
        float appBrightness = systemBrightness / 255f;
        Log.d(TAG, "getAppScreenBrightness called: system brightnessValue=" + appBrightness);
        return appBrightness;
    }

    /**
     * Sets keep screen on.
     *
     * @param keepScreenOn the keep screen on
     * @return the keep screen on
     */
    public boolean setKeepScreenOn(boolean keepScreenOn) {
        Log.d(TAG, "setKeepScreenOn called: keepScreenOn=" + keepScreenOn);

        if (mRootActivity != null) {
            Window window = mRootActivity.getWindow();
            if (keepScreenOn) {
                window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            } else {
                window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            }

            return true;
        }
        return false;
    }

    /**
     * Is keep screen on boolean.
     *
     * @return the boolean
     */
    public boolean isKeepScreenOn() {
        Log.d(TAG, "isKeepScreenOn called");

        if (mRootActivity != null) {
            Window window = mRootActivity.getWindow();
            WindowManager.LayoutParams lp = window.getAttributes();
            if ((lp.flags & WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON) != 0) {
                return true;
            }
        }
        return false;
    }

    /**
     * Request orientation boolean.
     *
     * @param direction the screen direction
     * @return the boolean
     */
    public boolean requestOrientation(int direction) {
        Log.d(TAG, "requestOrientation called: direction=" + direction);

        if (mRootActivity != null) {
            int orientation = ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;

            switch (direction) {
                case UNSPECIFIED:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
                break;
                case VERTICAL:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
                    break;
                case HORIZONTAL:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                    break;
                case REVERSE_VERTICAL:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;
                    break;
                case REVERSE_HORIZONTAL:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                    break;
                default:
                    Log.e(TAG, "unspecified orientation: " + orientation);
                    break;
            }

            mRootActivity.setRequestedOrientation(orientation);
            return true;
        }
        return false;
    }

    private void setLayoutInDisplayCutoutMode(Window window) {
        if (window != null) {
            WindowManager.LayoutParams layoutParams = window.getAttributes();
            try {
                Class<?> layoutParamsClass = Class.forName("android.view.WindowManager$LayoutParams");
                Field displayCutoutModeField = layoutParamsClass.getDeclaredField("layoutInDisplayCutoutMode");
                displayCutoutModeField.setAccessible(true);
                displayCutoutModeField.setInt(layoutParams, LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES);
                window.setAttributes(layoutParams);
            } catch (ClassNotFoundException e) {
                Log.e(TAG, "layoutParamsClass Class.forName failed, ClassNotFoundException.");
            } catch (IllegalAccessException e) {
                Log.e(TAG, "layoutParamsClass AccessException failed, IllegalAccessException.");
            } catch (NoSuchFieldException e) {
                Log.e(TAG, "layoutParamsClass getDeclaredField failed, NoSuchFieldException.");
            }
        } else {
            Log.e(TAG, "setLayoutInDisplayCutoutMode failed.");
            return;
        }
    }

    /**
     * Maintain the previous state after the application hot start.
     *
     * @param stageActivity The activity to be maintained.
     */
    public static void keepSystemUiVisibility(Activity stageActivity) {
        if (stageActivity != null && Build.VERSION.SDK_INT < API_30) {
            Window window = stageActivity.getWindow();
            View decorView = window.getDecorView();
            decorView.setSystemUiVisibility(uiOptionsStatic);
        }
    }

    /**
     * Set the system UI visibility.
     *
     * @return Setting successful or failed.
     */
    private boolean setSystemUiVisibilityInner() {
        if (mRootActivity != null) {
            Window window = mRootActivity.getWindow();
            if (Build.VERSION.SDK_INT >= API_28) {
                setLayoutInDisplayCutoutMode(window);
            }
            View decorView = window.getDecorView();
            decorView.setSystemUiVisibility(this.uiOptions_);
            uiOptionsStatic = uiOptions_;
            return true;
        } else {
            return false;
        }
    }

    /**
     * Set window layout to fullScreen.
     *
     * @param isFullScreen true or false.
     * @return Setting successful or failed.
     */
    public boolean setWindowLayoutFullScreen(boolean isFullScreen) {
        boolean result = false;
        if (isFullScreen) {
            this.uiOptions_ |= View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;
        } else {
            this.uiOptions_ &= ~View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;
        }
        result = setSystemUiVisibilityInner();
        return result;
    }

    /**
     * Set navigation bar status.
     *
     * @param status Hide or show.
     * @return Setting successful or failed.
     */
    public boolean setNavigationBarStatus(boolean status) {
        boolean result = false;
        if (status) {
            this.uiOptions_ |= View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
        } else {
            this.uiOptions_ &= ~View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
        }
        result = setSystemUiVisibilityInner();
        return result;
    }

    /**
     * Set navigationIndicator bar status.
     *
     * @param status Hide or show.
     * @return Setting successful or failed.
     */
    public boolean setNavigationIndicatorStatus(boolean status) {
        if (Build.VERSION.SDK_INT >= API_29) {
            return setNavigationBarStatus(status);
        } else {
            Log.e(TAG, "Not supported by the Android version.");
            return false;
        }
    }

    /**
     * Set status bar status.
     *
     * @param status Hide or show.
     * @return Setting successful or failed.
     */
    public boolean setStatusBarStatus(boolean status) {
        boolean result = false;
        if (status) {
            this.uiOptions_ |= View.SYSTEM_UI_FLAG_FULLSCREEN;
        } else {
            this.uiOptions_ &= ~View.SYSTEM_UI_FLAG_FULLSCREEN;
        }
        result = setSystemUiVisibilityInner();
        return result;
    }

    /**
     * Sets status bar and action bar status.
     *
     * @param hide the hide
     * @return the action bar status
     */
    public boolean setActionBarStatus(boolean hide) {
        Log.d(TAG, "setActionBarStatus called: hide=" + hide);

        if (mRootActivity != null) {
            Window window = mRootActivity.getWindow();
            View decorView = window.getDecorView();
            ActionBar actionBar = mRootActivity.getActionBar();
            if (hide) {
                if (actionBar != null && actionBar.isShowing()) {
                    actionBar.hide();
                }
                return true;
            } else {
                int uiOptions = View.SYSTEM_UI_FLAG_VISIBLE;
                decorView.setSystemUiVisibility(uiOptions);

                if (actionBar != null && !actionBar.isShowing()) {
                    actionBar.show();
                }
                return true;
            }
        }
        return false;
    }

    /**
     * Sets on dismiss listener.
     *
     * @param name     the name
     * @param listener the listener
     * @return the on dismiss listener
     */
    public boolean setOnDismissListener(String name, PopupWindow.OnDismissListener listener) {
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow != null) {
            subWindow.getSubWindowView().setOnDismissListener(listener);
            return true;
        }
        return false;
    }

    /**
     * Sets touch interceptor.
     *
     * @param name     the name
     * @param listener the listener
     * @return the touch interceptor
     */
    public boolean setTouchInterceptor(String name, View.OnTouchListener listener) {
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow != null) {
            subWindow.getSubWindowView().setTouchInterceptor(listener);
            return true;
        }
        return false;
    }

    /**
     * Get Screen rotation direction.
     *
     * @return Orientation type.
     */
    public int getScreenOrientation() {
        int result = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
        if (mRootActivity == null) {
            Log.e(TAG, "The mRootActivity is null, getScreenOrientation failed.");
            return result;
        }
        WindowManager windowManager = (WindowManager) mRootActivity.getSystemService(mRootActivity.WINDOW_SERVICE);
        if (windowManager == null) {
            Log.e(TAG, "The windowManager is null, getScreenOrientation failed.");
            return result;
        }
        Display display = windowManager.getDefaultDisplay();
        if (display == null) {
            Log.e(TAG, "The display is null, getScreenOrientation failed.");
            return result;
        }
        int rotation = display.getRotation();
        int orientation = mRootActivity.getResources().getConfiguration().orientation;
        if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
            if (rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_90) {
                result = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
            } else {
                result = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
            }
        }
        return result;
    }

    /**
     * Get screen display safety area.
     *
     * @return Safe Area.
     */
    public Rect getSafeArea() {
        Rect rect = new Rect(0, 0, 0, 0);
        if (mRootActivity != null && mRootActivity instanceof StageActivity) {
            int instanceId = ((StageActivity) mRootActivity).getInstanceId();
            int width = mRootActivity.getWindow().getDecorView().findViewById(android.R.id.content).getWidth();
            int height = mRootActivity.getWindow().getDecorView().findViewById(android.R.id.content).getHeight();
            int[] location = new int[LOCATION_SIZE];
            mRootActivity.getWindow().getDecorView().findViewById(android.R.id.content).getLocationOnScreen(location);
            int x = location[LOCATION_X];
            int y = location[LOCATION_Y];
            rect.set(x, y, width, height);
        } else {
            Log.e(TAG, "The mRootActivity is null or of the wrong type, getSafeArea failed.");
        }
        return rect;
    }

    /**
     * Check if the system UI is hidden.
     *
     * @param flag The system UI flag.
     * @return True if the system UI is hidden, false otherwise.
     */
    private boolean systemUIStatus(int flag) {
        boolean result = true;
        if (mRootActivity != null) {
            Window window = mRootActivity.getWindow();
            if (window == null) {
                Log.e(TAG, "Window is null for activity: " + mRootActivity.getClass().getSimpleName());
                return result;
            }

            View decorView = window.getDecorView();
            if (decorView == null) {
                Log.e(TAG, "DecorView is null for activity: " + mRootActivity.getClass().getSimpleName());
                return result;
            }

            int flags = decorView.getSystemUiVisibility();
            result = (flags & flag) == 0;
        } else {
            Log.e(TAG, "mRootActivity is null");
        }

        return result;
    }

    /**
     * Get the height of the StatusBar.
     *
     * @return Statusbar height.
     */
    public int getStatusBarHeight() {
        int result = NO_HEIGHT;
        if (mRootActivity != null) {
            int resourceId = mRootActivity.getResources().getIdentifier("status_bar_height", "dimen", "android");
            if (resourceId > NO_HEIGHT) {
                result = mRootActivity.getResources().getDimensionPixelSize(resourceId);
            }
        } else {
            Log.e(TAG, "The mRootActivity is null, getStatusBarHeight failed.");
        }
        return result;
    }

    /**
     * Get the height of the CutoutBar.
     *
     * @return CutoutBar height.
     */
    public int getCutoutBarHeight() {
        if (Build.VERSION.SDK_INT >= API_28) {
            return getStatusBarHeight();
        } else {
            Log.e(TAG, "Not supported by the Android version.");
            return NO_HEIGHT;
        }
    }

    /**
     * Get the NavigationBar status.
     *
     * @return True indicates show, false indicates hide.
     */
    public boolean getNavigationBarStatus() {
        return systemUIStatus(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
    }

    /**
     * Get the GestureBar status.
     *
     * @return True indicates show, false indicates hide.
     */
    public boolean getNavigationIndicatorStatus() {
        if (Build.VERSION.SDK_INT >= API_29) {
            return systemUIStatus(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
        } else {
            return false;
        }
    }

    /**
     * Hide window boolean.
     *
     * @param name the name
     * @return the boolean
     */
    public boolean hide(String name) {
        Log.d(TAG, "hide called. name=" + name);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow == null) {
            Log.e(TAG, "not found SubWindow: " + name);
            return false;
        }

        if (!subWindow.getSubWindowView().isShowing()) {
            Log.e(TAG, "not showing.");
            return false;
        }

        subWindow.destroyWindow();
        if (subWindow.getSubWindowView().isShowing()) {
            Log.e(TAG, "hide failed.");
            return false;
        }
        return true;
    }

    /**
     * set window focusable boolean.
     *
     * @param name   the name
     * @param isFocusable  if focusable
     * @return the boolean
     */
    public boolean setFocusable(String name, boolean isFocusable) {
        Log.d(TAG, "setFocusable called. name=" + name + ", isFocusable=" + isFocusable);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow == null) {
            Log.e(TAG, "not found SubWindow: " + name);
            return false;
        }

        subWindow.setFocusable(isFocusable);
        if (subWindow.getSubWindowView().isShowing()) {
            subWindow.getSubWindowView().update();
        }
        return true;
    }

    /**
     * set window touchable boolean.
     *
     * @param name   the name
     * @param isTouchable  if touchable
     * @return the boolean
     */
    public boolean setTouchable(String name, boolean isTouchable) {
        Log.d(TAG, "setTouchable called. name=" + name + ", isFocusable=" + isTouchable);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow == null) {
            Log.e(TAG, "not found SubWindow: " + name);
            return false;
        }

        subWindow.setTouchable(isTouchable);
        if (subWindow.getSubWindowView().isTouchable() != isTouchable) {
            Log.e(TAG, "setTouchable to " + isTouchable + " failed.");
            return false;
        }

        if (subWindow.getSubWindowView().isShowing()) {
            subWindow.getSubWindowView().update();
        }
        return true;
    }

    /**
     * request focus for window.
     *
     * @param name the name
     * @return the boolean
     */
    public boolean requestFocus(String name) {
        Log.d(TAG, "requestFocus called. name=" + name);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow == null) {
            Log.e(TAG, "not found SubWindow: " + name);
            return false;
        }

        return subWindow.requestFocus();
    }

    /**
     * set window touch hot area.
     *
     * @param name   the name
     * @param rectArray  list of hot area
     * @return the boolean
     */
    public boolean setTouchHotArea(String name, Rect[] rectArray) {
        Log.d(TAG, "setTouchHotArea called. name=" + name);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow == null) {
            Log.e(TAG, "not found SubWindow: " + name);
            return false;
        }

        subWindow.setTouchHotArea(rectArray);
        return true;
    }

    /**
     * Set fullScreen and hide systembar.
     *
     * @param name   the name
     * @param status true or false.
     * @return Setting successful or failed.
     */
    public boolean setFullScreen(String name, boolean status) {
        Log.d(TAG, "setFullScreen called. name=" + name);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow == null) {
            Log.e(TAG, "not found SubWindow: " + name);
            return false;
        }

        subWindow.setFullScreen(status);
        return true;
    }

    /**
     * set window on top.
     *
     * @param name   the name
     * @param status  if on top
     * @return the boolean
     */
    public boolean setOnTop(String name, boolean status) {
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow == null) {
            Log.e(TAG, "not found SubWindow: " + name);
            return false;
        }

        subWindow.setOnTop(status);
        return true;
    }

    /**
     * Called by native to register Window Handle.
     *
     * @param name   the name
     * @param subWindowHandle the handle of navive window
     * @return the boolean
     */
    public boolean registerSubWindow(String name, long subWindowHandle) {
        Log.d(TAG, "registerSubWindow called. name=" + name);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow == null) {
            Log.e(TAG, "not found SubWindow: " + name);
            return false;
        }

        subWindow.registerSubWindow(subWindowHandle);
        return true;
    }

    /**
     * Called by native to unregister Window Handle.
     *
     * @param name   the name
     * @return the boolean
     */
    public boolean unregisterSubWindow(String name) {
        Log.d(TAG, "unRegisterSubWindow called. name=" + name);
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow == null) {
            Log.e(TAG, "not found SubWindow: " + name);
            return false;
        }

        subWindow.unregisterSubWindow();
        return true;
    }

    /**
     * Set status bar property.
     *
     * @param backgroundColor Status bar background color.
     * @param contentColor Status bar content color.
     * @return Setting successful or failed.
     */
    public boolean setStatusBar(int backgroundColor, int contentColor) {
        Log.d(TAG, "setStatusBar called: backgroundColor=" + backgroundColor + ", contentColor=" + contentColor);

        if (mRootActivity != null) {
            Window window = mRootActivity.getWindow();
            View decorView = window.getDecorView();
            if (contentColor == UI_DARK_COLOR) {
                this.uiOptions_ |= View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR; // dark color
            } else if (contentColor == UI_LIGHT_COLOR) {
                this.uiOptions_ &= ~View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR; // light color
            }
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            decorView.setSystemUiVisibility(this.uiOptions_);
            window.setStatusBarColor(backgroundColor);
            return true;
        }
        return false;
    }

    /**
     * Set navigation bar property.
     *
     * @param backgroundColor Navigation bar background color.
     * @param contentColor Navigation bar content color.
     * @return Setting successful or failed.
     */
    public boolean setNavigationBar(int backgroundColor, int contentColor) {
        Log.d(TAG, "setNavigationBar called: backgroundColor=" + backgroundColor + ", contentColor=" + contentColor);

        if (mRootActivity != null) {
            Window window = mRootActivity.getWindow();
            if (window == null) {
                return false;
            }
            View decorView = window.getDecorView();
            if (decorView == null) {
                return false;
            }
            if (contentColor == UI_DARK_COLOR) {
                this.uiOptions_ |= View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR; // dark color
            } else if (contentColor == UI_LIGHT_COLOR) {
                this.uiOptions_ &= ~View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR; // light color
            }
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            decorView.setSystemUiVisibility(this.uiOptions_);
            window.setNavigationBarColor(backgroundColor);
            return true;
        }
        return false;
    }

    /**
     * Sets whether is private mode or not.
     *
     * @param isPrivacyMode privacy mode
     * @return Set privacy mode success or not
     */
    public boolean setWindowPrivacyMode(boolean isPrivacyMode) {
        Log.d(TAG, "setWindowPrivacyMode called: windowPrivacyMode=" + isPrivacyMode);

        if (mRootActivity != null) {
            Window window = mRootActivity.getWindow();
            if (window != null) {
                if (isPrivacyMode) {
                    window.addFlags(WindowManager.LayoutParams.FLAG_SECURE);
                } else {
                    window.clearFlags(WindowManager.LayoutParams.FLAG_SECURE);
                }
                return true;
            }
        }
        return false;
    }

    /**
     * Get screen display surface area.
     *
     * @return Surface Area.
     */
    public Rect getSurfaceRect() {
        Rect rect = new Rect();
        if (mRootActivity != null && mRootActivity instanceof StageActivity) {
            View decorView = mRootActivity.getWindow().getDecorView();
            int[] location = new int[LOCATION_SIZE];
            decorView.getLocationOnScreen(location);
            int x = location[LOCATION_X];
            int y = location[LOCATION_Y];
            rect.set(x, y, decorView.getWidth(), decorView.getHeight());
        } else {
            Log.e(TAG, "mRootActivity is null");
        }
        return rect;
    }

    private native void nativeSetupSubWindowManager();
}
