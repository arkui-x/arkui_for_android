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

    private static final HashMap<Integer, Map<Long, Surface>> surfaceMap = new HashMap<Integer, Map<Long, Surface>>();

    /**
     * Get surface by id
     *
     * @param instanceId id of instance
     * @param id id of surface
     * @return the surface
     */
    public static Surface getSurface(int instanceId, long id) {
        if (surfaceMap.containsKey(instanceId)) {
            return surfaceMap.get(instanceId).get(id);
        }
        return null;
    }

    /**
     * Add surface by id
     *
     * @param instanceId id of surface
     * @param surface the surface object
     */
    public static void addSurface(int instanceId, long id, Surface surface) {
        if (surfaceMap.containsKey(instanceId)) {
            surfaceMap.get(instanceId).put(id, surface);
            return;
        }

        Map<Long, Surface> innerMap = new HashMap<Long, Surface>();
        innerMap.put(id, surface);
        surfaceMap.put(instanceId, innerMap);
    }

    /**
     * Remove surface by id and instance id
     *
     * @param instanceId id of instance
     * @param id id of surface
     */
    public static void removeSurface(int instanceId, long id) {
        if (surfaceMap.containsKey(instanceId)) {
            Map<Long, Surface> innerMap = surfaceMap.get(instanceId);
            innerMap.remove(id);
            if (innerMap.isEmpty()) {
                surfaceMap.remove(instanceId);
            }
        }
    }
}
