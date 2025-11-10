/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
import android.os.Bundle;
import android.os.Trace;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.fragment.app.Fragment;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Set;

import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONException;

import ohos.ace.adapter.AceEnv;
import ohos.ace.adapter.AcePlatformPlugin;
import ohos.ace.adapter.ArkUIXPluginRegistry;
import ohos.ace.adapter.PluginContext;
import ohos.ace.adapter.WindowViewInterface;
import ohos.ace.adapter.WindowViewAospInterface;
import ohos.ace.adapter.WindowViewBuilder;
import ohos.ace.adapter.capability.bridge.BridgeManager;
import ohos.ace.adapter.capability.grantresult.GrantResult;
import ohos.ace.adapter.capability.keyboard.KeyboardHeightObserver;
import ohos.ace.adapter.capability.keyboard.KeyboardHeightProvider;
import ohos.ace.adapter.capability.video.AceVideoPluginAosp;
import ohos.ace.adapter.capability.web.AceWebPluginAosp;
import ohos.ace.adapter.capability.web.AceWebPluginBase;
import ohos.ace.adapter.capability.platformview.PlatformViewFactory;
import ohos.ace.adapter.capability.platformview.AcePlatformViewPluginAosp;

/**
 * A base class for the Ability Cross-platform Environment of the stage model to
 * run on Android.
 * This class is inherited from android Fragment.
 *
 * @since 1
 */
public class StageFragment extends Fragment implements KeyboardHeightObserver {
    private static final String LOG_TAG = "StageFragment";

    private static final String INSTANCE_DEFAULT_NAME = "default";

    private static final String WANT_PARAMS = "params";

    private static final int ERR_INVALID_PARAMETERS = -1;

    private static final int ERR_OK = 0;

    private static final int WANT_PARAMS_TYPE = 10;

    private static final String PHOTO_VIDEO_TYPE = "image/*;video/*";

    private static final String FILE_ALL_TYPE = "*/*";

    private static final String PHOTO_PICKER = "com.ohos.photos";

    private static final String FILE_PICKER = "com.ohos.filepicker";

    private static final String MULTIPLE = "multipleselect";

    private static final String PHOTO_URI_KEY = "select-item-list";

    private static final String FILE_URI_KEY = "ability.params.stream";

    private static boolean isForResult = true;

    private static final int RESULTCODE_OK = 0;

    private static final int RESULTCODE_ERROR = 1;

    private static final int RESULT_OK = -1;

    private int requestCode = 0;

    private int instanceId = InstanceIdGenerator.getAndIncrement();

    private String instanceName;

    private String moduleName;

    private String bundleName;

    private String abilityName;

    private StageFragmentDelegate fragmentDelegate = null;

    private WindowViewAospInterface windowView = null;

    private FrameLayout framelayout;

    private AcePlatformPlugin platformPlugin = null;

    private BridgeManager bridgeManager = null;

    private Set<String> pluginList = new HashSet();

    private ArkUIXPluginRegistry arkUIXPluginRegistry = null;

    private PluginContext pluginContext = null;

    private boolean isToResume;

    private AcePlatformViewPluginAosp platformViewPluginAosp = null;

    private boolean isBackground = true;

    private KeyboardHeightProvider keyboardHeightProvider;

    @Override
    public void onKeyboardHeightChanged(int height) {
        if (windowView != null) {
            windowView.keyboardHeightChanged(height);
        }
        if (platformPlugin != null) {
            platformPlugin.keyboardHeightChanged(height);
        }
    }

    @Override
    public void onAvoidAreaChanged() {
        if (windowView != null) {
            windowView.avoidAreaChanged();
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(LOG_TAG, "OnCreate called, instance name:" + getInstanceName());
        super.onCreate(savedInstanceState);
        Trace.beginSection("StageFragment::onCreate");

        fragmentDelegate = new StageFragmentDelegate();
        fragmentDelegate.attachStageFragment(getInstanceName(), this);
        Trace.beginSection("createWindowView");
        windowView = WindowViewBuilder.makeWindowViewAosp(this.getActivity(), instanceId, true);
        windowView.setInstanceId(instanceId);
        Trace.endSection();

        fragmentDelegate.setWindowView(getInstanceName(), windowView);
        String params = "";
        if (getArguments() != null) {
            params = getArguments().getString(WANT_PARAMS);
            if (params == null) {
                params = "";
            }
        }
        fragmentDelegate.dispatchOnCreate(getInstanceName(), params);

        initPlatformPlugin(this.getActivity(), instanceId, windowView);
        initArkUIXPluginRegistry();
        Trace.endSection();

        keyboardHeightProvider = new KeyboardHeightProvider(this.getActivity());
        keyboardHeightProvider.setKeyboardHeightObserver(this);
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
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.i(LOG_TAG, "OnCreateView called, instance name:" + getInstanceName());
        if (framelayout == null) {
            framelayout = new FrameLayout(getActivity());
            FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT);
            framelayout.setLayoutParams(params);
            framelayout.addView(windowView.getView(), 0);
        }
        return framelayout;
    }

