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

package ohos.stage.ability.adapter;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Set;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.os.Trace;

import ohos.ace.adapter.AceEnv;
import ohos.ace.adapter.AcePlatformPlugin;
import ohos.ace.adapter.ArkUIXPluginRegistry;
import ohos.ace.adapter.IArkUIXPlugin;
import ohos.ace.adapter.PluginContext;
import ohos.ace.adapter.capability.video.AceVideoPluginAosp;
import ohos.ace.adapter.capability.web.AceWebPluginAosp;
import ohos.ace.adapter.WindowView;

import ohos.ace.adapter.capability.bridge.BridgeManager;
import ohos.ace.adapter.capability.grantresult.GrantResult;
import ohos.ace.adapter.capability.surface.AceSurfacePluginAosp;
import ohos.ace.adapter.capability.keyboard.KeyboardHeightObserver;
import ohos.ace.adapter.capability.keyboard.KeyboardHeightProvider;

/**
 * A base class for the Ability Cross-platform Environment of the stage model to
 * run on Android.
 * This class is inherited from android Activity.
 * It is entrance of life-cycles of android applications.
 *
 * @since 1
 */
public class StageActivity extends Activity implements KeyboardHeightObserver {
    private static final String LOG_TAG = "StageActivity";

    private static final String INSTANCE_DEFAULT_NAME = "default";

    private static final String WANT_PARAMS = "params";

    private static final String TEST_PARAMS = "test";

    private static boolean isFrist = false;

    private int instanceId = InstanceIdGenerator.getAndIncrement();

    private String instanceName;

    private String bundleName;

    private String moduleName;

    private String abilityName;

    private StageActivityDelegate activityDelegate = null;

    private WindowView windowView = null;

    private AcePlatformPlugin platformPlugin = null;

    private BridgeManager bridgeManager = null;

    private static final int ERR_INVALID_PARAMETERS = -1;

    private static final int ERR_OK = 0;

    private KeyboardHeightProvider keyboardHeightProvider;

    private Set<String> pluginList = new HashSet<>();

    private ArkUIXPluginRegistry arkUIXPluginRegistry = null;

    private PluginContext pluginContext = null;

