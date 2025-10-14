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

package ohos.ace.adapter.capability.web;

import android.content.res.AssetManager;
import android.view.View;
import android.content.Context;
import android.util.DisplayMetrics;
import android.widget.FrameLayout;

import ohos.ace.adapter.ALog;

import java.io.IOException;
import java.io.InputStream;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

/**
 * The class for creating WebView on AOSP platform.
 *
 * @since 6
 */
public class AceWebPluginAosp extends AceWebPluginBase {
    private static final String LOG_TAG = "AceWebPluginAosp";

    private static final String RESOURCE_TYPE = "web";

    private static final String WEBVIEW_WIDTH = "width";

    private static final String WEBVIEW_HEIGHT = "height";

    private static final String WEBVIEW_TOP = "top";

    private static final String WEBVIEW_LEFT = "left";

    private static final String WEBVIEW_SRC = "src";

    private static final String WEBVIEW_PAGE_URL = "pageUrl";

    private static final String RICH_TEXT_INIT = "richTextInit";

    private static final String INCOGNITO_MODE = "incognitoMode";

    private static final String HTTP = "http://";

    private static final String HTTPS = "https://";

    private static final String ANDROID_VIRTUAL_ASSET_PREFIX = "file:///android_asset/";

    private static final String ARKUIX_PATH_MARKER = "files/arkui-x/";

    private static final long INVALID_CREATE_ID = -1;

    private final AtomicLong nextMapid = new AtomicLong(0L);

    private final View rootView;

    private Context context;

    private AceWeb aceWeb;

    private WebDataBaseManager dataBase;

    private AceWebPluginAosp(Context context, View view) {
        this.context = context;
        this.rootView = view;
        dataBase = WebDataBaseManager.getInstance(context);
    }

    /**
     * This is called to get the specific id of plugin instance.
     *
     * @return long the id number.
     */
    @Override
    public long getAtomicId() {
        return nextMapid.getAndIncrement();
    }

    /**
     * This is called to create AceWeb register.
     *
     * @param context the app context
     * @param view the root view
     * @return web register plugin
     */
    public static AceWebPluginAosp createRegister(Context context, View view) {
        return new AceWebPluginAosp(context, view);
    }

