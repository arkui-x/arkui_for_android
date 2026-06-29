/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
import android.hardware.display.DisplayManager;
import android.view.Display;
import android.graphics.Rect;

import androidx.window.layout.FoldingFeature;

import ohos.ace.adapter.ALog;

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
        ALog.d(TAG, "DisplayManagerAgent created.");
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
        ALog.d(TAG, "isFoldable called.");
        if (this.activity != null) {
            return FoldManager.isFoldable(this.activity);
        } else {
            ALog.e(TAG, "activity is null");
            return false;
        }
    }

    /**
     * get fold status of the device.
     *
     * @return the fold status
     */
    public int getFoldStatus() {
        ALog.d(TAG, "getFoldStatus called.");
        if (this.activity != null) {
            return FoldManager.getFoldStatus(this.activity);
        } else {
            ALog.e(TAG, "activity is null");
            return FoldInfo.FOLD_STATUS_UNKNOWN;
        }
    }

    /**
     * get all display ids.
     *
     * @return display id array
     */
    public int[] getAllDisplayIds() {
        if (displayManager == null) {
            return new int[0];
        }
        Display[] displays = displayManager.getDisplays();
        if (displays == null || displays.length == 0) {
            return new int[0];
        }
        int[] displayIds = new int[displays.length];
        for (int i = 0; i < displays.length; i++) {
            displayIds[i] = displays[i].getDisplayId();
        }
        return displayIds;
    }

    /**
     * get live crease region of current fold state.
     *
     * @return int array [left, top, width, height], empty when unsupported
     */
    public int[] getLiveCreaseRegion() {
        if (activity == null) {
            return new int[0];
        }
        FoldingFeature foldingFeature = FoldManager.getFoldingFeature(activity);
        if (foldingFeature == null) {
            return new int[0];
        }
        Rect bounds = foldingFeature.getBounds();
        if (bounds == null) {
            return new int[0];
        }
        if (bounds.width() < 0 || bounds.height() < 0) {
            return new int[0];
        }
        return new int[] {bounds.left, bounds.top, bounds.width(), bounds.height()};
    }

    /**
     * register display listener.
     */
    public void registerDisplayListener() {
        ALog.d(TAG, "registerDisplayListener called.");
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
                    ALog.d(TAG, "displayId = " + displayId);
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
        ALog.d(TAG, "unregisterDisplayListener called.");
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
