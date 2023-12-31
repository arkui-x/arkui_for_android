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

package ohos.ace.adapter.capability.web;

import ohos.ace.adapter.ALog;
import ohos.ace.adapter.AceResourcePlugin;

import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.atomic.AtomicLong;

/**
 * The class is the base class for creating web on Android and ohos platform.
 *
 * @since 1
 */
public abstract class AceWebPluginBase extends AceResourcePlugin {
    private static final String LOG_TAG = "AceWebPluginBase";

    private static Map<Long, AceWebBase> objectMap;

    private final AtomicLong nextWebId = new AtomicLong(0L);

    protected native void nativeInit();

    private static boolean hasInit = false;

    public AceWebPluginBase() {
        // plugin name is web, version is 1.0.
        super("web", 1.0f);
        objectMap = new HashMap<Long, AceWebBase>();
    }

    /**
     * This is called to get a atomic id.
     *
     * @return id
     */
    public long getAtomicId() {
        return nextWebId.getAndIncrement();
    }

    /**
     * This is called to add a resource object to map.
     *
     * @param id web id
     * @param web web object
     */
    public void addResource(long id, AceWebBase web) {
        objectMap.put(id, web);
        registerCallMethod(web.getCallMethod());
        if (!hasInit) {
            nativeInit();
            hasInit = true;
        }
    }

    /**
     * This is called to create AceWebBase.
     *
     * @param param calling param
     * @return resource id
     */
    public abstract long create(Map<String, String> param);

    /**
     * This is called to get AceWebBase object by id.
     *
     * @param id id of object
     * @return object or null if id not found
     */
    public Object getObject(long id) {
        if (objectMap.containsKey(id)) {
            return objectMap.get(id);
        }
        return null;
    }

    public static Map<Long, AceWebBase> getObjectMap() {
        return objectMap;
    }

    /**
     * This is called for activity resume.
     *
     */
    public void onActivityResume() {
        for (Map.Entry<Long, AceWebBase> entry : objectMap.entrySet()) {
            entry.getValue().onActivityResume();
        }
    }

    /**
     * This is called for activity pause.
     *
     */
    public void onActivityPause() {
        for (Map.Entry<Long, AceWebBase> entry : objectMap.entrySet()) {
            entry.getValue().onActivityPause();
        }
    }

    /**
     * This is called to release AceWebBase by id.
     *
     * @param id id of object
     * @return result of release
     */
    public boolean release(long id) {
        if (objectMap.containsKey(id)) {
            AceWebBase web = objectMap.get(id);
            unregisterCallMethod(web.getCallMethod());
            web.release();
            objectMap.remove(id);
            return true;
        }
        return false;
    }

    /**
     * This is called to release all AceWebBase.
     *
     */
    public void release() {
        for (Map.Entry<Long, AceWebBase> entry : objectMap.entrySet()) {
            entry.getValue().release();
        }
    }

    public void loadUrl(long id, String url, HashMap<String, String> header) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            webBase.loadUrl(url, header);
        }
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
