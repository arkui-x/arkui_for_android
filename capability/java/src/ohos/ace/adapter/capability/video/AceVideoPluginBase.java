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

package ohos.ace.adapter.capability.video;

import ohos.ace.adapter.AceResourcePlugin;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

/**
 * The class for creating VideoPlayer on Android platform.
 *
 * @since 1
 */
public abstract class AceVideoPluginBase extends AceResourcePlugin {
    private final AtomicLong nextVideoId = new AtomicLong(0L);

    private final Map<Long, AceVideoBase> objectMap;

    /**
     * constructor of AceVideoPluginBase
     *
     */
    public AceVideoPluginBase() {
        // plugin name is video, version is 1.0.
        super("video", 1.0f);
        objectMap = new HashMap<Long, AceVideoBase>();
    }

    /**
     * This is called to get a atomic id.
     *
     * @return id
     */
    public long getAtomicId() {
        return nextVideoId.getAndIncrement();
    }

    /**
     * This is called to add a resource object to map.
     *
     * @param id video id
     * @param video video object
     */
    public void addResource(long id, AceVideoBase video) {
        objectMap.put(id, video);
        registerCallMethod(video.getCallMethod());
    }

    /**
     * This is called to create AceVideoBase.
     *
     * @param param calling param
     * @return resource id
     */
    public abstract long create(Map<String, String> param);

    /**
     * This is called to get AceVideoBase object by id.
     *
     * @param id id of object
     * @return object or null if id not found
     */
    public Object getObject(long id) {
        return objectMap.get(id);
    }

    /**
     * This is called for activity resume.
     */
    public void onActivityResume() {
        for (Map.Entry<Long, AceVideoBase> entry : objectMap.entrySet()) {
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
        for (Map.Entry<Long, AceVideoBase> entry : objectMap.entrySet()) {
            entry.getValue().onActivityPause();
        }
    }

    /**
     * This is called to release AceVideoBase by id.
     *
     * @param id id of object
     * @return result of release
     */
    public boolean release(long id) {
        if (objectMap.containsKey(id)) {
            AceVideoBase video = objectMap.get(id);
            unregisterCallMethod(video.getCallMethod());
            video.release();
            objectMap.remove(id);
            return true;
        }
        return false;
    }

    /**
     * This is called to release all AceVideoBase.
     */
    public void release() {
        for (Map.Entry<Long, AceVideoBase> entry : objectMap.entrySet()) {
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