    @Override
    public long create(Map<String, String> param) {
        if (param.isEmpty() || !param.containsKey(RESOURCE_TYPE)) {
            ALog.e(LOG_TAG, "Params doesn't contain web");
            return INVALID_CREATE_ID;
        }
        try {
            long id = Long.parseLong(param.get(RESOURCE_TYPE));
            String webSrc = param.get(WEBVIEW_SRC);
            String pageUrl = param.get(WEBVIEW_PAGE_URL);

            // Create AceWeb
            aceWeb = new AceWeb(id, context, rootView, getEventCallback());
            String webincognitoMode = param.get(INCOGNITO_MODE);
            boolean richTextInit = Integer.parseInt(param.get(RICH_TEXT_INIT)) == 1;
            addResource(id, aceWeb, richTextInit);
            aceWeb.initWeb();
            aceWeb.setIncognitoMode(String.valueOf(webincognitoMode));
            aceWeb.setPageUrl(pageUrl);
            aceWeb.loadUrl(toAndroidAssetUrl(webSrc));
            int physicalWidth = toPhysicalPixels(Double.parseDouble(param.get(WEBVIEW_WIDTH)));
            int physicalHeight = toPhysicalPixels(Double.parseDouble(param.get(WEBVIEW_HEIGHT)));
            int top = toPhysicalPixels(Double.parseDouble(param.get(WEBVIEW_TOP)));
            int left = toPhysicalPixels(Double.parseDouble(param.get(WEBVIEW_LEFT)));
            validateVirtualDisplayDimensions(physicalWidth, physicalHeight);
            FrameLayout.LayoutParams params = aceWeb.buildLayoutParams(physicalWidth, physicalHeight, left, top);
            aceWeb.setWebLayout(physicalWidth, physicalHeight, left, top);
            aceWeb.addWebToSurface(params);
            addResourceStatic(richTextInit);
            return id;
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "NumberFormatException");
        }
        return INVALID_CREATE_ID;
    }

    // Creating a VirtualDisplay larger than the size of the device screen size
    private void validateVirtualDisplayDimensions(int width, int height) {
        DisplayMetrics metrics = context.getResources().getDisplayMetrics();
        if (width > metrics.widthPixels || height > metrics.heightPixels) {
            String message = "Creating the webview size: "
                    + "[" + width + ", " + height + "] may result in problems"
                    + "It is larger than the device screen size: "
                    + "[" + metrics.widthPixels + ", " + metrics.heightPixels + "].";
            ALog.w(LOG_TAG, message);
        }
        ALog.w(LOG_TAG, "Creating a webview size is less than the the device screen size");
    }

    /**
     * This is called to setWebDebuggingAccess.
     *
     * @param webDebuggingAccess whether open webDebuggingAccess
     */
    public void setWebDebuggingAccess(boolean webDebuggingAccess) {
        aceWeb.setWebDebuggingAccess(webDebuggingAccess);
    }

    /**
     * This is called to getWebDebuggingAccess.
     *
     * @param logicalPixels logicalPixels value
     * @return whether open webDebuggingAccess
     */
    private int toPhysicalPixels(double logicalPixels) {
        float density = 1.0f;
        return (int) Math.round(logicalPixels * density);
    }

    /**
     * This is called to create.
     *
     * @param webUrl web url
     * @return Android asset virtual url or original web url
     */
    private String toAndroidAssetUrl(String webUrl) {
        if (webUrl == null || webUrl.startsWith(HTTP) || webUrl.startsWith(HTTPS)) {
            return webUrl;
        }

        int pos = webUrl.indexOf(ARKUIX_PATH_MARKER);
        if (pos != -1) {
            String assetUrl = ANDROID_VIRTUAL_ASSET_PREFIX + webUrl.substring(pos + "files/".length());
            if (existsVirtualAsset(assetUrl)) {
                return assetUrl;
            }
        }
        return webUrl;
    }

    private boolean existsVirtualAsset(String url) {
        if (url == null || !url.startsWith(ANDROID_VIRTUAL_ASSET_PREFIX)) {
            return false;
        }

        AssetManager assetManager = this.context.getAssets();
        String assetPath = url.substring(ANDROID_VIRTUAL_ASSET_PREFIX.length());
        try (InputStream inputStream = assetManager.open(assetPath)) {
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    /**
     * This is called to getHttpAuthCredentials.
     *
     * @return httpAuthCredentials value
     */
    public boolean existHttpAuthCredentials() {
        return dataBase.existHttpAuthCredentials();
    }

    /**
     * This is called to deleteHttpAuthCredentials.
     */
    public void deleteHttpAuthCredentials() {
        dataBase.deleteAllAuthCredentials();
    }

    /**
     * This is called to saveHttpAuthCredentials.
     *
     * @param host host value
     * @param realm realm value
     * @param username username value
     * @param password password value
     */
    public void saveHttpAuthCredentials(String host, String realm, String username, String password) {
        dataBase.saveHttpAuthCredential(host, realm, username, password);
    }

    /**
     * This is called to getHttpAuthCredentials.
     *
     * @param host host value
     * @param realm realm value
     * @return httpAuthCredentials value
     */
    public Object getHttpAuthCredentials(String host, String realm) {
        return dataBase.getHttpAuthCredential(host, realm);
    }

    /**
     * This is called to getZoomAccess.
     *
     * @return zoomAccess value
     */
    public boolean getZoomAccess() {
        boolean access = true;
        if (aceWeb != null) {
            access = aceWeb.getZoomAccess();
        }
        return access;
    }
}
