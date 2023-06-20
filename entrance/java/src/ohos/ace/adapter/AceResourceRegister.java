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

package ohos.ace.adapter;

import java.util.HashMap;
import java.util.Map;

/**
 * The platform resource register.
 *
 * @since 1
 */
public final class AceResourceRegister {
    private static final String LOG_TAG = "AceResourceRegister";

    private static final String PARAM_AND = "#HWJS-&-#";

    private static final String PARMA_EQUALS = "#HWJS-=-#";

    private static final String PARAM_AT = "@";

    private static final int SPLIT_COUNT = 2;

    AceResourceRegister self = this;

    private Map<String, AceResourcePlugin> pluginMap;

    private Map<String, IAceOnCallResourceMethod> callMethodMap;

    private IAceOnResourceEvent callbackHandler;

    private long aceRegisterPtr;

    /**
     * Constructor of resource register.
     */
    public AceResourceRegister() {
        pluginMap = new HashMap<String, AceResourcePlugin>();
        callMethodMap = new HashMap<String, IAceOnCallResourceMethod>();
        callbackHandler = new IAceOnResourceEvent() {

            /**
             * Fire event to native
             *
             * @param eventId event id
             * @param param param
             */
            public void onEvent(String eventId, String param) {
                self.onEvent(eventId, param);
            }
        };
    }

    /**
     * Get the plugin map
     *
     * @return the plugin map
     */
    public Map<String, AceResourcePlugin> getPluginMap() {
        return pluginMap;
    }

    /**
     * Register resource plugin
     *
     * @param plugin resource plugin
     */
    public void registerPlugin(AceResourcePlugin plugin) {
        if (plugin == null) {
            ALog.e(LOG_TAG, "null plugin");
            return;
        }
        if (pluginMap.containsKey(plugin.pluginType())) {
            AceResourcePlugin oldPlugin = pluginMap.get(plugin.pluginType());
            if (plugin.version() <= oldPlugin.version()) {
                return;
            }
        }
        ALog.i(LOG_TAG, "register plugin " + plugin.pluginType());
        pluginMap.put(plugin.pluginType(), plugin);
        plugin.setEventCallback(this, callbackHandler);
    }

    /**
     * Register the native ptr
     *
     * @param aceRegisterPtr the native ptr of ResourceRegister
     */
    public void setRegisterPtr(long aceRegisterPtr) {
        this.aceRegisterPtr = aceRegisterPtr;
    }

    /**
     * Create resource with type and params
     *
     * @param resourceType type of resource
     * @param param param
     * @return id of resource
     */
    public long createResource(String resourceType, String param) {
        ALog.i(LOG_TAG, "createResource " + resourceType + " called");
        if (pluginMap.containsKey(resourceType)) {
            AceResourcePlugin plugin = pluginMap.get(resourceType);
            if (plugin != null) {
                return plugin.create(buildParamMap(param));
            } else {
                ALog.e(LOG_TAG, "createResource " + resourceType + " failed!");
            }
        }
        return -1;
    }

    /**
     * Release resource by hash
     *
     * @param resourceHash the hash of resource
     * @return result of release
     */
    public boolean releaseResource(String resourceHash) {
        try {
            String[] split = resourceHash.split(PARAM_AT);
            if (split.length == SPLIT_COUNT) {
                if (pluginMap.containsKey(split[0])) {
                    AceResourcePlugin plugin = pluginMap.get(split[0]);
                    if (plugin != null) {
                        long id = Long.parseLong(split[1]);
                        return plugin.release(id);
                    }
                }
            }
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "Number format exception");
        }
        return false;
    }

    /**
     * Get resource object by resource hash
     *
     * @param resourceHash hash of resource
     * @return object or null if resource not found
     */
    public Object getObject(String resourceHash) {
        try {
            String[] split = resourceHash.split(PARAM_AT);
            if (split.length == SPLIT_COUNT) {
                if (pluginMap.containsKey(split[0])) {
                    AceResourcePlugin plugin = pluginMap.get(split[0]);
                    if (plugin != null) {
                        long id = Long.parseLong(split[1]);
                        return plugin.getObject(id);
                    }
                }
            }
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "Number format exception");
        }
        return null;
    }

    /**
     * Get resource object by type and id
     *
     * @param resourceType type of resource
     * @param id id of resource
     * @return object or null if resource not found
     */
    public Object getObject(String resourceType, long id) {
        if (pluginMap.containsKey(resourceType)) {
            AceResourcePlugin plugin = pluginMap.get(resourceType);
            if (plugin != null) {
                return plugin.getObject(id);
            }
        }
        return null;
    }

    /**
     * Fire event to native
     *
     * @param eventId event id
     * @param param param
     */
    public void onEvent(String eventId, String param) {
        if (aceRegisterPtr != 0L) {
            nativeOnEvent(aceRegisterPtr, eventId, param);
        }
    }

    /**
     * Call resource method
     *
     * @param methodId method id
     * @param param param
     * @return result of method return
     */
    public String onCallMethod(String methodId, String param) {
        IAceOnCallResourceMethod resourceMethod = null;

        if (callMethodMap.containsKey(methodId)) {
            resourceMethod = callMethodMap.get(methodId);
        }

        if (resourceMethod != null) {
            return resourceMethod.onCall(buildParamMap(param));
        }
        return "no method found";
    }

    /**
     * Register call methods by id
     *
     * @param methodId method id
     * @param callMethod method call interface
     */
    public void registerCallMethod(String methodId, IAceOnCallResourceMethod callMethod) {
        callMethodMap.put(methodId, callMethod);
    }

    /**
     * Unregister call methods by id
     *
     * @param methodId method id
     */
    public void unregisterCallMethod(String methodId) {
        callMethodMap.remove(methodId);
    }

    /**
     * Release all plugin resources
     *
     */
    public void release() {
        for (Map.Entry<String, AceResourcePlugin> entry : pluginMap.entrySet()) {
            entry.getValue().release();
        }
        aceRegisterPtr = 0L;
    }

    /**
     * notify activity lifecycle changed to plugin.
     *
     * @param isBackground to background state
     */
    public void notifyLifecycleChanged(Boolean isBackground) {
        for (Map.Entry<String, AceResourcePlugin> entry : pluginMap.entrySet()) {
            entry.getValue().notifyLifecycleChanged(isBackground);
        }
    }

    private Map<String, String> buildParamMap(String params) {
        Map<String, String> paramMap = new HashMap<String, String>();

        if (params != null && !params.isEmpty()) {
            String[] paramSplit = params.split(PARAM_AND);

            for (String param : paramSplit) {
                String[] valueSplit = param.split(PARMA_EQUALS);

                if (valueSplit.length == SPLIT_COUNT) {
                    paramMap.put(valueSplit[0], valueSplit[1]);
                }
            }
        }
        return paramMap;
    }

    private native void nativeOnEvent(long resRigsterPtr, String eventId, String param);
}
