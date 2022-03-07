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

package ohos.ace.adapter.capability.texture;

import android.graphics.SurfaceTexture;
import android.view.Surface;

import ohos.ace.adapter.IAceOnResourceEvent;

/**
 * This class handles the lifecycle of a surface texture.
 *
 */
public class AceTexture {
    /**
     * IAceTexture.
     */
    protected final IAceTexture textureImpl;

    /**
     * SurfaceTexture.
     */
    protected final SurfaceTexture surfaceTexture;

    /**
     * surface.
     */
    protected Surface surface = null;

    /**
     * hasRegisterSurface.
     */
    protected boolean hasRegisterSurface = false;

    /**
     * id.
     */
    protected final long id;

    private boolean hasRegisterTexture = false;

    private SurfaceTexture.OnFrameAvailableListener onFrameListener = new SurfaceTexture.OnFrameAvailableListener() {
        @Override
        public void onFrameAvailable(SurfaceTexture texture) {
            markTextureFrame();
        }
    };

    public AceTexture(long id, IAceTexture textureImpl, IAceOnResourceEvent callback) {
        this.surfaceTexture = new SurfaceTexture(0);
        this.surfaceTexture.detachFromGLContext();
        this.surfaceTexture.setOnFrameAvailableListener(onFrameListener);
        this.id = id;
        this.textureImpl = textureImpl;
    }

    /**
     * This is called to get surface texture.
     *
     * @return SurfaceTexture
     */
    public SurfaceTexture getSurfaceTexture() {
        return surfaceTexture;
    }

    /**
     * Set default buffer size.
     *
     * @param width The surface width.
     * @param height The surface height.
     */
    public void setDefaultBufferSize(int width, int height) {
        if (surfaceTexture != null) {
            surfaceTexture.setDefaultBufferSize(width, height);
        }
    }

    /**
     * This is called to get surface.
     *
     * @return Surface
     */
    public Surface getSurface() {
        return new Surface(surfaceTexture);
    }

    /**
     * This is called to get resource id.
     *
     * @return resource id
     */
    public long getId() {
        return id;
    }

    /**
     * This is called to release.
     *
     */
    public void release() {
        surfaceTexture.setOnFrameAvailableListener(null);
        textureImpl.unregisterTexture(id);
        surfaceTexture.release();
    }

    /**
     * SurfaceView do it and nativeView not.
     *
     */
    public void setSurfaceTexture() {
        this.textureImpl.registerTexture(this.id, surfaceTexture);
    }

    /**
     * There is a different logic between SurfaceView and NativeView.
     *
     */
    public void markTextureFrame() {
        if (!hasRegisterTexture) {
            setSurfaceTexture();
            hasRegisterTexture = true;
        }

        this.textureImpl.markTextureFrameAvailable(id);
    }
}