    @Override
    public void onKeyboardHeightChanged(int height) {
        if (windowView != null) {
            windowView.keyboardHeightChanged(height);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(LOG_TAG, "StageActivity onCreate called");
        super.onCreate(savedInstanceState);
        Trace.beginSection("StageActivity::onCreate");

        Intent intent = getIntent();
        String params = "";
        if (intent != null) {
            params = intent.getStringExtra(WANT_PARAMS);
            if (params == null) {
                params = "";
            }
        }

        activityDelegate = new StageActivityDelegate();
        activityDelegate.attachStageActivity(getInstanceName(), this);
        getIntentToCreateDelegator(intent);
        Trace.beginSection("createWindowView");
        windowView = new WindowView(this);
        Trace.endSection();
        initPlatformPlugin(this, instanceId, windowView);
        initBridgeManager();

        initArkUIXPluginRegistry();
        Trace.beginSection("setContentView");
        setContentView(windowView);
        Trace.endSection();
        activityDelegate.setWindowView(getInstanceName(), windowView);
        activityDelegate.dispatchOnCreate(getInstanceName(), params);
        Trace.endSection();

        keyboardHeightProvider = new KeyboardHeightProvider(this);
 
        windowView.post(new Runnable() {
            public void run() {
                keyboardHeightProvider.start();
            }
        });
    }

    @Override
    protected void onStart() {
        Log.i(LOG_TAG, "StageActivity onStart called");
        super.onStart();
    }

    @Override
    protected void onResume() {
        Log.i(LOG_TAG, "StageActivity onResume called");
        super.onResume();
        Trace.beginSection("StageActivity::onResume");
        activityDelegate.dispatchOnForeground(getInstanceName());
        windowView.foreground();
        if (platformPlugin != null) {
            platformPlugin.notifyLifecycleChanged(false);
        }
        Trace.endSection();

        keyboardHeightProvider.setKeyboardHeightObserver(this);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        Log.i(LOG_TAG, "StageActivity onNewIntent called");
        super.onNewIntent(intent);
        activityDelegate.dispatchOnNewWant(getInstanceName());
    }

    @Override
    protected void onRestart() {
        Log.i(LOG_TAG, "StageActivity onRestart called");
        super.onRestart();
    }

    @Override
    protected void onStop() {
        Log.i(LOG_TAG, "StageActivity onStop called");
        super.onStop();
        activityDelegate.dispatchOnBackground(getInstanceName());
        windowView.background();
        if (platformPlugin != null) {
            platformPlugin.notifyLifecycleChanged(true);
        }
    }

    @Override
    protected void onDestroy() {
        Log.i(LOG_TAG, "StageActivity onDestroy called");
        super.onDestroy();
        activityDelegate.dispatchOnDestroy(getInstanceName());
        windowView.destroy();

        arkUIXPluginRegistry.unRegistryAllPlugins();
        keyboardHeightProvider.close();
        BridgeManager.unRegisterBridgeManager(instanceId);
        if (platformPlugin != null) {
            platformPlugin.releseResRegister(instanceId);
            Log.i(LOG_TAG, "StageActivity onDestroy releseResRegister called");
        }
    }

    @Override
    public void onBackPressed() {
        Log.i(LOG_TAG, "StageActivity onBackPressed called");
        if (!windowView.backPressed()) {
            super.onBackPressed();
        }
    }

    @Override
    public void dump(String prefix, FileDescriptor fd, PrintWriter writer, String[] args) {
        Log.i(LOG_TAG, "StageActivity dump called");
        AceEnv.dump(instanceId, prefix, fd, writer, args);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        Log.i(LOG_TAG, "onActivityResult called");
        super.onActivityResult(requestCode, resultCode, intent);
        String resultWantParams = "";
        if (intent != null) {
            resultWantParams = intent.getStringExtra(WANT_PARAMS);
            if (resultWantParams == null) {
                resultWantParams = "";
            }
        }
        activityDelegate.dispatchOnActivityResult(getInstanceName(), requestCode, resultCode, resultWantParams);
    }

    /**
     * Set the instance name, should called before super.onCreate()
     *
     * @param name the instance name to set
     */
    public void setInstanceName(String name) {
        if (name == null || name.isEmpty()) {
            return;
        }

        instanceName = name + String.valueOf(instanceId);
        String[] nameArray = instanceName.split(":");
        if (nameArray.length >= 3) {
            bundleName = nameArray[0];
            moduleName = nameArray[1];
            abilityName = nameArray[2];
        }
    }

    /**
     * Get the BridgeManager of StageActivity.
     *
     * @return The BridgeManager.
     */
    public BridgeManager getBridgeManager() {
        if (this.bridgeManager == null) {
            this.bridgeManager = new BridgeManager(instanceId);
        }
        return this.bridgeManager;
    }

    private void initBridgeManager() {
        if (bridgeManager == null) {
            getBridgeManager();
        }
        if (BridgeManager.findBridgeManager(instanceId) == null) {
            bridgeManager.nativeInit(instanceId);
            BridgeManager.registerBridgeManager(instanceId, bridgeManager);
        }
    }

    /**
     * Get the Id of StageActivity.
     *
     * @return The InstanceId.
     */
    public int getInstanceId() {
        initBridgeManager();
        return this.instanceId;
    }

    /**
     * Get the instance name.
     *
     * @return The instanceName.
     */
    public String getInstanceName() {
        if (instanceName == null) {
            return INSTANCE_DEFAULT_NAME;
        }
        return instanceName;
    }

    private void getIntentToCreateDelegator(Intent intent) {
        if (intent == null) {
            Log.w(LOG_TAG, "Intent is null.");
            return;
        }
        boolean hasTestValue = intent.hasExtra(TEST_PARAMS);
        if (hasTestValue && !isFrist) {
            Log.i(LOG_TAG, "Start creating abilityDelegate");
            String testBundleName = intent.getStringExtra("bundleName");
            String testModuleName = intent.getStringExtra("moduleName");
            String testRunerName = intent.getStringExtra("unittest");
            String timeout = intent.getStringExtra("timeout");
            activityDelegate.createAbilityDelegator(testBundleName, testModuleName, testRunerName, timeout);
            isFrist = true;
        } else {
            Log.i(LOG_TAG, "No need to start creating abilityDelegate");
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
            String packageName = getApplicationContext().getPackageName();
            Log.i(LOG_TAG, "Current package name: " + packageName);
            ComponentName componentName = null;
            if (packageName == bundleName) {
                componentName = new ComponentName(getBaseContext(), activityName);
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
            String packageName = getApplicationContext().getPackageName();
            Log.i(LOG_TAG, "Current package name: " + packageName);
            ComponentName componentName = null;
            if (packageName == bundleName) {
                componentName = new ComponentName(getBaseContext(), activityName);
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
     * Sets the result code and data to be returned by this Page ability to the caller and destroys this Page ability.
     *
     * @param resultWantParams  the data returned after the ability is destroyed.
     * @param resultCode        the result code returned after the ability is destroyed.
     */
    public void terminateActivityWithResult(String resultWantParams, int resultCode) {
        Log.i(LOG_TAG, "terminateActivityWithResult called, resultCode: " + resultCode);
        Intent intent = new Intent();
        intent.putExtra(WANT_PARAMS, resultWantParams);
        setResult(resultCode, intent);
        this.finish();
    }

    /**
     * Switch to the foreground.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    public int doActivityForeground() {
        Log.i(LOG_TAG, "doActivityForeground called");
        int error = ERR_OK;
        try {
            Intent intent = new Intent(getApplicationContext(), this.getClass());
            intent.setFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
            this.startActivity(intent);
        } catch (ActivityNotFoundException exception) {
            Log.e(LOG_TAG, "switch foreground err.");
            error = ERR_INVALID_PARAMETERS;
        }
        return error;
    }

    /**
     * Switch to the background.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    public int doActivityBackground() {
        Log.i(LOG_TAG, "doActivityBackground called");
        int error = ERR_OK;
        try {
            Intent intent = new Intent(getApplicationContext(), this.getClass());
            intent.setFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
            this.startActivity(intent);
        } catch (ActivityNotFoundException exception) {
            Log.e(LOG_TAG, "switch background err.");
            error = ERR_INVALID_PARAMETERS;
        }
        return error;
    }

    /**
     * Call this when your activity is done and should be closed.
     */
    public void finish() {
        Log.i(LOG_TAG, "StageActivity finish called");
        super.finish();
    }

    /**
     * Initialize platform plugins
     *
     * @param context    Application context
     * @param instanceId the instance id
     * @param windowView the window view
     */
    private void initPlatformPlugin(Context context, int instanceId, WindowView windowView) {
        Trace.beginSection("StageActivity::initPlatformPlugin");
        platformPlugin = new AcePlatformPlugin(context, instanceId, windowView);
        if (platformPlugin != null) {
            windowView.setInputConnectionClient(platformPlugin);
            platformPlugin.initTexturePlugin(instanceId);
            platformPlugin.addResourcePlugin(AceVideoPluginAosp.createRegister(context, moduleName));
            platformPlugin.addResourcePlugin(AceWebPluginAosp.createRegister(context));
            platformPlugin.addResourcePlugin(AceSurfacePluginAosp.createRegister(context));
        }
        Trace.endSection();
    }

    /**
     * Callback for the result from requesting permissions.
     *
     * @param requestCode  The request code passed in {@link #requestPermissions(String[], int)}.
     * @param permissions  permissions The requested permissions. Never null.
     * @param grantResults grantResults The grant results for the corresponding permissions.
     */
    public synchronized void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        Context context = getApplicationContext();
        GrantResult grantResultsClass = new GrantResult(context);
        grantResultsClass.onRequestPremissionCallback(permissions, grantResults);
    }

    /**
     * add ArkUI-X plugin to list for registry.
     * 
     * @param pluginName The full class name includes the package name of the plugin.
     * @since 11
     */
    public void addPlugin(String pluginName) {
        if (pluginName == null) {
            Log.e(LOG_TAG, "plugin name is null!");
        }
        else {
            Log.d(LOG_TAG, "add plugin: " + pluginName);
            pluginList.add(pluginName);
        }
    }

    /**
     * Initialize arkui-x plugins and arkui-x plugins registry.
     */
    private void initArkUIXPluginRegistry() {
        Trace.beginSection("StageActivity::intitArkUIXPlugins");
        this.pluginContext = new PluginContext(this, this.getBridgeManager());
        arkUIXPluginRegistry = new ArkUIXPluginRegistry(this.pluginContext);
        arkUIXPluginRegistry.registryPlugins(pluginList);
        Trace.endSection();
    }
}
