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

/**
 * ArkUI-X plugin.
 * 
 * Interface to be implemented by all ArkUI-X plugins.
 * 
 * @since 11
 */
public interface IArkUIXPlugin {
    /**
     * ArkUI-X plugin registry callback, called at registry.
     * 
     * @param pluginContext provides the resources that the plugin might need.
     * @since 11
     */
    void onRegistry(PluginContext pluginContext);

    /**
     * ArkUI-X plugin unregistry callback, called at unregistry.
     * 
     * @since 11
     */
    void onUnRegistry(PluginContext pluginContext);
}
