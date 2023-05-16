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

import android.app.Activity;
import android.view.View;
import android.view.SurfaceView;
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
    public void createSubWindow(String name, int type, int mode, int tag, int parentId, int x, int y, int width, int height) {
        Log.d(TAG, "createSubWindow called: "
            + String.format(Locale.ENGLISH, "name=%s type=%d mode=%d tag=%d parentId=%d x=%d y=%d width=%d height=%d",
                        name, type, mode, tag, parentId, x, y, width, height));
        SubWindow subWindow = mSubWindowMap.get(name);
        if (subWindow == null) {
            subWindow = new SubWindow(mRootActivity, name);
            subWindow.createSubWindow(type, mode, tag, parentId, x, y, width, height);
            mSubWindowMap.put(name, subWindow);
        } else {
            Log.e(TAG, "SubWindow " + name + " already created.");
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
