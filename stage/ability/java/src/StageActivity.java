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
import android.os.Bundle;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicInteger;
import android.util.Log;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import android.content.res.AssetManager;

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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(LOG_TAG, "StageActivity onCreate called");
        super.onCreate(savedInstanceState);
        activityDelegate = new StageActivityDelegate();
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

    /**
     * copy the resources from all modules
     */
    private void copyAllModuleResources() {
        String rootDirectory = "arkui-x";
        AssetManager assets = getAssets();
        String moduleResourcesDirectory = "";
        String moduleResourcesIndex = "";
        List<String> moduleResources = new ArrayList<>();
        try {
            String[] list = assets.list(rootDirectory);
            for (String name: list) {
                if ("systemres".equals(name)) {
                    moduleResources.add(name);
                } else {
                    moduleResourcesDirectory = name + "/" + "resources";
                    moduleResourcesIndex = name + "/" + "resources.index";
                    moduleResources.add(moduleResourcesDirectory);
                    moduleResources.add(moduleResourcesIndex);
                }
            }
        } catch (IOException e) {
            Log.e(LOG_TAG, "read resources err: " + e.getMessage());
        }
        for (String resourcesName : moduleResources) {
            copyFilesFromAssets(rootDirectory + "/" + resourcesName,
                getExternalFilesDir(null).getAbsolutePath() + "/" + resourcesName);
        }
    }

    /**
     * copy the file to the destination path
     * @param assetsPath the path in the assets directory
     * @param savePath the destination path for the copy
     */
    private void copyFilesFromAssets(String assetsPath, String savePath) {
        InputStream is = null;
        FileOutputStream fos = null;
        try {
            String[] fileNames = getAssets().list(assetsPath);
            File file = new File(savePath);
            if (fileNames.length > 0) {
                if (!file.exists()) {
                    file.mkdirs();
                }
                for (String fileName : fileNames) {
                    copyFilesFromAssets(assetsPath + "/" + fileName, savePath + "/" + fileName);
                }
            } else {
                if (file.exists()) {
                    return;
                }
                is = getAssets().open(assetsPath);
                fos = new FileOutputStream(file);
                byte[] buffer = new byte[1024];
                int byteCount = 0;
                while ((byteCount = is.read(buffer)) != -1) {
                    fos.write(buffer, 0, byteCount);
                }
                fos.flush();
            }
        } catch (IOException e) {
            Log.e(LOG_TAG, "read or write data err: " + e.getMessage());
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    Log.e(LOG_TAG, "InputStream close err: " + e.getMessage());
                }
            }
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    Log.e(LOG_TAG, "FileOutputStream close err: " + e.getMessage());
                }
            }
        }
    }
}
