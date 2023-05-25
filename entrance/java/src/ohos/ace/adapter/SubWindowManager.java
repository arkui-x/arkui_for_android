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
package ohos.ace.adapter;

import android.app.ActionBar;
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.provider.Settings;
import android.view.View;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;
import android.widget.PopupWindow;
import android.util.Log;

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

/**
 * The type Sub window manager.
 */
public class SubWindowManager {

    private static final String TAG = "SubWindowManager";

    private Activity mRootActivity;
    private Map<String, SubWindow> mSubWindowMap = new HashMap<>();
    private static SubWindowManager _sinstance;

    /*
     ** copy from native wm_common.h: enum class Orientation
     */
    private static final int VERTICAL = 1;
    private static final int HORIZONTAL = 2;
    private static final int REVERSE_VERTICAL = 3;
    private static final int REVERSE_HORIZONTAL = 4;

    /**
     * Gets instance.
     *
     * @return the instance
     */
    public static final SubWindowManager getInstance() {
        if (_sinstance == null) {
            synchronized (SubWindowManager.class) {
                if (_sinstance == null) {
                    _sinstance = new SubWindowManager();
                }
            }
        }

        return _sinstance;
    }

    private SubWindowManager() {
        Log.d(TAG, "SubWindowManager created.");
        nativeSetupSubWindowManager();
    }

    /**
     * Sets activity.
     *
     * @param activity the activity
     */
    public void setActivity(Activity activity) {
        Log.d(TAG, "setActivity called.");
        mRootActivity = activity;
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
     */
    public boolean createSubWindow(String name, int type, int mode, int tag, int parentId, int x, int y, int width,
        int height) {
        Log.d(TAG,
            "createSubWindow called: " + String.format(Locale.ENGLISH,
                "name=%s type=%d mode=%d tag=%d parentId=%d x=%d y=%d width=%d height=%d", name, type, mode, tag,
                parentId, x, y, width, height));
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
        Log.d(TAG, "getWindowId called. ");
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
            if (subWindow.getSubWindowView().isShowing()) {
                subWindow.resize(width, height);
                return true;
            } else {
                Log.e(TAG, "resize failed due to not shown.");
                return false;
            }
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
            if (subWindow.getSubWindowView().isShowing()) {
                subWindow.moveWindowTo(x, y);
                return true;
            } else {
                Log.e(TAG, "moveWindowTo failed due to not shown.");
                return false;
            }
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
            if (subWindow.getSubWindowView().isShowing()) {
                subWindow.destroyWindow();
                mSubWindowMap.remove(name);
                return true;
            } else {
                Log.e(TAG, "destroyWindow failed due to not shown.");
                return false;
            }
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
            window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
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
                case VERTICAL:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
                    break;
                case HORIZONTAL:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                    break;
                case REVERSE_VERTICAL:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;
                    break;
                case REVERSE_HORIZONTAL:
                    orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
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

    /**
     * Sets status bar and action bar status.
     *
     * @param hide the hide
     * @return the status bar status
     */
    public boolean setStatusBarStatus(boolean hide) {
        Log.d(TAG, "setStatusBarStatus called: hide=" + hide);

        if (mRootActivity != null) {
            Window window = mRootActivity.getWindow();
            View decorView = window.getDecorView();
            ActionBar actionBar = mRootActivity.getActionBar();
            if (hide) {
                // Hide the status bar.
                int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN;
                decorView.setSystemUiVisibility(uiOptions);
                // Remember that you should never show the action bar if the
                // status bar is hidden, so hide that too if necessary.
                if (actionBar != null && actionBar.isShowing()) {
                    actionBar.hide();
                }
                return true;
            } else {
                int uiOptions = View.SYSTEM_UI_FLAG_VISIBLE;
                decorView.setSystemUiVisibility(uiOptions);
                return true;
            }
        }
        return false;
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

    private native void nativeSetupSubWindowManager();
}
