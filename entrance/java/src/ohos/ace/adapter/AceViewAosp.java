/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

package ohos.ace.adapter;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.PixelFormat;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import ohos.ace.adapter.capability.clipboard.ClipboardPluginAosp;
import ohos.ace.adapter.capability.editing.TextInputPluginAosp;
import ohos.ace.adapter.capability.environment.EnvironmentAosp;
import ohos.ace.adapter.capability.storage.PersistentStorageAosp;
import ohos.ace.adapter.capability.texture.AceTexturePluginAosp;
import ohos.ace.adapter.capability.texture.IAceTexture;
import ohos.ace.adapter.capability.vibrator.VibratorPluginAosp;
import ohos.ace.adapter.capability.plugin.PluginManager;

import java.io.File;
import java.nio.ByteBuffer;

/**
 * This class is AceView implement and handles the lifecycle of surface.
 *
 * @since 1
 */
public class AceViewAosp extends SurfaceView implements IAceView, SurfaceHolder.Callback {
    private static final String LOG_TAG = "AceViewAosp";

    /**
     * Device type default, keep same with native in system properties
     */
    private static final int DEVICE_TYPE_DEFAULT = 0;

    /**
     * Device type tv, keep same with native in system properties
     */
    private static final int DEVICE_TYPE_TV = 1;

    private static final WindowManager.LayoutParams MATCH_PARENT = new WindowManager.LayoutParams(
            WindowManager.LayoutParams.MATCH_PARENT, WindowManager.LayoutParams.MATCH_PARENT);

    private final ViewPortMetrics metrics;

    private volatile int surfaceState = SURFACE_STATE_UNINITIALIZED;

    private Surface surface;

    private int surfaceWidth = 0;

    private int surfaceHeight = 0;

    private int instanceId = 0;

    private long nativeViewPtr = 0L;

    private View animateView;

    private AceResourceRegister resRegister;

    private ClipboardPluginAosp clipboardPlugin;

    private TextInputPluginAosp textInputPlugin;

    private EnvironmentAosp environmentPlugin;

    private PersistentStorageAosp persistentStoragePlugin;

    private VibratorPluginAosp vibratorPlugin;

    private PluginManager pluginManager;

    /**
     * Constructor of AceViewAosp
     *
     * @param context    Application context
     * @param instanceId The id of instance
     * @param density    The display pixel ratio
     * @param isWearable If the device is wearable
     */
    public AceViewAosp(Context context, int instanceId, float density, boolean isWearable) {
        super(context);
        ALog.i(LOG_TAG, "AceViewAosp created");
        this.instanceId = instanceId;
        setFocusableInTouchMode(true);
        createNativePtr(instanceId);
        getHolder().addCallback(this);
        Object manager = context.getSystemService(Context.WINDOW_SERVICE);
        if (manager instanceof WindowManager) {
            AceVsyncWaiter.getInstance((WindowManager) manager);
        }
        metrics = new ViewPortMetrics();
        metrics.devicePixelRatio = density;

        initCacheFilePath();
        initResRegister();
        initPlugins(context);
    }

    /**
     * Create native view and get it's pointer
     *
     * @param instanceId the instance of container
     */
    protected void createNativePtr(int instanceId) {
        if (nativeViewPtr == 0L) {
            nativeViewPtr = nativeCreateSurfaceHandle(this, instanceId);
        }
    }

    @Override
    public void releaseNativeView() {
        if (nativeViewPtr != 0L) {
            nativeDestroySurfaceHandle(nativeViewPtr);
            nativeViewPtr = 0L;
        }
    }

