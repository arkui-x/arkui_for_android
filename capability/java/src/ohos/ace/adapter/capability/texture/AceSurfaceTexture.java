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

import android.graphics.SurfaceTexture;
import android.view.Surface;

import ohos.ace.adapter.AceTextureHolder;
import ohos.ace.adapter.AceSurfaceHolder;
import ohos.ace.adapter.ALog;

/**
 * Wrapper for AceSurfaceTexture implementing IAceSurfaceTexture.
 *
 * @since 26
 */
public class AceSurfaceTexture implements IAceSurfaceTexture {
    private static final String LOG_TAG = "AceSurfaceTexture";

    private final long id;
    private int instanceId = -1;
    private final IAceTexture textureImpl;
    private SurfaceTexture surfaceTexture;
    private Surface surface;
    private OnFrameAvailableListener onFrameAvailableListener;
    private int textureWidth = 0;
    private int textureHeight = 0;

    public AceSurfaceTexture(int instanceId, long id, IAceTexture textureImpl) {
        this.instanceId = instanceId;
        this.id = id;
        this.textureImpl = textureImpl;
        this.surfaceTexture = new SurfaceTexture(0);
        this.surfaceTexture.detachFromGLContext();
        this.surfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
            @Override
            public void onFrameAvailable(SurfaceTexture st) {
                if (onFrameAvailableListener != null) {
                    onFrameAvailableListener.onFrameAvailable(AceSurfaceTexture.this);
                }
            }
        });
        this.surface = new Surface(this.surfaceTexture);
        AceSurfaceHolder.addSurface(instanceId, this.id, surface);
        this.textureImpl.registerSurface(this.id, surface);
        AceTextureHolder.addSurfaceTexture(this.id, this.surfaceTexture);
        AceTextureHolder.addSurface(this.id, this.surface);
        this.textureImpl.registerTexture(this.id, this.surfaceTexture);
    }

    @Override
    public void setOnFrameAvailableListener(OnFrameAvailableListener listener) {
        this.onFrameAvailableListener = listener;
    }

    @Override
    public void setTextureSize(int textureWidth, int textureHeight) {
        this.textureWidth = textureWidth;
        this.textureHeight = textureHeight;
        if (this.surfaceTexture != null) {
            try {
                this.surfaceTexture.setDefaultBufferSize(textureWidth, textureHeight);
            } catch (IllegalArgumentException | IllegalStateException e) {
                ALog.e(LOG_TAG, "setDefaultBufferSize failed.");
            }
        }
    }

    @Override
    public void updateTexImage() {
        if (this.surfaceTexture == null) {
            return;
        }
        this.surfaceTexture.updateTexImage();
    }

    /**
     * Returns the current 4x4 texture transform matrix from the underlying {@code SurfaceTexture}.
     * <p>
     * The returned array contains 16 {@code float} values in column-major order and can be used
     * to map texture coordinates for correct rendering.
     *
     * @return a 16-element transform matrix, or an empty array if the underlying
     *         {@code surfaceTexture} is not available
     */
    public float[] getTransformMatrix() {
        if (this.surfaceTexture == null) {
            return new float[0];
        }
        float[] transform = new float[16];
        this.surfaceTexture.getTransformMatrix(transform);
        return transform;
    }

    @Override
    public long getTextureId() {
        return 0;
    }

    @Override
    public Surface getSurface() {
        return this.surface;
    }

    @Override
    public SurfaceTexture getTexture() {
        return this.surfaceTexture;
    }

    @Override
    public void release() {
        textureImpl.unregisterTexture(id);
        textureImpl.unregisterSurface(id);
        AceTextureHolder.removeSurface(id);
        AceTextureHolder.removeSurfaceTexture(id);
        AceSurfaceHolder.removeSurface(instanceId, id);
        if (this.surfaceTexture != null) {
            this.surfaceTexture.setOnFrameAvailableListener(null);
            this.surfaceTexture.release();
        }
        if (this.surface != null) {
            this.surface.release();
        }
    }

    /**
     * Attaches the underlying SurfaceTexture to the OpenGL ES context using the specified texture ID.
     *
     * @param textureId the OpenGL texture object ID to which the SurfaceTexture will be attached
     */
    public void attachToGLContext(int textureId) {
        if (this.surfaceTexture != null) {
            this.surfaceTexture.attachToGLContext(textureId);
        }
    }

    /**
     * Detaches the underlying SurfaceTexture from the current OpenGL ES context.
     * <p>
     * This method should be called when the SurfaceTexture is no longer needed
     * in the current GL context, or before attaching it to a different GL context.
     * If the SurfaceTexture is already detached or null, this method has no effect.
     */
    public void detachFromGLContext() {
        if (this.surfaceTexture != null) {
            this.surfaceTexture.detachFromGLContext();
        }
    }
}
