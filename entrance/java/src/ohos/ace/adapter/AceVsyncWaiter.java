/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

import android.view.Choreographer;
import android.view.WindowManager;

/**
 * This class used to hanlde vsync signal.
 *
 * @since 1
 */
public class AceVsyncWaiter {
    private static final float ONE_SECOND_IN_NANO = 1000000000.0f;

    private static final float EPSILON = 0.001f;

    private static final Object LOCK = new Object();

    private static volatile AceVsyncWaiter instance;

    private static WindowManager windowManager;

    /**
     * Get instance of AceVsyncWaiter
     *
     * @param wm associated system window manager
     * @return the instance
     */
    public static AceVsyncWaiter getInstance(WindowManager wm) {
        if (instance == null) {
            synchronized (LOCK) {
                if (instance == null) {
                    windowManager = wm;
                    instance = new AceVsyncWaiter();
                }
            }
        }
        return instance;
    }

    /**
     * Wait for next vsync
     *
     * @param nativeRef native reference which calling
     */
    public static void waitForVsync(long nativeRef) {
        Choreographer.getInstance().postFrameCallback(new Choreographer.FrameCallback() {
            @Override
            public void doFrame(long frameTimeNanos) {
                float fps = windowManager.getDefaultDisplay().getRefreshRate();
                if (!isValid(fps)) {
                    return;
                }
                long refreshPeriodNanos = (long) (ONE_SECOND_IN_NANO / fps);
                getInstance(windowManager).nativeOnVsync(frameTimeNanos, frameTimeNanos + refreshPeriodNanos, nativeRef);
            }
        });
    }

    private AceVsyncWaiter() {
    }

    private static boolean isValid(float fps) {
        return fps > EPSILON;
    }

    private native void nativeOnVsync(long frameTimeNanos, long frameTargetTimeNanos, long nativeRef);
}