    @Override
    public long getNativePtr() {
        return nativeViewPtr;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        ALog.i(LOG_TAG, "surfaceCreated");
        setFocusable(true);
        requestFocus();
        surfaceState = SURFACE_STATE_CREATED;
        surface = holder.getSurface();
        nativeSurfaceCreated(nativeViewPtr, surface);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        surface = holder.getSurface();
        holder.setFormat(PixelFormat.TRANSLUCENT);
        surfaceWidth = width;
        surfaceHeight = height;
        metrics.physicialWidth = width;
        metrics.physicialHeight = height;
        updateViewportMetrics();
        int orientation = Configuration.ORIENTATION_PORTRAIT;
        Context context = getContext();
        if (context != null && context.getResources() != null) {
            orientation = context.getResources().getConfiguration().orientation;
        }
        ALog.i(LOG_TAG, "surface changed w=" + width + " h=" + height);
        nativeSurfaceChanged(nativeViewPtr, width, height, orientation);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        ALog.d(LOG_TAG, "surfaceDestroyed");
        if (surfaceState == SURFACE_STATE_UNINITIALIZED) {
            return;
        }
        surfaceState = SURFACE_STATE_UNINITIALIZED;
        if (nativeViewPtr != 0L) {
            nativeSurfaceDestroyed(nativeViewPtr);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (nativeViewPtr == 0L) {
            return super.onTouchEvent(event);
        }

        try {
            ByteBuffer packet = AceEventProcessorAosp.processTouchEvent(event);
            nativeDispatchPointerDataPacket(nativeViewPtr, packet, packet.position());
            return true;
        } catch (AssertionError error) {
            ALog.e(LOG_TAG, "process touch event failed: " + error.getMessage());
            return false;
        }
    }

    @Override
    public final WindowInsets onApplyWindowInsets(WindowInsets insets) {
        boolean statusBarHidden = (SYSTEM_UI_FLAG_FULLSCREEN & getWindowSystemUiVisibility()) != 0;
        boolean navigationBarHidden = (SYSTEM_UI_FLAG_HIDE_NAVIGATION & getWindowSystemUiVisibility()) != 0;

        metrics.physicialPaddingTop = statusBarHidden ? 0 : insets.getSystemWindowInsetTop();
        metrics.physicialPaddingBottom = navigationBarHidden ? 0 : insets.getSystemWindowInsetBottom();

        updateViewportMetrics();
        return super.onApplyWindowInsets(insets);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (nativeViewPtr == 0L) {
            return super.onKeyDown(keyCode, event);
        }

        if (nativeDispatchKeyEvent(nativeViewPtr, event.getKeyCode(), event.getAction(), event.getRepeatCount(),
                event.getEventTime(), event.getDownTime())) {
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (nativeViewPtr == 0L) {
            return super.onKeyUp(keyCode, event);
        }

        if (nativeDispatchKeyEvent(nativeViewPtr, event.getKeyCode(), event.getAction(), event.getRepeatCount(),
                event.getEventTime(), event.getDownTime())) {
            return true;
        }
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        if (textInputPlugin != null) {
            return textInputPlugin.createInputConnection(this, outAttrs);
        }
        return super.onCreateInputConnection(outAttrs);
    }

    @Override
    public void onShow() {
    }

    @Override
    public void onHide() {
    }

    @Override
    public void initDeviceType() {
        Context context = getContext();
        if (context == null) {
            return;
        }
        Resources resource = getResources();
        if (resource != null) {
            Configuration configuration = resource.getConfiguration();
            if (configuration != null) {
                int deviceType = DEVICE_TYPE_DEFAULT;
                int uiModeType = configuration.uiMode & Configuration.UI_MODE_TYPE_MASK;
                if (uiModeType == Configuration.UI_MODE_TYPE_TELEVISION) {
                    deviceType = DEVICE_TYPE_TV;
                }
                nativeInitDeviceType(deviceType);
            }
        }
    }

    @Override
    public void viewCreated() {
        ALog.i(LOG_TAG, "AceView view created");
        animateView = createAnimateView();
        if (animateView == null) {
            return;
        }
        if (!(getContext() instanceof Activity)) {
            return;
        }
        Activity activity = (Activity) getContext();
        activity.addContentView(animateView, MATCH_PARENT);
        onFirstFrame();
    }

    @Override
    public void addResourcePlugin(AceResourcePlugin plugin) {
        if (resRegister != null) {
            resRegister.registerPlugin(plugin);
        }
    }

    /**
     * destroy view and release it surface
     *
     */
    public void destroy() {
        if (surfaceState == SURFACE_STATE_UNINITIALIZED) {
            return;
        }
        surfaceState = SURFACE_STATE_UNINITIALIZED;
        nativeDestroySurfaceHandle(nativeViewPtr);
        nativeViewPtr = 0L;
    }

    /**
     * Initialize resource register
     *
     */
    public void initResRegister() {
        resRegister = new AceResourceRegister();
        if (nativeViewPtr == 0L) {
            return;
        }
        long resRegisterPtr = nativeInitResRegister(nativeViewPtr, resRegister);
        if (resRegisterPtr == 0L) {
            return;
        }
        resRegister.setRegisterPtr(resRegisterPtr);

        IAceTexture textureImpl = new IAceTexture() {
            @Override
            public void registerSurface(long textureId, Object surface) {
                if (nativeViewPtr == 0L) {
                    return;
                }
                nativeRegisterSurface(nativeViewPtr, textureId, surface);
            }

            @Override
            public void registerTexture(long textureId, Object surfaceTexture) {
                if (nativeViewPtr == 0L) {
                    return;
                }
                nativeRegisterTexture(nativeViewPtr, textureId, surfaceTexture);
            }

            @Override
            public void markTextureFrameAvailable(long textureId) {
                if (nativeViewPtr == 0L) {
                    return;
                }
                nativeMarkTextureFrameAvailable(nativeViewPtr, textureId);
            }

            @Override
            public void unregisterTexture(long textureId) {
                if (nativeViewPtr == 0L) {
                    return;
                }
                nativeUnregisterTexture(nativeViewPtr, textureId);
            }

            @Override
            public void unregisterSurface(long textureId) {
                if (nativeViewPtr == 0L) {
                    return;
                }
                nativeUnregisterSurface(nativeViewPtr, textureId);
            }
        };
        resRegister.registerPlugin(AceTexturePluginAosp.createRegister(textureImpl));
    }

    /**
     * Init cache image and file path
     *
     */
    public void initCacheFilePath() {
        if (nativeViewPtr == 0L) {
            return;
        }
        Context context = getContext();
        if (context == null) {
            return;
        }
        File filePath = context.getFilesDir();
        if (filePath == null) {
            ALog.e(LOG_TAG, "Get cache path failed");
            return;
        }
        File destImageDir = new File(filePath, "cache_images");
        if (!destImageDir.exists() && !destImageDir.mkdirs()) {
            ALog.e(LOG_TAG, "Create cache image path failed");
        }
        File destFileDir = new File(filePath, "cache_files");
        if (!destFileDir.exists() && !destFileDir.mkdirs()) {
            ALog.e(LOG_TAG, "Create cache file path failed");
        }
        nativeInitCacheFilePath(nativeViewPtr, destImageDir.getPath(), destFileDir.getPath());
    }

    /**
     * Called by native when render first frame.
     *
     */
    public void onFirstFrame() {
        ALog.i(LOG_TAG, "AceView onFirstFrame");
        if (animateView == null) {
            return;
        }
        ViewParent parent = animateView.getParent();
        if (parent instanceof ViewGroup) {
            ((ViewGroup) parent).removeView(animateView);
        }
        animateView = null;
    }

    private void initPlugins(Context context) {
        clipboardPlugin = new ClipboardPluginAosp(context);
        textInputPlugin = new TextInputPluginAosp(this, instanceId);
        environmentPlugin = new EnvironmentAosp(context);
        persistentStoragePlugin = new PersistentStorageAosp(context);
        vibratorPlugin = new VibratorPluginAosp(context);
        pluginManager = new PluginManager(context);
    }

    private View createAnimateView() {
        if (!(getContext() instanceof Activity)) {
            return null;
        }
        Activity activity = (Activity) getContext();
        View view = new View(activity);
        view.setLayoutParams(MATCH_PARENT);
        view.setBackgroundColor(nativeGetBackgroundColor(nativeViewPtr));
        return view;
    }

    private void updateViewportMetrics() {
        if (nativeViewPtr == 0L) {
            return;
        }
        nativeSetViewportMetrics(nativeViewPtr, metrics.devicePixelRatio, metrics.physicialWidth,
                metrics.physicialHeight, metrics.physicialPaddingTop,
                metrics.physicialPaddingRight, metrics.physicialPaddingBottom, metrics.physicialPaddingLeft,
                metrics.physicialViewInsetTop, metrics.physicialViewInsetRight,
                metrics.physicialViewInsetBottom, metrics.physicialViewInsetLeft, metrics.systemGestureInsetTop,
                metrics.systemGestureInsetRight, metrics.systemGestureInsetBottom, metrics.systemGestureInsetLeft);
    }

    private native long nativeCreateSurfaceHandle(AceViewAosp view, int instanceId);

    private native void nativeSurfaceCreated(long viewPtr, Surface surface);

    private native void nativeSurfaceChanged(long viewPtr, int width, int height, int orientation);

    private native void nativeSurfaceDestroyed(long viewPtr);

    private native void nativeDestroySurfaceHandle(long viewPtr);

    private native void nativeSetViewportMetrics(long viewPtr, float devicePixelRatio, int physicialWidth,
            int physicialHeight, int physicialPaddingTop, int physicialPaddingRight, int physicialPaddingBottom,
            int physicialPaddingLeft, int physicialViewInsetTop, int physicialViewInsetRight,
            int physicialViewInsetBottom,
            int physicialViewInsetLeft, int systemGestureInsetTop, int systemGestureInsetRight,
            int systemGestureInsetBottom,
            int systemGestureInsetLeft);

    private native boolean nativeDispatchPointerDataPacket(long viewPtr, ByteBuffer buffer, int position);

    private native void nativeRegisterTexture(long viewPtr, long textureId, Object surfaceTexture);

    private native void nativeUnregisterTexture(long viewPtr, long textureId);

    private native void nativeRegisterSurface(long viewPtr, long textureId, Object surface);

    private native void nativeUnregisterSurface(long viewPtr, long textureId);

    private native void nativeMarkTextureFrameAvailable(long viewPtr, long textureId);

    private native long nativeInitResRegister(long viewPtr, AceResourceRegister resRegister);

    private native void nativeInitCacheFilePath(long viewPtr, String imagePath, String filePath);

    private native boolean nativeDispatchKeyEvent(long viewPtr, int keyCode, int action, int repeatTime, long timeStamp,
            long timeStampStart);

    private native void nativeInitDeviceType(int deviceType);

    private native int nativeGetBackgroundColor(long viewPtr);

    private native void nativeSetCallback(long viewPtr, Object view);
}
