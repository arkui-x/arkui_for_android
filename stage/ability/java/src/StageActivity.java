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

import java.io.File;
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
import android.os.Trace;
import android.util.Log;
import android.view.View;
import android.os.Build;

import java.util.List;
import java.util.Set;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicReference;

import android.content.ClipData;
import android.net.Uri;

import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONException;

import ohos.ace.adapter.AceEnv;
import ohos.ace.adapter.AcePlatformPlugin;
import ohos.ace.adapter.ArkUIXPluginRegistry;
import ohos.ace.adapter.IArkUIXPlugin;
import ohos.ace.adapter.PluginContext;
import ohos.ace.adapter.WindowViewInterface;
import ohos.ace.adapter.WindowViewAospInterface;
import ohos.ace.adapter.WindowViewBuilder;
import ohos.ace.adapter.capability.video.AceVideoPluginAosp;
import ohos.ace.adapter.capability.web.AceWebPluginAosp;
import ohos.ace.adapter.capability.platformview.IPlatformView;
import ohos.ace.adapter.capability.platformview.PlatformViewFactory;
import ohos.ace.adapter.capability.platformview.AcePlatformViewPluginAosp;
import ohos.ace.adapter.capability.web.AceWebPluginBase;
import ohos.ace.adapter.capability.bridge.BridgeManager;
import ohos.ace.adapter.capability.grantresult.GrantResult;
import ohos.ace.adapter.capability.keyboard.KeyboardHeightObserver;
import ohos.ace.adapter.capability.keyboard.KeyboardHeightProvider;
import android.view.View;
import android.content.res.Configuration;

