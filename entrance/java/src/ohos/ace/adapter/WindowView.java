/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * This class is AceView implement and handles the lifecycle of surface.
 *
 * @since 1
 */
public class WindowView extends SurfaceView implements SurfaceHolder.Callback {
    private static final String LOG_TAG = "WindowView";

    private long nativeWindowPtr = 0L;

    private Surface delayNotifyCreateSurface = null;

    private boolean delayNotifySurfaceChanged = false;
    private int delayNotifyChangedWidth = 0;
    private int delayNotifyChangedHeight = 0;

    /**
     * Constructor of AceViewAosp
     *
     * @param context Application context
     */
    public WindowView(Context context) {
        super(context);
        initView();
    }

    private void initView() {
        ALog.i(LOG_TAG, "WindowView created");
        getHolder().addCallback(this);
    }

    /**
     * Called by native to register Window Handle.
     *
     * @param windowHandle the handle of navive window
     */
    public void registerWindow(long windowHandle) {
        nativeWindowPtr = windowHandle;

        if (delayNotifyCreateSurface != null) {
            nativeSurfaceCreated(nativeWindowPtr, delayNotifyCreateSurface);
            delayNotifyCreateSurface = null;
        }

        if (delayNotifySurfaceChanged) {
            nativeSurfaceChanged(nativeWindowPtr, delayNotifyChangedWidth, delayNotifyChangedHeight);
            delayNotifySurfaceChanged = false;
            delayNotifyChangedWidth = 0;
            delayNotifyChangedHeight = 0;
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        ALog.i(LOG_TAG, "surfaceCreated");
        Surface surface = holder.getSurface();
        if (nativeWindowPtr == 0L) {
            delayNotifyCreateSurface = surface;
        } else {
            nativeSurfaceCreated(nativeWindowPtr, surface);
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        ALog.i(LOG_TAG, "surface changed w=" + width + " h=" + height);

        if (nativeWindowPtr == 0L) {
            delayNotifyChangedWidth = width;
            delayNotifyChangedHeight = height;
        } else {
            nativeSurfaceChanged(nativeWindowPtr, width, height);
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        ALog.d(LOG_TAG, "surfaceDestroyed");
        if (nativeWindowPtr != 0L) {
            nativeSurfaceDestroyed(nativeWindowPtr);
        }
    }

    private native void nativeSurfaceCreated(long viewPtr, Surface surface);

    private native void nativeSurfaceChanged(long viewPtr, int width, int height);

    private native void nativeSurfaceDestroyed(long viewPtr);

}
