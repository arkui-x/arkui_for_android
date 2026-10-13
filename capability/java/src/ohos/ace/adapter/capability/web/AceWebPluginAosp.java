/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.content.Context;
import android.util.DisplayMetrics;
import android.webkit.WebSettings;
import android.widget.FrameLayout;

import ohos.ace.adapter.ALog;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
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

    private static final String WEBVIEW_FILE_ACCESS = "fileAccess";

    private static final String WEBVIEW_PAGE_URL = "pageUrl";

    private static final String RICH_TEXT_INIT = "richTextInit";

    private static final String INCOGNITO_MODE = "incognitoMode";

    private static final String RUN_JAVASCRIPT_ON_DOCUMENT_END = "runJavaScriptOnDocumentEnd";

    private static final String HTTP = "http://";

    private static final String HTTPS = "https://";

    private static final String ANDROID_VIRTUAL_ASSET_PREFIX = "file:///android_asset/";

    private static final String ARKUIX_PATH_MARKER = "files/arkui-x/";

    private static final long INVALID_CREATE_ID = -1;

    private static final long DEFAULT_USER_AGENT_TIMEOUT_MS = 1000L;

    private static final Object DEFAULT_USER_AGENT_LOCK = new Object();

    private static volatile String cachedDefaultUserAgent = null;

    private static volatile boolean defaultUserAgentLoadAttempted = false;

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

    private static void ensureDefaultUserAgent(Context context) {
        // The user agent is loaded at most once per process: after a failed attempt
        // (e.g. main-thread timeout) subsequent calls return the empty fallback
        // instead of blocking on the timed initialization again.
        if (defaultUserAgentLoadAttempted || context == null) {
            return;
        }
        synchronized (DEFAULT_USER_AGENT_LOCK) {
            if (!defaultUserAgentLoadAttempted) {
                defaultUserAgentLoadAttempted = true;
                cachedDefaultUserAgent = loadDefaultUserAgentOnMainThread(context);
            }
        }
    }

    private static String loadDefaultUserAgentOnMainThread(Context context) {
        if (Looper.myLooper() == Looper.getMainLooper()) {
            return WebSettings.getDefaultUserAgent(context);
        }
        final String[] userAgent = new String[1];
        CountDownLatch latch = new CountDownLatch(1);
        new Handler(Looper.getMainLooper()).post(() -> {
            try {
                userAgent[0] = WebSettings.getDefaultUserAgent(context);
            } finally {
                latch.countDown();
            }
        });
        try {
            if (!latch.await(DEFAULT_USER_AGENT_TIMEOUT_MS, TimeUnit.MILLISECONDS)) {
                ALog.e(LOG_TAG, "get default user agent timeout");
                return null;
            }
        } catch (InterruptedException e) {
            ALog.e(LOG_TAG, "get default user agent interrupted");
            return null;
        }
        return userAgent[0] == null ? "" : userAgent[0];
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

    /**
     * Returns the default user agent string on Android.
     *
     * <p>The value is lazily initialized on the first call (on the UI thread) and reused on
     * all subsequent calls, so that constructing the plugin does not initialize the WebView
     * provider during application startup. The initialization is attempted only once per
     * process; if it fails, subsequent calls return the empty fallback.</p>
     *
     * @return the default user agent string, or empty string if unavailable
     */
    public String getDefaultUserAgent() {
        if (cachedDefaultUserAgent == null) {
            ensureDefaultUserAgent(context);
        }
        return cachedDefaultUserAgent != null ? cachedDefaultUserAgent : "";
    }

    @Override
    public long create(Map<String, String> param) {
        if (param.isEmpty() || !param.containsKey(RESOURCE_TYPE)) {
            ALog.e(LOG_TAG, "Params doesn't contain web");
            return INVALID_CREATE_ID;
        }
        try {
            long id = Long.parseLong(param.get(RESOURCE_TYPE));
            String pageUrl = param.get(WEBVIEW_PAGE_URL);

            // Create AceWeb
            aceWeb = new AceWeb(id, context, rootView, getEventCallback());
            String webincognitoMode = param.get(INCOGNITO_MODE);
            boolean richTextInit = Integer.parseInt(param.get(RICH_TEXT_INIT)) == 1;
            addResource(id, aceWeb, richTextInit);
            aceWeb.initWeb();
            aceWeb.setIncognitoMode(String.valueOf(webincognitoMode));
            aceWeb.setPageUrl(pageUrl);
            if (param.containsKey(RUN_JAVASCRIPT_ON_DOCUMENT_END)) {
                aceWeb.runJavaScriptOnDocumentEnd(param);
            }
            // Apply fileAccess before loadUrl to ensure file:// URLs can be loaded
            String fileAccessValue = param.get(WEBVIEW_FILE_ACCESS);
            if (fileAccessValue != null) {
                Map<String, String> fileAccessParam = new HashMap<>();
                fileAccessParam.put(WEBVIEW_FILE_ACCESS, fileAccessValue);
                aceWeb.fileAccess(fileAccessParam);
            }
            String webSrc = param.get(WEBVIEW_SRC);
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
