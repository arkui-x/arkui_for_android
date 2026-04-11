/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
    private static final float SCALE_X_DEFAULT = 1.0f;
    private static final float SCALE_Y_DEFAULT = 1.0f;
    private static final float SCALE_Z_DEFAULT = 1.0f;
    private static final float AXIS_X_DEFAULT = 0.0f;
    private static final float AXIS_Y_DEFAULT = 0.0f;
    private static final float AXIS_Z_DEFAULT = 0.0f;
    private static final float ROTATION_X_DEFAULT = 0.0f;
    private static final float ROTATION_Y_DEFAULT = 0.0f;
    private static final float ROTATION_Z_DEFAULT = 0.0f;
    private static final float ROTATION_ANGLE_DEFAULT = 0.0f;
    private static final float TRANSLATION_X_DEFAULT = 0.0f;
    private static final float TRANSLATION_Y_DEFAULT = 0.0f;
    private static final float TRANSLATION_Z_DEFAULT = 0.0f;
    private static final String CENTER_X_DEFAULT = "50%";
    private static final String CENTER_Y_DEFAULT = "50%";
    private static final String PERSPECTIVE_DEFAULT = "";

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
    private float scaleX = SCALE_X_DEFAULT;
    private float scaleY = SCALE_Y_DEFAULT;
    private float scaleZ = SCALE_Z_DEFAULT;
    private float axisX = AXIS_X_DEFAULT;
    private float axisY = AXIS_Y_DEFAULT;
    private float axisZ = AXIS_Z_DEFAULT;
    private float rotationX = ROTATION_X_DEFAULT;
    private float rotationY = ROTATION_Y_DEFAULT;
    private float rotationZ = ROTATION_Z_DEFAULT;
    private float rotationAngle = ROTATION_ANGLE_DEFAULT;
    private float translationX = TRANSLATION_X_DEFAULT;
    private float translationY = TRANSLATION_Y_DEFAULT;
    private float translationZ = TRANSLATION_Z_DEFAULT;
    private String centerX = CENTER_X_DEFAULT;
    private String centerY = CENTER_Y_DEFAULT;
    private String perspective = PERSPECTIVE_DEFAULT;
    private boolean isTransformMatrix = false;
    private float[] transformMatrix;
    private float[] affineMatrix;

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
            viewWrapper.setTextureId(textureId);
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

    /**
     * Calculate the camera distance for 3D view effect
     *
     * @return camera distance value in pixels, adjusted by screen density
     */
    private float calculateCameraDistance() {
        /**
         * 1.Get screen density.
         * 2.Math.sqrt(width * width + height * height) calculates the diagonal length of the view
         * 3.Multiplying by 0.5f yields half of the diagonal length,
         * representing "the maximum distance from the view's center to its corner".
         * 4.Multiplying by density converts device-independent pixels (dp) to actual physical pixels,
         * ensuring consistent effects across devices with different density.
         */
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
        if (!isTransformMatrix) {
            viewWrapper.setCameraDistance(cameraDistance.floatValue());
        } else {
            viewWrapper.setCameraDistance(Float.MAX_VALUE);
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

    private Float parseFloatParam(Map<String, String> params, String key, Float defaultValue) {
        Float value = defaultValue;
        if (params.containsKey(key)) {
            try {
                value = Float.parseFloat(params.get(key));
            } catch (NumberFormatException ignored) {
                ALog.e(LOG_TAG, "parse failed " + key);
            }
        }
        return value;
    }

    private String parseStringParam(Map<String, String> params, String key, String defaultValue) {
        String value = defaultValue;
        if (params.containsKey(key)) {
            value = params.get(key);
        }
        return value;
    }

    @Override
    public String setScale(Map<String, String> params) {
        scaleX = parseFloatParam(params, X, SCALE_X_DEFAULT);
        scaleY = parseFloatParam(params, Y, SCALE_Y_DEFAULT);
        scaleZ = parseFloatParam(params, Z, SCALE_Z_DEFAULT);
        centerX = parseStringParam(params, CENTER_X, CENTER_X_DEFAULT);
        centerY = parseStringParam(params, CENTER_Y, CENTER_Y_DEFAULT);
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
        axisX = parseFloatParam(params, X, AXIS_X_DEFAULT);
        axisY = parseFloatParam(params, Y, AXIS_Y_DEFAULT);
        axisZ = parseFloatParam(params, Z, AXIS_Z_DEFAULT);
        rotationAngle = parseFloatParam(params, ANGLE, ROTATION_ANGLE_DEFAULT);
        centerX = parseStringParam(params, CENTER_X, CENTER_X_DEFAULT);
        centerY = parseStringParam(params, CENTER_Y, CENTER_Y_DEFAULT);
        perspective = parseStringParam(params, PERSPECTIVE, PERSPECTIVE_DEFAULT);

        calculateRotationComponents(axisX, axisY, axisZ, rotationAngle);
        return SUCCESS;
    }

    @Override
    public String setTranslation(Map<String, String> params) {
        translationX = parseFloatParam(params, X, TRANSLATION_X_DEFAULT);
        translationY = parseFloatParam(params, Y, TRANSLATION_Y_DEFAULT);
        translationZ = parseFloatParam(params, Z, TRANSLATION_Z_DEFAULT);

        translationX *= context.getResources().getDisplayMetrics().density;
        translationY *= context.getResources().getDisplayMetrics().density;
        translationZ *= context.getResources().getDisplayMetrics().density;
        return SUCCESS;
    }

    @Override
    public String setTransformMatrix(Map<String, String> params) {
        transformMatrix = new float[16];
        for (int i = 0; i < 16; i++) {
            String key = "m" + i;
            if (!params.containsKey(key)) {
                ALog.e(LOG_TAG, "setTransformMatrix failed: missing key " + key);
                return FAIL;
            }
            try {
                transformMatrix[i] = Float.parseFloat(params.get(key));
            } catch (NumberFormatException ignored) {
                ALog.e(LOG_TAG, "setTransformMatrix failed: parse error for " + key);
                return FAIL;
            }
        }

        float w = transformMatrix[15];
        if (Float.compare(w, 0.0f) == 0) {
            w = 1.0f;
        }
        affineMatrix = new float[16];
        for (int i = 0; i < 16; i++) {
            affineMatrix[i] = transformMatrix[i] / w;
        }

        extractScale();
        extractRotation();
        extractTranslation(transformMatrix);
        isTransformMatrix = true;

        return SUCCESS;
    }

    private void extractScale() {
        MathContext mc = new MathContext(20, RoundingMode.HALF_UP);

        BigDecimal m00 = BigDecimal.valueOf(affineMatrix[0]);
        BigDecimal m01 = BigDecimal.valueOf(affineMatrix[1]);
        BigDecimal m02 = BigDecimal.valueOf(affineMatrix[2]);
        BigDecimal m10 = BigDecimal.valueOf(affineMatrix[4]);
        BigDecimal m11 = BigDecimal.valueOf(affineMatrix[5]);
        BigDecimal m12 = BigDecimal.valueOf(affineMatrix[6]);
        BigDecimal m20 = BigDecimal.valueOf(affineMatrix[8]);
        BigDecimal m21 = BigDecimal.valueOf(affineMatrix[9]);
        BigDecimal m22 = BigDecimal.valueOf(affineMatrix[10]);

        BigDecimal sx = m00.multiply(m00, mc).add(m01.multiply(m01, mc), mc).add(m02.multiply(m02, mc), mc);
        BigDecimal sy = m10.multiply(m10, mc).add(m11.multiply(m11, mc), mc).add(m12.multiply(m12, mc), mc);
        BigDecimal sz = m20.multiply(m20, mc).add(m21.multiply(m21, mc), mc).add(m22.multiply(m22, mc), mc);

        scaleX = sqrt(sx, 20).floatValue();
        scaleY = sqrt(sy, 20).floatValue();
        scaleZ = sqrt(sz, 20).floatValue();

        if (Float.compare(scaleX, 0.0f) == 0) {
            scaleX = 1.0f;
        }
        if (Float.compare(scaleY, 0.0f) == 0) {
            scaleY = 1.0f;
        }
        if (Float.compare(scaleZ, 0.0f) == 0) {
            scaleZ = 1.0f;
        }
    }

    private void extractRotation() {
        BigDecimal bdScaleX = BigDecimal.valueOf(scaleX);
        BigDecimal bdScaleY = BigDecimal.valueOf(scaleY);
        BigDecimal bdScaleZ = BigDecimal.valueOf(scaleZ);

        if (bdScaleX.compareTo(BigDecimal.ZERO) == 0) {
            bdScaleX = BigDecimal.ONE;
        }
        if (bdScaleY.compareTo(BigDecimal.ZERO) == 0) {
            bdScaleY = BigDecimal.ONE;
        }
        if (bdScaleZ.compareTo(BigDecimal.ZERO) == 0) {
            bdScaleZ = BigDecimal.ONE;
        }

        MathContext mc = new MathContext(20, RoundingMode.HALF_UP);
        BigDecimal m00 = BigDecimal.valueOf(affineMatrix[0]).divide(bdScaleX, mc);
        BigDecimal m10 = BigDecimal.valueOf(affineMatrix[4]).divide(bdScaleY, mc);
        BigDecimal m20 = BigDecimal.valueOf(affineMatrix[8]).divide(bdScaleZ, mc);
        BigDecimal m21 = BigDecimal.valueOf(affineMatrix[9]).divide(bdScaleZ, mc);
        BigDecimal m22 = BigDecimal.valueOf(affineMatrix[10]).divide(bdScaleZ, mc);

        BigDecimal m21m21 = m21.multiply(m21, mc);
        BigDecimal m22m22 = m22.multiply(m22, mc);
        BigDecimal sqrtTerm = sqrt(m21m21.add(m22m22), 20);

        rotationY = -(float) Math.atan2(-m20.doubleValue(), sqrtTerm.doubleValue());
        rotationX = -(float) Math.atan2(m21.doubleValue(), m22.doubleValue());
        rotationZ = -(float) Math.atan2(m10.doubleValue(), m00.doubleValue());

        rotationX = (float) Math.toDegrees(rotationX);
        rotationY = (float) Math.toDegrees(rotationY);
        rotationZ = (float) Math.toDegrees(rotationZ);
    }

    private void extractTranslation(float[] originalMatrix) {
        MathContext mc = new MathContext(20, RoundingMode.HALF_UP);

        BigDecimal txRaw = BigDecimal.valueOf(originalMatrix[12]);
        BigDecimal tyRaw = BigDecimal.valueOf(originalMatrix[13]);
        BigDecimal tzRaw = BigDecimal.valueOf(originalMatrix[14]);

        BigDecimal px = BigDecimal.valueOf(originalMatrix[3]);
        BigDecimal py = BigDecimal.valueOf(originalMatrix[7]);
        BigDecimal pz = BigDecimal.valueOf(originalMatrix[11]);

        translationX = txRaw.subtract(px, mc).floatValue();
        translationY = tyRaw.subtract(py, mc).floatValue();
        translationZ = tzRaw.subtract(pz, mc).floatValue();
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