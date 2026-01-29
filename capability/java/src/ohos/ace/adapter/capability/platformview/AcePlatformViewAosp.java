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
import android.graphics.Matrix;
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
import java.math.BigDecimal;
import java.math.MathContext;
import java.math.RoundingMode;
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
    private static final String X = "X";
    private static final String Y = "Y";
    private static final String Z = "Z";
    private static final String ANGLE = "angle";
    private static final String CENTER_X = "centerX";
    private static final String CENTER_Y = "centerY";
    private static final String PERSPECTIVE = "perspective";
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
    private float scaleX = 1.0f;
    private float scaleY = 1.0f;
    private float scaleZ = 1.0f;
    private float axisX = 0.0f;
    private float axisY = 0.0f;
    private float axisZ = 0.0f;
    private float rotationX = 0.0f;
    private float rotationY = 0.0f;
    private float rotationZ = 0.0f;
    private float rotationAngle = 0.0f;
    private float translationX = 0.0f;
    private float translationY = 0.0f;
    private float translationZ = 0.0f;
    private String centerX = "50%";
    private String centerY = "50%";
    private String perspective = "1";
    private float[] transformMatrix;

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
        if (viewWrapper == null) {
            viewWrapper = new PlatformViewWrapper(this.context);
        }
        if (params.containsKey(KEY_TEXTUREID)) {
            try {
                this.textureId = Integer.parseInt(params.get(KEY_TEXTUREID));
            } catch (NumberFormatException ignored) {
                return FAIL;
            }
            Surface newSurface = getSurface();
            viewWrapper.setSurface(newSurface);
        }

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
        if (viewWrapper == null) {
            viewWrapper = new PlatformViewWrapper(this.context);
        }
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
        updateMatrix();
        viewWrapper.resize(width, height);
        final FrameLayout.LayoutParams viewWrapperLayoutParams = new FrameLayout.LayoutParams(width, height);
        viewWrapperLayoutParams.leftMargin = offsetX;
        viewWrapperLayoutParams.topMargin = offsetY;
        viewWrapper.setLayoutParams(viewWrapperLayoutParams);
        synchronized (PLATFORM_VIEW_LOCK) {
            if (platformView != null) {
                final View embeddedView = platformView.getView();
                embeddedView.setLayoutParams(new FrameLayout.LayoutParams(width, height));
            }
        }
    }

    private float calculateCameraDistance() {
        float density = context.getResources().getDisplayMetrics().density;
        BigDecimal dist = BigDecimal.valueOf(Math.sqrt(width * width + height * height) * 0.5f * density);
        return dist.floatValue() * 4;
    }

    private void updateMatrix() {
        if (viewWrapper == null) {
            viewWrapper = new PlatformViewWrapper(this.context);
        }
        float pivotX = viewWrapper.getPivotX();
        float pivotY = viewWrapper.getPivotY();

        pivotX = centerX.contains("%") ? width * (Float.parseFloat(centerX.replace("%", "")) / 100)
                : Float.parseFloat(centerX);
        pivotY = centerY.contains("%") ? height * (Float.parseFloat(centerY.replace("%", "")) / 100)
                : Float.parseFloat(centerY);

        float perspectiveValue = 0.0f;
        try {
            perspectiveValue = Float.parseFloat(perspective);
        } catch (NumberFormatException ignored) {
            perspectiveValue = 0.0f;
        }

        BigDecimal cameraDistance;
        int densityDpi = context.getResources().getDisplayMetrics().densityDpi;
        if (perspectiveValue > 0.00f) {
            viewWrapper.setRotationX(-rotationX);
            viewWrapper.setRotationY(-rotationY);
            cameraDistance = BigDecimal.valueOf(perspectiveValue).multiply(BigDecimal.valueOf(densityDpi));
        } else if (perspectiveValue < -0.00f) {
            viewWrapper.setRotationX(rotationX);
            viewWrapper.setRotationY(rotationY);
            cameraDistance = BigDecimal.valueOf(perspectiveValue).multiply(BigDecimal.valueOf(densityDpi));
        } else {
            viewWrapper.setRotationX(rotationX);
            viewWrapper.setRotationY(rotationY);
            cameraDistance = BigDecimal.valueOf(calculateCameraDistance());
        }
        viewWrapper.setRotation(rotationZ);
        viewWrapper.setTranslationX(translationX);
        viewWrapper.setTranslationY(translationY);
        viewWrapper.setTranslationZ(translationZ);
        viewWrapper.setPivotX(pivotX);
        viewWrapper.setPivotY(pivotY);
        viewWrapper.setScaleX(scaleX);
        viewWrapper.setScaleY(scaleY);
        viewWrapper.setCameraDistance(cameraDistance.floatValue());
    }

    /**
     * Process touch event for platform view.
     */
    public void processTouchEvent() {
        MotionEvent eventClone = getMotionEvent();
        if (eventClone != null) {
            synchronized (PLATFORM_VIEW_LOCK) {
                if (platformView != null) {
                    viewWrapper.dispatchTouchEvent(eventClone);
                }
            }
        }
    }

    private MotionEvent getMotionEvent() {
        if (motionEvent == null) {
            return null;
        }
        int pointerCount = motionEvent.getPointerCount();
        Matrix viewMatrix = new Matrix(viewWrapper.getMatrix());
        Matrix inverseMatrix = new Matrix();
        boolean hasInverse = viewMatrix.invert(inverseMatrix);
        MotionEvent.PointerProperties[] pointerProperties = new MotionEvent.PointerProperties[pointerCount];
        MotionEvent.PointerCoords[] pointerCoords = new MotionEvent.PointerCoords[pointerCount];
        for (int i = 0; i < motionEvent.getPointerCount(); i++) {
            pointerProperties[i] = new MotionEvent.PointerProperties();
            motionEvent.getPointerProperties(i, pointerProperties[i]);

            MotionEvent.PointerCoords originCoords = new MotionEvent.PointerCoords();
            motionEvent.getPointerCoords(i, originCoords);

            float[] pts = new float[] {
                originCoords.x - offsetX,
                originCoords.y - offsetY
            };

            if (hasInverse) {
                inverseMatrix.mapPoints(pts);
            }
            pointerCoords[i] = new MotionEvent.PointerCoords(originCoords);
            pointerCoords[i].x = pts[0];
            pointerCoords[i].y = pts[1];
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
    public String setScale(Map<String, String> params) {
        if (params.containsKey(X)) {
            try {
                scaleX = Float.parseFloat(params.get(X));
            } catch (NumberFormatException ignored) {
                ALog.e(LOG_TAG, "parse scaleX failed");
                return FAIL;
            }
        }
        if (params.containsKey(Y)) {
            try {
                scaleY = Float.parseFloat(params.get(Y));
            } catch (NumberFormatException ignored) {
                ALog.e(LOG_TAG, "parse scaleY failed");
                return FAIL;
            }
        }
        if (params.containsKey(CENTER_X)) {
            try {
                centerX = params.get(CENTER_X);
            } catch (NumberFormatException ignored) {
                ALog.e(LOG_TAG, "parse centerX failed");
                return FAIL;
            }
        }
        if (params.containsKey(CENTER_Y)) {
            try {
                centerY = params.get(CENTER_Y);
            } catch (NumberFormatException ignored) {
                ALog.e(LOG_TAG, "parse centerY failed");
                return FAIL;
            }
        }
        return SUCCESS;
    }

    private static BigDecimal sqrt(BigDecimal value, int scale) {
        if (value.compareTo(BigDecimal.ZERO) <= 0) {
            return BigDecimal.ZERO;
        }
        BigDecimal two = BigDecimal.valueOf(2);
        BigDecimal x = new BigDecimal(Math.sqrt(value.doubleValue()));
        BigDecimal prev;
        do {
            prev = x;
            x = value.divide(x, scale, RoundingMode.HALF_UP).add(x).divide(two, scale, RoundingMode.HALF_UP);
        } while (!x.equals(prev));
        return x;
    }

    private void calculateRotationComponents(float axisX, float axisY, float axisZ, float rotationAngle) {
        MathContext mc = new MathContext(20, RoundingMode.HALF_UP);
        BigDecimal bdAxisX = BigDecimal.valueOf(axisX);
        BigDecimal bdAxisY = BigDecimal.valueOf(axisY);
        BigDecimal bdAxisZ = BigDecimal.valueOf(axisZ);

        BigDecimal sumOfSquares = bdAxisX.multiply(bdAxisX, mc)
                .add(bdAxisY.multiply(bdAxisY, mc), mc)
                .add(bdAxisZ.multiply(bdAxisZ, mc), mc);

        BigDecimal norm = sqrt(sumOfSquares, 20);
        if (norm.compareTo(BigDecimal.ZERO) <= 0) {
            norm = BigDecimal.ONE;
        }

        BigDecimal bdRotationAngle = BigDecimal.valueOf(rotationAngle);
        BigDecimal scale = bdRotationAngle.divide(norm, mc);

        rotationX = scale.multiply(bdAxisX, mc).floatValue();
        rotationY = scale.multiply(bdAxisY, mc).floatValue();
        rotationZ = scale.multiply(bdAxisZ, mc).floatValue();
    }

    @Override
    public String setRotation(Map<String, String> params) {
        if (!params.containsKey(X)) {
            ALog.e(LOG_TAG, "setRotation failed: X is illegal");
            return FAIL;
        }
        if (!params.containsKey(Y)) {
            ALog.e(LOG_TAG, "setRotation failed: Y is illegal");
            ALog.e(LOG_TAG, "setRotation failed: Y is illegal");
            return FAIL;
        }
        if (!params.containsKey(Z)) {
            ALog.e(LOG_TAG, "setRotation failed: Z is illegal");
            return FAIL;
        }
        if (!params.containsKey(ANGLE)) {
            ALog.e(LOG_TAG, "setRotation failed: ANGLE is illegal");
            return FAIL;
        }
        if (!params.containsKey(CENTER_X)) {
            ALog.e(LOG_TAG, "setRotation failed: CENTER_X is illegal");
            return FAIL;
        }
        if (!params.containsKey(CENTER_Y)) {
            ALog.e(LOG_TAG, "setRotation failed: CENTER_Y is illegal");
            return FAIL;
        }
        if (!params.containsKey(PERSPECTIVE)) {
            ALog.e(LOG_TAG, "setRotation failed: PERSPECTIVE is illegal");
            return FAIL;
        }

        axisX = Float.parseFloat(params.get(X));
        axisY = Float.parseFloat(params.get(Y));
        axisZ = Float.parseFloat(params.get(Z));
        rotationAngle = Float.parseFloat(params.get(ANGLE));
        centerX = params.get(CENTER_X);
        centerY = params.get(CENTER_Y);
        perspective = params.get(PERSPECTIVE);
        calculateRotationComponents(axisX, axisY, axisZ, rotationAngle);
        return SUCCESS;
    }

    @Override
    public String setTranslation(Map<String, String> params) {
        if (params.containsKey(X)) {
            try {
                translationX = Float.parseFloat(params.get(X));
            } catch (NumberFormatException ignored) {
                ALog.e(LOG_TAG, "parse scaleX failed");
                return FAIL;
            }
        }
        if (params.containsKey(Y)) {
            try {
                translationY = Float.parseFloat(params.get(Y));
            } catch (NumberFormatException ignored) {
                ALog.e(LOG_TAG, "parse scaleY failed");
                return FAIL;
            }
        }
        if (params.containsKey(Z)) {
            try {
                translationZ = Float.parseFloat(params.get(Z));
            } catch (NumberFormatException ignored) {
                ALog.e(LOG_TAG, "parse scaleY failed");
                return FAIL;
            }
        }
        return SUCCESS;
    }

    @Override
    public String setTransformMatrix(Map<String, String> params) {
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