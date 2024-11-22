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

import java.util.Set;

/**
 * Plugin Registry.
 * 
 * Interface to be implemented by Plugin Registry.
 */
public interface IPluginRegistry {
     /**
      * add a plugin to PluginRegistry.
      * 
      * @param name The full class name includes the package name of the plugin.
      */
     void registryPlugin(String name);

     /**
      * add plugins to PluginRegistry.
      * 
      * @param pluginList A set of plugins that need to be added.
      */
     void registryPlugins(Set<String> pluginList);

     /**
      * remove a plugin from PluginRegistry.
      * 
      * @param name The full class name includes the package name of the plugin.
      */
     void unRegistryPlugin(String name);

     /**
      * remove plugins to PluginRegistry.
      * 
      * @param pluginList A set of plugins that need to be removed.
      */
     void unRegistryPlugins(Set<String> pluginList);

     /**
      * remove all plugins from PluginRegistry.
      */
     void unRegistryAllPlugins();

     /**
      * Check whether a plugin has been added to PluginRegistry.
      * 
      * @param name The full class name includes the package name of the plugin.
      * @return return true if the plugin has been added to PluginRegistry.
      */
     boolean hasPlugin(String name);

     /**
      * get a plugin instance that has been added to PluginRegistry.
      * 
      * @param name The full class name includes the package name of the plugin.
      * @return return plugin if the plugin has been added to PluginRegistry.
      */
     IArkUIXPlugin getPlugin(String name);
}
