/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.platformview;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

import ohos.ace.adapter.AceResourcePlugin;
/**
 * The class for creating  PlatformView on Android platform.
 *
 * @since 1
 */
public abstract class AcePlatformViewPluginBase extends AceResourcePlugin {
    private final AtomicLong nextPlatformViewId = new AtomicLong(0L);

    private Map<Long, AcePlatformViewBase> objectMap;

    /**
     * constructor of AcePlatformViewPluginBase
     *
     */
    public AcePlatformViewPluginBase() {
        // plugin name is PlatformView, version is 1.0.
        super("platformview", 1.0f);
        objectMap = new HashMap<Long, AcePlatformViewBase>();
    }

    /**
     * This is called to get a atomic id.
     *
     * @return id
     */
    public long getAtomicId() {
        return nextPlatformViewId.getAndIncrement();
    }

    /**
     * This is called to add a resource object to map.
     *
     * @param id PlatformView id
     * @param view PlatformView object
     */
    public void addResource(long id, AcePlatformViewBase view) {
        objectMap.put(id, view);
        registerCallMethod(view.getCallMethod());
    }

    /**
     * This is called to create AcePlatformViewBase.
     *
     * @param param calling param
     * @return resource id
     */
    @Override
    public abstract long create(Map<String, String> param);

    /**
     * This is called to register PlatformViewFactory.
     *
     * @param platformViewFactory PlatformViewFactory object
     */
    public abstract void registerPlatformViewFactory(PlatformViewFactory platformViewFactory);

    /**
     * This is called to get AcePlatformViewBase object by id.
     *
     * @param id id of object
     * @return object or null if id not found
     */
    @Override
    public Object getObject(long id) {
        return objectMap.get(id);
    }

    public Map<Long, AcePlatformViewBase> getObjectMap() {
        return objectMap;
    }

    /**
     * This is called for activity resume.
     */
    public void onActivityResume() {
        for (Map.Entry<Long, AcePlatformViewBase> entry : objectMap.entrySet()) {
            if (entry.getValue() == null) {
                continue;
            }
            entry.getValue().onActivityResume();
        }
    }

    /**
     * This is called for activity pause.
     */
    public void onActivityPause() {
        for (Map.Entry<Long, AcePlatformViewBase> entry : objectMap.entrySet()) {
            entry.getValue().onActivityPause();
        }
    }

    /**
     * This is called to release AcePlatformViewBase by id.
     *
     * @param id id of object
     * @return result of release
     */
    @Override
    public boolean release(long id) {
        if (objectMap.containsKey(id)) {
            AcePlatformViewBase view = objectMap.get(id);
            unregisterCallMethod(view.getCallMethod());
            view.release();
            objectMap.remove(id);
            return true;
        }
        return false;
    }

    /**
     * This is called to release all AcePlatformViewBase.
     */
    @Override
    public void release() {
        for (Map.Entry<Long, AcePlatformViewBase> entry : objectMap.entrySet()) {
            entry.getValue().release();
        }
        objectMap.clear();
    }

    /**
     * notify activity lifecycle changed to plugin.
     *
     * @param isBackground to background state
     */
    @Override
    public void notifyLifecycleChanged(Boolean isBackground) {
        if (isBackground) {
            onActivityPause();
        } else {
            onActivityResume();
        }
    }
}