/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
import android.util.Log;
import android.hardware.display.DisplayManager;

/**
 * the display manager agent, a shared instance to get the fold info or connection info from the activity.
 *
 * @since 2025-05-22
 */

public class DisplayManagerAgent {
    private static final String TAG = "DisplayManagerAgent";
    private static volatile DisplayManagerAgent sInstance;

    private Activity activity;
    private DisplayManager displayManager;
    private DisplayManager.DisplayListener displayListener;

    private DisplayManagerAgent() {
        Log.d(TAG, "DisplayManagerAgent created.");
    }

    /**
     * Gets instance.
     *
     * @return the instance
     */
    public static final DisplayManagerAgent getInstance() {
        if (sInstance == null) {
            synchronized (DisplayManagerAgent.class) {
                if (sInstance == null) {
                    sInstance = new DisplayManagerAgent();
                }
            }
        }
        return sInstance;
    }

    /**
     * Sets activity.
     *
     * @param activity the Activity
     */
    public void setActivity(Activity activity) {
        this.activity = activity;
        displayManager = (DisplayManager) activity.getSystemService(Context.DISPLAY_SERVICE);
        nativeSetupDisplayManagerAgent();
    }

    /**
     * get foldable of the device.
     *
     * @return true: foldable, false: not foldable
     */
    public boolean isFoldable() {
        Log.d(TAG, "isFoldable called.");
        if (this.activity != null) {
            return FoldManager.isFoldable(this.activity);
        } else {
            Log.e(TAG, "activity is null");
            return false;
        }
    }

    /**
     * get fold status of the device.
     *
     * @return the fold status
     */
    public int getFoldStatus() {
        Log.d(TAG, "getFoldStatus called.");
        if (this.activity != null) {
            return FoldManager.getFoldStatus(this.activity);
        } else {
            Log.e(TAG, "activity is null");
            return FoldInfo.FOLD_STATUS_UNKNOWN;
        }
    }

    /**
     * register display listener.
     */
    public void registerDisplayListener() {
        Log.d(TAG, "onChange called.");
        if (displayListener == null) {
            displayListener = new DisplayManager.DisplayListener() {

                @Override
                public void onDisplayAdded(int displayId) {
                    nativeOnDisplayAdded(displayId);
                }

                @Override
                public void onDisplayRemoved(int displayId) {
                    nativeOnDisplayRemoved(displayId);
                }

                @Override
                public void onDisplayChanged(int displayId) {
                    Log.d(TAG, "displayId = " + displayId);
                    nativeOnDisplayChanged(displayId);
                }
            };
            displayManager.registerDisplayListener(displayListener, null);
        }

    }

    /**
     * unregister display listener.
     */
    public void unregisterDisplayListener() {
        Log.d(TAG, "offChange called.");
        if (displayListener != null) {
            displayManager.unregisterDisplayListener(displayListener);
            displayListener = null;
        }
    }

    private native void nativeSetupDisplayManagerAgent();
    private native void nativeOnDisplayAdded(int displayId);
    private native void nativeOnDisplayRemoved(int displayId);
    private native void nativeOnDisplayChanged(int displayId);
}
