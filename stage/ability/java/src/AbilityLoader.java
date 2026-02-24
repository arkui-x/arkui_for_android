/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

package ohos.stage.ability.adapter;

import ohos.ace.adapter.ALog;

/**
 * This class is responsible for loading abilities.
 *
 * @since 2025-12-03
 */
public class AbilityLoader {
    private static final String LOG_TAG = "AbilityLoader";
    private static final String ABILITY_LOADER_INSTANCE_ID = "100000";

    /**
     * Constructor.
     */
    public AbilityLoader() {
        ALog.i(LOG_TAG, "Constructor called.");
    }

    /**
     * Load ability.
     *
     * @param bundleName The bundle name.
     * @param moduleName The module name.
     * @param abilityName The ability name.
     * @param params the want params.
     */
    static public void loadAbility(String bundleName, String moduleName, String abilityName, String params) {
        if (bundleName == null || bundleName.isEmpty()) {
            ALog.e(LOG_TAG, "bundleName is invalid.");
            return;
        }
        if (moduleName == null || moduleName.isEmpty()) {
            ALog.e(LOG_TAG, "moduleName is invalid.");
            return;
        }
        if (abilityName == null || abilityName.isEmpty()) {
            ALog.e(LOG_TAG, "abilityName is invalid.");
            return;
        }
        String instanceName = bundleName + ":" + moduleName + ":" + abilityName + ":" + ABILITY_LOADER_INSTANCE_ID;
        if (params == null) {
            params = "";
        }
        nativeDispatchOnCreate(instanceName, params);
    }

    /**
     * Unload ability.
     *
     * @param bundleName The bundle name.
     * @param moduleName The module name.
     * @param abilityName The ability name.
     */
    static public void unloadAbility(String bundleName, String moduleName, String abilityName) {
        if (bundleName == null || bundleName.isEmpty()) {
            ALog.e(LOG_TAG, "bundleName is invalid.");
            return;
        }
        if (moduleName == null || moduleName.isEmpty()) {
            ALog.e(LOG_TAG, "moduleName is invalid.");
            return;
        }
        if (abilityName == null || abilityName.isEmpty()) {
            ALog.e(LOG_TAG, "abilityName is invalid.");
            return;
        }
        String instanceName = bundleName + ":" + moduleName + ":" + abilityName + ":" + ABILITY_LOADER_INSTANCE_ID;
        nativeDispatchOnDestroy(instanceName);
    }

    static private native void nativeDispatchOnCreate(String instanceName, String params);
    static private native void nativeDispatchOnDestroy(String instanceName);
}
