/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.platformview;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.view.Surface;
import android.widget.FrameLayout;

import ohos.ace.adapter.AceTextureHolder;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.IAceOnResourceEvent;
import ohos.ace.adapter.capability.platformview.IPlatformView;

import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

/**
 * This class handles the lifecycle of a PlatformView.
 *
 * @since 1
 */
public class AcePlatformViewAosp extends AcePlatformViewBase {
    private static final String LOG_TAG = "AcePlatformViewAosp";

    private static final String SUCCESS = "success";
    private static final String FAIL = "fail";
    private static final String KEY_TEXTUREID = "textureId";
    private static final String PLATFORM_VIEW_HEIGHT = "platformViewHeight";
    private static final String PLATFORM_VIEW_WIDTH = "platformViewWidth";
    private static final String PLATFORM_VIEW_TOP = "platformViewTop";
    private static final String PLATFORM_VIEW_LEFT = "platformViewLeft";
    private static final String PLATFORM_VIEW_TOUCH_POINT_OFFSET_X = "platformViewTouchPointOffsetX";
    private static final String PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y = "platformViewTouchPointOffsetY";
    private static final Object PLATFORM_VIEW_LOCK = new Object();

    private final Context context;
    private final Handler mainHandler;

    private PlatformViewWrapper viewWrapper;

    private Handler asyncHandler;
    private HandlerThread handlerThread;

    private long textureId = 0L;
    private AtomicLong viewId = new AtomicLong(0L);
    private IPlatformView platformView = null;
    private MotionEvent motionEvent;

    private int offsetX = 0;
    private int offsetY = 0;
    private int width = 0;
    private int height = 0;

    /**
     * constructor of PlatformView on AOSP platform
     *
     * @param id       the id of PlatformView
     * @param name     name of PlatformView
     * @param context  context of application
     * @param callback resource callback
     */
    public AcePlatformViewAosp(long id, Context context, IAceOnResourceEvent callback) {
        super(id, callback);

        this.context = context;
        mainHandler = new Handler(Looper.getMainLooper());
        handlerThread = new HandlerThread("platformview-" + id);
        handlerThread.start();
        Looper looper = handlerThread.getLooper();
        if (looper != null) {
            asyncHandler = new Handler(looper);
        } else {
            asyncHandler = new Handler();
        }
    }

    public void setPlatformView(IPlatformView platformView) {
        synchronized (PLATFORM_VIEW_LOCK) {
            this.platformView = platformView;
        }
    }

    @Override
    public void release() {
        runAsync(() -> {
            synchronized (PLATFORM_VIEW_LOCK) {
                if (platformView != null) {
                    runOnUIThread(() -> {
                        platformView.onDispose();
                        platformView = null;
                    });
                }
            }
        });
        ViewParent parent = viewWrapper.getParent();
        if (parent instanceof ViewGroup) {
            ViewGroup viewGroup = (ViewGroup) parent;
            viewGroup.removeView(viewWrapper);
        }
    }

    @Override
    public String registerPlatformView(Map<String, String> params) {
        if (params == null) {
            ALog.e(LOG_TAG, "registerPlatformView failed: params is null");
            return FAIL;
        }
        if (!params.containsKey(KEY_TEXTUREID)) {
            ALog.e(LOG_TAG, "registerPlatformView failed: Textureid is illegal");
            return FAIL;
        }

        try {
            this.textureId = Integer.parseInt(params.get(KEY_TEXTUREID));
        } catch (NumberFormatException ignored) {
            return FAIL;
        }

        Surface newSurface = getSurface();
        viewWrapper = new PlatformViewWrapper(this.context);
        viewWrapper.setSurface(newSurface);

        runOnUIThread(() -> {
            synchronized (PLATFORM_VIEW_LOCK) {
                if (platformView != null) {
                    final View embeddedView = platformView.getView();
                    if (embeddedView.getParent() != null) {
                        ViewGroup vg = (ViewGroup) embeddedView.getParent();
                        vg.removeView(embeddedView);
                    }
                    viewWrapper.addView(embeddedView);
                    viewWrapper.displayPlatformView();
                    platformViewReady();
                }
            }
            updateLayout();
        });
        return SUCCESS;
    }

