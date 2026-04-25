/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

package ohos.ace.adapter.capability.common;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import android.graphics.Bitmap;
import android.os.Handler;
import android.os.HandlerThread;
import android.view.PixelCopy;
import android.view.Surface;

import ohos.ace.adapter.ALog;

/**
 * Shared helper for copying Surface content into a native direct buffer.
 *
 * @since 26
 */
public class AceSurfaceCaptureHelper {
    private static final String FALSE = "false";
    private static final String SUCCESS = "success";
    private static final String NATIVE_BUFFER_KEY = "nativeBuffer";
    private static final int BYTES_PER_PIXEL = 4;
    private static final int TIMEOUT_MILLIS = 2000;
    private static final int MAX_CAPTURE_DIMENSION = 4096;
    private static final long MAX_CAPTURE_BUFFER_SIZE = 64L * 1024L * 1024L;

    private final String threadName;
    private final CaptureConfig captureConfig;
    private HandlerThread pixelCopyThread;
    private Handler pixelCopyHandler;

    public AceSurfaceCaptureHelper(String threadName, CaptureConfig captureConfig) {
        this.threadName = threadName;
        this.captureConfig = captureConfig;
    }

    /**
     * Parsed and validated capture parameters.
     */
    private static class CaptureParams {
        private final long pointer;
        private final int width;
        private final int height;
        private final long bufferSize;

        CaptureParams(long pointer, int width, int height, long bufferSize) {
            this.pointer = pointer;
            this.width = width;
            this.height = height;
            this.bufferSize = bufferSize;
        }
    }

    /**
     * Configuration for surface capture behavior.
     */
    public static class CaptureConfig {
        private final String widthKey;
        private final String heightKey;
        private final String logTag;
        private final SurfaceProvider surfaceProvider;
        private final DirectBufferProvider directBufferProvider;

        public CaptureConfig(String widthKey, String heightKey, String logTag, SurfaceProvider surfaceProvider,
                DirectBufferProvider directBufferProvider) {
            this.widthKey = widthKey;
            this.heightKey = heightKey;
            this.logTag = logTag;
            this.surfaceProvider = surfaceProvider;
            this.directBufferProvider = directBufferProvider;
        }
    }

    /**
     * Provides the target surface for pixel copy.
     */
    public interface SurfaceProvider {
        /**
         * Get the target surface.
         *
         * @return target surface
         */
        Surface getSurface();
    }

    /**
     * Provides a native direct buffer mapped from a native pointer.
     */
    public interface DirectBufferProvider {
        /**
         * Create a direct buffer from native pointer.
         *
         * @param pointer    native pointer
         * @param bufferSize buffer size in bytes
         * @return direct buffer mapped to native memory
         */
        ByteBuffer createDirectBufferFromPointer(long pointer, long bufferSize);
    }

    /**
     * Capture surface pixels into the specified native buffer.
     *
     * @param params capture parameters
     * @return capture result
     */
    public String capture(Map<String, String> params) {
        CaptureParams captureParams = parseCaptureParams(params);
        if (captureParams == null) {
            return FALSE;
        }

        ByteBuffer cppBuffer = captureConfig.directBufferProvider.createDirectBufferFromPointer(
                captureParams.pointer, captureParams.bufferSize);
        if (cppBuffer == null) {
            ALog.e(captureConfig.logTag, "surfaceCapture: createDirectBufferFromPointer failed");
            return FALSE;
        }

        Bitmap bitmap = Bitmap.createBitmap(captureParams.width, captureParams.height, Bitmap.Config.ARGB_8888);
        AtomicBoolean shouldRecycle = new AtomicBoolean(true);
        try {
            if (!copySurfaceToBitmap(captureConfig.surfaceProvider.getSurface(), bitmap, shouldRecycle)) {
                return FALSE;
            }

            cppBuffer.order(ByteOrder.nativeOrder());
            cppBuffer.rewind();
            if ((long) cppBuffer.remaining() < captureParams.bufferSize) {
                ALog.e(captureConfig.logTag, "surfaceCapture: buffer remaining is insufficient");
                return FALSE;
            }
            bitmap.copyPixelsToBuffer(cppBuffer);
            return SUCCESS;
        } catch (IllegalArgumentException | IllegalStateException e) {
            ALog.e(captureConfig.logTag, "surfaceCapture: Exception occurred during pixel copy or buffer allocation");
            return FALSE;
        } finally {
            if (shouldRecycle.get() && !bitmap.isRecycled()) {
                bitmap.recycle();
            }
        }
    }

