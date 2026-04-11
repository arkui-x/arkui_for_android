/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;

import android.graphics.SurfaceTexture;
import android.view.Surface;

/**
 * The class to hold all surfaceTextures in plugins
 *
 * @since 2024-5-31
 */
public final class AceTextureHolder {
    private static final String LOG_TAG = "AceTextureHolder";

    private static final Map<Long, SurfaceTexture> surfaceTextureMap = new ConcurrentHashMap<Long, SurfaceTexture>();
    private static final Map<Long, Surface> surfaceMap = new ConcurrentHashMap<Long, Surface>();

    /**
     * Get surfaceTexture by id
     *
     * @param id id of surfaceTexture
     * @return the surfaceTexture
     */
    public static SurfaceTexture getSurfaceTexture(long id) {
        ALog.i(LOG_TAG, "getSurfaceTexture");
        return surfaceTextureMap.get(id);
    }

    /**
     * Get surface by id
     *
     * @param id id of surface
     * @return the surface
     */
    public static Surface getSurface(long id) {
        Surface surface = surfaceMap.get(id);
        if (surface != null && !surface.isValid()) {
            ALog.w(LOG_TAG, "getSurface returns an invalid(released) surface, remove it.");
            surfaceMap.remove(id);
            return null;
        }
        return surface;
    }

    /**
     * Add surface by id
     *
     * @param id      id of surface
     * @param surface the surface object
     */
    public static void addSurface(long id, Surface surface) {
        if (surface != null && surface.isValid()) {
            surfaceMap.put(id, surface);
        } else {
            ALog.w(LOG_TAG, "addSurface with an invalid(released) surface, ignore it.");
        }
    }

    /**
     * Remove surface by id
     *
     * @param id id of surface
     */
    public static void removeSurface(long id) {
        surfaceMap.remove(id);
    }

    /**
     * Add surfaceTexture by id
     *
     * @param id      id of surfaceTexture
     * @param surfaceTexture the surfaceTexture object
     */
    public static void addSurfaceTexture(long id, SurfaceTexture surfaceTexture) {
        ALog.i(LOG_TAG, "addSurfaceTexture");
        surfaceTextureMap.put(id, surfaceTexture);
    }

    /**
     * Remove surfaceTexture by id
     *
     * @param id id of surfaceTexture
     */
    public static void removeSurfaceTexture(long id) {
        ALog.i(LOG_TAG, "removeSurfaceTexture");
        surfaceTextureMap.remove(id);
    }
}
