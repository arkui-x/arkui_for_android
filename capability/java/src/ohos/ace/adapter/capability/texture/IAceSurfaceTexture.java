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

/**
 * Interface for handling surface texture lifecycle and interactions.
 *
 * @since 26
 */
public interface IAceSurfaceTexture {
    /**
     * Listener for frame available events.
     */
    interface OnFrameAvailableListener {
        /**
         * Called when a new frame is available to be rendered on the given surface texture.
         *
         * @param texture The IAceSurfaceTexture instance that has a new frame available.
         */
        void onFrameAvailable(IAceSurfaceTexture texture);
    }

    /**
     * Registers a listener to be notified when a new frame is available on the surface texture.
     *
     * @param listener the listener to be invoked when a new frame is available
     */
    void setOnFrameAvailableListener(OnFrameAvailableListener listener);

    /**
     * Sets the size of the texture.
     *
     * @param textureWidth  the width of the texture in pixels
     * @param textureHeight the height of the texture in pixels
     */
    void setTextureSize(int textureWidth, int textureHeight);

    /**
     * Update the texture image and return transform matrix or related data.
     */
    void updateTexImage();

    /**
     * Retrieves the transformation matrix associated with the surface texture.
     *
     * @return a float array representing the 4x4 transformation matrix used to transform
     *         texture coordinates. The array is typically of length 16 and follows column-major order.
     */
    float[] getTransformMatrix();

    /**
     * Attaches the SurfaceTexture to the OpenGL ES context using the specified texture name.
     *
     * @param texture the OpenGL texture object name to which the SurfaceTexture will be attached
     */
    void attachToGLContext(int texture);

    /**
     * Detaches the surface texture from the current OpenGL context.
     * After calling this method, the surface texture can be attached to a different GL context.
     * This is typically used when the rendering context is being changed or destroyed.
     */
    void detachFromGLContext();

    /**
     * Retrieves the unique identifier for the surface texture.
     *
     * @return the texture ID as a long value.
     */
    long getTextureId();

    /**
     * Retrieves the {@link Surface} associated with this texture.
     *
     * @return the {@link Surface} object used for rendering or displaying content.
     */
    Surface getSurface();

    /**
     * Retrieves the underlying {@link SurfaceTexture} associated with this instance.
     *
     * @return the {@code SurfaceTexture} object managed by this interface, or {@code null} if not available.
     */
    SurfaceTexture getTexture();

    /**
     * Releases any resources or references held by this surface texture.
     * After calling this method, the surface texture should not be used.
     */
    void release();
}