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

import java.util.HashMap;
import java.util.Map;

import android.view.Surface;

/**
 * The class to hold all surfaces in plugins
 *
 * @since 1
 */
public final class AceSurfaceHolder {
    private static final String LOG_TAG = "AceSurfaceHolder";

    private static final Map<Long, Surface> surfaceMap = new HashMap<Long, Surface>();

    /**
     * Get surface by id
     *
     * @param id id of surface
     * @return the surface
     */
    public static Surface getSurface(long id) {
        return surfaceMap.get(id);
    }

    /**
     * Add surface by id
     *
     * @param id      id of surface
     * @param surface the surface object
     */
    public static void addSurface(long id, Surface surface) {
        surfaceMap.put(id, surface);
    }

    /**
     * Remove surface by id
     *
     * @param id id of surface
     */
    public static void removeSurface(long id) {
        surfaceMap.remove(id);
    }
}
