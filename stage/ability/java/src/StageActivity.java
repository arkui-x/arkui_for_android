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

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import ohos.ace.adapter.AcePlatformPlugin;
import ohos.ace.adapter.WindowView;

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

    private int instanceId = InstanceIdGenerator.getAndIncrement();

    private String instanceName;

    private StageActivityDelegate activityDelegate = null;

    private WindowView windowView = null;

    private AcePlatformPlugin platformPlugin = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(LOG_TAG, "StageActivity onCreate called");
        super.onCreate(savedInstanceState);
        activityDelegate = new StageActivityDelegate();
        activityDelegate.attachStageActivity(getInstanceName(), this);

        windowView = new WindowView(this);
        initPlatformPlugin(this, instanceId, windowView);

        setContentView(windowView);
        activityDelegate.setWindowView(getInstanceName(), windowView);
        activityDelegate.dispatchOnCreate(getInstanceName());
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
     */
    public void startActivity(String bundleName, String activityName) {
        Log.i(LOG_TAG, "startActivity called, bundleName: " + bundleName + ", activityName: " + activityName);
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
        this.startActivity(intent);
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
    }
}