import ohos.ace.adapter.FoldManager;
import androidx.core.util.Consumer;
import androidx.window.java.layout.WindowInfoTrackerCallbackAdapter;
import androidx.window.layout.DisplayFeature;
import androidx.window.layout.FoldingFeature;
import androidx.window.layout.WindowInfoTracker;
import androidx.window.layout.WindowLayoutInfo;
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

    private static final int WANT_PARAMS_TYPE = 10;

    private static final String PHOTO_VIDEO_TYPE = "image/*;video/*";

    private static final String FILE_ALL_TYPE = "*/*";

    private static final String PHOTO_PICKER = "com.ohos.photos";

    private static final String FILE_PICKER = "com.ohos.filepicker";

    private static final String MULTIPLE = "multipleselect";

    private static final String PHOTO_URI_KEY = "select-item-list";

    private static final String FILE_URI_KEY = "ability.params.stream";

    private static final String ARKUI_X_DIR = "arkui-x";

    private static boolean isForResult = true;

    private static final int RESULTCODE_OK = 0;

    private static final int RESULTCODE_ERROR = 1;

    private static final int RESULT_OK = -1;

    private static final int ERR_INVALID_PARAMETERS = -1;

    private static final int ERR_OK = 0;

    private static boolean isFrist = false;

    private static final int ANDROID_API_31 = 31;

    private int requestCode = 0;

    private int instanceId = InstanceIdGenerator.getAndIncrement();

    private String instanceName;

    private String bundleName;

    private String moduleName;

    private String abilityName;

    private StageActivityDelegate activityDelegate = null;

    private WindowViewAospInterface windowView = null;

    private AcePlatformPlugin platformPlugin = null;

    private BridgeManager bridgeManager = null;

    private KeyboardHeightProvider keyboardHeightProvider;

    private Set<String> pluginList = new HashSet<>();

    private ArkUIXPluginRegistry arkUIXPluginRegistry = null;

    private PluginContext pluginContext = null;

    private AcePlatformViewPluginAosp platformViewPluginAosp = null;

    private WindowInfoTrackerCallbackAdapter foldWindowInfoCallback = null;

    private Consumer<WindowLayoutInfo> foldConsumer = null;

    @Override
    public void onKeyboardHeightChanged(int height) {
        if (windowView != null) {
            windowView.keyboardHeightChanged(height);
        }
        if (platformPlugin != null) {
            platformPlugin.keyboardHeightChanged(height);
        }
    }

    public void onAvoidAreaChanged() {
        if (windowView != null) {
            windowView.avoidAreaChanged();
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
        windowView = WindowViewBuilder.makeWindowViewAosp(this, instanceId, isUseSurfaceView());
        Trace.endSection();
        windowView.setInstanceId(instanceId);
        initPlatformPlugin(this, instanceId, windowView);
        initArkUIXPluginRegistry();
        Trace.beginSection("setContentView");
        setContentView(windowView.getView());
        Trace.endSection();
        activityDelegate.setWindowView(getInstanceName(), windowView);
        activityDelegate.dispatchOnCreate(getInstanceName(), params);
        Trace.endSection();

        keyboardHeightProvider = new KeyboardHeightProvider(this);

        windowView.getView().post(new Runnable() {
            /**
             * Called from the thread that created the view hierarchy
             */
            public void run() {
                keyboardHeightProvider.start();
            }
        });
    }

    @Override
    protected void onStart() {
        Log.i(LOG_TAG, "StageActivity onStart called");
        super.onStart();
        Trace.beginSection("StageActivity::onStart");
        activityDelegate.dispatchOnForeground(getInstanceName(), this);
        windowView.foreground();
        if (platformPlugin != null) {
            platformPlugin.notifyLifecycleChanged(false);
        }
        Trace.endSection();

        keyboardHeightProvider.setKeyboardHeightObserver(this);
        SubWindowManager.keepSystemUiVisibility(this);
    }

    @Override
    protected void onResume() {
        Log.i(LOG_TAG, "StageActivity onResume called");
        super.onResume();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        Log.i(LOG_TAG, "StageActivity onNewIntent called");
        super.onNewIntent(intent);

        String params = "";
        if (intent != null) {
            params = intent.getStringExtra(WANT_PARAMS);
            if (params == null) {
                params = "";
            }
        }
        activityDelegate.dispatchOnNewWant(getInstanceName(), params);
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
        activityDelegate.dispatchOnDestroy(getInstanceName());
        windowView.destroy();
        arkUIXPluginRegistry.unRegistryAllPlugins();
        keyboardHeightProvider.close();
        this.bridgeManager = null;
        if (platformPlugin != null) {
            platformPlugin.release();
            Log.i(LOG_TAG, "StageActivity onDestroy platformPlugin release called");
        }
        super.onDestroy();
        Log.i(LOG_TAG, "StageActivity onDestroy end");
        offFoldStatusChange();
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
        if (!FILE_PICKER.equals(bundleName) && !PHOTO_PICKER.equals(bundleName)) {
            handleNonPickerActivityResult(requestCode, resultCode, intent);
            return;
        }
        super.onActivityResult(requestCode, resultCode, intent);
        if (intent == null) {
            return;
        }
        List<String> uriList = extractUriList(intent);
        if (uriList.isEmpty()) {
            Log.w(LOG_TAG, "No data or clip data found in the intent");
            return;
        }
        String resultWantParams = buildResultJson(uriList);
        activityDelegate.dispatchOnActivityResult(getInstanceName(), requestCode,
            resultCode == RESULT_OK ? RESULTCODE_OK : RESULTCODE_ERROR, resultWantParams, intent);
    }

    private void handleNonPickerActivityResult(int requestCode, int resultCode, Intent intent) {
        Log.i(LOG_TAG, "onActivityResult called");
        super.onActivityResult(requestCode, resultCode, intent);
        String resultWantParams = "";
        if (intent != null) {
            resultWantParams = intent.getStringExtra(WANT_PARAMS);
            if (resultWantParams == null) {
                resultWantParams = "";
            }
        }
        activityDelegate.dispatchOnActivityResult(getInstanceName(), requestCode, resultCode, resultWantParams, intent);
    }

    private List<String> extractUriList(Intent intent) {
        List<String> uriList = new ArrayList<>();
        Uri data = intent.getData();
        if (data != null) {
            uriList.add(data.toString());
        }
        ClipData clipData = intent.getClipData();
        if (clipData != null) {
            for (int i = 0; i < clipData.getItemCount(); i++) {
                Uri uri = clipData.getItemAt(i).getUri();
                if (uri != null) {
                    uriList.add(uri.toString());
                }
            }
        }
        return uriList;
    }

    private String buildResultJson(List<String> uriList) {
        JSONObject resultJson = new JSONObject();
        try {
            JSONArray valueArray = new JSONArray();
            for (String uriStr : uriList) {
                valueArray.put(uriStr);
            }
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("key", PHOTO_PICKER.equals(bundleName) ? PHOTO_URI_KEY : FILE_URI_KEY);
            jsonObject.put("type", WANT_PARAMS_TYPE);
            jsonObject.put("value", valueArray.toString());
            resultJson.put(WANT_PARAMS, new JSONArray(Arrays.asList(jsonObject)));
        } catch (JSONException e) {
            Log.e(LOG_TAG, "Error creating JSON object: " + e.getMessage());
        }
        return resultJson.toString();
    }

    /**
     * Set use surface view or texture view.
     *
     * @return If true create surface view else create texture view.
     */
    public boolean isUseSurfaceView() {
        return true;
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
            String fullModuleName = bundleName + "." + moduleName;
            String modulePath = ARKUI_X_DIR + "/" + fullModuleName;
            if (isExistDir(modulePath) && nameArray.length >= 4) {
                moduleName = fullModuleName;
                instanceName = nameArray[0] + ":" + fullModuleName + ":" + nameArray[2] + ":" + nameArray[3];
            }
        }
    }

    private boolean isExistDir(String path) {
        if (path == null || path.isEmpty()) {
            return false;
        }
        File file = new File(getFilesDir(), path);
        return file.exists() && file.isDirectory();
    }

    /**
     * Get the BridgeManager of StageActivity.
     *
     * @return The BridgeManager.
     */
    public BridgeManager getBridgeManager() {
        if (this.bridgeManager == null) {
            this.bridgeManager = BridgeManager.getInstance();
        }
        return this.bridgeManager;
    }

    /**
     * Get the Id of StageActivity.
     *
     * @return The InstanceId.
     */
    public int getInstanceId() {
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
        String testBundleName = intent.getStringExtra("bundleName");
        String testModuleName = intent.getStringExtra("moduleName");
        String testRunerName = intent.getStringExtra("unittest");
        String timeout = intent.getStringExtra("timeout");
        boolean isExist = !(testBundleName == null || testModuleName == null ||
                testRunerName == null || timeout == null);
        if (hasTestValue && !isFrist && isExist) {
            Log.i(LOG_TAG, "Start creating abilityDelegate");
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
            if (packageName.equals(bundleName)) {
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
     * Start a new activity.
     *
     * @param bundleName   the package name.
     * @param activityName the activity name.
     * @param type         the file or photo type.
     * @param params       the want params.
     * @return Returns ERR_OK on success, others on failure.
     */
    public int startActivity(String bundleName, String activityName, String type, String params) {
        isForResult = false;
        String value = "";
        try {
            JSONArray paramsArray = new JSONObject(params).getJSONArray("params");
            for (int i = 0; i < paramsArray.length(); i++) {
                JSONObject param = paramsArray.getJSONObject(i);
                if ("uri".equals(param.getString("key"))) {
                    value = param.getString("value");
                    break;
                }
            }
            return createPicker(bundleName, type, value);
        } catch (JSONException e) {
            Log.e(LOG_TAG, "json parse error: " + e.getMessage());
            return ERR_INVALID_PARAMETERS;
        }
    }

    private int createPicker(String bundleName, String type, String value) {
        Intent intent = null;
        try {
            if (PHOTO_PICKER.equals(bundleName)) {
                intent = new Intent(Intent.ACTION_PICK);
                intent.setType(!type.isEmpty() ? type : PHOTO_VIDEO_TYPE);
            } else if (FILE_PICKER.equals(bundleName)) {
                intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
                intent.addCategory(Intent.CATEGORY_OPENABLE);
                intent.setType(!type.isEmpty() ? type : FILE_ALL_TYPE);
            } else {
                return ERR_INVALID_PARAMETERS;
            }
            if (MULTIPLE.equals(value)) {
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
            }
            if (isForResult) {
                this.startActivityForResult(intent, requestCode);
            } else {
                this.startActivity(intent);
            }
            return ERR_OK;
        } catch (ActivityNotFoundException e) {
            Log.e(LOG_TAG, "Activity not found: " + e.getMessage());
            return ERR_INVALID_PARAMETERS;
        }
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
            if (packageName.equals(bundleName)) {
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
     * Start a new activity for which you would like a result when it finished.
     *
     * @param bundleName   the package name.
     * @param activityName the activity name.
     * @param type         the file or photo type.
     * @param params       the want params.
     * @param requestCode  If >= 0, this code will be returned in onActivityResult() when the activity exits.
     * @return Returns ERR_OK on success, others on failure.
     */
    public int startActivityForResult(String bundleName, String activityName, String type,
                                      String params, int requestCode) {
        this.requestCode = requestCode;
        this.bundleName = bundleName;
        this.isForResult = true;
        String value = "";
        try {
            JSONArray paramsArray = new JSONObject(params).getJSONArray("params");
            for (int i = 0; i < paramsArray.length(); i++) {
                JSONObject param = paramsArray.getJSONObject(i);
                if ("uri".equals(param.getString("key"))) {
                    value = param.getString("value");
                    break;
                }
            }
            return createPicker(bundleName, type, value);
        } catch (JSONException e) {
            Log.e(LOG_TAG, "json parse error: " + e.getMessage());
            return ERR_INVALID_PARAMETERS;
        }
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
    private void initPlatformPlugin(Context context, int instanceId, WindowViewInterface windowView) {
        Trace.beginSection("StageActivity::initPlatformPlugin");
        platformPlugin = new AcePlatformPlugin(context, instanceId, windowView.getView());
        if (platformPlugin != null) {
            windowView.setInputConnectionClient(platformPlugin);
            platformPlugin.initTexturePlugin(instanceId);
            platformPlugin.addResourcePlugin(AceVideoPluginAosp.createRegister(context, instanceName));
            AceWebPluginBase web = AceWebPluginAosp.createRegister(context, windowView.getView());
            windowView.setWebPlugin(web);
            platformPlugin.addResourcePlugin(web);
            platformViewPluginAosp = AcePlatformViewPluginAosp.createRegister(context);
            platformPlugin.addResourcePlugin(platformViewPluginAosp);
            windowView.setPlatformViewPlugin(platformViewPluginAosp);
            platformPlugin.initSurfacePlugin(context, instanceId);
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
        } else {
            Log.d(LOG_TAG, "add plugin: " + pluginName);
            pluginList.add(pluginName);
        }
    }

    /**
     * Initialize arkui-x plugins and arkui-x plugins registry.
     */
    private void initArkUIXPluginRegistry() {
        Trace.beginSection("StageActivity::intitArkUIXPlugins");
        arkUIXPluginRegistry = new ArkUIXPluginRegistry(getPluginContext());
        arkUIXPluginRegistry.registryPlugins(pluginList);
        Trace.endSection();
    }

    /**
     * Get the PluginContext of StageActivity.
     *
     * @return The PluginContext.
     */
    public PluginContext getPluginContext() {
        if (this.pluginContext == null) {
            this.pluginContext = new PluginContext(this, getBridgeManager(), this.moduleName);
        }
        return this.pluginContext;
    }

    /**
     * Report to the system that your app is now fully drawn.
     */
    public void reportDrawnCompleted() {
        Log.i(LOG_TAG, "Report fully drawn start." + System.nanoTime());
        reportFullyDrawn();
        Log.i(LOG_TAG, "Report fully drawn end." + System.nanoTime());
    }

    /**
     * Register the platformView to activity. before super.onCreate.
     *
     * @param platformViewFactory The platformViewFactory.
     */
    public void registerPlatformViewFactory(PlatformViewFactory platformViewFactory) {
        if (platformViewPluginAosp == null) {
            Log.i(LOG_TAG, "PlatformViewPluginAosp is null");
            return;
        }
        if (platformViewFactory == null) {
            Log.i(LOG_TAG, "PlatformViewFactory is null");
            return;
        }
        platformViewPluginAosp.registerPlatformViewFactory(platformViewFactory);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        if ((newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE
                || newConfig.orientation == Configuration.ORIENTATION_PORTRAIT) && windowView != null) {
            windowView.setWindowOrientation(true);
        }
    }

private void handleFoldStatusChange(WindowLayoutInfo windowLayoutInfo) {
    windowLayoutInfo.getDisplayFeatures().stream()
        .filter(FoldingFeature.class::isInstance)
        .map(FoldingFeature.class::cast)
        .findFirst()
        .ifPresent(feature -> {
            FoldManager.setFoldable(true);
            int foldStatus = FoldManager.getStatusFromFeature(this, feature);
            FoldManager.setFoldStatus(foldStatus);
            activityDelegate.dispatchFoldStatusChange(getInstanceName(), foldStatus);
        });
}
    private Consumer<WindowLayoutInfo> getfoldConsumer() {
        if (foldConsumer != null) {
            return foldConsumer;
        }
        foldConsumer = windowLayoutInfo -> {
            this.runOnUiThread(() -> handleFoldStatusChange(windowLayoutInfo));
        };
        return foldConsumer;
    }

    /**
     * Register the fold status change listener.
     *
     * @return the instance name.
     */
    public String onFoldStatusChange() {
        if (Build.VERSION.SDK_INT < ANDROID_API_31) {
            Log.e(LOG_TAG, "Android SDK needs to be greater than 31!");
            return null;
        }
        if (foldWindowInfoCallback != null && foldConsumer != null) {
            return instanceName;
        }
        try {
            WindowInfoTracker windowInfoTracker = WindowInfoTracker.getOrCreate(this);
            foldWindowInfoCallback = new WindowInfoTrackerCallbackAdapter(windowInfoTracker);
            foldWindowInfoCallback.addWindowLayoutInfoListener(this, this::runOnUiThread, getfoldConsumer());
        } catch (NoClassDefFoundError e) {
            foldWindowInfoCallback = null;
            foldConsumer = null;
            Log.e(LOG_TAG, "androidx.window is not support!\n" + e.toString());
            return null;
        }
        return instanceName;
    }

    /**
     * get FoldingFeature though listener, it is used by getFoldStatus.
     *
     * @return FoldingFeature.
     */
    public FoldingFeature getFoldingFeatureFromListener() {
        AtomicReference<FoldingFeature> foldingFeature = new AtomicReference<>();
        if (foldWindowInfoCallback != null) {
            Consumer<WindowLayoutInfo> consumerWindowLayout = FoldManager
                                                            .getConsumerWindowLayout(this, foldingFeature);
            foldWindowInfoCallback.addWindowLayoutInfoListener(this, this::runOnUiThread, consumerWindowLayout);
            foldWindowInfoCallback.removeWindowLayoutInfoListener(consumerWindowLayout);
        }
        return foldingFeature.get();
    }

    /**
     * close fold motoring.
     *
     */
    public void offFoldStatusChange() {
        if (foldWindowInfoCallback != null && foldConsumer != null) {
            foldWindowInfoCallback.removeWindowLayoutInfoListener(foldConsumer);
            foldConsumer = null;
            foldWindowInfoCallback = null;
        }
    }

    /**
     * get fold WindowInfo callback.
     *
     * @return foldWindowInfoCallback.
     */
    public WindowInfoTrackerCallbackAdapter getFoldWindowInfoCallback() {
        return foldWindowInfoCallback;
    }

}
