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

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.os.Process;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

import ohos.ace.adapter.AceEnv;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.AppModeConfig;
import ohos.ace.adapter.LoggerAosp;

import org.json.JSONObject;

/**
 * This class is responsible for communicating with the stage application delegate jni.
 *
 * @since 1
 */
public class StageApplicationDelegate {
    private static final String LOG_TAG = "StageApplicationDelegate";

    private StageApplication stageApplication = null;

    private static final String ASSETS_SUB_PATH = "arkui-x";

    private static final String TEMP_DIR = "/temp";

    private static final String FILES_DIR = "/files";

    private static final String PREFERENCE_DIR = "/preference";

    private static final String DATABASE_DIR = "/database";

    /**
     * Constructor.
     */
    public StageApplicationDelegate() {
        Log.i(LOG_TAG, "Constructor called.");
    }

    /**
     * Initialize stage application.
     * 
     * @param object the stage application.
     */
    public void initApplication(StageApplication object) {
        Log.i(LOG_TAG, "init application.");
        stageApplication = object;

        ALog.setLogger(new LoggerAosp());
        AceEnv.getInstance();
        AppModeConfig.setAppMode("stage");

        attachStageApplication(stageApplication);

        Context context = stageApplication.getApplicationContext();
        setPidAndUid(Process.myPid(), getUid(context));

        String apkPath = context.getPackageCodePath();
        setHapPath(apkPath);
        setNativeAssetManager(stageApplication.getAssets());

        setAssetsFileRelativePath(routerTraverseAssets(ASSETS_SUB_PATH));
        createStagePath();

        copyAllModuleResources();
        setResourcesFilePrefixPath(stageApplication.getExternalFilesDir((String)null).getAbsolutePath());

        setLocale(
            Locale.getDefault().getLanguage(), Locale.getDefault().getCountry(), Locale.getDefault().getScript());

        launchApplication();
        initConfiguration();
    }

    private int getUid(Context context) {
        int uid = 0;
        try {
            PackageManager pm = context.getPackageManager();
            if (pm == null) {
                Log.d(LOG_TAG, "get uid when package manager is null");
                return uid;
            }
            ApplicationInfo applicationInfo = pm.getApplicationInfo(
                stageApplication.getPackageName(), PackageManager.GET_META_DATA);
            uid = applicationInfo.uid;
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(LOG_TAG, "get uid failed, error: " + e.getMessage());
        }
        return uid;
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
        AssetManager assetManager = stageApplication.getAssets();
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
        String filesDir = stageApplication.getApplicationContext().getFilesDir().getPath();
        String[] fileDirNames = {TEMP_DIR, FILES_DIR, PREFERENCE_DIR, DATABASE_DIR};
        for (int i = 0; i < fileDirNames.length; i++) {
            makeNewDir(filesDir + fileDirNames[i]);
        }
        setFileDir(filesDir);
        String cacheDir = stageApplication.getApplicationContext().getCacheDir().getPath();
        setCacheDir(cacheDir);
    }

    /**
     * copy the resources from all modules
     */
    private void copyAllModuleResources() {
        String rootDirectory = "arkui-x";
        AssetManager assets = stageApplication.getAssets();
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
                stageApplication.getExternalFilesDir(null).getAbsolutePath() + "/" + resourcesName);
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
            String[] fileNames = stageApplication.getAssets().list(assetsPath);
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
                is = stageApplication.getAssets().open(assetsPath);
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

    private void initConfiguration() {
        Log.i(LOG_TAG, "StageApplication initConfiguration called");
        Configuration cfg = stageApplication.getResources().getConfiguration();
        JSONObject json = StageConfiguration.convertConfiguration(cfg);
        nativeInitConfiguration(json.toString());
    }

