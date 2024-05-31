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

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.net.Uri;
import android.view.Display;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.webkit.ConsoleMessage;
import android.webkit.CookieManager;
import android.webkit.DownloadListener;
import android.webkit.GeolocationPermissions;
import android.webkit.HttpAuthHandler;
import android.webkit.JsResult;
import android.webkit.JsPromptResult;
import android.webkit.PermissionRequest;
import android.webkit.ValueCallback;
import android.webkit.WebBackForwardList;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.FrameLayout;
import android.view.MotionEvent;
import android.webkit.WebMessage;
import android.webkit.WebMessagePort;

import java.util.List;
import java.util.ArrayList;

import ohos.ace.adapter.ALog;
import ohos.ace.adapter.IAceOnCallResourceMethod;
import ohos.ace.adapter.IAceOnResourceEvent;
import ohos.ace.adapter.capability.web.AceWebErrorReceiveObject;
import ohos.ace.adapter.capability.web.AceWebHttpErrorReceiveObject;
import ohos.ace.adapter.capability.web.AceWebScrollObject;
import ohos.ace.adapter.capability.web.AceWebConsoleMessageObject;
import ohos.ace.adapter.capability.web.AceWebOverrideUrlObject;

import java.util.Map;

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
    private static final String NTC_JAVASCRIPT_ACCESS = "javascriptAccess";
    private static final String NTC_MIN_FONT_SIZE = "minFontSize";
    private static final String NTC_HORIZONTAL_SCROLLBAR_ACCESS = "horizontalScrollBarAccess";
    private static final String NTC_VERTICAL_SCROLLBAR_ACCESS = "verticalScrollBarAccess";
    private static final String NTC_BACKGROUND_COLOR = "backgroundColor";
    private static final String NTC_MEDIA_PLAY_GESTURE_ACCESS = "mediaPlayGestureAccess";
    private static final String WEB_MESSAGE_PORT_ONE = "port1";
    private static final String WEB_MESSAGE_PORT_TWO = "port2";

    private static final int NO_ERROR = 0;

    private static final int CAN_NOT_POST_MESSAGE = 17100010;

    private static final int CAN_NOT_REGISTER_MESSAGE_EVENT = 17100006;

    private static String currentPageUrl;

    private static String routerUrl;

    private float width;

    private float height;

    private float left;

    private float top;

    private final IAceOnResourceEvent callback;

    private final Context context;

    private final AceWebView webView;

    private boolean isWebOnPage = true;

    private MotionEvent motionEvent;

    private List<WebMessagePort> webMessagePorts = new ArrayList<WebMessagePort>();

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

    public AceWeb(long id, Context context, IAceOnResourceEvent callback) {
        super(id, callback);
        this.callback = callback;
        this.context = context;
        webView = new AceWebView(context);
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
        View contentView = activity.getWindow().getDecorView().findViewById(android.R.id.content);
        android.widget.FrameLayout contentView1;
        if (contentView instanceof FrameLayout) {
            contentView1 = (FrameLayout) contentView;
            contentView1.addView(webView, 0, params);

            int countNew = contentView1.getChildCount();
            for (int index = 0; index < countNew; index++) {
                View view = contentView1.getChildAt(index);
                String viewClassName = view.getClass().getName();
                if (viewClassName.equals("ohos.ace.adapter.WindowView")) {
                    view.setZ(100.f);
                } else {
                    view.setZ(-1.f);
                }
            }
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
        webSettings.setAllowContentAccess(false);

        webSettings.setAllowFileAccessFromFileURLs(true);
        WebView.setWebContentsDebuggingEnabled(false);
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
        webView.onPause();
    }

    @Override
    public void onActivityResume() {
        webView.onResume();
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
        webView.getSettings().setSupportZoom(access);
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
}
