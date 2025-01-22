/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.web;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import android.content.Context;
import android.webkit.GeolocationPermissions;
import android.webkit.ValueCallback;

/**
 * This class is used for Web component geolocation rights management
 *
 * @since 1
 */
public class AceGeolocationPermissions {
    private static final String LOG_TAG = "AceGeolocationPermissions";
    private static final int INTEGER_ZERO = 0;
    private static final int INTEGER_ONE = 1;
    private static final long INSERT_FAILED = -1L;
    private static final int DELETE_FAILED = -1;
    private static final String INVALID_ORIGIN = "INVALID_ORIGIN";
    private static final String ORIGIN_END_WITH = "/";

    private Context context;

    private WebDataBaseManager dataBase;

    /**
     * constructor of AceGeolocationPermissions
     *
     * @param context application context
     */
    public AceGeolocationPermissions(Context context) {
        this.dataBase = WebDataBaseManager.getInstance(context);
        nativeInit();
    }

    /**
     * Methods to allow geolocation permissions.
     *
     * @param origin  Url source.
     * @param result  Whether to allow geolocation results.
     * @param incognito Is the current web component in privacy mode.
     */
    public void allowGeolocation(String origin, boolean result, boolean incognito) {
        if (!origin.endsWith(ORIGIN_END_WITH)) {
            origin = origin + ORIGIN_END_WITH;
        }
        if (incognito) {
            Integer resultValue = result ? INTEGER_ONE : INTEGER_ZERO;
            WebDataBaseGeolocationPermissions geolocationPermissions = new WebDataBaseGeolocationPermissions(origin,
                    resultValue);
            Long returnResult = dataBase.getGeolocationPermissionsDao()
                    .insertPermissionByOrigin(geolocationPermissions);
            if (returnResult == INSERT_FAILED) {
                throw new RuntimeException("Failed to insert geolocation permission for origin: " + origin);
            }
        } else {
            try {
                GeolocationPermissions.getInstance().allow(origin);
            } catch (Exception e) {
                throw new RuntimeException(
                    "GeolocationPermissions Failed to allowGeolocation  geolocation permissions");
            }
        }
    }

    /**
     * Remove the geolocation permission of the specified URL source.
     *
     * @param origin Url source.
     * @param incognito Is the current web component in privacy mode.
     */
    public void deleteGeolocation(String origin, boolean incognito) {
        if (!origin.endsWith(ORIGIN_END_WITH)) {
            origin = origin + ORIGIN_END_WITH;
        }
        if (incognito) {
            Integer returnResult = dataBase.getGeolocationPermissionsDao().clearPermissionByOrigin(origin);
            if (returnResult == DELETE_FAILED) {
                throw new RuntimeException("Failed to clear geolocation permission for origin: " + origin);
            }
        } else {
            try {
                GeolocationPermissions.getInstance().clear(origin);
            } catch (Exception e) {
                throw new RuntimeException(
                    "GeolocationPermissions Failed to deleteGeolocation  geolocation permissions");
            }
        }
    }

    /**
     * Ways to remove all geolocation permissions.
     *
     * @param incognito Is the current web component in privacy mode.
     */
    public void deleteAllGeolocation(boolean incognito) {
        if (incognito) {
            Integer returnResult = dataBase.getGeolocationPermissionsDao().clearAllPermission();
            if (returnResult == DELETE_FAILED) {
                throw new RuntimeException("Failed to clear all geolocation permissions");
            }
        } else {
            try {
                GeolocationPermissions.getInstance().clearAll();
            } catch (Exception e) {
                throw new RuntimeException(
                    "GeolocationPermissions Failed to deleteAllGeolocation  geolocation permissions");
            }
        }
    }

    /**
     * Gets geolocation permission information for the specified URL source.
     *
     * @param origin Url source.
     * @param asyncCallbackInfoId ID of asynchronous callback information.
     * @param incognito Is the current web component in privacy mode.
     */
    public void getAccessibleGeolocation(String origin, long asyncCallbackInfoId, boolean incognito) {
        if (!origin.endsWith(ORIGIN_END_WITH)) {
            origin = origin + ORIGIN_END_WITH;
        }
        if (incognito) {
            WebDataBaseGeolocationPermissions queriedPermission = dataBase.getGeolocationPermissionsDao()
                    .getPermissionResultByOrigin(origin);
            if (queriedPermission == null) {
                onReceiveGetAccessibleGeolocationError(INVALID_ORIGIN, asyncCallbackInfoId);
            } else {
                boolean value = queriedPermission.getResult() == INTEGER_ONE ? true : false;
                onReceiveGetAccessibleGeolocationValue(value, asyncCallbackInfoId);
            }
        } else {
            List<String> originsList = new ArrayList<>();
            final String finalOrigin = origin;
            GeolocationPermissions.getInstance().getOrigins(new ValueCallback<Set<String>>() {
                @Override
                public void onReceiveValue(Set<String> value) {
                    originsList.addAll(value);
                    boolean exist = originsList.contains(finalOrigin);
                    if (exist) {
                        GeolocationPermissions.getInstance().getAllowed(finalOrigin, new ValueCallback<Boolean>() {
                            @Override
                            public void onReceiveValue(Boolean value) {
                                onReceiveGetAccessibleGeolocationValue(value, asyncCallbackInfoId);
                            }
                        });
                    } else {
                        onReceiveGetAccessibleGeolocationError(INVALID_ORIGIN, asyncCallbackInfoId);
                    }
                }
            });
        }
    }

    /**
     * Gets all geolocation permission information for a mode.
     *
     * @param asyncCallbackInfoId ID of asynchronous callback information.
     * @param incognito  Is the current web component in privacy mode.
     */
    public void getStoredGeolocation(long asyncCallbackInfoId, boolean incognito) {
        if (incognito) {
            List<WebDataBaseGeolocationPermissions> queriedPermissions = dataBase.getGeolocationPermissionsDao()
                    .getOriginsByPermission();
            List<String> originsList = new ArrayList<>();
            for (WebDataBaseGeolocationPermissions permission : queriedPermissions) {
                originsList.add(permission.getOrigin());
            }
            String[] originsArray = originsList.toArray(new String[INTEGER_ZERO]);
            onReceiveGetStoredGeolocationValue(originsArray, asyncCallbackInfoId);
        } else {
            GeolocationPermissions.getInstance().getOrigins(new ValueCallback<Set<String>>() {
                @Override
                public void onReceiveValue(Set<String> value) {
                    String[] array = new String[value.size()];
                    value.toArray(array);
                    onReceiveGetStoredGeolocationValue(array, asyncCallbackInfoId);
                }
            });
        }
    }

    /**
     * initialization method.
     */
    protected native void nativeInit();

    /**
     * Callback method that gets the geolocation permission status of the specified source.
     *
     * @param value Specify the geolocation permission status of the Url source.
     * @param asyncCallbackInfoId ID of asynchronous callback information.
     */
    protected native static void onReceiveGetAccessibleGeolocationValue(boolean value, long asyncCallbackInfoId);

    /**
     * Callback method that gets the geolocation permission status of the specified source.
     *
     * @param errCode error message.
     * @param asyncCallbackInfoId ID of asynchronous callback information.
     */
    protected native static void onReceiveGetAccessibleGeolocationError(String errCode, long asyncCallbackInfoId);

    /**
     * Receive the result of getting the stored geolocation value.
     *
     * @param value Stores all source information for geolocation permission status.
     * @param asyncCallbackInfoId ID of asynchronous callback information.
     */
    protected native static void onReceiveGetStoredGeolocationValue(String[] value, long asyncCallbackInfoId);
}