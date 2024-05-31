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

package ohos.ace.adapter.capability.texture;

/**
 * The Texture interface defined from ACE engine.
 *
 * @since 1
 */
public interface IAceTexture {
    /**
     * Register the texture to rendering engine.
     *
     * @param textureId id, same as texture component id
     * @param surfaceTexture surface texture object
     */
    void registerTexture(long textureId, Object surfaceTexture);

    /**
     * Register Surface from texture to rendering engine.
     *
     * @param textureId id, same as texture component id
     * @param surfaceTexture surface texture object
     */
    void registerSurface(long textureId, Object surface);

    /**
     * Mark texture frame available.
     *
     * @param textureId id
     */
    void markTextureFrameAvailable(long textureId);

    /**
     * Unregister texture to rendering engine.
     *
     * @param textureId id
     */
    void unregisterTexture(long textureId);

    /**
     * Unregister surface to rendering engine.
     *
     * @param textureId id
     */
    void unregisterSurface(long textureId);
}
