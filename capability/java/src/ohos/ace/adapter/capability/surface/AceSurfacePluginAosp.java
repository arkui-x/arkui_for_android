/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.surface;

import ohos.ace.adapter.AceResourcePlugin;
import ohos.ace.adapter.ALog;

import android.content.Context;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

/**
 * The class for creating Surface on Android platform.
 *
 * @since 1
 */
public class AceSurfacePluginAosp extends AceResourcePlugin {
    private static final String LOG_TAG = "AceSurfacePluginAosp";
    private final AtomicLong nextSurfaceId = new AtomicLong(0L);

    private final Map<Long, AceSurfaceView> objectMap;

    private final IAceSurface surfaceImpl;

    private Context context = null;

    private int instanceId = 0;

    private AceSurfacePluginAosp(Context context, IAceSurface surfaceImpl, int instanceId) {
        // plugin name is texture, version is 1.0.
        super("surface", 1.0f);
        this.objectMap = new HashMap<Long, AceSurfaceView>();
        this.surfaceImpl = surfaceImpl;
        this.context = context;
        this.instanceId = instanceId;
    }

    /**
     * Create a surface resource register.
     *
     * @param context the context of host activity
     * @param surfaceImpl the interface of ace surface
     * @param instanceId the id of the instance
     * @return surface plugin
     */
    public static AceSurfacePluginAosp createRegister(Context context, IAceSurface surfaceImpl, int instanceId) {
        return new AceSurfacePluginAosp(context, surfaceImpl, instanceId);
    }

    @Override
    public long create(Map<String, String> param) {
        AceSurfaceView aceSurface = new AceSurfaceView(this.context, nextSurfaceId.get(),
                getEventCallback(), param, surfaceImpl, instanceId);
        objectMap.put(nextSurfaceId.get(), aceSurface);
        registerCallMethod(aceSurface.getCallMethod());
        return nextSurfaceId.getAndIncrement();
    }

    /**
     * Get AceSurfaceView object by id.
     *
     * @param id id of object
     * @return object or null if id not found
     */
    @Override
    public Object getObject(long id) {
        return objectMap.get(id);
    }

    /**
     * Release AceSurfaceView by id.
     *
     * @param id id of object
     * @return result of release
     */
    @Override
    public boolean release(long id) {
        if (objectMap.containsKey(id)) {
            AceSurfaceView aceSurface = objectMap.get(id);
            unregisterCallMethod(aceSurface.getCallMethod());
            aceSurface.release();
            objectMap.remove(id);
            return true;
        }
        return false;
    }

    /**
     * Release all AceSurfaceView objects.
     *
     */
    @Override
    public void release() {
        for (Map.Entry<Long, AceSurfaceView> entry : objectMap.entrySet()) {
            entry.getValue().release();
        }
        objectMap.clear();
    }
}