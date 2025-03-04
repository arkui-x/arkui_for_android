/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

/**
 * The class to hold all surfaceTextures in plugins
 *
 * @since 2024-5-31
 */
public final class AceTextureHolder {
    private static final String LOG_TAG = "AceTextureHolder";

    private static final Map<Long, SurfaceTexture> surfaceTextureMap = new ConcurrentHashMap<Long, SurfaceTexture>();

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
