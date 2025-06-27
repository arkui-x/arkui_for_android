/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
import android.app.ActivityManager;
import android.app.Application;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.ComponentName;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Build;
import android.os.LocaleList;
import android.os.Process;
import android.os.Trace;
import android.provider.Settings;
import android.util.Log;
import android.util.DisplayMetrics;
import android.view.WindowManager;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.List;
import java.util.Locale;

import ohos.ace.adapter.AcePlatformCapability;
import ohos.ace.adapter.AceEnv;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.AppModeConfig;
import ohos.ace.adapter.LoggerAosp;
import ohos.ace.adapter.ILogger;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * This class is responsible for communicating with the stage application delegate jni.
 *
 * @since 1
 */
public class StageApplicationDelegate {
    private static final String LOG_TAG = "StageApplicationDelegate";

    private static final String ASSETS_SUB_PATH = "arkui-x";

    private static final String TEMP_DIR = "/temp";

    private static final String FILES_DIR = "/files";

    private static final String PREFERENCE_DIR = "/preference";

    private static final String DATABASE_DIR = "/database";

    private static final String RESOURCES_DIR = "resources";

    private static final String SYSTEMRES_DIR = "systemres";

    private static final String SHARED_PREFERENCES_NAME = "assets_path";

    private static final String ASSETS_PATH_KEY = "assets_path_key";

    private static final String APP_VERSION_CODE = "versionCode";

    private static final String WANT_PARAMS = "params";

    private static final String ARKUIX_LIBS = "/arkui-x/libs/";

    private static final String ARKUIX_LIB_NAME = "/libarkui_android.so";

    private static final String ARCH_ARM64 = "arm64-v8a";

    private static final String ARCH_ARM = "armeabi-v7a";

    private static final String ARCH_X86 = "x86_64";

    private static final String CACERT_FILE = "/cacert.ca";

    private static final String ARKUIX_JSON = "arkui-x.json";

    private static final String LANGUAGE_SHARE_PREFERENCE = "language_prefs";

    private static final String KEY_LANGUAGE = "app_language";

    private static final int ERR_INVALID_PARAMETERS = -1;

    private static final int ERR_OK = 0;

    private static final int DEFAULT_VERSION_CODE = -1;

    private static final int LOG_MIN = 0;

    private static final int LOG_MAX = 4;

    private static final int COUNT_ONE = 1;

    private static final int COUNT_ZERO = 0;

    private static boolean isInitialized = false;

    private static boolean isCopyNativeLibs = false;

    private static boolean isCopyResources = false;

    private Application stageApplication = null;

    private volatile Activity topActivity = null;

    private AcePlatformCapability platformCapability = null;

    private AtomicInteger activityCount = new AtomicInteger(0);

    private boolean isBackground = false;

    private String assetsModulePath = "";

    /**
     * Constructor.
     */
    public StageApplicationDelegate() {
        Log.i(LOG_TAG, "Constructor called.");
    }

    /**
     * Dynamic load sandbox lib
     *
     * @return true:load libray success,else false
     */
    public boolean loadLibraryFromAppData() {
        try {
            String str = stageApplication.getApplicationContext().getApplicationInfo().nativeLibraryDir;
            String architecture = str.substring(str.lastIndexOf('/') + 1);
            if ("arm64".equals(architecture)) {
                architecture = ARCH_ARM64;
            } else if ("arm".equals(architecture)) {
                architecture = ARCH_ARM;
            } else {
                architecture = ARCH_X86;
            }
            Log.i(LOG_TAG, "Current system CPU architecture : " + architecture);
            String path = stageApplication.getApplicationContext().getFilesDir().getPath() + ARKUIX_LIBS
                            + architecture + ARKUIX_LIB_NAME;
            Log.i(LOG_TAG, "Dynamically loading path : " + path);
            System.load(path);
            return true;
        } catch (UnsatisfiedLinkError error) {
            ALog.e(LOG_TAG, "System.load failed " + error.getMessage());
            return false;
        }
    }

