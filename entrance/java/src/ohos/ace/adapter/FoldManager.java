/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
import android.os.Build;
import android.util.Log;
import androidx.core.util.Consumer;
import androidx.window.java.layout.WindowInfoTrackerCallbackAdapter;
import androidx.window.layout.DisplayFeature;
import androidx.window.layout.FoldingFeature;
import androidx.window.layout.WindowInfoTracker;
import androidx.window.layout.WindowLayoutInfo;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;

/** 
 * fold manager that get fold status and fold display mode.
 *
 * @since 2025-05-22
 */
public class FoldManager {

    private static final String TAG = "FoldManager";

    private static final int ANDROID_API_31 = 31;

    private static boolean foldAble = false;

    private static int foldStatus = FoldInfo.FOLD_STATUS_UNKNOWN;

    private FoldManager() {
    }

    /**
     * get is foldable of the device.
     *
     * @param activity the activity
     * @return true if foldable
     */
    public static boolean isFoldable(Activity activity) {
        if (Build.VERSION.SDK_INT < ANDROID_API_31) {
            return false;
        }
        FoldingFeature foldingFeature = null;
        if (isOpenFoldListener(activity)) {
            foldingFeature = getFoldingFeatureInvoke(activity);
        } else {
            foldingFeature = getFoldingFeature(activity);
        }
        return foldingFeature != null;
    }

    /**
     * get fold status
     *
     * @param activity the activity
     * @return fold status
     */
    public static int getFoldStatus(Activity activity) {
        if (Build.VERSION.SDK_INT < ANDROID_API_31) {
            return FoldInfo.FOLD_STATUS_UNKNOWN;
        }
        FoldingFeature foldingFeature = null;
        if (isOpenFoldListener(activity)) {
            foldingFeature = getFoldingFeatureInvoke(activity);
        } else {
            foldingFeature = getFoldingFeature(activity);
        }
        return getStatusFromFeature(activity, foldingFeature);
    }

    /** 
     * get fold status from the folding feature
     *
     * @param activity the activity
     * @param foldingFeature the folding feature
     * @return fold status
     */
    public static int getStatusFromFeature(Activity activity, FoldingFeature foldingFeature) {
        if (foldingFeature == null) {
            Log.e(TAG, "foldingFeature is null.");
            return FoldInfo.FOLD_STATUS_UNKNOWN;
        }
        if (foldingFeature.getState() == FoldingFeature.State.FLAT) {
            return FoldInfo.FOLD_STATUS_EXPANDED;
        } else if (foldingFeature.getState() == FoldingFeature.State.HALF_OPENED) {
            return FoldInfo.FOLD_STATUS_HALF_FOLDED;
        } else {
            return FoldInfo.FOLD_STATUS_UNKNOWN;
        }
    }

    /**
     * get fold feature
     *
     * @param activity the activity
     * @return FoldingFeature
     */
    public static FoldingFeature getFoldingFeature(Activity activity) {
        if (activity == null) {
            return null;
        }
        try {
            AtomicReference<FoldingFeature> foldingFeature = new AtomicReference<>();

            WindowInfoTracker windowInfoTracker = WindowInfoTracker.getOrCreate(activity);
            WindowInfoTrackerCallbackAdapter windowInfoTrackerCallbackAdapter = new WindowInfoTrackerCallbackAdapter(
                windowInfoTracker);
            Consumer<WindowLayoutInfo> consumerWindowLayout = getConsumerWindowLayout(activity, foldingFeature);
            windowInfoTrackerCallbackAdapter.addWindowLayoutInfoListener(activity, Runnable::run, consumerWindowLayout);
            windowInfoTrackerCallbackAdapter.removeWindowLayoutInfoListener(consumerWindowLayout);
            return foldingFeature.get();
        } catch (NoClassDefFoundError e) {
            Log.e(TAG, "androidx.window is not support!\n" + e.toString());
            return null;
        }
    }

    /**
     * get status of fold motoring in activity
     *
     * @param activity the activity
     * @return open or not
     */
    public static boolean isOpenFoldListener(Activity activity) {
        try {
            Class<?> clazz = activity.getClass();
            Method getFoldMethod = clazz.getMethod("getFoldWindowInfoCallback");
            Object result = getFoldMethod.invoke(activity);
            return result != null;
        } catch (InvocationTargetException e) {
            Log.e(TAG, "invoke WindowInfoCallback fail:" + e.getMessage());
            return false;
        } catch (NoSuchMethodException e) {
            Log.e(TAG, "invoke WindowInfoCallback fail:" + e.getMessage());
            return false;
        } catch (IllegalAccessException e) {
            Log.e(TAG, "invoke WindowInfoCallback fail:" + e.getMessage());
            return false;
        }
    }

    /** 
     * get FoldingFeature of fold motoring in activity
     *
     * @param activity the activity
     * @return FoldingFeature
     */
    public static FoldingFeature getFoldingFeatureInvoke(Activity activity) {
        try {
            Class<?> clazz = activity.getClass();
            Method getFoldMethod = clazz.getMethod("getFoldingFeatureFromListener");
            Object result = getFoldMethod.invoke(activity);
            if (result == null) {
                return null;
            }
            if (result instanceof FoldingFeature) {
                return (FoldingFeature) result;
            }
            return null;
        } catch (InvocationTargetException e) {
            Log.e(TAG, "invoke getFoldingFeature fail:" + e.getMessage());
            return null;
        } catch (NoSuchMethodException e) {
            Log.e(TAG, "invoke getFoldingFeature fail:" + e.getMessage());
            return null;
        } catch (IllegalAccessException e) {
            Log.e(TAG, "invoke getFoldingFeature fail:" + e.getMessage());
            return null;
        }
    }

    /**
     * set foldable from activity
     *
     * @param foldable is ture or false
     */
    public static void setFoldable(boolean foldable) {
        foldAble = foldable;
    }

    /**
     * set fold status from activity
     *
     * @param newFoldStatus fold status
     */
    public static void setFoldStatus(int newFoldStatus) {
        foldStatus = newFoldStatus;
    }

    /**
     * get an instance of windowLayoutInfo
     *
     * @param activity the activity
     * @param foldingFeature the folding feature
     * @return Consumer<WindowLayoutInfo>
     */
    public static Consumer<WindowLayoutInfo> getConsumerWindowLayout(Activity activity,
        AtomicReference<FoldingFeature> foldingFeature) {
        return windowLayoutInfo -> {
                activity.runOnUiThread(() -> {
                    handleFoldStatusChange(windowLayoutInfo, foldingFeature);
                });
            };
    }

    private static void handleFoldStatusChange(WindowLayoutInfo windowLayoutInfo,
        AtomicReference<FoldingFeature> foldingFeature) {
        windowLayoutInfo.getDisplayFeatures().stream()
            .filter(FoldingFeature.class::isInstance)
            .map(FoldingFeature.class::cast)
            .findFirst()
            .ifPresent(feature -> {
                foldingFeature.set((FoldingFeature) feature);
            });
    }
}
