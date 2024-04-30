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

package ohos.stage.ability.adapter;

import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Trace;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.fragment.app.Fragment;

import java.util.HashSet;
import java.util.Set;

import ohos.ace.adapter.AcePlatformPlugin;
import ohos.ace.adapter.ArkUIXPluginRegistry;
import ohos.ace.adapter.PluginContext;
import ohos.ace.adapter.WindowView;
import ohos.ace.adapter.capability.bridge.BridgeManager;
import ohos.ace.adapter.capability.surface.AceSurfacePluginAosp;
import ohos.ace.adapter.capability.video.AceVideoPluginAosp;
import ohos.ace.adapter.capability.web.AceWebPluginAosp;
import ohos.ace.adapter.capability.web.AceWebPluginBase;
import ohos.stage.ability.adapter.StageApplicationDelegate;

/**
 * A base class for the Ability Cross-platform Environment of the stage model to
* run on Android.
* This class is inherited from android Fragment.
*
* @since 1
*/
public class StageFragment extends Fragment {
    private static final String LOG_TAG = "StageFragment";

    private static final String INSTANCE_DEFAULT_NAME = "default";

    private int instanceId = InstanceIdGenerator.getAndIncrement();

    private String instanceName;

    private String moduleName;

    private String bundleName;

    private String abilityName;

    private StageFragmentDelegate fragmentDelegate = null;

    private WindowView windowView = null;

    private FrameLayout framelayout;

    private static final String WANT_PARAMS = "params";

    private static final int ERR_INVALID_PARAMETERS = -1;

    private static final int ERR_OK = 0;

    private AcePlatformPlugin platformPlugin = null;

    private BridgeManager bridgeManager = null;

    private Set<String> pluginList = new HashSet();

    private ArkUIXPluginRegistry arkUIXPluginRegistry = null;

    private PluginContext pluginContext = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(LOG_TAG, "OnCreate called, instance name:" + getInstanceName());
        super.onCreate(savedInstanceState);
        Trace.beginSection("StageFragment::onCreate");

        fragmentDelegate = new StageFragmentDelegate();
        fragmentDelegate.attachStageFragment(getInstanceName(), this);
        Trace.beginSection("createWindowView");
        windowView = new WindowView(this.getActivity());
        Trace.endSection();

        fragmentDelegate.setWindowView(getInstanceName(), windowView);
        fragmentDelegate.dispatchOnCreate(getInstanceName());