    /**
     * Initialize stage application.
     *
     * @param application the stage application.
     */
    public void initApplication(Application application) {
        Log.i(LOG_TAG, "init application.");
        if (isInitialized) {
            if (!isCopyResources) {
                Log.i(LOG_TAG, "The application is initialized, but copy resource failed.");
                stageApplication = application;
                copyAllModuleResources();
            }
            return;
        }
        isInitialized = true;
        stageApplication = application;

        ALog.setLogger(new LoggerAosp());
        boolean isDynamic = isDynamicUpdateLibs();
        if (isDynamic) {
            if (!loadLibraryFromAppData()) {
                AceEnv.getInstance().isLibraryLoaded();
            }
        } else {
            if (!AceEnv.getInstance().isLibraryLoaded()) {
                loadLibraryFromAppData();
            }
        }
        Trace.beginSection("initApplication");
        AppModeConfig.initAppMode();

        attachStageApplication();

        Context context = stageApplication.getApplicationContext();
        setPidAndUid(Process.myPid(), getUid(context));

        Trace.beginSection("prepareAssets");
        setIsDynamicLoadLibs(isDynamic);
        String apkPath = context.getPackageCodePath();
        setHapPath(apkPath);
        setNativeAssetManager(stageApplication.getAssets());

        nativeSetAppLibDir(context.getApplicationInfo().nativeLibraryDir);
        createStagePath();

        try {
            if (stageApplication.getAssets().list(ASSETS_SUB_PATH).length != 0) {
                copyAllModuleResources();
                setAssetsFileRelativePath(assetsModulePath);
            }
        } catch (Exception e) {
            Log.e(LOG_TAG, "get Assets path failed, error: " + e.getMessage());
        }
        setResourcesFilePrefixPath(stageApplication.getApplicationContext().getFilesDir().getPath() +
                                    "/" + ASSETS_SUB_PATH);
        setLocaleInfo();
        Trace.endSection();

        launchApplication();
        initConfiguration();
        setPackageName();
        File file = stageApplication.getExternalFilesDir((String) null);
        if (file != null) {
            createCacertFile(file.getAbsolutePath());
        }

        initActivity();
        initPlatformCapability(context);
        Trace.endSection();
    }

