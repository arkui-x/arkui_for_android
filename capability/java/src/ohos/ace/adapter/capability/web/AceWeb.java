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

package ohos.ace.adapter.capability.web;

import android.app.Activity;
import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.net.Uri;
import android.os.Environment;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.webkit.ConsoleMessage;
import android.webkit.CookieManager;
import android.webkit.DownloadListener;
import android.webkit.GeolocationPermissions;
import android.webkit.HttpAuthHandler;
import android.webkit.JsPromptResult;
import android.webkit.JsResult;
import android.webkit.PermissionRequest;
import android.webkit.URLUtil;
import android.webkit.ValueCallback;
import android.webkit.WebBackForwardList;
import android.webkit.WebChromeClient;
import android.webkit.WebMessage;
import android.webkit.WebMessagePort;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.FrameLayout;
import android.animation.ValueAnimator;
import java.io.File;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.ProtocolException;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.IAceOnCallResourceMethod;
import ohos.ace.adapter.IAceOnResourceEvent;
import ohos.ace.adapter.capability.web.AceWebConsoleMessageObject;
import ohos.ace.adapter.capability.web.AceWebErrorReceiveObject;
import ohos.ace.adapter.capability.web.AceWebHttpErrorReceiveObject;
import ohos.ace.adapter.capability.web.AceWebOverrideUrlObject;
import ohos.ace.adapter.capability.web.AceWebScrollObject;

/**
 * This class handles the lifecycle of a AceWebview.
 *
 * @since 1
 */
public class AceWeb extends AceWebBase {
    private static final String LOG_TAG = "AceWeb";

    private static final String ACE_OBJECT_NAME = "internalAceWebJS";

    private static final String WEBVIEW_WIDTH = "width";

    private static final String WEBVIEW_HEIGHT = "height";

    private static final String WEBVIEW_POSITION_LEFT = "left";

    private static final String WEBVIEW_POSITION_TOP = "top";

    private static final String WEBVIEW_SRC = "src";

    private static final String WEBVIEW_PAGE_URL = "pageUrl";

    private static final String WEBVIEW_PAGE_INVALID = "pageInvalid";

    private static final String PAGE_INVALID_FLAG = "1";

    private static final String SUCCESS_TAG = "success";

    private static final String FAIL_TAG = "fail";

    private static final String WEBVIEW_ACESS_STEP = "accessStep";
    private static final String WEBVIEW_PARAM_LOAD_URL = "load_url";
    private static final String NTC_PARAM_DEL_INTERFACE = "javaScriptInterfaceName";
    private static final String NTC_PARAM_LOADDATA_BASEURL = "load_data_base_url";
    private static final String NTC_PARAM_LOADDATA_DATA = "load_data_data";
    private static final String NTC_PARAM_LOADDATA_MIMETYPE = "load_data_mimetype";
    private static final String NTC_PARAM_LOADDATA_ENCODING = "load_data_encoding";
    private static final String NTC_PARAM_LOADDATA_HISTORY = "load_data_history_url";
    private static final String NTC_PARAM_REGISTER_JS_NAME = "jsInterfaceName";
    private static final String NTC_PARAM_REGISTER_JS_METHODLIST = "jsInterfaceMethodList";
    private static final String NTC_PARAM_SCROLLTO_X = "scroll_to_x";
    private static final String NTC_PARAM_SCROLLTO_Y = "scroll_to_y";
    private static final String NTC_PARAM_SCROLLBY_DELTAX = "scroll_by_deltax";
    private static final String NTC_PARAM_SCROLLBY_DELTAY = "scroll_by_deltay";
    private static final String NTC_ZOOM_FACTOR = "zoom_factor";

    private static final String NTC_ZOOM_ACCESS = "zoomAccess";
    private static final String NTC_ONLINE_IMAGE_ACCESS = "onlineImageAccess";
    private static final String NTC_GEOLOCATION_ACCESS = "geolocationAccess";
    private static final String NTC_BLOCK_NETWORK = "blockNetwork";
    private static final String NTC_MIXED_MODE = "mixedMode";
    private static final String NTC_DOM_STORAGE_ACCESS = "domStorageAccess";
    private static final String NTC_CACHE_MODE = "cacheMode";
    private static final String NTC_IMAGE_ACCESS = "imageAccess";
    private static final String NTC_JAVASCRIPT_ACCESS = "javascriptAccess";
    private static final String NTC_MIN_FONT_SIZE = "minFontSize";
    private static final String NTC_HORIZONTAL_SCROLLBAR_ACCESS = "horizontalScrollBarAccess";
    private static final String NTC_VERTICAL_SCROLLBAR_ACCESS = "verticalScrollBarAccess";
    private static final String NTC_BACKGROUND_COLOR = "backgroundColor";
    private static final String NTC_MEDIA_PLAY_GESTURE_ACCESS = "mediaPlayGestureAccess";
    private static final String WEB_MESSAGE_PORT_ONE = "port1";
    private static final String WEB_MESSAGE_PORT_TWO = "port2";
    private static final String ONE_STRING = "1";
    private static final String WEB_WEBVIEW_DB = "webview.db";
    private static final String WEB_WEBVIEW_CACHE = "webviewCache.db";

    private static final String WEB_DOWNLOAD_CONCAT_DIR = "/Temp/";

    private static final String WEB_DOWNLOAD_START_EVENT = "webDownloadStartEvent";

    private static final String WEB_DOWNLOAD_UPDATE_EVENT = "webDownloadUpdateEvent";

    private static final String WEB_DOWNLOAD_FAILED_EVENT = "webDownloadFailedEvent";

    private static final String WEB_DOWNLOAD_COMPLETE_EVENT = "webDownloadCompleteEvent";

    private static final int NO_ERROR = 0;

    private static final int WEBVIEW_PAGE_TOP = 0;

    private static final int WEBVIEW_PAGE_HALF = 2;

    private static final int WEBVIEW_DURATION = 500;

    private static final int WEB_DOWNLOAD_TASK_NUM = 3;

    private static final int WEB_DOWNLOAD_PERCENT = 100;

    private static final int WEB_DOWNLOAD_TIMER_DELAY = 500;

    private static final long WEB_DOWNLOAD_SPEED_RATE = 2L;

    private static final int CAN_NOT_POST_MESSAGE = 17100010;

    private static final int CAN_NOT_REGISTER_MESSAGE_EVENT = 17100006;

    private static final float WEBVIEW_ZOOMIN_VALUE = 1.2f;

    private static final float WEBVIEW_ZOOMOUT_VALUE = 0.8f;

    private static final int WEB_NETWORK_DISCONNECTED = 22;

    private static final int WEB_DOWNLOAD_USER_CANCELED = 40;

    private static final int WEB_DOWNLOAD_RUNNING = 0;

    private static final int WEB_DOWNLOAD_COMPLETED = 1;

    private static final int WEB_DOWNLOAD_CANCEL = 2;

    private static final int WEB_DOWNLOAD_INTERRUPTED = 3;

    private static final int WEB_DOWNLOAD_DEPANDING = 4;

    private static final Object WEB_LOCK = new Object();

    private static final int MIXED_MODE_DEFAULT_VALUE = 0;

    private static final int CACHE_MODE_OH_DEFAULT_VALUE = 0;

    private static final int CACHE_MODE_DEFAULT_VALUE = -1;

    private static String currentPageUrl;

    private static String routerUrl;

    private float width;

    private float height;

    private float left;

    private float top;

    private final IAceOnResourceEvent callback;

    private final Context context;

    private final AceWebView webView;

    private final View rootView;

    private boolean isWebOnPage = true;

    private boolean isZoomAccess = true;

    private boolean isIncognitoMode = false;

    private MotionEvent motionEvent;

    private ExecutorService downloadExecutor_;

    private List<WebMessagePort> webMessagePorts = new ArrayList<WebMessagePort>();

    private WebviewBroadcastReceive webviewBroadcastReceive_;

    private Timer webDownloadUpdateTimer_ = null;

    private HashMap<Long, String> webDownloadItemIdMap_ = new HashMap<Long, String>();

    private HashMap<String, AceWebDownloadItemObject> webDownloadItemMap_ =
        new HashMap<String, AceWebDownloadItemObject>();

    public class AceWebView extends WebView {
        private static final String LOG_TAG = "AceWebView";

        public AceWebView(Context context) {
            super(context);
        }

        @Override
        protected void onScrollChanged(int l, int t, int oldl, int oldt) {
            super.onScrollChanged(l, t, oldl, oldt);
            AceWebScrollObject object = new AceWebScrollObject(l, t);
            AceWeb.this.fireScrollChanged(object);
        }
    }