    @Override
    public void onResume() {
        Log.i(LOG_TAG, "OnResume called, instance name:" + getInstanceName());
        super.onResume();
        if (isHidden()) {
            Log.i(LOG_TAG, "onResume called, isHidden");
            isToResume = true;
            return;
        }
        isToResume = false;
        foreground();
    }

    @Override
    public void onStart() {
        Log.i(LOG_TAG, "onStart called, instance name:" + getInstanceName());
        super.onStart();
        if (isHidden()) {
            Log.i(LOG_TAG, "onStart called, isHidden");
            isToResume = true;
            return;
        }
        isToResume = false;
        foreground();
    }

    private void foreground() {
        if (!isBackground) {
            return;
        }
        isBackground = false;
        Trace.beginSection("StageFragment::foreground");
        fragmentDelegate.dispatchOnForeground(getInstanceName(), this);
        windowView.foreground();
        if (platformPlugin != null) {
            platformPlugin.notifyLifecycleChanged(false);
        }
        Trace.endSection();
    }

    @Override
    public void onHiddenChanged(boolean isHidden) {
        super.onHiddenChanged(isHidden);
        Log.i(LOG_TAG, "onHiddenChanged called, isHidden:" + isHidden);
        if (!isHidden && isToResume) {
            isToResume = false;

            Trace.beginSection("StageFragment::onHiddenChanged");
            fragmentDelegate.dispatchOnForeground(getInstanceName(), this);
            windowView.foreground();
            if (platformPlugin != null) {
                platformPlugin.notifyLifecycleChanged(false);
            }
            Trace.endSection();
        }
        setWindowOnTop(!isHidden);
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
        background();
    }

    @Override
    public void onPause() {
        Log.i(LOG_TAG, "onPause called, instance name:" + getInstanceName());
        super.onPause();
        background();
    }

    private void background() {
        if (isBackground) {
            return;
        }
        isBackground = true;
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
        keyboardHeightProvider.close();
        if (platformPlugin != null) {
            platformPlugin.release();
            Log.i(LOG_TAG, "StageFragment onDestroy releseResRegister called");
        }
    }

    @Override
    public void dump(String prefix, FileDescriptor fd, PrintWriter writer, String[] args) {
        Log.i(LOG_TAG, "StageFragment dump called");
        AceEnv.dump(instanceId, prefix, fd, writer, args);
    }

    /**
     * called by nativeActivity when back pressed
     *
     * @return Returns true then nativeActivity should call super.onBackPressed()
     */
    public boolean onBackPressed() {
        Log.i(LOG_TAG, "StageFragment onBackPressed called");
        return !windowView.backPressed();
    }

    /**
     * called when other view covers fragment
     *
     * @param isTop fragment is the top view
     */
    public void setWindowOnTop(boolean isTop) {
        Log.i(LOG_TAG, "setWindowOnTop " + isTop);
        if (windowView != null) {
            windowView.setHide(!isTop);
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
     * Register the platformView to fragment
     *
     * @param platformViewFactory The platformViewFactory.
     */
    public void registerPlatformViewFactory(PlatformViewFactory platformViewFactory) {
        if (platformViewPluginAosp == null) {
            Log.w(LOG_TAG, "PlatformViewPluginAosp is null");
            return;
        }
        if (platformViewFactory == null) {
            Log.w(LOG_TAG, "PlatformViewFactory is null");
            return;
        }
        platformViewPluginAosp.registerPlatformViewFactory(platformViewFactory);
    }


    /**
     * Initialize platform plugins
     *
     * @param context    Application context
     * @param instanceId the instance id
     * @param windowView the window view
     */
    private void initPlatformPlugin(Context context, int instanceId, WindowViewInterface windowView) {
        Trace.beginSection("StageFragment::initPlatformPlugin");
        platformPlugin = new AcePlatformPlugin(context, instanceId, windowView.getView());
        if (platformPlugin != null) {
            windowView.setInputConnectionClient(platformPlugin);
            platformPlugin.initTexturePlugin(instanceId);
            platformPlugin.addResourcePlugin(AceVideoPluginAosp.createRegister(context, moduleName));
            AceWebPluginBase web = AceWebPluginAosp.createRegister(context, windowView.getView());
            windowView.setWebPlugin(web);
            platformPlugin.addResourcePlugin(web);
            platformPlugin.initSurfacePlugin(context, instanceId);
            platformViewPluginAosp = AcePlatformViewPluginAosp.createRegister(context);
            platformPlugin.addResourcePlugin(platformViewPluginAosp);
            windowView.setPlatformViewPlugin(platformViewPluginAosp);
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
        Log.i(LOG_TAG, "onRequestPermissionsResult");
        Context context = getActivity().getApplicationContext();
        GrantResult grantResultsClass = new GrantResult(context);
        grantResultsClass.onRequestPremissionCallback(permissions, grantResults);
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
            bridgeManager = BridgeManager.getInstance();
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