    private void initActivity() {
        stageApplication.registerActivityLifecycleCallbacks(new Application.ActivityLifecycleCallbacks() {
            @Override
            public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
            }

            @Override
            public void onActivityDestroyed(Activity activity) {
            }

            @Override
            public void onActivityStarted(Activity activity) {
                activityCount.getAndIncrement();
                if (activityCount.get() == COUNT_ONE && isBackground) {
                    isBackground = false;
                    nativeDispatchApplicationOnForeground();
                }
            }

            @Override
            public void onActivityResumed(Activity activity) {
                topActivity = activity;
            }

            @Override
            public void onActivityPaused(Activity activity) {
            }

            @Override
            public void onActivityStopped(Activity activity) {
                activityCount.getAndDecrement();
                if (activityCount.get() <= COUNT_ZERO && !isBackground) {
                    isBackground = true;
                    nativeDispatchApplicationOnBackground();
                }
                if (topActivity == activity) {
                    topActivity = null;
                }
            }

            @Override
            public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
            }
        });
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

    private String getAssetsPath(boolean isNewVersion) {
        if (stageApplication == null) {
            Log.e(LOG_TAG, "stageApplication is null");
            return "";
        }

        SharedPreferences sharedPreferences =
                stageApplication.getSharedPreferences(SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
        if (sharedPreferences == null) {
            Log.e(LOG_TAG, "sharedPreferences is null");
            return "";
        }

        String path = sharedPreferences.getString(ASSETS_PATH_KEY, "");
        if (!path.isEmpty() && !isNewVersion) {
            return path;
        }

        SharedPreferences.Editor edit = sharedPreferences.edit();
        if (edit == null) {
            Log.e(LOG_TAG, "edit is null");
            return "";
        }

        String assetsPath = routerTraverseAssets(ASSETS_SUB_PATH);
        edit.putString(ASSETS_PATH_KEY, assetsPath);
        edit.commit();

        return assetsPath;
    }

    /**
     * Read the relative paths of all files in the assets folder of the android platform
     *
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
     *
     * @param assetsList accepts the files in the assets folder
     * @param path       the path under the assets folder
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
                } else if (RESOURCES_DIR.equals(list[i]) || SYSTEMRES_DIR.equals(list[i])) {
                    continue;
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
        String[] fileDirNames = {TEMP_DIR, FILES_DIR, PREFERENCE_DIR, DATABASE_DIR, "/" + ASSETS_SUB_PATH};
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
        if (stageApplication == null) {
            Log.e(LOG_TAG, "stageApplication is null");
            return;
        }
        int versionCode = getAppVersionCode();
        if (versionCode == DEFAULT_VERSION_CODE) {
            Log.e(LOG_TAG, "Getting app version code failed.");
            return;
        }
        SharedPreferences sharedPreferences =
                stageApplication.getSharedPreferences(SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
        if (sharedPreferences == null) {
            Log.e(LOG_TAG, "sharedPreferences is null");
            return;
        }

        isCopyResources = true;

        int oldVersionCode = sharedPreferences.getInt(APP_VERSION_CODE, DEFAULT_VERSION_CODE);
        boolean isDebug = isApkInDebug(stageApplication);
        Log.i(LOG_TAG, "Old version code is: " + oldVersionCode +
            ", current version code is: " + versionCode +
            ", apk is debug: " + isDebug);
        if (!isDebug && oldVersionCode >= versionCode) {
            assetsModulePath = getAssetsPath(false);
            Log.i(LOG_TAG, "The resource has been copied.");
            return;
        }

        isCopyNativeLibs = true;
        assetsModulePath = getAssetsPath(true);

        Log.i(LOG_TAG, "Start copying resources.");
        AssetManager assets = stageApplication.getAssets();
        String moduleResourcesDirectory = "";
        String moduleResourcesIndex = "";
        List<String> moduleResources = new ArrayList<>();
        try {
            String[] list = assets.list(ASSETS_SUB_PATH);
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
            isCopyResources = false;
        }

        if (moduleResources.isEmpty()) {
            Log.e(LOG_TAG, "The moduleResources is empty.");
        }

        for (String resourcesName : moduleResources) {
            copyFilesFromAssets(ASSETS_SUB_PATH + "/" + resourcesName,
                    stageApplication.getApplicationContext().getFilesDir().getPath() +
                    "/" + ASSETS_SUB_PATH + "/" + resourcesName);
        }

        SharedPreferences.Editor edit = sharedPreferences.edit();
        if (edit == null) {
            Log.e(LOG_TAG, "edit is null");
            return;
        }
        if (isCopyResources) {
            edit.putInt(APP_VERSION_CODE, versionCode);
            edit.commit();
        }
    }

    /**
     * copy the file to the destination path
     *
     * @param assetsPath the path in the assets directory
     * @param savePath   the destination path for the copy
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
            isCopyResources = false;
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
        float fontScale = Settings.System.getFloat(stageApplication.getContentResolver(), Settings.System.FONT_SCALE,
            1.0f);
        Configuration cfg = stageApplication.getResources().getConfiguration();
        cfg.fontScale = fontScale;
        double diagonalSize = getDeviceTypeByPhysicalSize();
        JSONObject json = StageConfiguration.convertConfiguration(cfg, diagonalSize);
        nativeInitConfiguration(json.toString());
    }

    private void initPlatformCapability(Context context) {
        platformCapability = new AcePlatformCapability(context);
    }

    private double getDeviceTypeByPhysicalSize() {
        // Find the current window manager, if none is found we default to the current device as a mobile phone.
        WindowManager windowManager = (WindowManager) stageApplication
                .getSystemService(stageApplication.WINDOW_SERVICE);
        if (windowManager == null) {
            return 0;
        }

        DisplayMetrics metrics = new DisplayMetrics();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            windowManager.getDefaultDisplay().getRealMetrics(metrics);
        } else {
            windowManager.getDefaultDisplay().getMetrics(metrics);
        }

        double width = metrics.widthPixels / (double) metrics.xdpi;
        double height = metrics.heightPixels / (double) metrics.ydpi;
        double diagonalSize = Math.sqrt(Math.pow(width, 2) + Math.pow(height, 2));
        return diagonalSize;
    }

    /**
     * Get running process info.
     *
     * @return class RunningProcessInfo in List
     */
    public Object getRunningProcessInfo() {
        List<RunningProcessInfo> processInfos = new ArrayList<RunningProcessInfo>();

        ActivityManager activityMgr =
                (ActivityManager) stageApplication.getSystemService(stageApplication.ACTIVITY_SERVICE);
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
     * Finish user test.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    public int finishUserTest() {
        Log.i(LOG_TAG, "Finish user test called");
        int error = ERR_OK;
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        stageApplication.getApplicationContext().startActivity(intent);
        android.os.Process.killProcess(android.os.Process.myPid());
        System.exit(0);
        return error;
    }

    /**
     * Get the package name and activity name at the top of the stack.
     *
     * @return Return the ability name.
     */
    public String getTopActivity() {
        String topAbility = null;
        if (topActivity != null) {
            if (topActivity instanceof StageActivity) {
                topAbility = ((StageActivity) topActivity).getInstanceName();
            }
        }
        return topAbility;
    }

    /**
     * Print message.
     *
     * @param msg the log msg.
     */
    public void print(String msg) {
        if (msg.length() <= 1000) {
            Log.i(LOG_TAG, "print message: " + msg);
        } else {
            Log.w(LOG_TAG, "print: The total length of the message exceed 1000 characters.");
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
            String packageName = stageApplication.getApplicationContext().getPackageName();
            ComponentName componentName = null;
            if (packageName.equals(bundleName)) {
                componentName = new ComponentName(stageApplication.getApplicationContext(), activityName);
            } else {
                componentName = new ComponentName(bundleName, activityName);
            }
            intent.setComponent(componentName);
            intent.putExtra(WANT_PARAMS, params);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            stageApplication.getApplicationContext().startActivity(intent);
        } catch (ActivityNotFoundException exception) {
            Log.e("StageApplication", "start activity err.");
            error = ERR_INVALID_PARAMETERS;
        }
        return error;
    }

    /**
     * Set whether to dynamic load libraries.
     *
     * @param isDynamic true:dynamic load libraries,else false.
     */
    public void setIsDynamicLoadLibs(boolean isDynamic) {
        nativeSetIsDynamicLoadLibs(isDynamic);
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
        Log.i(LOG_TAG, "isCopyNativeLibs is " + isCopyNativeLibs);
        nativeLaunchApplication(isCopyNativeLibs);
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
        setLocaleInfo();
        JSONObject json = StageConfiguration.convertConfiguration(newConfig, -1.0);
        nativeOnConfigurationChanged(json.toString());
    }

    /**
     * Set locale to native.
     *
     * @param language the language.
     * @param country  the country.
     * @param script   the script.
     */
    public void setLocale(String language, String country, String script) {
        nativeSetLocale(language, country, script);
    }

    /**
     * Attach stage application to native.
     */
    public void attachStageApplication() {
        nativeAttachStageApplicationDelegate(this);
    }

    private void setPackageName() {
        String packageName = stageApplication.getApplicationContext().getPackageName();
        nativeSetPackageName((packageName != null) ? packageName : "");
    }

    private void setLocaleInfo() {
        Locale locale;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            locale = Resources.getSystem().getConfiguration().getLocales().get(0);
        } else {
            locale = Locale.getDefault();
        }
        Context context = stageApplication.getApplicationContext();
        String preferredLanguage = getAppPreferredLanguage(context);
        if (!preferredLanguage.isEmpty()) {
            locale = Locale.forLanguageTag(preferredLanguage);
        }
        String language = locale.getLanguage();
        Log.i(LOG_TAG, "language: " + language);
        String script;
        switch (language) {
            case "ug": {
                script = "Arab";
                break;
            }
            case "bo": {
                script = "Tibt";
                break;
            }
            default: {
                script = locale.getScript();
                break;
            }
        }
        setLocale(language, locale.getCountry(), script);
    }

    private void readFile(BufferedWriter writer, File file) {
        if (!file.isFile() || !file.canRead()) {
            return;
        }
        try {
            BufferedReader reader = new BufferedReader(new FileReader(file.getPath()));
            try {
                String line;
                while ((line = reader.readLine()) != null) {
                    writer.write(line);
                    writer.newLine();
                }
            } finally {
                reader.close();
            }
        } catch (FileNotFoundException e) {
            Log.e(LOG_TAG, "read cacert err: " + e.getMessage());
        } catch (IOException e) {
            Log.e(LOG_TAG, "read cacert err: " + e.getMessage());
        }
    }

    private void createCacertFile(String path) {
        try {
            File file = new File(path + CACERT_FILE);
            if (file.exists()) {
                file.deleteOnExit();
            }
            if (!file.createNewFile()) {
                return;
            }
        } catch (IOException e) {
            Log.e(LOG_TAG, "write cacert err: " + e.getMessage());
        }

        try {
            BufferedWriter writer = new BufferedWriter(new FileWriter(path + CACERT_FILE));
            try {
                File dir = new File("/system/etc/security/cacerts/");
                File[] files = dir.listFiles();
                if (files == null) {
                    return;
                }
                for (File file : files) {
                    readFile(writer, file);
                }
            } finally {
                writer.close();
            }
        } catch (FileNotFoundException e) {
            Log.e(LOG_TAG, "read cacert err: " + e.getMessage());
        } catch (IOException e) {
            Log.e(LOG_TAG, "read cacert err: " + e.getMessage());
        }
    }

    private int getAppVersionCode() {
        int appVersionCode = DEFAULT_VERSION_CODE;
        try {
            PackageInfo packageInfo = stageApplication.getApplicationContext()
                    .getPackageManager()
                    .getPackageInfo(stageApplication.getPackageName(), 0);
            appVersionCode = packageInfo.versionCode;
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(LOG_TAG, "Getting package info err: " + e.getMessage());
        }
        return appVersionCode;
    }

    private boolean isApkInDebug(Context context) {
        try {
            ApplicationInfo info = context.getApplicationInfo();
            return (info.flags & ApplicationInfo.FLAG_DEBUGGABLE) != 0;
        } catch (Exception e) {
            Log.e(LOG_TAG, "Getting is apk in debug err: " + e.getMessage());
            return false;
        }
    }

    /**
     * Set log interface.
     *
     * @param logger the log interface.
     */
    public void setLogInterface(ILogger logger) {
        try {
            ALog.setLogger(logger);
            nativeSetLogger(logger);
        } catch (UnsatisfiedLinkError e) {
            Log.e(LOG_TAG, "logInterface: JNI is not registered.");
        }
    }

    /**
     * Set log level.
     *
     * @param logLevel the log level.
     */
    public void setLogLevel(int logLevel) {
        try {
            if (logLevel < LOG_MIN || logLevel > LOG_MAX) {
                Log.e(LOG_TAG, "logLevel is invalid.");
                return;
            }
            ALog.setLoggerLevel(logLevel);
            nativeSetLogLevel(logLevel);
        } catch (UnsatisfiedLinkError e) {
            Log.e(LOG_TAG, "logInterface: JNI is not registered.");
        }
    }

    private boolean isDynamicUpdateLibs() {
        String jsonPath = extractJsonPathsFromAssets();
        if (!jsonPath.isEmpty()) {
            String assetsJsonContent = readAssetsFile(jsonPath);
            String sandBoxJsonContent = readJsonFile(
                    stageApplication.getApplicationContext().getFilesDir().getPath() + "/" + jsonPath);
            if (assetsJsonContent == null || sandBoxJsonContent == null) {
                Log.w(LOG_TAG, "File JsonContent is null");
                return false;
            }
            String assetsVersion = extractVersionFromJson(assetsJsonContent);
            String sandBoxVersion = extractVersionFromJson(sandBoxJsonContent);
            if (!assetsVersion.isEmpty() && !sandBoxVersion.isEmpty()) {
                return compareVersion(assetsVersion, sandBoxVersion);
            }
        }
        return false;
    }

    private boolean compareVersion(String assetsVersion, String sandBoxVersion) {
        String[] assetsCode = assetsVersion.split("\\.");
        String[] sandBoxCode = sandBoxVersion.split("\\.");
        int minLength = Math.min(assetsCode.length, sandBoxCode.length);
        for (int i = 0; i < minLength; i++) {
            try {
                int assetsCodeValue = Integer.parseInt(assetsCode[i]);
                int sandBoxCodeValue = Integer.parseInt(sandBoxCode[i]);
                if (assetsCodeValue < sandBoxCodeValue) {
                    return true;
                } else if (assetsCodeValue > sandBoxCodeValue) {
                    return false;
                }
            } catch (NumberFormatException e) {
                Log.e(LOG_TAG, "Error NumberFormat : " + e.getMessage());
                return false;
            }
        }
        return assetsCode.length < sandBoxCode.length;
    }

    private String extractVersionFromJson(String jsonContent) {
        if (jsonContent == null || jsonContent.isEmpty()) {
            return "";
        }
        try {
            JSONObject jsonObject = new JSONObject(jsonContent);
            String version = jsonObject.optString("version", "");
            if (!version.isEmpty()) {
                return version;
            }
        } catch (JSONException e) {
            Log.e(LOG_TAG, "Error JSON : " + e.getMessage());
        }
        return "";
    }

    private String extractJsonPathsFromAssets() {
        String allPath = getAssetsPath(true);
        if (allPath != null && !allPath.isEmpty()) {
            String[] pathArray = allPath.split(";");
            for (String path : pathArray) {
                if (path.contains(ARKUIX_JSON)) {
                    return path;
                }
            }
        }
        return "";
    }

    private String readAssetsFile(String path) {
        try (InputStream inputStream = stageApplication.getAssets().open(path);
             BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8))) {
            StringBuilder content = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line);
            }
            return content.toString();
        } catch (IOException e) {
            Log.e(LOG_TAG, "assets reading err: " + e.getMessage());
        }
        return null;
    }

    private String readJsonFile(String path) {
        File file = new File(path);
        if (!file.exists()) {
            Log.e(LOG_TAG, "File does not exist: " + path);
            return null;
        }
        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
            StringBuilder content = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line);
            }
            return content.toString();
        } catch (IOException e) {
            Log.e(LOG_TAG, "reading err: " + e.getMessage());
        }
        return null;
    }

    /**
     * Attach language to context.
     *
     * @param context the application context.
     * @return Context after attach the language
     */
    public static Context attachLanguageContext(Context context) {
        String preferredLanguage = getAppPreferredLanguage(context);
        Resources resources = context.getResources();
        if (resources == null || preferredLanguage.isEmpty()) {
            return context;
        }
        Locale locale = Locale.forLanguageTag(preferredLanguage);
        Configuration configuration;
        int buildVersion = Build.VERSION.SDK_INT;
        if (buildVersion <= Build.VERSION_CODES.N_MR1) {
            configuration = resources.getConfiguration();
            configuration.setLocale(locale);
            configuration.setLocales(new LocaleList(locale));
            resources.updateConfiguration(configuration, resources.getDisplayMetrics());
            return context;
        } else {
            configuration = new Configuration();
            configuration.setLocale(locale);
            configuration.setLocales(new LocaleList(locale));
            return context.createConfigurationContext(configuration);
        }
    }

    private static String getAppPreferredLanguage(Context context) {
        SharedPreferences prefs = context.getSharedPreferences(LANGUAGE_SHARE_PREFERENCE, Context.MODE_PRIVATE);
        String savedLang = prefs.getString(KEY_LANGUAGE, null);
        if (savedLang == null || savedLang.isEmpty()) {
            return "";
        }
        return savedLang;
    }

    private native void nativeSetAssetManager(Object assetManager);

    private native void nativeSetIsDynamicLoadLibs(boolean isDynamic);

    private native void nativeSetHapPath(String hapPath);

    private native void nativeSetPackageName(String hapPath);

    private native void nativeLaunchApplication(boolean isCopyNativeLibs);

    private native void nativeSetAssetsFileRelativePath(String path);

    private native void nativeSetCacheDir(String cacheDir);

    private native void nativeSetFileDir(String filesDir);

    private native void nativeSetAppLibDir(String libDir);

    private native void nativeSetResourcesFilePrefixPath(String path);

    private native void nativeSetPidAndUid(int pid, int uid);

    private native void nativeInitConfiguration(String data);

    private native void nativeOnConfigurationChanged(String data);

    private native void nativeSetLocale(String language, String country, String script);

    private native void nativeAttachStageApplicationDelegate(StageApplicationDelegate object);

    private native void nativeSetLogLevel(int level);

    private native void nativeSetLogger(Object logger);

    private native void nativeDispatchApplicationOnForeground();

    private native void nativeDispatchApplicationOnBackground();

    /**
     * Native calls the Preload the abc file interface.
     *
     * @param moduleName Preload the name of module.
     * @param abilityName Preload the name of ability.
     */
    protected static native void nativePreloadModule(String moduleName, String abilityName);
}