    @Override
    public String updateLayout(Map<String, String> params) {
        if (params == null) {
            ALog.e(LOG_TAG, "updateLayout failed: params is null");
            return FAIL;
        }
        if (!params.containsKey(PLATFORM_VIEW_HEIGHT)) {
            ALog.e(LOG_TAG, "updateLayout failed: platformViewHeight is illegal");
            return FAIL;
        }
        if (!params.containsKey(PLATFORM_VIEW_WIDTH)) {
            ALog.e(LOG_TAG, "updateLayout failed: platformViewWidth is illegal");
            return FAIL;
        }
        if (!params.containsKey(PLATFORM_VIEW_TOP)) {
            ALog.e(LOG_TAG, "updateLayout failed: platformViewTop is illegal");
            return FAIL;
        }
        if (!params.containsKey(PLATFORM_VIEW_LEFT)) {
            ALog.e(LOG_TAG, "updateLayout failed: platformViewLeft is illegal");
            return FAIL;
        }

        try {
            offsetX = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_LEFT)));
            offsetY = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_TOP)));
            width = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_WIDTH)));
            height = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_HEIGHT)));

            runOnUIThread(() -> {
                updateLayout();
            });
        } catch (NumberFormatException e) {
            ALog.e(LOG_TAG, "NumberFormatException, setSurfaceSize failed");
            return FAIL;
        }
        return SUCCESS;
    }

    private int toPhysicalPixels(double logicalPixels) {
        float density = 1.0f;
        return (int) Math.round(logicalPixels * density);
    }

    private void updateLayout() {
        if (viewWrapper == null) {
            ALog.e(LOG_TAG, "updateLayout failed: viewWrapper is null");
            return;
        }
        viewWrapper.resize(width, height);
        final FrameLayout.LayoutParams viewWrapperLayoutParams = new FrameLayout.LayoutParams(width, height);
        viewWrapper.setLayoutParams(viewWrapperLayoutParams);
        synchronized (PLATFORM_VIEW_LOCK) {
            if (platformView != null) {
                final View embeddedView = platformView.getView();
                embeddedView.setLayoutParams(new FrameLayout.LayoutParams(width, height));
            }
        }
    }

    /**
     * Process touch event for platform view.
     */
    public void processTouchEvent() {
        MotionEvent eventClone = getMotionEvent();
        if (eventClone != null) {
            synchronized (PLATFORM_VIEW_LOCK) {
                if (platformView != null) {
                    platformView.getView().dispatchTouchEvent(eventClone);
                }
            }
        }
    }

    private MotionEvent getMotionEvent() {
        if (motionEvent == null) {
            return null;
        }
        if (offsetX == 0 && offsetY == 0) {
            return MotionEvent.obtain(motionEvent);
        }
        int pointerCount = motionEvent.getPointerCount();
        MotionEvent.PointerProperties[] pointerProperties = new MotionEvent.PointerProperties[pointerCount];
        MotionEvent.PointerCoords[] pointerCoords = new MotionEvent.PointerCoords[pointerCount];
        for (int i = 0; i < motionEvent.getPointerCount(); i++) {
            pointerProperties[i] = new MotionEvent.PointerProperties();
            motionEvent.getPointerProperties(i, pointerProperties[i]);

            MotionEvent.PointerCoords originCoords = new MotionEvent.PointerCoords();
            motionEvent.getPointerCoords(i, originCoords);

            pointerCoords[i] = new MotionEvent.PointerCoords(originCoords);
            pointerCoords[i].x -= offsetX;
            pointerCoords[i].y -= offsetY;
        }
        MotionEvent translatedEvent = MotionEvent.obtain(
                motionEvent.getDownTime(),
                motionEvent.getEventTime(),
                motionEvent.getAction(),
                motionEvent.getPointerCount(),
                pointerProperties,
                pointerCoords,
                motionEvent.getMetaState(),
                motionEvent.getButtonState(),
                motionEvent.getXPrecision(),
                motionEvent.getYPrecision(),
                motionEvent.getDeviceId(),
                motionEvent.getEdgeFlags(),
                motionEvent.getSource(),
                motionEvent.getFlags());
        return translatedEvent;
    }

    @Override
    public String touchDown(Map<String, String> params) {
        if (!params.containsKey(PLATFORM_VIEW_TOUCH_POINT_OFFSET_X)) {
            ALog.e(LOG_TAG, "touchDown failed: platformViewTouchPointOffsetX is illegal");
            return FAIL;
        }
        if (!params.containsKey(PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y)) {
            ALog.e(LOG_TAG, "touchDown failed: platformViewTouchPointOffsetX is illegal");
            return FAIL;
        }
        offsetX = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_TOUCH_POINT_OFFSET_X)));
        offsetY = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y)));
        processTouchEvent();
        return SUCCESS;
    }

    @Override
    public String touchUp(Map<String, String> params) {
        if (!params.containsKey(PLATFORM_VIEW_TOUCH_POINT_OFFSET_X)) {
            ALog.e(LOG_TAG, "touchUp failed: platformViewTouchPointOffsetX is illegal");
            return FAIL;
        }
        if (!params.containsKey(PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y)) {
            ALog.e(LOG_TAG, "touchUp failed: platformViewTouchPointOffsetX is illegal");
            return FAIL;
        }
        offsetX = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_TOUCH_POINT_OFFSET_X)));
        offsetY = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y)));
        processTouchEvent();
        return SUCCESS;
    }

    @Override
    public String touchMove(Map<String, String> params) {
        if (!params.containsKey(PLATFORM_VIEW_TOUCH_POINT_OFFSET_X)) {
            ALog.e(LOG_TAG, "touchMove failed: platformViewTouchPointOffsetX is illegal");
            return FAIL;
        }
        if (!params.containsKey(PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y)) {
            ALog.e(LOG_TAG, "touchMove failed: platformViewTouchPointOffsetX is illegal");
            return FAIL;
        }
        offsetX = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_TOUCH_POINT_OFFSET_X)));
        offsetY = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y)));
        processTouchEvent();
        return SUCCESS;
    }

    @Override
    public String touchCancel(Map<String, String> params) {
        if (!params.containsKey(PLATFORM_VIEW_TOUCH_POINT_OFFSET_X)) {
            ALog.e(LOG_TAG, "touchCancel failed: platformViewTouchPointOffsetX is illegal");
            return FAIL;
        }
        if (!params.containsKey(PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y)) {
            ALog.e(LOG_TAG, "touchCancel failed: platformViewTouchPointOffsetX is illegal");
            return FAIL;
        }
        offsetX = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_TOUCH_POINT_OFFSET_X)));
        offsetY = toPhysicalPixels(Double.parseDouble(params.get(PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y)));
        processTouchEvent();
        return SUCCESS;
    }

    @Override
    public String dispose(Map<String, String> params) {
        synchronized (PLATFORM_VIEW_LOCK) {
            if (platformView != null) {
                platformView.onDispose();
                platformView = null;
            }
        }
        return SUCCESS;
    }

    @Override
    public void onActivityPause() {
    }

    @Override
    public void onActivityResume() {
    }

    @Override
    public void setTouchEvent(MotionEvent event) {
        motionEvent = MotionEvent.obtain(event);
    }

    @Override
    public String runAsync(Runnable runnable) {
        asyncHandler.post(runnable);
        return SUCCESS;
    }

    private void runOnUIThread(Runnable task) {
        if (Looper.myLooper() == Looper.getMainLooper()) {
            task.run();
        } else {
            mainHandler.post(task);
        }
    }

    private Surface getSurface() {
        Surface surface = null;
        SurfaceTexture surfaceTexture = AceTextureHolder.getSurfaceTexture(textureId);
        if (surfaceTexture != null) {
            surface = new Surface(surfaceTexture);
        }
        return surface;
    }

    /**
     * Get atomic id
     *
     * @return atomic id
     */
    public long getAtomicId() {
        return viewId.getAndIncrement();
    }
}