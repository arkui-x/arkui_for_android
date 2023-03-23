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

    private int surfaceWidth = 0;
    private int surfaceHeight = 0;

    private Surface delayNotifyCreateSurface = null;
    private boolean delayNotifySurfaceChanged = false;
    private boolean delayNotifySurfaceDestroyed = false;

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
        delayNotifyIfNeeded();
    }

    private void delayNotifyIfNeeded() {
        if (nativeWindowPtr == 0L) {
            ALog.e(LOG_TAG, "delay notify, nativeWindow is invalid!");
            return;
        }

        if (delayNotifyCreateSurface != null) {
            ALog.i(LOG_TAG, "delay notify surfaceCreated");
            nativeSurfaceCreated(nativeWindowPtr, delayNotifyCreateSurface);
            delayNotifyCreateSurface = null;
        }

        if (delayNotifySurfaceChanged) {
            ALog.i(LOG_TAG,
                    "delay notify surface changed w=" + surfaceWidth + " h=" + surfaceHeight);
            nativeSurfaceChanged(nativeWindowPtr, surfaceWidth, surfaceHeight);
            delayNotifySurfaceChanged = false;
        }

        if (delayNotifySurfaceDestroyed) {
            ALog.i(LOG_TAG, "delay notify surfaceDestroyed");
            nativeSurfaceDestroyed(nativeWindowPtr);
            delayNotifySurfaceDestroyed = false;
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Surface surface = holder.getSurface();
        if (nativeWindowPtr == 0L) {
            ALog.w(LOG_TAG, "surfaceCreated nativeWindow not ready, delay notify");
            delayNotifyCreateSurface = surface;
        } else {
            ALog.i(LOG_TAG, "surfaceCreated");
            nativeSurfaceCreated(nativeWindowPtr, surface);
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        surfaceWidth = width;
        surfaceHeight = height;
        if (nativeWindowPtr == 0L) {
            ALog.w(LOG_TAG, "surfaceChanged nativeWindow not ready, delay notify");
            delayNotifySurfaceChanged = true;
        } else {
            ALog.i(LOG_TAG, "surfaceChanged w=" + surfaceWidth + " h=" + surfaceHeight);
            nativeSurfaceChanged(nativeWindowPtr, surfaceWidth, surfaceHeight);
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (nativeWindowPtr == 0L) {
            ALog.w(LOG_TAG, "surfaceDestroyed nativeWindow not ready, delay notify");
            delayNotifySurfaceDestroyed = true;
        } else {
            ALog.i(LOG_TAG, "surfaceDestroyed");
            nativeSurfaceDestroyed(nativeWindowPtr);
        }
    }

    private native void nativeSurfaceCreated(long viewPtr, Surface surface);

    private native void nativeSurfaceChanged(long viewPtr, int width, int height);

    private native void nativeSurfaceDestroyed(long viewPtr);

}
