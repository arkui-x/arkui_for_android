/**
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

import android.content.Context;
import android.util.Log;

import java.util.HashSet;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;
import java.util.Set;

public class ArkUIXPluginRegistry implements IPluginRegistry {
    private static final String LOG_TAG = "ArkUIXPluginRegistry";
    private PluginContext pluginContext;
    private Map<String, IArkUIXPlugin> pluginMap = new ConcurrentHashMap<>();

    public ArkUIXPluginRegistry(PluginContext pluginContext) {
        this.pluginContext = pluginContext;
    }

    /**
     * add a ArkUI-X plugin to ArkUI-X PluginRegistry.
     *
     * @param name The full class name includes the package name of the plugin.
     */
    public void registryPlugin(String name) {
        Log.d(LOG_TAG, "registry plugin: " + name);
        Class pluginClass = null;
        try {
            pluginClass = Class.forName(name);
        } catch (ClassNotFoundException e) {
            Log.e(LOG_TAG, "pluginClass do not found: " + e.getMessage());
            return;
        }

        try {
            if (hasPlugin(name)) {
                Log.e(LOG_TAG, "plugin:" + name + "already registered");
            } else {
                IArkUIXPlugin plugin = null;
                plugin = (IArkUIXPlugin) pluginClass.newInstance();
                pluginMap.put(name, plugin);
                plugin.onRegistry(this.pluginContext);
                Log.d(LOG_TAG, "plugin: " + name + "is successfully registered");
            }
        } catch (IllegalAccessException e) {
            Log.e(LOG_TAG, "registry plugin fail: " + e.getMessage());
        } catch (InstantiationException e) {
            Log.e(LOG_TAG, "registry plugin fail: " + e.getMessage());
        }
    }

    /**
     * add ArkUI-X plugins to ArkUI-X PluginRegistry.
     *
     * @param pluginList A set of ArkUI-X plugins that need to be added.
     */
    public void registryPlugins(Set<String> pluginList) {
        for (String pluginName : pluginList) {
            registryPlugin(pluginName);
        }
    }

    /**
     * remove a ArkUI-X plugin frome ArkUI-X PluginRegistry.
     *
     * @param name The full class name includes the package name of the plugin.
     */
    public void unRegistryPlugin(String name) {
        IArkUIXPlugin plugin = pluginMap.get(name);
        if (plugin != null) {
            Log.d(LOG_TAG, "unregistry plugin: " + name);
            plugin.onUnRegistry(this.pluginContext);
            pluginMap.remove(name);
        }
    }

    /**
     * remove ArkUI-X plugins to ArkUI-X PluginRegistry.
     *
     * @param pluginList A set of ArkUI-X plugins that need to be removed.
     */
    public void unRegistryPlugins(Set<String> pluginList) {
        for (String pluginName : pluginList) {
            unRegistryPlugin(pluginName);
        }
    }

    /**
     * remove all plugins from ArkUI-X PluginRegistry.
     */
    public void unRegistryAllPlugins() {
        unRegistryPlugins(new HashSet<>(pluginMap.keySet()));
        pluginMap.clear();
    }

    /**
     * Check whether a ArkUI-X plugin has been added to ArkUI-X PluginRegistry.
     *
     * @param name The full class name includes the package name of the plugin.
     * @return return true if the plugin has been added to ArkUI-X PluginRegistry.
     */
    public boolean hasPlugin(String name) {
        return pluginMap.containsKey(name);
    }

    /**
     * get a ArkUI-X plugin instance that has been added to ArkUI-X PluginRegistry.
     *
     * @param name The full class name includes the package name of the plugin.
     * @return return plugin if the plugin has been added to ArkUI-X PluginRegistry.
     */
    public IArkUIXPlugin getPlugin(String name) {
        if (hasPlugin(name)) {
            return pluginMap.get(name);
        }
        Log.e(LOG_TAG, "get plugin: " + name + " failed!");
        return null;
    }
}