    /**
     * Get running process info.
     *
     * @return class RunningProcessInfo in List
     */
    public Object getRunningProcessInfo() {
        Log.i(LOG_TAG, "Get running process info called");
        List<RunningProcessInfo> processInfos = new ArrayList<RunningProcessInfo>();

        ActivityManager activityMgr =
            (ActivityManager)stageApplication.getSystemService(stageApplication.ACTIVITY_SERVICE);
        if (activityMgr == null) {
            Log.e(LOG_TAG, "activityMgr is null");
            return processInfos;
        }
        List<ActivityManager.RunningAppProcessInfo> processList = activityMgr.getRunningAppProcesses();
        if (processList == null) {
            Log.e(LOG_TAG, "processList is null");
            return processInfos;
        }
        for (ActivityManager.RunningAppProcessInfo info : processList) {
            RunningProcessInfo processInfo = new RunningProcessInfo();
            processInfo.pid = info.pid;
            processInfo.processName = info.processName;
            processInfo.pkgList = Arrays.asList(info.pkgList);
            processInfos.add(processInfo);
        }
        return processInfos;
    }

    /**
     * Set asset manager object to native.
     *
     * @param assetManager the asset manager.
     */
    public void setNativeAssetManager(AssetManager assetManager) {
        nativeSetAssetManager(assetManager);
    }

    /**
     * Set hap path to native.
     *
     * @param hapPath the hap path.
     */
    public void setHapPath(String hapPath) {
        nativeSetHapPath(hapPath);
    }

    /**
     * Set assets file relative path to native.
     *
     * @param path the assets file relative path.
     */
    public void setAssetsFileRelativePath(String path) {
        nativeSetAssetsFileRelativePath(path);
    }

    /**
     * Launch application.
     */
    public void launchApplication() {
        nativeLaunchApplication();
    }

    /**
     * Set cache dir to native.
     *
     * @param cacheDir the cache dir.
     */
    public void setCacheDir(String cacheDir) {
        nativeSetCacheDir(cacheDir);
    }

    /**
     * Set file dir to native.
     *
     * @param filesDir the file dir.
     */
    public void setFileDir(String filesDir) {
        nativeSetFileDir(filesDir);
    }

    /**
     * Set resources file prefix path to native.
     *
     * @param path the resources file prefix path.
     */
    public void setResourcesFilePrefixPath(String path) {
        nativeSetResourcesFilePrefixPath(path);
    }

    /**
     * Set pid and uid to native.
     *
     * @param pid the process id.
     * @param uid the uid.
     */
    public void setPidAndUid(int pid, int uid) {
        nativeSetPidAndUid(pid, uid);
    }

    /**
     * Called by the system when the device configuration changes while your component is running.
     *
     * @param newConfig the configuration.
     */
    public void onConfigurationChanged(Configuration newConfig) {
        JSONObject json = StageConfiguration.convertConfiguration(newConfig);
        nativeOnConfigurationChanged(json.toString());
    }

    /**
     * Set locale to native.
     *
     * @param language the language.
     * @param country the country.
     * @param script the script.
     */
    public void setLocale(String language, String country, String script) {
        nativeSetLocale(language, country, script);
    }

    /**
     * Attach stage application to native.
     *
     * @param object the stage application.
     */
    public void attachStageApplication(StageApplication object) {
        nativeAttachStageApplication(object);
    }

    private native void nativeSetAssetManager(Object assetManager);
    private native void nativeSetHapPath(String hapPath);
    private native void nativeLaunchApplication();
    private native void nativeSetAssetsFileRelativePath(String path);
    private native void nativeSetCacheDir(String cacheDir);
    private native void nativeSetFileDir(String filesDir);
    private native void nativeSetResourcesFilePrefixPath(String path);
    private native void nativeSetPidAndUid(int pid, int uid);
    private native void nativeInitConfiguration(String data);
    private native void nativeOnConfigurationChanged(String data);
    private native void nativeSetLocale(String language, String country, String script);
    private native void nativeAttachStageApplication(StageApplication object);
}
