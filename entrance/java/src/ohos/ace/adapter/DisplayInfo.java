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

import android.content.Context;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.WindowManager;
import android.util.Log;

public class DisplayInfo {

    private static final String TAG = "DisplayInfo";
    private static volatile DisplayInfo _sinstance;

    public static final DisplayInfo getInstance() {
        if(_sinstance == null) {
            synchronized (DisplayInfo.class) {
                if(_sinstance == null) {
                    _sinstance = new DisplayInfo();
                }
            }
        }

        return _sinstance;
    }

    private Context mContext;
    private WindowManager mWindowManager;

    private DisplayInfo () {
        Log.d(TAG, "DisplayInfo created.");
    }

    public void setContext(Context context) {
        mContext = context;
        mWindowManager = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
        nativeSetupDisplayInfo();
    }

    public int getDisplayId() {
        Log.d(TAG, "getDisplayId called.");
        Display defaultDisplay = mWindowManager.getDefaultDisplay();
        return defaultDisplay.getDisplayId();
    }

    public int getOrentation() {
        Log.d(TAG, "getOrentation called.");
        Display defaultDisplay = mWindowManager.getDefaultDisplay();
        return defaultDisplay.getRotation();
    }

    public int getWidth() {
        Log.d(TAG, "getWidth called.");
        DisplayMetrics metrics = new DisplayMetrics();
        Display defaultDisplay = mWindowManager.getDefaultDisplay();
        defaultDisplay.getRealMetrics(metrics);
        return metrics.widthPixels;
    }

    public int getHeight() {
        Log.d(TAG, "getHeight called.");
        DisplayMetrics metrics = new DisplayMetrics();
        Display defaultDisplay = mWindowManager.getDefaultDisplay();
        defaultDisplay.getRealMetrics(metrics);
        return metrics.heightPixels;
    }

    public float getRefreshRate() {
        Log.d(TAG, "getRefreshRate called.");
        Display defaultDisplay = mWindowManager.getDefaultDisplay();
        return defaultDisplay.getRefreshRate();
    }

    private native void nativeSetupDisplayInfo();
}