    private CaptureParams parseCaptureParams(Map<String, String> params) {
        if (params == null) {
            ALog.e(captureConfig.logTag, "surfaceCapture: params is null");
            return null;
        }

        String pointerValue = params.get(NATIVE_BUFFER_KEY);
        String widthValue = params.get(captureConfig.widthKey);
        String heightValue = params.get(captureConfig.heightKey);
        if (pointerValue == null || widthValue == null || heightValue == null) {
            ALog.e(captureConfig.logTag, "surfaceCapture: missing required params");
            return null;
        }

        long pointer;
        int width;
        int height;
        try {
            pointer = Long.parseUnsignedLong(pointerValue);
            width = Integer.parseInt(widthValue);
            height = Integer.parseInt(heightValue);
        } catch (NumberFormatException e) {
            ALog.e(captureConfig.logTag, "NumberFormatException, surfaceCapture failed");
            return null;
        }

        if (width <= 0 || height <= 0 || pointer == 0L) {
            ALog.e(captureConfig.logTag,
                "surfaceCapture: invalid params, w=" + width + ", h=" + height + ", p=" + pointer);
            return null;
        }

        if (width > MAX_CAPTURE_DIMENSION || height > MAX_CAPTURE_DIMENSION) {
            ALog.e(captureConfig.logTag,
                "surfaceCapture: dimensions exceed limit, w=" + width + ", h=" + height);
            return null;
        }

        long bufferSize = (long) width * height * BYTES_PER_PIXEL;
        if (bufferSize > MAX_CAPTURE_BUFFER_SIZE) {
            ALog.e(captureConfig.logTag,
                "surfaceCapture: bufferSize exceeds limit, w=" + width + ", h=" + height + ", bufferSize="
                + bufferSize);
            return null;
        }

        return new CaptureParams(pointer, width, height, bufferSize);
    }

    /**
     * Release PixelCopy thread resources.
     */
    public synchronized void release() {
        if (pixelCopyThread != null) {
            pixelCopyThread.quitSafely();
            pixelCopyThread = null;
            pixelCopyHandler = null;
        }
    }

    private boolean copySurfaceToBitmap(Surface surface, Bitmap bitmap, AtomicBoolean shouldRecycle) {
        if (surface == null || !surface.isValid()) {
            ALog.e(captureConfig.logTag, "surface is null or invalid, surfaceCapture failed");
            return false;
        }

        Handler handler = ensureHandler();
        CountDownLatch latch = new CountDownLatch(1);
        AtomicInteger resultHolder = new AtomicInteger(-1);
        AtomicBoolean isCompleted = new AtomicBoolean(false);
        PixelCopy.request(surface, bitmap, (int copyResult) -> {
            resultHolder.set(copyResult);
            if (!isCompleted.compareAndSet(false, true)) {
                if (!bitmap.isRecycled()) {
                    bitmap.recycle();
                }
            }
            latch.countDown();
        }, handler);
        try {
            if (!latch.await(TIMEOUT_MILLIS, TimeUnit.MILLISECONDS)) {
                ALog.e(captureConfig.logTag, "surfaceCapture: PixelCopy timeout");
                if (isCompleted.compareAndSet(false, true)) {
                    shouldRecycle.set(false);
                }
                return false;
            }
        } catch (InterruptedException e) {
            ALog.e(captureConfig.logTag, "surfaceCapture: wait interrupted");
            if (isCompleted.compareAndSet(false, true)) {
                shouldRecycle.set(false);
            }
            return false;
        }

        if (resultHolder.get() != PixelCopy.SUCCESS) {
            ALog.e(captureConfig.logTag, "surfaceCapture: PixelCopy failed, code=" + resultHolder.get());
            return false;
        }
        return true;
    }

    private synchronized Handler ensureHandler() {
        if (pixelCopyThread == null || pixelCopyHandler == null || !pixelCopyThread.isAlive()) {
            pixelCopyThread = new HandlerThread(threadName);
            pixelCopyThread.start();
            pixelCopyHandler = new Handler(pixelCopyThread.getLooper());
        }
        return pixelCopyHandler;
    }
}