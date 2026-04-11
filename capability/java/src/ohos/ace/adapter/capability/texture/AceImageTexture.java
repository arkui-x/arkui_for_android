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

package ohos.ace.adapter.capability.texture;

import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.HardwareBuffer;
import android.os.Handler;
import android.os.Looper;
import android.view.Surface;
import ohos.ace.adapter.AceSurfaceHolder;
import ohos.ace.adapter.AceTextureHolder;
import ohos.ace.adapter.ALog;

/**
 * This class handles the lifecycle of a surface texture.
 *
 * @since 26
 */
public class AceImageTexture implements IAceSurfaceTexture {
    private static final String LOG_TAG = "AceImageTexture";
    private static final int MAX_IMAGES = 3;

    private int instanceId = -1;
    private long id = 0L;
    private IAceTexture textureImpl;
    private Surface surface = null;
    private Surface oldSurface = null;
    private int textureWidth = 0;
    private int textureHeight = 0;
    private long imageTextureId = 0L;
    private long listenerId = -1;
    private IAceSurfaceTexture.OnFrameAvailableListener onFrameAvailableListener = null;
    private OnImageAvailableListener listener = null;
    private Handler mainHandler;
    private boolean hasRegistered = false;
    private volatile int rotationDegrees = 0;

    public AceImageTexture(int instanceId, long id, IAceTexture textureImpl) {
        this.instanceId = instanceId;
        this.id = id;
        this.textureImpl = textureImpl;

        mainHandler = new Handler(Looper.getMainLooper());
        if (!init()) {
            ALog.e(LOG_TAG, "init failed.");
        }
    }

    interface OnImageAvailableListener {
        /**
         * Callback method invoked when a new image is available for the specified texture.
         *
         * @param textureName The unique identifier of the texture for which the image is available.
         */
        void onImageAvailableListener(long textureName);
    }

    private native long nativeCreateImageTexture();

    private native void nativeDeleteImageTexture(long handle);

    private native Surface nativeGetImageSurface(
        long handle, int width, int height, int format, long usageFlags, int maxImages);

    private native long nativeSetImageAvailableListener(long handle, OnImageAvailableListener listener);

    private native void nativeGetId(long handle, long listenerId, long textureId);

    private native void nativeRemoveImageAvailableListener(long handle, long listenerId);

    private boolean init() {
        this.imageTextureId = nativeCreateImageTexture();
        if (this.imageTextureId == 0) {
            ALog.e(LOG_TAG, "init: failed to create ImageReader");
            return false;
        }
        nativeGetId(this.imageTextureId, this.instanceId, this.id);
        listener = new OnImageAvailableListener() {
            @Override
            public void onImageAvailableListener(long textureHandle) {
                notifyFrameAvailable();
            }
        };
        listenerId = nativeSetImageAvailableListener(imageTextureId, listener);
        return true;
    }

    private void notifyFrameAvailable() {
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (onFrameAvailableListener != null) {
                    onFrameAvailableListener.onFrameAvailable(AceImageTexture.this);
                }
            }
        });
    }

    private void createImageReader() {
        if (this.surface != null) {
            AceSurfaceHolder.removeSurface(this.instanceId, this.id);
            AceTextureHolder.removeSurface(this.id);
        }
        if (hasRegistered) {
            this.textureImpl.unregisterSurface(this.id);
            this.textureImpl.unregisterTexture(this.id);
        }
        Surface newSurface = nativeGetImageSurface(this.imageTextureId, textureWidth, textureHeight,
            ImageFormat.PRIVATE, HardwareBuffer.USAGE_GPU_SAMPLED_IMAGE, MAX_IMAGES);
        if (this.oldSurface != null) {
            this.oldSurface.release();
        }
        if (this.surface != null) {
            this.oldSurface = this.surface;
            this.surface = null;
        }
        if (newSurface != null) {
            this.surface = newSurface;
            AceTextureHolder.addSurface(this.id, this.surface);
            AceSurfaceHolder.addSurface(this.instanceId, this.id, this.surface);
            this.textureImpl.registerSurface(this.id, this.surface);
            this.textureImpl.registerTexture(this.id, this.surface);
            hasRegistered = true;
        }
    }

    /**
     * Registers a listener to be notified when a new frame is available on the surface texture.
     *
     * @param listener the listener to be invoked when a new frame is available
     */
    @Override
    public void setOnFrameAvailableListener(OnFrameAvailableListener listener) {
        onFrameAvailableListener = listener;
    }

    /**
     * Sets the size of the texture.
     *
     * @param width  the width of the texture in pixels
     * @param height the height of the texture in pixels
     */
    @Override
    public void setTextureSize(int width, int height) {
        if (width > 0 && height > 0 && (this.textureWidth != width || this.textureHeight != height)) {
            this.textureWidth = width;
            this.textureHeight = height;
            ALog.i(LOG_TAG, "setTextureSize: width " + width + ", height " + height);
            createImageReader();
        }
    }

    /**
     * Update the texture image and return transform matrix or related data.
     */
    @Override
    public void updateTexImage() {
    }

    /**
     * Returns the transformation matrix associated with the image texture.
     * <p>
     * The transformation matrix can be used to apply scaling, rotation, or translation
     * to the texture when rendering. By default, this implementation returns an empty
     * float array, indicating that no transformation is applied.
     *
     * @return a float array representing the transformation matrix, or an empty array if none.
     */
    public float[] getTransformMatrix() {
        return new float[0];
    }

    /**
     * Attaches the SurfaceTexture to the OpenGL ES context using the specified texture name.
     *
     * @param texture the OpenGL texture object name to which the SurfaceTexture will be attached
     */
    @Override
    public void attachToGLContext(int texture) {}

    /**
     * Detaches the image texture from the current OpenGL context.
     * This method should be called when the texture is no longer needed or before the OpenGL context is destroyed,
     * to release any associated resources and prevent memory leaks.
     */
    public void detachFromGLContext() {}

    /**
     * Retrieves the unique identifier for the surface texture.
     *
     * @return the texture ID as a long value.
     */
    @Override
    public long getTextureId() {
        return imageTextureId;
    }

    /**
     * Retrieves the {@link Surface} associated with this texture.
     *
     * @return the {@link Surface} object used for rendering or displaying content.
     */
    @Override
    public Surface getSurface() {
        return surface;
    }

    /**
     * Retrieves the underlying {@link SurfaceTexture} associated with this instance.
     *
     * @return the {@code SurfaceTexture} object managed by this interface, or {@code null} if not available.
     */
    @Override
    public SurfaceTexture getTexture() {
        return null;
    }

    /**
     * Releases any resources or references held by this surface texture.
     * After calling this method, the surface texture should not be used.
     */
    @Override
    public void release() {
        ALog.i(LOG_TAG, "release: texture id = " + imageTextureId);
        AceTextureHolder.removeSurface(this.id);
        AceSurfaceHolder.removeSurface(instanceId, id);
        if (hasRegistered) {
            this.textureImpl.unregisterSurface(this.id);
            this.textureImpl.unregisterTexture(this.id);
        }

        nativeRemoveImageAvailableListener(imageTextureId, listenerId);
        nativeDeleteImageTexture(imageTextureId);
        if (this.surface != null) {
            this.surface.release();
        }
        if (this.oldSurface != null) {
            this.oldSurface.release();
        }
        this.hasRegistered = false;
        this.surface = null;
        this.listener = null;
        this.onFrameAvailableListener = null;
        this.textureWidth = 0;
        this.textureHeight = 0;
        this.mainHandler.removeCallbacksAndMessages(null);
    }
}