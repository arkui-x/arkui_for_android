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

import android.content.ComponentName;
import android.app.Activity;
import android.os.Bundle;
import java.util.concurrent.atomic.AtomicInteger;
import android.util.Log;
import android.content.Intent;
import ohos.ace.adapter.WindowView;

/**
 * A base class for the Ability Cross-platform Environment to run on
 * Android. This class is inherited from
 * android Activity. It is entrance of life-cycles of android applications.
 */
public class StageActivity extends Activity {
    private static final String LOG_TAG = "StageActivity";

    private static final String INSTANCE_DEFAULT_NAME = "default";

    private static final AtomicInteger ID_GENERATOR = new AtomicInteger(1);

    private int activityId = ID_GENERATOR.getAndIncrement();

    private String instanceName;

    private StageActivityDelegate activityDelegate = null;

    private WindowView windowView = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(LOG_TAG, "StageActivity onCreate called");
        super.onCreate(savedInstanceState);
        activityDelegate = new StageActivityDelegate();
        activityDelegate.attachStageActivity(this);

        windowView = new WindowView(this, activityId);
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
    }

    @Override
    protected void onDestroy() {
        Log.i(LOG_TAG, "StageActivity onDestroy called");
        super.onDestroy();
        activityDelegate.dispatchOnDestroy(getInstanceName());
    }

    @Override
    public void onBackPressed() {
        Log.i(LOG_TAG, "StageActivity onBackPressed called");
    }

    /**
     * set the instance name, should called before super.onCreate()
     *
     * @param name the instance name to set
     */
    public void setInstanceName(String name) {
        if (name == null || name.isEmpty()) {
            return;
        }

        instanceName = name + String.valueOf(activityId);
        activityId = ID_GENERATOR.getAndIncrement();
    }

    private String getInstanceName() {
        if (instanceName == null) {
            return INSTANCE_DEFAULT_NAME;
        }
        return instanceName;
    }

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

    public void finish() {
        Log.i(LOG_TAG, "StageActivity finish called");
        super.finish();
    }
}
