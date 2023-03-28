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

import android.app.Application;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Process;
import android.util.Log;
import ohos.ace.adapter.AceEnv;
import android.content.Context;
import android.content.res.AssetManager;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import ohos.ace.adapter.AppModeConfig;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileOutputStream;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.LoggerAosp;
import java.util.Locale;

/**
 * This class extends from Android Application, the entry of app
 */
public class StageApplication extends Application {
    private static final String LOG_TAG = "StageApplication";

    private static final String ASSETS_SUB_PATH = "arkui-x";

    private static final String TEMP_DIR = "/temp";

    private static final String FILES_DIR = "/files";

    private static final String PREFERENCE_DIR = "/preference";

    private static final String DATABASE_DIR = "/database";

    private StageApplicationDelegate appDelegate = null;

    /**
     * Call when Application is created.
     *
     */
    @Override
    public void onCreate() {
        Log.i(LOG_TAG, "StageApplication onCreate called");
        super.onCreate();
        ALog.setLogger(new LoggerAosp());
        AceEnv.getInstance();
        AppModeConfig.setAppMode("stage");
        initApplication();
    }

    public void initApplication() {
        appDelegate = new StageApplicationDelegate();
        appDelegate.setPidAndUid(Process.myPid(), getUid(this));

        Context context = getApplicationContext();
        String apkPath = context.getPackageCodePath();
        appDelegate.setHapPath(apkPath);
        appDelegate.setNativeAssetManager(getAssets());

        appDelegate.setAssetsFileRelativePath(routerTraverseAssets(ASSETS_SUB_PATH));
        createStagePath();

        copyAllModuleResources();
        appDelegate.setResourcesFilePrefixPath(getExternalFilesDir((String)null).getAbsolutePath());

        appDelegate.setLocale(
            Locale.getDefault().getLanguage(), Locale.getDefault().getCountry(), Locale.getDefault().getScript());

        appDelegate.launchApplication();
    }

    /**
     * Read the relative paths of all files in the assets folder of the android platform
     * @param path the path under the assets folder
     * @return result of method return
     */
    public String routerTraverseAssets(String path) {
        List<String> assetsList = new ArrayList<String>();
        traverseAssets(assetsList, path);

        String assets = "";
        for (String sub : assetsList) {
            assets += (sub + ";");
        }
        return assets;
    }

    /**
     * Read the relative paths of all files in the assets folder of the android platform
     * @param assetsList accepts the files in the assets folder
     * @param path the path under the assets folder
     * @return result of method return
     */
    private void traverseAssets(List<String> assetsList, String path) {
        AssetManager assetManager = getAssets();
        try {
            String[] list = assetManager.list(path);
            if (list == null || list.length <= 0) {
                return;
            }
            for (int i = 0; i < list.length; i++) {
                if (list[i].contains(".")) {
                    assetsList.add(path + "/" + list[i]);
                }
                String subPath;
                if ("".equals(path)) {
                    subPath = list[i];
                } else {
                    subPath = path + "/" + list[i];
                }
                traverseAssets(assetsList, subPath);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void makeNewDir(String path) {
        File dir = new File(path);
        if (!dir.exists()) {
            dir.mkdirs();
        }
    }

    private void createStagePath() {
        String filesDir = getApplicationContext().getFilesDir().getPath();
        String[] fileDirNames = {TEMP_DIR, FILES_DIR, PREFERENCE_DIR, DATABASE_DIR};
        for (int i = 0; i < fileDirNames.length; i++) {
            makeNewDir(filesDir + fileDirNames[i]);
        }
        appDelegate.setFileDir(filesDir);
        String cacheDir = getApplicationContext().getCacheDir().getPath();
        appDelegate.setCacheDir(cacheDir);
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
            for (String name : list) {
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

    private int getUid(Context context) {
        int uid = 0;
        try {
            PackageManager pm = context.getPackageManager();
            if (pm == null) {
                Log.d(LOG_TAG, "get uid when package manager is null");
                return uid;
            }
            ApplicationInfo applicationInfo = pm.getApplicationInfo(getPackageName(), PackageManager.GET_META_DATA);
            uid = applicationInfo.uid;
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(LOG_TAG, "get uid failed, error: " + e.getMessage());
        }
        return uid;
    }
}