        initPlatformPlugin(this.getActivity(), instanceId, windowView);
        initBridgeManager();
        initArkUIXPluginRegistry();
        Trace.endSection();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.i(LOG_TAG, "OnCreateView called, instance name:" + getInstanceName());
        if (framelayout == null) {
            framelayout = new FrameLayout(getActivity());
            FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT);
            framelayout.setLayoutParams(params);
            framelayout.addView(windowView, 0);
        }        
        return framelayout;
    }

    @Override
    public void onResume() {
        Log.i(LOG_TAG, "OnResume called, instance name:" + getInstanceName());
        super.onResume();
        Trace.beginSection("StageFragment::onResume");
        fragmentDelegate.dispatchOnForeground(getInstanceName());
        windowView.foreground();
        if (platformPlugin != null) {
            platformPlugin.notifyLifecycleChanged(false);
        }
        Trace.endSection();
    }

    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        super.setUserVisibleHint(isVisibleToUser);
        if (getView() == null || windowView == null) {
            return;
        }
        Trace.beginSection("StageFragment::setUserVisibleHint");  
        if (isVisibleToUser) {
            Log.i(LOG_TAG, "StageFragment isVisible to User, instance name:" + getInstanceName());
            if (platformPlugin != null) {
                platformPlugin.notifyLifecycleChanged(false);
            }
        } else {
            Log.i(LOG_TAG, "StageFragment is not visible to User, instance name:" + getInstanceName());
            if (platformPlugin != null) {
                platformPlugin.notifyLifecycleChanged(true);
            }
        }
        Trace.endSection();
    }

    @Override
    public void onStop() {
        Log.i(LOG_TAG, "OnStop called, instance name:" + getInstanceName());
        super.onStop();
        fragmentDelegate.dispatchOnBackground(getInstanceName());
        windowView.background();
        if (platformPlugin != null) {
            platformPlugin.notifyLifecycleChanged(true);
        }
    }

    @Override
    public void onDestroy() {
        Log.i(LOG_TAG, "OnDestroy called, instance name:" + getInstanceName());
        super.onDestroy();
        fragmentDelegate.dispatchOnDestroy(getInstanceName());
        windowView.destroy();
        arkUIXPluginRegistry.unRegistryAllPlugins();
        BridgeManager.unRegisterBridgeManager(instanceId);
        if (platformPlugin != null) {
            platformPlugin.release();
            Log.i(LOG_TAG, "StageFragment onDestroy releseResRegister called");
        }
    }

    /**
     * Start a new activity.
     *
     * @param bundleName   the package name.
     * @param activityName the activity name.
     * @param params       the want params.
     * @return Returns ERR_OK on success, others on failure.
     */
    public int startActivity(String bundleName, String activityName, String params) {
        Log.i(LOG_TAG, "startActivity called, bundleName: " + bundleName + ", activityName: " + activityName);
        int error = ERR_OK;
        try {
            Intent intent = new Intent();
            String packageName = this.getActivity().getApplicationContext().getPackageName();
            Log.i(LOG_TAG, "Current package name: " + packageName);
            ComponentName componentName = null;
            if (packageName.equals(bundleName)) {
                componentName = new ComponentName(this.getActivity().getBaseContext(), activityName);
            } else {
                componentName = new ComponentName(bundleName, activityName);
            }
            intent.setComponent(componentName);
            intent.putExtra(WANT_PARAMS, params);
            this.startActivity(intent);
        } catch (ActivityNotFoundException exception) {
            Log.e("StageApplication", "start activity err: " + exception.getMessage());
            error = ERR_INVALID_PARAMETERS;
        }
        return error;
    }

    /**
     * Start a new activity for which you would like a result when it finished.
     *
     * @param bundleName   the package name.
     * @param activityName the activity name.
     * @param params       the want params.
     * @param requestCode  If >= 0, this code will be returned in onActivityResult() when the activity exits.
     * @return Returns ERR_OK on success, others on failure.
     */
    public int startActivityForResult(String bundleName, String activityName, String params, int requestCode) {
        Log.i(LOG_TAG, "startActivityForResult called, bundleName: " + bundleName + ", activityName: " + activityName
            + ", requestCode: " + requestCode);
        int error = ERR_OK;
        try {
            Intent intent = new Intent();
            String packageName = this.getActivity().getApplicationContext().getPackageName();
            Log.i(LOG_TAG, "Current package name: " + packageName);
            ComponentName componentName = null;
            if (packageName.equals(bundleName)) {
                componentName = new ComponentName(this.getActivity().getBaseContext(), activityName);
            } else {
                componentName = new ComponentName(bundleName, activityName);
            }
            intent.setComponent(componentName);
            intent.putExtra(WANT_PARAMS, params);
            this.startActivityForResult(intent, requestCode);
        } catch (ActivityNotFoundException exception) {
            Log.e(LOG_TAG, "start activity for result err: " + exception.getMessage());
            error = ERR_INVALID_PARAMETERS;
        }
        return error;
    }

    /**
     * Set the instance name, should called before super.onCreate()
    *
    * @param name the instance name to set
    */
    public void setInstanceName(String name) {
        if (name != null && !name.isEmpty()) {
            instanceName = name + String.valueOf(instanceId);
            String[] nameArray = instanceName.split(":");
            if (nameArray.length >= 3) {
                bundleName = nameArray[0];
                moduleName = nameArray[1];
                abilityName = nameArray[2];
            }
        }
    }

    /**
     * Get the instance name.
    *
    * @return The instanceName.
    */
    public String getInstanceName() {
        return instanceName == null ? INSTANCE_DEFAULT_NAME : instanceName;
    }

    /**
     * Initialize platform plugins
     *
     * @param context    Application context
     * @param instanceId the instance id
     * @param windowView the window view
     */
    private void initPlatformPlugin(Context context, int instanceId, WindowView windowView) {
        Trace.beginSection("StageFragment::initPlatformPlugin");
        platformPlugin = new AcePlatformPlugin(context, instanceId, windowView);
        if (platformPlugin != null) {
            windowView.setInputConnectionClient(platformPlugin);
            platformPlugin.initTexturePlugin(instanceId);
            platformPlugin.addResourcePlugin(AceVideoPluginAosp.createRegister(context, moduleName));
            AceWebPluginBase web = AceWebPluginAosp.createRegister(context, windowView);
            windowView.setWebPlugin(web);
            platformPlugin.addResourcePlugin(web);
            platformPlugin.initSurfacePlugin(context, instanceId);
        }
        Trace.endSection();
    }

    private void initBridgeManager() {
        Trace.beginSection("StageFragment::initBridgeManager");
        if (bridgeManager == null) {
            getBridgeManager();
        }
        if (BridgeManager.findBridgeManager(instanceId) == null) {
            bridgeManager.nativeInit(instanceId);
            BridgeManager.registerBridgeManager(instanceId, bridgeManager);
        }
        Trace.endSection();
    }

    /**
     * Initialize arkui-x plugins and arkui-x plugins registry.
     */
    private void initArkUIXPluginRegistry() {
        Trace.beginSection("StageFragment::intitArkUIXPlugins");
        arkUIXPluginRegistry = new ArkUIXPluginRegistry(getPluginContext());
        arkUIXPluginRegistry.registryPlugins(pluginList);
        Trace.endSection();
    }

    /**
     * Get the BridgeManager of StageFragment.
     *
     * @return The BridgeManager.
     */
    public BridgeManager getBridgeManager() {
        if (bridgeManager == null) {
            bridgeManager = new BridgeManager(instanceId);
        }
        return bridgeManager;
    }

    public PluginContext getPluginContext() {
        if (pluginContext == null) {
            pluginContext = new PluginContext(this.getActivity(), getBridgeManager(), moduleName);
        }
        return pluginContext;
    }

    /**
     * Call this when your fragment is done and should be closed.
     */
    public void finish() {
        Log.i(LOG_TAG, "StageFragment finish called.");
        return;
    }
}
