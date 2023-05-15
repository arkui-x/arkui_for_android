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

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import ohos.ace.adapter.AceEnv;
import ohos.ace.adapter.AcePlatformPlugin;
import ohos.ace.adapter.capability.video.AceVideoPluginAosp;
import ohos.ace.adapter.WindowView;

import ohos.ace.adapter.capability.grantresult.GrantResult;
import ohos.ace.adapter.capability.surface.AceSurfacePluginAosp;

/**
 * A base class for the Ability Cross-platform Environment of the stage model to run on Android.
 * This class is inherited from android Activity.
 * It is entrance of life-cycles of android applications.
 *
 * @since 1
 */
public class StageActivity extends Activity {
    private static final String LOG_TAG = "StageActivity";

    private static final String INSTANCE_DEFAULT_NAME = "default";

    private static final String WANT_PARAMS = "params";

    private int instanceId = InstanceIdGenerator.getAndIncrement();

    private String instanceName;

    private String bundleName;

    private String moduleName;

    private String abilityName;

    private StageActivityDelegate activityDelegate = null;

    private WindowView windowView = null;

    private AcePlatformPlugin platformPlugin = null;

    private static final int ERR_INVALID_PARAMETERS = -1;

    private static final int ERR_OK = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(LOG_TAG, "StageActivity onCreate called");
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();
        String params = "";
        if (intent != null) {
            params = intent.getStringExtra(WANT_PARAMS);
        }
        Log.i(LOG_TAG, "params: " + params);

        activityDelegate = new StageActivityDelegate();
        activityDelegate.attachStageActivity(getInstanceName(), this);

        windowView = new WindowView(this);
        initPlatformPlugin(this, instanceId, windowView);

        setContentView(windowView);
        activityDelegate.setWindowView(getInstanceName(), windowView);
        activityDelegate.dispatchOnCreate(getInstanceName(), params);
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
        activityDelegate.dispatchOnForeground(getInstanceName());
        windowView.foreground();
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
    }

    @Override
    protected void onDestroy() {
        Log.i(LOG_TAG, "StageActivity onDestroy called");
        super.onDestroy();
        activityDelegate.dispatchOnDestroy(getInstanceName());
        windowView.destroy();
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
     * Get the Id of StageActivity.
     *
     * @return The InstanceId.
     */
    public int getInstanceId() {
        return this.instanceId;
    }

    private String getInstanceName() {
        if (instanceName == null) {
            return INSTANCE_DEFAULT_NAME;
        }
        return instanceName;
    }

    /**
     * Start a new activity.
     *
     * @param bundleName the package name.
     * @param activityName the activity name.
     * @param params the want params.
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
     * Call this when your activity is done and should be closed.
     */
    public void finish() {
        Log.i(LOG_TAG, "StageActivity finish called");
        super.finish();
    }

    /**
     * Initialize platform plugins
     *
     * @param context Application context
     * @param instanceId the instance id
     * @param windowView the window view
     */
    private void initPlatformPlugin(Context context, int instanceId, WindowView windowView) {
        platformPlugin = new AcePlatformPlugin(context, instanceId, windowView, 0L);
        if (platformPlugin != null) {
            windowView.setInputConnectionClient(platformPlugin);
            platformPlugin.initTexturePlugin(instanceId);
            platformPlugin.addResourcePlugin(AceVideoPluginAosp.createRegister(context, moduleName));
            platformPlugin.addResourcePlugin(AceSurfacePluginAosp.createRegister(context));
        }
    }

    /**
     * Callback for the result from requesting permissions.
     *
     * @param requestCode The request code passed in {@link #requestPermissions(String[], int)}.
     * @param permissions permissions The requested permissions. Never null.
     * @param grantResults grantResults The grant results for the corresponding permissions.
     */
    public synchronized void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        Context context = getApplicationContext();
        GrantResult grantResultsClass = new GrantResult(context);
        grantResultsClass.onRequestPremissionCallback(permissions, grantResults);
    }
}