    public AceWeb(long id, Context context, View view, IAceOnResourceEvent callback) {
        super(id, callback);
        this.callback = callback;
        this.context = context;
        this.rootView = view;
        webView = new AceWebView(context);
        downloadExecutor_ = Executors.newFixedThreadPool(WEB_DOWNLOAD_TASK_NUM);
        registerWebviewReceiver(context);
    }

    @Override
    public void release() {
        if (webView != null) {
            removeWebFromSurface(webView);
            webView.destroy();
        }
    }

    public WebView getWebview() {
        return webView;
    }

    /**
     * This is to mark the ace page url which webview located.
     *
     * @param pageUrl the page url of the current Ace app.
     */
    public void setPageUrl(String pageUrl) {
        currentPageUrl = pageUrl;
    }

    public String getUrl() {
        if (this.webView == null) {
            return "";
        }
        return this.webView.getUrl();
    }

    @Override
    public String updateWebLayout(Map<String, String> params) {
        if ((!params.containsKey(WEBVIEW_POSITION_LEFT)) || (!params.containsKey(WEBVIEW_POSITION_TOP)) ||
                (!params.containsKey(WEBVIEW_WIDTH)) || (!params.containsKey(WEBVIEW_HEIGHT))) {
            ALog.w(LOG_TAG, "setWebLayout fail");
            return FAIL_TAG;
        }
        try {
            left = Float.parseFloat(params.get(WEBVIEW_POSITION_LEFT));
            top = Float.parseFloat(params.get(WEBVIEW_POSITION_TOP));
            width = Float.parseFloat(params.get(WEBVIEW_WIDTH));
            height = Float.parseFloat(params.get(WEBVIEW_HEIGHT));
            removeWebFromSurface(webView);
            addWebToSurface(buildLayoutParams(width, height, left, top));
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "updateWebLayout NumberFormatException");
            return FAIL_TAG;
        }
        return SUCCESS_TAG;
    }

    /**
     * This is called to add the webview to the surface to display.
     *
     * @param params layout params
     */
    public void addWebToSurface(FrameLayout.LayoutParams params) {
        Activity activity = (Activity) webView.getContext();
        if (activity.getWindow() == null) {
            return;
        }
        if (this.rootView == null) {
            ALog.w(LOG_TAG, "addWebToSurface rooView null");
            return;
        }
        View contentView = (ViewGroup) this.rootView.getParent();
        ViewGroup contentView1;
        if (contentView instanceof FrameLayout) {
            contentView1 = (FrameLayout) contentView;
            int contentChildCount = contentView1.getChildCount();
            contentView1.addView(webView, contentChildCount - 1, params);
            for (int index = 0; index < contentChildCount; index++) {
                View view = contentView1.getChildAt(index);
                String viewClassName = view.getClass().getName();
                if (viewClassName.equals("ohos.ace.adapter.WindowView")) {
                    view.setZ(100.f);
                } else {
                    view.setZ(-1.f);
                }
            }
        } else {
            ALog.w(LOG_TAG, "addWebToSurface error, not FrameLayout");
        }
    }

    /**
     * This is called to remove the webview to the surface.
     *
     * @param webView the instance of the webview.
     */
    public void removeWebFromSurface(View webView) {
        if (webView == null) {
            ALog.e(LOG_TAG, "cannot remove web because web is null");
            return;
        }
        if (webView.getParent() instanceof ViewGroup) {
            ((ViewGroup) webView.getParent()).removeView(webView);
        }
    }

    /**
     * This is called to set the size and position of webview.
     *
     * @param width  the width of the webview size.
     * @param height the height of the webview size.
     * @param left   the left of the webview pos.
     * @param top    the top of the webview pos.
     */
    public void setWebLayout(float width, float height, float left, float top) {
        this.width = width;
        this.height = height;
        this.left = left;
        this.top = top;
    }

    /**
     * This is called to add the event listener for web.
     *
     */
    public void initWebEventListener() {
        if (webView == null) {
            return;
        }
        webView.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                return false;
            }
        });
    }

    /**
     * This is called to add callback interface for web.
     *
     */
    public void initWebCallBack() {
        if (webView == null) {
            return;
        }
        webView.setWebViewClient(new WebViewClient() {
            @Override
            public void onReceivedError(WebView view, WebResourceRequest request, WebResourceError error) {
                AceWebErrorReceiveObject object = new AceWebErrorReceiveObject(error, request);
                AceWeb.this.fireErrorReceive(object);
            }

            @Override
            public void onPageStarted(WebView view, String url, Bitmap favicon) {
                AceWeb.this.onLoadingPage(url);
            }

            @Override
            public void onPageFinished(WebView view, String url) {
                AceWeb.this.onPageLoaded(url);
            }

            @Override
            public void onReceivedHttpError(WebView view, WebResourceRequest request, WebResourceResponse response) {
                AceWebHttpErrorReceiveObject object = new AceWebHttpErrorReceiveObject(request, response);
                AceWeb.this.fireHttpErrorReceive(object);
            }

            @Override
            public void doUpdateVisitedHistory(WebView view, String url, boolean isReload) {
                AceWeb.this.fireRefreshHistory(url);
            }

            @Override
            public void onScaleChanged(WebView view, float oldScale, float newScale) {
                AceWebScaleObject object = new AceWebScaleObject(oldScale, newScale);
                AceWeb.this.fireScaleChanged(object);
            }

            @Override
            public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
                AceWebOverrideUrlObject object = new AceWebOverrideUrlObject(request);
                return AceWeb.this.fireUrlLoadIntercept(object);
            }

            @Override
            public void onPageCommitVisible(WebView view, String url) {
                AceWeb.this.firePageVisible(url);
            }

            @Override
            public void onReceivedHttpAuthRequest(WebView view, HttpAuthHandler handler, String host, String realm) {
                AceWebHttpAuthRequestObject object = new AceWebHttpAuthRequestObject(handler, host, realm, context);
                AceWeb.this.fireHttpAuthRequestReceive(object);
            }
        });
        webView.setWebChromeClient(new WebChromeClient() {
            @Override
            public void onProgressChanged(WebView view, int newProgress) {
                AceWeb.this.firePageChanged(newProgress);
            }

            @Override
            public void onReceivedTitle(WebView view, String title) {
                AceWeb.this.firePageRecvTitle(title);
            }

            @Override
            public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
                AceWebConsoleMessageObject object = new AceWebConsoleMessageObject(consoleMessage);
                return AceWeb.this.firePageOnConsoleMessage(object);
            }

            @Override
            public boolean onShowFileChooser(WebView webView, ValueCallback<Uri[]> filePathCallback,
                    WebChromeClient.FileChooserParams fileChooserParams) {
                AceWebFileChooserObject object = new AceWebFileChooserObject(filePathCallback, fileChooserParams);
                return AceWeb.this.firePageOnShowFileChooser(object);
            }

            @Override
            public void onGeolocationPermissionsShowPrompt(String origin, GeolocationPermissions.Callback callback) {
                AceWebGeolocationPermissionsShowObject object = new AceWebGeolocationPermissionsShowObject(origin,
                        callback);
                AceWeb.this.firePageGeoPermission(object);
            }

            @Override
            public void onGeolocationPermissionsHidePrompt() {
                AceWeb.this.firePageGeoHidePermission();
            }

            @Override
            public void onPermissionRequest(PermissionRequest request) {
                AceWebPermissionRequestObject object = new AceWebPermissionRequestObject(request);
                AceWeb.this.firePermissionRequest(object);
            }

            @Override
            public boolean onJsPrompt(WebView view, String url, String message, String defaultValue,
                    JsPromptResult result) {
                AceWebJsPromptObject object = new AceWebJsPromptObject(url, message, defaultValue, result);
                boolean jsResult = AceWeb.this.fireJsPrompt(object);
                if (!jsResult) {
                    object.cancel();
                }
                return true;
            }

            @Override
            public boolean onJsAlert(WebView view, String url, String message, JsResult result) {
                AceWebJsDialogObject object = new AceWebJsDialogObject(url, message, result);
                boolean jsResult = AceWeb.this.fireJsAlert(object);
                if (!jsResult) {
                    object.cancel();
                }
                return true;
            }

            @Override
            public boolean onJsConfirm(WebView view, String url, String message, JsResult result) {
                AceWebJsDialogObject object = new AceWebJsDialogObject(url, message, result);
                boolean jsResult = AceWeb.this.fireJsConfirm(object);
                if (!jsResult) {
                    object.cancel();
                }
                return true;
            }
        });
        webView.setDownloadListener(new DownloadListener() {
            @Override
            public void onDownloadStart(String url, String userAgent, String contentDisposition, String mimetype,
                    long contentLength) {
                AceWebDownloadStartObject object = new AceWebDownloadStartObject(url, userAgent, contentDisposition,
                        mimetype, contentLength);
                AceWeb.this.fireDownloadStart(object);
            }
        });
    }

    /**
     * This is called to init the web.
     *
     */
    public void initWeb() {
        if (webView == null) {
            return;
        }
        webView.setFocusable(true);
        webView.setFocusableInTouchMode(true);
        webView.requestFocus();
        initWebSetting();
        initWebEventListener();
        initWebCallBack();

        CookieManager.getInstance().setAcceptCookie(true);
        CookieManager.getInstance().setAcceptThirdPartyCookies(webView, true);
    }

    /**
     * This is called when an error occured.
     *
     * @param url         the url of current webview is loading.
     * @param errorCode   the specific code of the error.
     * @param description the detail description of the error.
     */
    public void onError(String url, int errorCode, String description) {
        fireError(url, errorCode, description);
    }

    /**
     * This is called when the page is loading.
     *
     * @param url the url of current webview is loading.
     */
    public void onLoadingPage(String url) {
        firePageStarted(url);
    }

    /**
     * This is called when the page is already loaded.
     *
     * @param url the url of current webview is loaded.
     */
    public void onPageLoaded(String url) {
        firePageFinished(url);
    }

    /**
     * This is called to set the websetting.
     *
     */
    public void initWebSetting() {
        final WebSettings webSettings = webView.getSettings();
        webSettings.setJavaScriptEnabled(true);

        webSettings.setUseWideViewPort(true);
        webSettings.setLoadWithOverviewMode(true);

        webSettings.setSupportZoom(true);
        webSettings.setBuiltInZoomControls(true);
        webSettings.setDisplayZoomControls(false);

        webSettings.setCacheMode(WebSettings.LOAD_DEFAULT);
        webSettings.setAllowFileAccess(true);
        webSettings.setJavaScriptCanOpenWindowsAutomatically(true);
        webSettings.setLoadsImagesAutomatically(true);
        webSettings.setDefaultTextEncodingName("utf-8");
        webSettings.setDomStorageEnabled(true);
        webSettings.setBlockNetworkLoads(false);
        webSettings.setMixedContentMode(WebSettings.MIXED_CONTENT_NEVER_ALLOW);
        webSettings.setBlockNetworkImage(true);
        webSettings.setGeolocationEnabled(true);
        webSettings.setAllowContentAccess(false);

        webSettings.setAllowFileAccessFromFileURLs(true);
        WebView.setWebContentsDebuggingEnabled(false);
    }

    /**
     * Set stealth mode.
     *
     * @param incognitoModeValue The incoming stealth mode value.
     */
    public void setIncognitoMode(String incognitoModeValue) {
        final WebSettings webSettings = webView.getSettings();
        if (ONE_STRING.equals(incognitoModeValue)) {
            CookieManager.getInstance().removeAllCookies(null);
            CookieManager.getInstance().flush();
            context.deleteDatabase(WEB_WEBVIEW_DB);
            context.deleteDatabase(WEB_WEBVIEW_CACHE);
            webView.clearCache(true);
            webView.clearHistory();
            webView.clearFormData();
            webSettings.setCacheMode(WebSettings.LOAD_NO_CACHE);
            isIncognitoMode = true;
        } else {
            webSettings.setAppCacheEnabled(true);
            String appCachePath = webView.getContext().getApplicationContext().getDir(
                            "cache", Context.MODE_PRIVATE).getPath();
            webSettings.setAppCachePath(appCachePath);
            isIncognitoMode = false;
        }
    }

    /**
     * This is called to use basic size and pos to create the layout type.
     *
     * @param width  the width of the webview size.
     * @param height the height of the webview size.
     * @param left   the left of the webview pos.
     * @param top    the top of the webview pos.
     * @return LayoutParams
     */
    public FrameLayout.LayoutParams buildLayoutParams(float width, float height, float left, float top) {
        Activity activity = (Activity) webView.getContext();
        FrameLayout.LayoutParams params;
        if (height != 0 && width != 0) {
            params = new FrameLayout.LayoutParams((int) width, (int) height);
            params.setMargins((int) left, (int) top, 0, 0);
        } else {
            Display display = activity.getWindowManager().getDefaultDisplay();
            Point size = new Point();
            display.getSize(size);
            int newWidth = size.x;
            int newHeight = size.y;
            params = new FrameLayout.LayoutParams(newWidth, newHeight);
        }
        return params;
    }

    /**
     * This is called to judge whether the two float numbers are equal.
     *
     * @param a one float number.
     * @param b the other float number.
     * @return boolean true means equal, false means not equal
     */
    public boolean isEqual(float a, float b) {
        if (Float.isNaN(a) || Float.isNaN(b) || Float.isInfinite(a) || Float.isInfinite(b)) {
            return false;
        }
        return (a - b) < 0.001d;
    }

    /**
     * This is called to load the given url.
     *
     * @param url the url the web needs to load.
     * @return the result.
     */
    public String loadUrl(String url) {
        if (webView != null) {
            this.webView.loadUrl(url);
            return SUCCESS_TAG;
        }
        return FAIL_TAG;
    }

    /**
     * This is called to load the given url.
     *
     * @param params the url the web needs to load.
     */
    @Override
    public void loadUrl(String url, Map<String, String> header) {
        if (this.webView == null) {
            return;
        }
        this.webView.loadUrl(url, header);
    }

    /**
     * This is called to load the data.
     *
     * @param params the string map.
     * @return String the result of the function.
     */
    @Override
    public String loadData(Map<String, String> params) {
        if (!params.containsKey(NTC_PARAM_LOADDATA_DATA)) {
            return FAIL_TAG;
        }
        if (!params.containsKey(NTC_PARAM_LOADDATA_MIMETYPE)) {
            return FAIL_TAG;
        }
        if (!params.containsKey(NTC_PARAM_LOADDATA_ENCODING)) {
            return FAIL_TAG;
        }
        String data = params.get(NTC_PARAM_LOADDATA_DATA);
        String mimeType = params.get(NTC_PARAM_LOADDATA_MIMETYPE);
        String encoding = params.get(NTC_PARAM_LOADDATA_ENCODING);
        if (params.containsKey(NTC_PARAM_LOADDATA_BASEURL)) {
            String baseUrl = params.get(NTC_PARAM_LOADDATA_BASEURL);
            String historyUrl = params.get(NTC_PARAM_LOADDATA_HISTORY);
            this.webView.loadDataWithBaseURL(baseUrl, data, mimeType, encoding, historyUrl);
            return SUCCESS_TAG;
        } else {
            this.webView.loadData(data, mimeType, encoding);
            return SUCCESS_TAG;
        }
    }

    /**
     * This is called to reloads the current URL.
     *
     * @param params the string map.
     * @return String the result of the function.
     */
    @Override
    public String reload(Map<String, String> params) {
        if (webView != null) {
            this.webView.reload();
            return SUCCESS_TAG;
        }
        return FAIL_TAG;
    }

    /**
     * This is called to stop loading the current URL.
     *
     * @param params the string map.
     * @return String the result of the function.
     */
    @Override
    public String stopLoading(Map<String, String> params) {
        if (webView != null) {
            this.webView.stopLoading();
            return SUCCESS_TAG;
        }
        return FAIL_TAG;
    }

    /**
     * This is called to update the current URL.
     *
     * @param params the string map.
     * @return String the result of the function.
     */
    @Override
    public String updateUrl(Map<String, String> params) {
        if (params == null || webView == null) {
            return FAIL_TAG;
        }
        if (!params.containsKey(WEBVIEW_SRC)) {
            return FAIL_TAG;
        }
        String webSrc = params.get(WEBVIEW_SRC);
        if (webView != null) {
            this.webView.loadUrl(webSrc);
            return SUCCESS_TAG;
        }
        return FAIL_TAG;
    }

    /**
     * This is called to router back webview.
     *
     * @param params the string map.
     * @return String the result of the function.
     */
    @Override
    public String routerBack(Map<String, String> params) {
        if (webView != null) {
            removeWebFromSurface(webView);
            return SUCCESS_TAG;
        }
        return FAIL_TAG;
    }

    @Override
    public String changePageUrl(Map<String, String> params) {
        if (webView == null || !params.containsKey(WEBVIEW_PAGE_URL)) {
            return FAIL_TAG;
        }
        String newPageUrl = params.get(WEBVIEW_PAGE_URL);
        if (newPageUrl.equals(currentPageUrl)) {
            addWebToSurface(buildLayoutParams(width, height, left, top));
            webView.onResume();
            isWebOnPage = true;
        }
        return SUCCESS_TAG;
    }

    @Override
    public String pagePathInvalid(Map<String, String> params) {
        if (webView == null || !params.containsKey(WEBVIEW_PAGE_INVALID)) {
            return FAIL_TAG;
        }
        String isPagePathInvalid = params.get(WEBVIEW_PAGE_INVALID);
        if (!routerUrl.equals(currentPageUrl) && webView != null
                && PAGE_INVALID_FLAG.equals(isPagePathInvalid)) {
            isWebOnPage = false;
            removeWebFromSurface(webView);
            webView.onPause();
        }
        return SUCCESS_TAG;
    }

    /**
     * This is called to sets whether the WebView should support zooming.
     *
     * @param isSupport the boolean var to specify.
     */
    public void setSupportZoom(boolean isSupport) {
        if (webView != null) {
            webView.getSettings().setSupportZoom(isSupport);
        }
    }

    /**
     * This is called to sets whether the WebView should support for the "viewport"
     * HTML meta tag
     * or should use a wide viewport.
     *
     * @param isSupport the boolean var to specify.
     */
    public void setUseWideViewPort(boolean isSupport) {
        if (webView != null) {
            webView.getSettings().setUseWideViewPort(isSupport);
        }
    }

    @Override
    public void onActivityPause() {
        ALog.i(LOG_TAG, "onActivityPause enter.");
    }

    @Override
    public void onActivityResume() {
        ALog.i(LOG_TAG, "onActivityResume enter.");
    }

    /**
     * This is called to destroy the webview when the activity is destroyed.
     *
     */
    public void onActivityDestroy() {
        webView.destroy();
    }

    @Override
    public String backward(Map<String, String> params) {
        this.webView.goBack();
        return SUCCESS_TAG;
    }

    @Override
    public String forward(Map<String, String> params) {
        this.webView.goForward();
        return SUCCESS_TAG;
    }

    @Override
    public String accessBackward(Map<String, String> params) {
        return String.valueOf(webView.canGoBack());
    }

    @Override
    public String accessForward(Map<String, String> params) {
        return String.valueOf(webView.canGoForward());
    }

    @Override
    public String accessStep(Map<String, String> params) {
        if (params == null || webView == null) {
            return String.valueOf(false);
        }
        if (!params.containsKey(WEBVIEW_ACESS_STEP)) {
            return String.valueOf(false);
        }
        String accessStep = params.get(WEBVIEW_ACESS_STEP);
        try {
            return String.valueOf(webView.canGoBackOrForward(Integer.parseInt(accessStep)));
        } catch (NumberFormatException e) {
            ALog.e(LOG_TAG, "JSAPI parse int error, accessStep:" + accessStep);
            return String.valueOf(false);
        }
    }

    @Override
    public String getHitTest(Map<String, String> params) {
        int hitTestType = webView.getHitTestResult().getType();
        return String.valueOf(hitTestType);
    }

    @Override
    public String onActive(Map<String, String> params) {
        this.webView.onResume();
        return SUCCESS_TAG;
    }

    @Override
    public String requestFocus(Map<String, String> params) {
        this.webView.requestFocus();
        return SUCCESS_TAG;
    }

    @Override
    public String onInactive(Map<String, String> params) {
        this.webView.onPause();
        return SUCCESS_TAG;
    }

    @Override
    public void setTouchEvent(MotionEvent event) {
        motionEvent = MotionEvent.obtain(event);
    }

    public void processTouchEvent() {
        if (motionEvent == null || webView == null) {
            return;
        }
        MotionEvent eventClone = MotionEvent.obtain(motionEvent);
        float deltaX = eventClone.getX() - left;
        float deltaY = eventClone.getY() - top;
        if ((deltaX > 0) && (deltaX < width) && (deltaY > 0) && (deltaY < height)) {
            eventClone.offsetLocation(-left, -top);
            webView.dispatchTouchEvent(eventClone);
        }
        motionEvent = null;
    }

    @Override
    public String touchDown(Map<String, String> params) {
        processTouchEvent();
        return SUCCESS_TAG;
    }

    @Override
    public String touchUp(Map<String, String> params) {
        processTouchEvent();
        return SUCCESS_TAG;
    }

    @Override
    public String touchMove(Map<String, String> params) {
        processTouchEvent();
        return SUCCESS_TAG;
    }

    @Override
    public String touchCancel(Map<String, String> params) {
        processTouchEvent();
        return SUCCESS_TAG;
    }

    @Override
    public String updateLayout(Map<String, String> params) {
        if (webView != null) {
            try {
                left = Float.parseFloat(params.get(WEBVIEW_POSITION_LEFT));
                top = Float.parseFloat(params.get(WEBVIEW_POSITION_TOP));
                width = Float.parseFloat(params.get(WEBVIEW_WIDTH));
                height = Float.parseFloat(params.get(WEBVIEW_HEIGHT));
                webView.setLayoutParams(buildLayoutParams(width, height, left, top));
                return SUCCESS_TAG;
            } catch (NumberFormatException ignored) {
                ALog.w(LOG_TAG, "updateWebLayout NumberFormatException");
                return FAIL_TAG;
            }
        }
        return FAIL_TAG;
    }

    @Override
    public String zoomAccess(Map<String, String> params) {
        if (!params.containsKey(NTC_ZOOM_ACCESS) || webView == null) {
            return FAIL_TAG;
        }
        boolean access = true;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_ZOOM_ACCESS));
            access = accessNum == 1 ? true : false;
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "zoomAccess NumberFormatException");
            return FAIL_TAG;
        }
        isZoomAccess = access;
        webView.getSettings().setSupportZoom(access);
        return SUCCESS_TAG;
    }

    private String getErrorInfo(String params) {
        ALog.w(LOG_TAG, params + " NumberFormatException");
        return FAIL_TAG;
    }

    @Override
    public String onlineImageAccess(Map<String, String> params) {
        if (!params.containsKey(NTC_ONLINE_IMAGE_ACCESS) || webView == null) {
            return FAIL_TAG;
        }
        boolean access = true;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_ONLINE_IMAGE_ACCESS));
            access = accessNum != 0;
        } catch (NumberFormatException ignored) {
            return getErrorInfo("onlineImageAccess");
        }
        webView.getSettings().setBlockNetworkImage(access);
        return SUCCESS_TAG;
    }

    @Override
    public String geolocationAccess(Map<String, String> params) {
        if (!params.containsKey(NTC_GEOLOCATION_ACCESS) || webView == null) {
            return FAIL_TAG;
        }
        boolean access = true;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_GEOLOCATION_ACCESS));
            access = accessNum != 0;
        } catch (NumberFormatException ignored) {
            return getErrorInfo("geolocationAccess");
        }
        webView.getSettings().setGeolocationEnabled(access);
        return SUCCESS_TAG;
    }

    @Override
    public String blockNetwork(Map<String, String> params) {
        if (!params.containsKey(NTC_BLOCK_NETWORK) || webView == null) {
            return FAIL_TAG;
        }
        boolean access = true;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_BLOCK_NETWORK));
            access = accessNum != 0;
        } catch (NumberFormatException ignored) {
            return getErrorInfo("blockNetwork");
        }
        webView.getSettings().setBlockNetworkLoads(access);
        return SUCCESS_TAG;
    }

    @Override
    public String mixedMode(Map<String, String> params) {
        if (!params.containsKey(NTC_MIXED_MODE) || webView == null) {
            return FAIL_TAG;
        }
        int access = MIXED_MODE_DEFAULT_VALUE;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_MIXED_MODE));
            access = accessNum;
        } catch (NumberFormatException ignored) {
            return getErrorInfo("mixedMode");
        }
        webView.getSettings().setMixedContentMode(access);
        return SUCCESS_TAG;
    }

    @Override
    public String domStorageAccess(Map<String, String> params) {
        if (!params.containsKey(NTC_DOM_STORAGE_ACCESS) || webView == null) {
            return FAIL_TAG;
        }
        boolean access = true;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_DOM_STORAGE_ACCESS));
            access = accessNum != 0;
        } catch (NumberFormatException ignored) {
            return getErrorInfo("domStorageAccess");
        }
        webView.getSettings().setDomStorageEnabled(access);
        return SUCCESS_TAG;
    }

    @Override
    public String cacheMode(Map<String, String> params) {
        if (!params.containsKey(NTC_CACHE_MODE) || webView == null) {
            return FAIL_TAG;
        }
        int access = CACHE_MODE_DEFAULT_VALUE;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_CACHE_MODE));
            access = accessNum == CACHE_MODE_OH_DEFAULT_VALUE ? CACHE_MODE_DEFAULT_VALUE : accessNum;
        } catch (NumberFormatException ignored) {
            return getErrorInfo("cacheMode");
        }
        webView.getSettings().setCacheMode(access);
        return SUCCESS_TAG;
    }

    @Override
    public String imageAccess(Map<String, String> params) {
        if (!params.containsKey(NTC_IMAGE_ACCESS) || webView == null) {
            return FAIL_TAG;
        }
        boolean access = true;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_IMAGE_ACCESS));
            access = accessNum != 0;
        } catch (NumberFormatException ignored) {
            return getErrorInfo("imageAccess");
        }
        webView.getSettings().setLoadsImagesAutomatically(access);
        return SUCCESS_TAG;
    }

    @Override
    public String javascriptAccess(Map<String, String> params) {
        if (!params.containsKey(NTC_JAVASCRIPT_ACCESS) || webView == null) {
            return FAIL_TAG;
        }
        boolean access = true;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_JAVASCRIPT_ACCESS));
            access = accessNum == 1 ? true : false;
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "javascriptAccess NumberFormatException");
            return FAIL_TAG;
        }
        webView.getSettings().setJavaScriptEnabled(access);
        return SUCCESS_TAG;
    }

    @Override
    public String minFontSize(Map<String, String> params) {
        if (!params.containsKey(NTC_MIN_FONT_SIZE) || webView == null) {
            return FAIL_TAG;
        }
        int fontSize = 0;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_MIN_FONT_SIZE));
            fontSize = accessNum;
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "minFontSize NumberFormatException");
            return FAIL_TAG;
        }
        webView.getSettings().setMinimumFontSize(fontSize);
        return SUCCESS_TAG;
    }

    @Override
    public String horizontalScrollBarAccess(Map<String, String> params) {
        if (!params.containsKey(NTC_HORIZONTAL_SCROLLBAR_ACCESS) || webView == null) {
            return FAIL_TAG;
        }
        boolean access = true;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_HORIZONTAL_SCROLLBAR_ACCESS));
            access = accessNum == 1 ? true : false;
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "horizontalScrollBarAccess NumberFormatException");
            return FAIL_TAG;
        }
        webView.setHorizontalScrollBarEnabled(access);
        return SUCCESS_TAG;
    }

    @Override
    public String verticalScrollBarAccess(Map<String, String> params) {
        if (!params.containsKey(NTC_VERTICAL_SCROLLBAR_ACCESS) || webView == null) {
            return FAIL_TAG;
        }
        boolean access = true;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_VERTICAL_SCROLLBAR_ACCESS));
            access = accessNum == 1 ? true : false;
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "verticalScrollBarAccess NumberFormatException");
            return FAIL_TAG;
        }
        webView.setVerticalScrollBarEnabled(access);
        return SUCCESS_TAG;
    }

    @Override
    public String backgroundColor(Map<String, String> params) {
        if (!params.containsKey(NTC_BACKGROUND_COLOR) || webView == null) {
            return FAIL_TAG;
        }
        int backgroundColor = 0;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_BACKGROUND_COLOR));
            backgroundColor = accessNum;
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "backgroundColor NumberFormatException");
            return FAIL_TAG;
        }
        webView.setBackgroundColor(backgroundColor);
        return SUCCESS_TAG;
    }

    @Override
    public String mediaPlayGestureAccess(Map<String, String> params) {
        if (!params.containsKey(NTC_MEDIA_PLAY_GESTURE_ACCESS) || webView == null) {
            return FAIL_TAG;
        }
        boolean access = true;
        try {
            int accessNum = Integer.parseInt(params.get(NTC_MEDIA_PLAY_GESTURE_ACCESS));
            access = accessNum == 1 ? true : false;
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "mediaPlayGestureAccess NumberFormatException");
            return FAIL_TAG;
        }
        webView.getSettings().setMediaPlaybackRequiresUserGesture(access);
        return SUCCESS_TAG;
    }

    @Override
    public void evaluateJavascript(String script, long asyncCallbackInfoId) {
        if (this.webView == null) {
            return;
        }
        this.webView.evaluateJavascript(script, new ValueCallback<String>() {
            @Override
            public void onReceiveValue(String value) {
                ALog.w(LOG_TAG, "evaluateJavascript onReceiveValue:" + value);
                // native c++
                AceWebPluginBase.onReceiveValue(value, asyncCallbackInfoId);
            }
        });
    }

    @Override
    public void evaluateJavascriptExt(String script, long asyncCallbackInfoId) {
        if (this.webView == null) {
            return;
        }
        this.webView.evaluateJavascript(script, new ValueCallback<String>() {
            @Override
            public void onReceiveValue(String value) {
                if (value.equals("null")) {
                    value = "This type not support, only string is supported";
                }
                AceWebPluginBase.onReceiveRunJavaScriptExtValue(value, asyncCallbackInfoId);
            }
        });
    }

    @Override
    public WebBackForwardList getBackForwardEntries() {
        if (this.webView == null) {
            return null;
        }
        return webView.copyBackForwardList();
    }

    @Override
    public void clearCache(boolean includeDiskFiles) {
        if (this.webView == null) {
            return;
        }
        webView.clearCache(includeDiskFiles);
    }

    @Override
    public void goBackOrForward(int steps) {
        if (this.webView == null) {
            return;
        }
        webView.goBackOrForward(steps);
    }

    @Override
    public String getTitle() {
        if (this.webView == null) {
            return "";
        }
        return webView.getTitle();
    }

    @Override
    public int getContentHeight() {
        if (this.webView == null) {
            return -1;
        }
        return webView.getContentHeight();
    }

    @Override
    public String[] createWebMessagePorts() {
        WebMessagePort messagePorts[] = webView.createWebMessageChannel();
        webMessagePorts.clear();
        for (WebMessagePort port : messagePorts) {
            webMessagePorts.add(port);
        }
        String[] ports = { WEB_MESSAGE_PORT_ONE, WEB_MESSAGE_PORT_TWO };
        return ports;
    }

    public WebMessagePort getWebMessagePort(String port) {
        if (webMessagePorts.isEmpty()) {
            return null;
        }
        if (port.equals(WEB_MESSAGE_PORT_ONE)) {
            return webMessagePorts.get(0);
        }
        if (port.equals(WEB_MESSAGE_PORT_TWO)) {
            return webMessagePorts.get(1);
        }
        return null;
    }

    @Override
    public void postWebMessage(String message, String[] ports, String targetUri) {
        if (!webMessagePorts.isEmpty()) {
            int length = ports.length;
            WebMessagePort messagePorts[] = new WebMessagePort[length];
            for (int i = 0; i < length; i++) {
                messagePorts[i] = getWebMessagePort(ports[i]);
            }
            WebMessage webMessage = new WebMessage(message, messagePorts);
            try {
                webView.postWebMessage(webMessage, Uri.parse(targetUri));
            } catch (IllegalStateException e) {
                ALog.e(LOG_TAG, "Port is already transferred");
            }
        }
    }

    @Override
    public void closeWebMessagePort(String portHandle) {
        if (webMessagePorts.isEmpty() || portHandle == null) {
            return;
        }
        WebMessagePort port = getWebMessagePort(portHandle);
        if (port != null) {
            try {
                port.close();
            } catch (IllegalStateException e) {
                ALog.e(LOG_TAG, "Port is already transferred");
            }
        }
    }

    @Override
    public int postMessageEvent(String portHandle, String webMessageData) {
        if (webMessagePorts.isEmpty() || portHandle == null) {
            return CAN_NOT_POST_MESSAGE;
        }
        WebMessagePort port = getWebMessagePort(portHandle);
        if (port == null) {
            return CAN_NOT_POST_MESSAGE;
        }
        WebMessage webMessage = new WebMessage(webMessageData);
        try {
            port.postMessage(webMessage);
        } catch (IllegalStateException e) {
            ALog.e(LOG_TAG, "postMessageEvent has already disenabled");
            return CAN_NOT_POST_MESSAGE;
        }
        return NO_ERROR;
    }

    /**
     * Send message to HTML5.
     *
     * @param portHandle Message port handle.
     * @param webMessageData The webMessageData is a message sent to H5.
     */
    @Override
    public int postMessageEventExt(String portHandle, String webMessageData) {
        if (webMessagePorts.isEmpty() || portHandle == null) {
            return CAN_NOT_POST_MESSAGE;
        }
        WebMessagePort port = getWebMessagePort(portHandle);
        if (port == null) {
            return CAN_NOT_POST_MESSAGE;
        }
        ALog.e(LOG_TAG, "postMessageEventExt webMessageData:" + webMessageData);
        WebMessage webMessage = new WebMessage(webMessageData);
        try {
            port.postMessage(webMessage);
        } catch (IllegalStateException e) {
            ALog.e(LOG_TAG, "postMessageEventExt has already disenabled");
            return CAN_NOT_POST_MESSAGE;
        }
        return NO_ERROR;
    }

    @Override
    public int onWebMessagePortEvent(long id, String portHandle) {
        if (webMessagePorts.isEmpty() || portHandle == null) {
            return CAN_NOT_REGISTER_MESSAGE_EVENT;
        }
        WebMessagePort port = getWebMessagePort(portHandle);
        if (port == null) {
            return CAN_NOT_REGISTER_MESSAGE_EVENT;
        }
        try {
            port.setWebMessageCallback(new WebMessagePort.WebMessageCallback() {
                @Override
                public void onMessage(WebMessagePort port, WebMessage message) {
                    if (message == null) {
                        return;
                    }
                    // native c++
                    AceWebPluginBase.onMessage(id, portHandle, message.getData());
                }
            });
        } catch (IllegalStateException e) {
            ALog.e(LOG_TAG, "onWebMessagePortEvent has already disenabled");
            return CAN_NOT_REGISTER_MESSAGE_EVENT;
        }
        return NO_ERROR;
    }

    /**
     * Start a download task with url.
     *
     * @param id Wevbiew id.
     * @param url The url of the download task.
     */
    @Override
    public void startDownload(long id, String url) {
        String guid = AceWebDownloadHelperObject.createDownloadGuid();
        String suggestedName = URLUtil.guessFileName(url, null, null);
        AceWebDownloadItemObject object = new AceWebDownloadItemObject(id, guid, suggestedName,
            WEB_DOWNLOAD_DEPANDING, url);
        if (object == null) {
            ALog.e(LOG_TAG, "can't create webdownload data");
            return;
        }
        synchronized (WEB_LOCK) {
            webDownloadItemMap_.put(guid, object);
        }
        try {
            downloadExecutor_.execute(new WebDownloadHeadRunnable(guid));
        } catch (RejectedExecutionException e) {
            ALog.e(LOG_TAG, "startDownload failed:" + e);
            sendWebviewDownloadFailedBroadcast(guid, 0);
        }
    }

    /**
     * Change task download path.
     *
     * @param id Wevbiew id.
     * @param guid The unique identifier of the download task.
     * @param path The path of the download task.
     */
    @Override
    public void start(long id, String guid, String path) {
        AceWebDownloadItemObject object;
        synchronized (WEB_LOCK) {
            object = webDownloadItemMap_.get(guid);
            if (object == null) {
                ALog.e(LOG_TAG, "can't find webdownload data");
                return;
            }
            setDownloadPath(path, object);
            DownloadManager.Request request = new DownloadManager.Request(Uri.parse(object.getUrl()));
            request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
            request.setAllowedOverMetered(false);
            request.setVisibleInDownloadsUi(true);
            request.setAllowedOverMetered(true);
            request.setAllowedNetworkTypes(DownloadManager.Request.NETWORK_WIFI |
                DownloadManager.Request.NETWORK_MOBILE);
            File downloadFile = new File(object.getRealPath(), object.getRealName());
            Uri downloadUri = Uri.fromFile(downloadFile);
            request.setDestinationUri(downloadUri);
            Object downloadService = context.getSystemService(Context.DOWNLOAD_SERVICE);
            if (downloadService != null && downloadService instanceof DownloadManager) {
                DownloadManager downloadManager = (DownloadManager) downloadService;
                if (downloadManager != null) {
                    long downloadId = downloadManager.enqueue(request);
                    object.setDownloadId(downloadId);
                    webDownloadItemIdMap_.put(downloadId, guid);
                    startDownloadUpdateTimer();
                    return;
                }
            }
            sendWebviewDownloadFailedBroadcast(object.getGuid(), 0);
        }
    }

    /**
     * Cancel download task.
     *
     * @param id Wevbiew id.
     * @param guid The unique identifier of the download task.
     */
    @Override
    public void cancel(long id, String guid) {
        synchronized (WEB_LOCK) {
            AceWebDownloadItemObject object = webDownloadItemMap_.get(guid);
            if (object == null) {
                ALog.e(LOG_TAG, "download failed, start aceWebDownloadItemObject is null");
                return;
            }
            DownloadManager downloadManager = null;
            Object downloadService = context.getSystemService(Context.DOWNLOAD_SERVICE);
            if (downloadService != null && downloadService instanceof DownloadManager) {
                downloadManager = (DownloadManager) downloadService;
            }
            if (downloadManager != null) {
                downloadManager.remove(object.getDownloadId());
            }
            updateDownloadDataWithState(object, WEB_DOWNLOAD_CANCEL, WEB_DOWNLOAD_USER_CANCELED);
            AceWebPluginBase.onDownloadFailedObject(object.getWebId(), object);
            clearAllDownloadDataByGuid(guid, object.getDownloadId(), object.getFullPath());
        }
    }

    /**
     * Create runnable to get download head data.
     */
    class WebDownloadHeadRunnable implements Runnable {
        private final String guid_;

        public WebDownloadHeadRunnable(String guid) {
            this.guid_ = guid;
        }

        /**
         * Execute runnable to get download head data.
         */
        @Override
        public void run() {
            AceWebDownloadItemObject object;
            synchronized (WEB_LOCK) {
                object = webDownloadItemMap_.get(this.guid_);
                if (object == null) {
                    clearAllDownloadDataByGuid(this.guid_, -1, object.getFullPath());
                    ALog.e(LOG_TAG, "can't find webdownload data");
                    return;
                }
            }
            HttpURLConnection connection = null;
            try {
                Object httpConnection = new URL(object.getUrl()).openConnection();
                if (httpConnection != null && httpConnection instanceof HttpURLConnection) {
                    connection = (HttpURLConnection) httpConnection;
                }
                if (connection == null) {
                    sendWebviewDownloadBroadcastByKey(WEB_DOWNLOAD_START_EVENT, this.guid_);
                    return;
                }
                connection.setRequestMethod("HEAD");
                object.setMethod("GET");
                int responseCode = connection.getResponseCode();
                if (responseCode == HttpURLConnection.HTTP_OK) {
                    long fileSize = connection.getContentLength();
                    String mimeType = connection.getContentType();
                    object.setTotalBytes(fileSize == -1 ? 0 : fileSize);
                    object.setMimeType(mimeType == null ? "" : mimeType);
                }
                connection.disconnect();
                sendWebviewDownloadBroadcastByKey(WEB_DOWNLOAD_START_EVENT, this.guid_);
            } catch (ProtocolException e) {
                sendWebviewDownloadFailedBroadcast(object.getGuid(), 0);
                ALog.e(LOG_TAG, "connect failed," + e);
            } catch (IOException e) {
                sendWebviewDownloadFailedBroadcast(object.getGuid(), WEB_NETWORK_DISCONNECTED);
                ALog.e(LOG_TAG, "connect failed," + e);
            }
        }
    }

    private int queryDownloadState(AceWebDownloadItemObject object) {
        DownloadManager downloadManager = null;
        Object downloadService = context.getSystemService(Context.DOWNLOAD_SERVICE);
        if (downloadService != null && downloadService instanceof DownloadManager) {
            downloadManager = (DownloadManager) downloadService;
        }
        DownloadManager.Query query = new DownloadManager.Query();
        if (downloadManager == null || query == null || object == null) {
            ALog.e(LOG_TAG, "queryDownloadState failed");
            return DownloadManager.STATUS_PENDING;
        }
        query.setFilterById(object.getDownloadId());
        Cursor cursor = downloadManager.query(query);
        if (cursor.moveToFirst()) {
            try {
                object.setState(cursor.getInt(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_STATUS)));
                int errCode = cursor.getInt(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_REASON));
                object.setLastErrorCode(AceWebDownloadHelperObject.convertsHttpErrorCode(errCode));
                object.setReceivedBytes(cursor.getLong(cursor.getColumnIndex(
                    DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR)));
                long totalBytes = cursor.getLong(cursor.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES));
                if (totalBytes != -1) {
                    object.setTotalBytes(totalBytes);
                }
            } catch (IllegalArgumentException e) {
                ALog.e(LOG_TAG, "query failed," + e);
            }
        }
        return object.getState();
    }

    private void startDownloadUpdateTimer() {
        synchronized (WEB_LOCK) {
            if (webDownloadUpdateTimer_ != null) {
                return;
            }
            webDownloadUpdateTimer_ = new Timer();
        }
        TimerTask timerTask = new TimerTask() {
            @Override
            public void run() {
                synchronized (WEB_LOCK) {
                    if (webDownloadItemMap_.isEmpty()) {
                        stopDownloadUpdateTimer();
                    }
                    for (Map.Entry<String, AceWebDownloadItemObject> objectPairs : webDownloadItemMap_.entrySet()) {
                        AceWebDownloadItemObject object = objectPairs.getValue();
                        if (object == null || object.getDownloadId() == -1) {
                            continue;
                        }
                        int state = queryDownloadState(object);
                        if (state == DownloadManager.STATUS_RUNNING) {
                            updateDownloadDataWithState(object, WEB_DOWNLOAD_RUNNING, 0);
                            sendWebviewDownloadBroadcastByKey(WEB_DOWNLOAD_UPDATE_EVENT, object.getGuid());
                        } else if (state == DownloadManager.STATUS_PAUSED) {
                            if (object.getLastErrorCode() != 0) {
                                DownloadManager downloadManager = null;
                                Object downloadService = context.getSystemService(Context.DOWNLOAD_SERVICE);
                                if (downloadService != null && downloadService instanceof DownloadManager) {
                                    downloadManager = (DownloadManager) downloadService;
                                }
                                if (downloadManager != null) {
                                    downloadManager.remove(object.getDownloadId());
                                }
                                updateDownloadDataWithState(object, WEB_DOWNLOAD_INTERRUPTED,
                                    object.getLastErrorCode());
                                sendWebviewDownloadFailedBroadcast(object.getGuid(), object.getLastErrorCode());
                            }
                        } else if (state == DownloadManager.STATUS_FAILED) {
                            updateDownloadDataWithState(object, WEB_DOWNLOAD_INTERRUPTED, object.getLastErrorCode());
                            sendWebviewDownloadFailedBroadcast(object.getGuid(), 0);
                        }
                    }
                }
            }
        };
        if (webDownloadUpdateTimer_ != null && timerTask != null) {
            webDownloadUpdateTimer_.schedule(timerTask, 0, WEB_DOWNLOAD_TIMER_DELAY);
        }
    }

    private void stopDownloadUpdateTimer() {
        synchronized (WEB_LOCK) {
            if (webDownloadUpdateTimer_ != null && webDownloadItemMap_.isEmpty()) {
                webDownloadUpdateTimer_.cancel();
                webDownloadUpdateTimer_ = null;
            }
        }
    }

    private void updateDownloadDataWithState(AceWebDownloadItemObject object, int state, int errCode) {
        if (object != null) {
            object.setState(state);
            object.setLastErrorCode(errCode);
            if (state == WEB_DOWNLOAD_CANCEL || state == WEB_DOWNLOAD_INTERRUPTED) {
                object.setPercentComplete(0);
                object.setReceivedBytes(0);
                object.setFullPath("");
                return;
            }
            long lastBytes = object.getLastBytes();
            long receivedBytes = object.getReceivedBytes();
            long totalBytes = object.getTotalBytes();
            object.setCurrentSpeed((receivedBytes - lastBytes) * WEB_DOWNLOAD_SPEED_RATE > totalBytes ?
                totalBytes : (receivedBytes - lastBytes) * WEB_DOWNLOAD_SPEED_RATE);
            object.setLastBytes(receivedBytes);
            if (totalBytes > 0) {
                object.setPercentComplete(Math.round(WEB_DOWNLOAD_PERCENT * receivedBytes / totalBytes));
            }
        }
    }

    private void onWebviewDownloadFailedInMain(String guid, int err) {
        synchronized (WEB_LOCK) {
            AceWebDownloadItemObject object = webDownloadItemMap_.get(guid);
            if (object != null) {
                updateDownloadDataWithState(object, WEB_DOWNLOAD_INTERRUPTED, err);
                AceWebPluginBase.onDownloadFailedObject(object.getWebId(), object);
                clearAllDownloadDataByGuid(guid, -1, object.getFullPath());
            }
        }
    }

    private void actionWebviewDownloadStart(Intent intent) {
        String guid = intent.getStringExtra("guid");
        AceWebDownloadItemObject object;
        synchronized (WEB_LOCK) {
            object = webDownloadItemMap_.get(guid);
            if (object != null) {
                AceWebPluginBase.onBeforeDownloadObject(object.getWebId(), object);
            }
        }
    }

    private void actionWebviewDownloadUpdate(Intent intent) {
        String guid = intent.getStringExtra("guid");
        synchronized (WEB_LOCK) {
            AceWebDownloadItemObject object = webDownloadItemMap_.get(guid);
            if (object != null) {
                AceWebPluginBase.onDownloadUpdatedObject(object.getWebId(), object);
            }
        }
    }

    private void actionWebviewDownloadFailed(Intent intent) {
        String guid = intent.getStringExtra("guid");
        int errCode = intent.getIntExtra("errCode", 0);
        onWebviewDownloadFailedInMain(guid, errCode);
    }

    private void actionWebviewDownloadComplete(Intent intent) {
        Long downloadId = intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, -1);
        synchronized (WEB_LOCK) {
            String guid = webDownloadItemIdMap_.get(downloadId);
            AceWebDownloadItemObject object;
            object = webDownloadItemMap_.get(guid);
            if (object != null) {
                int state = queryDownloadState(object);
                if (state == DownloadManager.STATUS_FAILED) {
                    updateDownloadDataWithState(object, WEB_DOWNLOAD_INTERRUPTED, object.getLastErrorCode());
                    AceWebPluginBase.onDownloadFailedObject(object.getWebId(), object);
                } else {
                    updateDownloadDataWithState(object, WEB_DOWNLOAD_COMPLETED, 0);
                    AceWebPluginBase.onDownloadFinishObject(object.getWebId(), object);
                }
                clearAllDownloadDataByGuid(guid, object.getDownloadId(), object.getFullPath());
            }
        }
    }

    /**
     * Receive broadcast.
     */
    class WebviewBroadcastReceive extends BroadcastReceiver {
        /**
         * Receive broadcast.
         *
         * @param context Application context.
         * @param intent Intent object.
         */
        @Override
        public void onReceive(Context context, Intent intent) {
            String strAction = intent.getAction();
            if (WEB_DOWNLOAD_START_EVENT.equals(strAction)) {
                actionWebviewDownloadStart(intent);
            } else if (WEB_DOWNLOAD_UPDATE_EVENT.equals(strAction)) {
                actionWebviewDownloadUpdate(intent);
            } else if (WEB_DOWNLOAD_FAILED_EVENT.equals(strAction)) {
                actionWebviewDownloadFailed(intent);
            } else if (DownloadManager.ACTION_DOWNLOAD_COMPLETE.equals(strAction)) {
                actionWebviewDownloadComplete(intent);
            }
        }
    };

    private void registerWebviewReceiver(Context context) {
        if (webviewBroadcastReceive_ == null) {
            webviewBroadcastReceive_ = new WebviewBroadcastReceive();
        }

        IntentFilter itFilter = new IntentFilter();
        if (itFilter == null) {
            ALog.e(LOG_TAG, "registerWebviewReceiver itFilter is null!");
            webviewBroadcastReceive_ = null;
            return;
        }
        itFilter.addAction(DownloadManager.ACTION_DOWNLOAD_COMPLETE);
        itFilter.addAction(WEB_DOWNLOAD_UPDATE_EVENT);
        itFilter.addAction(WEB_DOWNLOAD_START_EVENT);
        itFilter.addAction(WEB_DOWNLOAD_FAILED_EVENT);
        context.registerReceiver(webviewBroadcastReceive_, itFilter);
    }

    private void sendWebviewDownloadBroadcastByKey(String eventName, String guid) {
        Intent intent = new Intent(eventName);
        if (intent != null) {
            intent.putExtra("guid", guid);
            sendWebviewDownloadBroadcast(intent);
        }
    }

    private void sendWebviewDownloadFailedBroadcast(String guid, int err) {
        Intent intent = new Intent(WEB_DOWNLOAD_FAILED_EVENT);
        if (intent != null) {
            intent.putExtra("guid", guid);
            intent.putExtra("errCode", err);
            sendWebviewDownloadBroadcast(intent);
        }
    }

    private void sendWebviewDownloadBroadcast(Intent intent) {
        if (context != null) {
            context.sendBroadcast(intent);
        }
    }

    private void setDownloadPath(String path, AceWebDownloadItemObject object) {
        String fullPath = object.getFullPath();
        if (!fullPath.equals("")) {
            return;
        }
        String finalPath = "";
        if (AceWebDownloadHelperObject.isLegalPath(path)) {
            finalPath = AceWebDownloadHelperObject.getNonDuplicatePath(path);
        } else {
            String fileName = object.getSuggestedFileName();
            File downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
            finalPath = AceWebDownloadHelperObject.getNonDuplicatePath(downloadsDir.getPath() +
                WEB_DOWNLOAD_CONCAT_DIR + fileName);
        }
        int divideIndex = finalPath.lastIndexOf('/');
        if (divideIndex != -1) {
            String realPath = finalPath.substring(0, divideIndex + 1);
            String realName = finalPath.substring(divideIndex + 1);
            object.setRealName(realName);
            object.setRealPath(realPath);
        } else {
            object.setRealPath(finalPath);
        }
        object.setFullPath(finalPath);
    }

    private void clearAllDownloadDataByGuid(String guid, long downloadId, String path) {
        synchronized (WEB_LOCK) {
            webDownloadItemMap_.remove(guid);
            webDownloadItemIdMap_.remove(downloadId);
            AceWebDownloadHelperObject.removeFilePathFromList(path);
            stopDownloadUpdateTimer();
        }
    }

    /**
     * Monitor messages sent by H5
     *
     * @param id Wevbiew id.
     * @param portHandle Message port handle.
     */
    @Override
    public int onWebMessagePortEventExt(long id, String portHandle) {
        if (webMessagePorts.isEmpty() || portHandle == null) {
            return CAN_NOT_REGISTER_MESSAGE_EVENT;
        }
        WebMessagePort port = getWebMessagePort(portHandle);
        if (port == null) {
            return CAN_NOT_REGISTER_MESSAGE_EVENT;
        }
        try {
            port.setWebMessageCallback(new WebMessagePort.WebMessageCallback() {
                @Override
                public void onMessage(WebMessagePort port, WebMessage message) {
                    if (message == null) {
                        return;
                    }
                    // native c++
                    AceWebPluginBase.onMessageEventExt(id, portHandle, message.getData());
                }
            });
        } catch (IllegalStateException e) {
            ALog.e(LOG_TAG, "onWebMessagePortEventExt has already disenabled");
            return CAN_NOT_REGISTER_MESSAGE_EVENT;
        }
        return NO_ERROR;
    }

    public String scrollTo(Map<String, String> params) {
        if (!params.containsKey(NTC_PARAM_SCROLLTO_X) || !params.containsKey(NTC_PARAM_SCROLLTO_Y) || webView == null) {
            return FAIL_TAG;
        }
        int scrollX = 0, scrollY = 0;
        try {
            scrollX = Integer.parseInt(params.get(NTC_PARAM_SCROLLTO_X));
            scrollY = Integer.parseInt(params.get(NTC_PARAM_SCROLLTO_Y));
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "scrollTo NumberFormatException");
            return FAIL_TAG;
        }
        webView.scrollTo(scrollX, scrollY);
        return SUCCESS_TAG;
    }

    @Override
    public String scrollBy(Map<String, String> params) {
        if (!params.containsKey(NTC_PARAM_SCROLLBY_DELTAX) || !params.containsKey(NTC_PARAM_SCROLLBY_DELTAY)
            || webView == null) {
            return FAIL_TAG;
        }
        int deltaX = 0, deltaY = 0;
        try {
            deltaX = Integer.parseInt(params.get(NTC_PARAM_SCROLLBY_DELTAX));
            deltaY = Integer.parseInt(params.get(NTC_PARAM_SCROLLBY_DELTAY));
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "scrollBy NumberFormatException");
            return FAIL_TAG;
        }

        int curentScrollX = webView.getScrollX();
        int curentScrollY = webView.getScrollY();

        int offsetX = curentScrollX + deltaX;
        int offsetY = curentScrollX + deltaY;

        webView.scrollBy(offsetX < 0 ? -curentScrollX : deltaX, offsetY < 0 ? -curentScrollY : deltaY);
        return SUCCESS_TAG;
    }

    @Override
    public String zoom(Map<String, String> params) {
        if (params == null || webView == null) {
            return FAIL_TAG;
        }
        if (!params.containsKey(NTC_ZOOM_FACTOR)) {
            return FAIL_TAG;
        }
        String zoomFactor = params.get(NTC_ZOOM_FACTOR);
        if (webView != null) {
            this.webView.zoomBy(Float.parseFloat(zoomFactor));
            return SUCCESS_TAG;
        }
        return FAIL_TAG;
    }

    @Override
    public String zoomIn() {
        if (webView == null) {
            return FAIL_TAG;
        }
        if (webView != null) {
            this.webView.zoomBy(WEBVIEW_ZOOMIN_VALUE);
            return SUCCESS_TAG;
        }
        return FAIL_TAG;
    }

    @Override
    public String zoomOut() {
        if (webView == null) {
            return FAIL_TAG;
        }
        if (webView != null) {
            this.webView.zoomBy(WEBVIEW_ZOOMOUT_VALUE);
            return SUCCESS_TAG;
        }
        return FAIL_TAG;
    }

    @Override
    public void pageUp(boolean top) {
        if (this.webView == null) {
            return;
        }
        int webViewHeight = webView.getHeight();
        int halfScreenHeight = webViewHeight / WEBVIEW_PAGE_HALF;
        int currentOffsetY = this.webView.getScrollY();
        int finalY = top ? WEBVIEW_PAGE_TOP : Math.max(0, currentOffsetY - halfScreenHeight);
        ValueAnimator animator = ValueAnimator.ofInt(currentOffsetY, finalY);
        animator.setDuration(WEBVIEW_DURATION);
        animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                int value = (int) animation.getAnimatedValue();
                webView.scrollTo(0, value);
            }
        });
        animator.start();
    }

    @Override
    public String getOriginalUrl() {
        if (this.webView == null) {
            return "";
        }
        return this.webView.getOriginalUrl();
    }

    public boolean getZoomAccess() {
        return isZoomAccess;
    }

    @Override
    public String clearHistory(Map<String, String> params) {
        if (webView != null) {
            this.webView.clearHistory();
            return SUCCESS_TAG;
        }
        return FAIL_TAG;
    }

    @Override
    public void setUserAgentString(String userAgent) {
        final WebSettings webSettings = webView.getSettings();
        webSettings.setUserAgentString(userAgent);
    }

    @Override
    public String getUserAgentString() {
        final WebSettings webSettings = webView.getSettings();
        return webSettings.getUserAgentString();
    }

    /**
     * setWebDebuggingAccess.
     *
     * @param params is webDebuggingAccess boolean.
     */
    public void setWebDebuggingAccess(boolean webDebuggingAccess) {
        WebView.setWebContentsDebuggingEnabled(webDebuggingAccess);
    }

    @Override
    public String pageDown(boolean bottom) {
        if (this.webView != null) {
            int webViewHeight = webView.getHeight();
            int ContentHeight = (int) (webView.getContentHeight() * webView.getScale() - webViewHeight);
            int halfScreenHeight = webViewHeight / WEBVIEW_PAGE_HALF;
            int currentOffsetY = webView.getScrollY();
            int currentOffsetX = webView.getScrollX();
            int finalY = bottom ? ContentHeight : Math.min(ContentHeight, currentOffsetY + halfScreenHeight);
            ValueAnimator animator = ValueAnimator.ofInt(currentOffsetY, finalY);
            animator.setDuration(WEBVIEW_DURATION);
            animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
                @Override
                public void onAnimationUpdate(ValueAnimator animation) {
                    int value = (int) animation.getAnimatedValue();
                    webView.scrollTo(currentOffsetX, value);
                }
            });
            animator.start();
            return SUCCESS_TAG;
        }
        return FAIL_TAG;
    }

    @Override
    public String postUrl(String url, byte[] postData) {
        if (webView != null) {
            this.webView.postUrl(url, postData);
            return SUCCESS_TAG;
        }
        return FAIL_TAG;
    }
}
