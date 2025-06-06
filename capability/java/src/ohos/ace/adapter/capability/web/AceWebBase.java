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

import android.view.MotionEvent;
import android.webkit.WebBackForwardList;
import java.util.HashMap;
import java.util.Map;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.IAceOnCallResourceMethod;
import ohos.ace.adapter.IAceOnResourceEvent;

/**
 * This class handles the lifecycle of a web.
 *
 * @since 1
 */
public abstract class AceWebBase {
    private static final String LOG_TAG = "AceWebBase";

    /** Should be the same with corresponding var
     * in core/components/web/resource/web_resource.cpp
     */
    private static final String PARAM_NONE = "";

    private static final String PARAM_AND = "#HWJS-&-#";

    private static final String PARAM_VALUE = "value";

    private static final String PARAM_EQUALS = "#HWJS-=-#";

    private static final String PARAM_BEGIN = "#HWJS-?-#";

    private static final String METHOD = "method";

    private static final String EVENT = "event";

    private static final String RESULT_FAIL = "fail";

    private static final String WEB_FLAG = "web@";

    private final long id;

    private final IAceOnResourceEvent callback;

    private Map<String, IAceOnCallResourceMethod> callMethodMap;

    public AceWebBase(long id, IAceOnResourceEvent callback) {
        this.id = id;
        this.callback = callback;
        callMethodMap = new HashMap<String, IAceOnCallResourceMethod>();
        this.callMethodMapPutXWardMethod();
        this.callMethodMapPutLoadMethod();
        this.callMethodMapPutXactive();
        this.callMethodMapPutPageChange();
        this.callMethodMapWebPlugin();
        this.callMethodMapWebAtrribute();
    }

    private void callMethodMapWebAtrribute() {
        IAceOnCallResourceMethod zoomAccess = (param) -> zoomAccess(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "zoomAccess" + PARAM_BEGIN, zoomAccess);
        IAceOnCallResourceMethod javascriptAccess = (param) -> javascriptAccess(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "javascriptAccess" + PARAM_BEGIN,
            javascriptAccess);
        IAceOnCallResourceMethod minFontSize = (param) -> minFontSize(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "minFontSize" + PARAM_BEGIN, minFontSize);
        IAceOnCallResourceMethod horizontalScrollBarAccess = (param) -> horizontalScrollBarAccess(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "horizontalScrollBarAccess" + PARAM_BEGIN,
            horizontalScrollBarAccess);
        IAceOnCallResourceMethod verticalScrollBarAccess = (param) -> verticalScrollBarAccess(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "verticalScrollBarAccess" + PARAM_BEGIN,
            verticalScrollBarAccess);
        IAceOnCallResourceMethod backgroundColor = (param) -> backgroundColor(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "backgroundColor" + PARAM_BEGIN,
            backgroundColor);
        IAceOnCallResourceMethod mediaPlayGestureAccess = (param) -> mediaPlayGestureAccess(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "mediaPlayGestureAccess" + PARAM_BEGIN,
            mediaPlayGestureAccess);
        IAceOnCallResourceMethod onlineImageAccess = (param) -> onlineImageAccess(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "onlineImageAccess" + PARAM_BEGIN,
            onlineImageAccess);
        IAceOnCallResourceMethod geolocationAccess = (param) -> geolocationAccess(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "geolocationAccess" + PARAM_BEGIN,
            geolocationAccess);
        IAceOnCallResourceMethod blockNetwork = (param) -> blockNetwork(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "blockNetwork" + PARAM_BEGIN, blockNetwork);
        IAceOnCallResourceMethod mixedMode = (param) -> mixedMode(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "mixedMode" + PARAM_BEGIN, mixedMode);
        IAceOnCallResourceMethod domStorageAccess = (param) -> domStorageAccess(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "domStorageAccess" + PARAM_BEGIN,
            domStorageAccess);
        IAceOnCallResourceMethod cacheMode = (param) -> cacheMode(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "cacheMode" + PARAM_BEGIN, cacheMode);
        IAceOnCallResourceMethod imageAccess = (param) -> imageAccess(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD + PARAM_EQUALS + "imageAccess" + PARAM_BEGIN, imageAccess);
    }

    private void callMethodMapPutXWardMethod() {
        IAceOnCallResourceMethod callBackward = (param) -> backward(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "backward" +
            PARAM_BEGIN,
            callBackward);
        IAceOnCallResourceMethod callForward = (param) -> forward(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "forward" +
            PARAM_BEGIN,
            callForward);
        IAceOnCallResourceMethod callAccessBackward = (param) -> accessBackward(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "accessBackward" +
            PARAM_BEGIN,
            callAccessBackward);
        IAceOnCallResourceMethod callAccessForward = (param) -> accessForward(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "accessForward" +
            PARAM_BEGIN,
            callAccessForward);
    }

    private void callMethodMapPutLoadMethod() {
        IAceOnCallResourceMethod callUpdateWebLayout = (param) -> updateWebLayout(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "layout" +
            PARAM_BEGIN,
            callUpdateWebLayout);
        IAceOnCallResourceMethod callReload = (param) -> reload(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "reload" +
            PARAM_BEGIN,
            callReload);
        IAceOnCallResourceMethod callLoadData = (param) -> loadData(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "loadData" +
            PARAM_BEGIN,
            callLoadData);
        IAceOnCallResourceMethod callAcessStep = (param) -> accessStep(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "accessStep" +
            PARAM_BEGIN,
            callAcessStep);
        IAceOnCallResourceMethod callRefresh = (param) -> reload(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "refresh" +
            PARAM_BEGIN,
            callRefresh);
        IAceOnCallResourceMethod callstopLoading = (param) -> stopLoading(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "stop" +
            PARAM_BEGIN,
            callstopLoading);
        IAceOnCallResourceMethod callZoom = (param) -> zoom(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "zoom" +
            PARAM_BEGIN,
            callZoom);
    }

    private void callMethodMapPutXactive() {
        IAceOnCallResourceMethod callOnActive = (param) -> onActive(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "onActive" +
            PARAM_BEGIN,
            callOnActive);
        IAceOnCallResourceMethod callOnInactive = (param) -> onInactive(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "onInactive" +
            PARAM_BEGIN,
            callOnInactive);
    }

    private void callMethodMapPutPageChange() {
        IAceOnCallResourceMethod callGetHitTest = (param) -> getHitTest(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "getHitTest" +
            PARAM_BEGIN,
            callGetHitTest);
        IAceOnCallResourceMethod callClearHistory = (param) -> clearHistory(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "clearHistory" +
            PARAM_BEGIN,
            callClearHistory);
        IAceOnCallResourceMethod callRequestFocus = (param) -> requestFocus(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "requestFocus" +
            PARAM_BEGIN,
            callRequestFocus);
        IAceOnCallResourceMethod callUpdateUrl = (param) -> updateUrl(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "updateUrl" +
            PARAM_BEGIN,
            callUpdateUrl);
        IAceOnCallResourceMethod callRouterBack = (param) -> routerBack(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "routerBack" +
            PARAM_BEGIN,
            callRouterBack);
        IAceOnCallResourceMethod callChangePageUrl = (param) -> changePageUrl(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "changePageUrl" +
            PARAM_BEGIN,
            callChangePageUrl);
        IAceOnCallResourceMethod callPagePathInvalid = (param) -> pagePathInvalid(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "pagePathInvalid" +
            PARAM_BEGIN,
            callPagePathInvalid);
    }

    private void callMethodMapWebPlugin() {
        IAceOnCallResourceMethod callTouchDown = (param) -> touchDown(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "touchDown" +
            PARAM_BEGIN,
            callTouchDown);
        IAceOnCallResourceMethod callTouchUp = (param) -> touchUp(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "touchUp" +
            PARAM_BEGIN,
            callTouchUp);
        IAceOnCallResourceMethod callTouchMove = (param) -> touchMove(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "touchMove" +
            PARAM_BEGIN,
            callTouchMove);
        IAceOnCallResourceMethod callTouchCancel = (param) -> touchCancel(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "touchCancel" +
            PARAM_BEGIN,
            callTouchCancel);
        IAceOnCallResourceMethod callUpdateLayout = (param) -> updateLayout(param);
        this.callMethodMap.put(WEB_FLAG + id + METHOD +
            PARAM_EQUALS +
            "updateLayout" +
            PARAM_BEGIN,
            callUpdateLayout);
    }

    /**
     * This is called to release.
     *
     */
    public abstract void release();

    /**
     * This is called to get resource call methods.
     *
     * @return Map of call method
     */
    public Map<String, IAceOnCallResourceMethod> getCallMethod() {
        return callMethodMap;
    }

    /**
     * This is called to get id.
     *
     * @return resource id.
     */
    public long getId() {
        return id;
    }

    /**
     * This is called to set touch event.
     *
     * @param event is touch event.
     */
    public abstract void setTouchEvent(MotionEvent event);

    /**
     * This is called to update web layout.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String updateWebLayout(Map<String, String> params);

    /**
     * This is called to reload the url.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String reload(Map<String, String> params);

    /**
     * This is called to update the url.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String updateUrl(Map<String, String> params);

    /**
     * This is called to notity the webview that page url is changed.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String changePageUrl(Map<String, String> params);

    /**
     * This is called to notity the webview whether the page path is invalid.
     * The webview is cleared from the view tree only when the page path is valid.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String pagePathInvalid(Map<String, String> params);

    /**
     * This is called to routerBack webview.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String routerBack(Map<String, String> params);

    /**
     * This is called to fire on page started event.
     *
     * @param url the current url of this page.
     */
    public void firePageStarted(String url) {
        callback.onEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS + "onPageStarted" + PARAM_BEGIN, url);
    }

    private String makeEventHash(String eventName) {
        return WEB_FLAG + id + EVENT + PARAM_EQUALS + eventName + PARAM_BEGIN;
    }

    /**
     * This is called to fire on page download event.
     *
     * @param obj the object of this page download event.
     */
    public void fireDownloadStart(Object obj) {
        this.nativeOnObjectEvent(makeEventHash("onDownloadStart"), "onDownloadStart", obj);
    }

    /**
     * This is called to fire invoke method.
     *
     * @param obj the object of this invoke event.
     */
    public void fireJSInvokeMethod(Object obj) {
        this.nativeOnObjectEvent(makeEventHash("onJSInvokeMethod"), "onJSInvokeMethod", obj);
    }

    /**
     * This is called to run js .
     *
     * @param value the value of js.
     */
    public void fireRunJSRecvValue(String value) {
        callback.onEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS + "onRunJSRecvValue" + PARAM_BEGIN, value);
    }

    /**
     * This is called to refresh history.
     *
     * @param obj the jni object.
     */
    public void fireRefreshHistory(Object obj) {
        this.nativeOnObjectEvent(makeEventHash("onRefreshAccessedHistory"), "onRefreshAccessedHistory", obj);
    }

    /**
     * This is called to Full ScreenEnter.
     *
     * @param obj the jni object.
     */
    public void fireFullScreenEnter(Object obj) {
        this.nativeOnObjectEvent(makeEventHash("onFullScreenEnter"), "onFullScreenEnter", obj);
    }

    /**
     * This is called to rnative Full ScreenExit.
     *
     * @param obj the jni object.
     */
    public void fireFullScreenExit(Object obj) {
        this.nativeOnObjectEvent(makeEventHash("onFullScreenExit"), "onFullScreenExit", obj);
    }

    /**
     * This is call to intercept load url.
     *
     * @param obj the jni object.
     */
    public boolean fireUrlLoadIntercept(Object obj) {
        return this.nativeOnObjectEventWithBoolReturn(makeEventHash("onLoadIntercept"), "onLoadIntercept", obj);
    }

    /**
     * This is call to intercept request.
     *
     * @param obj the jni object.
     * @return Object is WebResourceResponse.
     */
    public Object fireShouldInterceptRequest(Object obj) {
        return this.nativeOnObjectEventWithObjectReturn(
            makeEventHash("onInterceptRequest"), "onInterceptRequest", obj);
    }

    /**
     * This is call to exit the render.
     *
     * @param reasonCode the reason code.
     */
    public void fireRenderExited(int reasonCode) {
        String param = String.valueOf(reasonCode);
        callback.onEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS + "onRenderExited" + PARAM_BEGIN, param);
    }

    /**
     * This is called to fire error event.
     *
     * @param url the current url of this page.
     * @param errorCode the specific code id for this error.
     * @param description the detail description of this error.
     */
    public void fireError(String url, int errorCode, String description) {
        String param = "url=" + url + "&errorCode=" + errorCode + "&description=" + description;
        callback.onEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS + "onPageError" + PARAM_BEGIN, param);
    }

    /**
     * This is called to fire on page visible event.
     *
     * @param url The URL corresponding to the page navigation that triggered this callback.
     */
    public void firePageVisible(String url) {
        callback.onEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS + "onPageVisible" + PARAM_BEGIN, url);
    }

    /**
     * This is called to fire on page finished event.
     *
     * @param url the current url of this page.
     */
    public void firePageFinished(String url) {
        callback.onEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS + "onPageFinished" + PARAM_BEGIN, url);
    }

    /**
     * This is called to fire on page change event.
     *
     * @param newProgress the progress of this page.
     */
    public void firePageChanged(int newProgress) {
        String param = String.valueOf(newProgress);
        callback.onEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS + "onProgressChanged" + PARAM_BEGIN, param);
    }

    /**
     * This is called to fire on page scroll event.
     *
     * @param obj the jni object of this event.
     */
    public void fireScrollChanged(Object obj) {
        this.nativeOnObjectEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onScroll" + PARAM_BEGIN, "onScroll", obj);
    }

    /**
     * This is called to fire on page scale event.
     *
     * @param obj the jni object of this event.
     */
    public void fireScaleChanged(Object obj) {
        this.nativeOnObjectEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onScaleChange" + PARAM_BEGIN, "onScaleChange", obj);
    }

    /**
     * This is called to fire on page title event.
     *
     * @param title the title of this page.
     */
    public void firePageRecvTitle(String title) {
        callback.onEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS + "onReceivedTitle" + PARAM_BEGIN, title);
    }

    /**
     * This is called to hide the permission of geolaction.
     *
     */
    public void firePageGeoHidePermission() {
        callback.onEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS + "onGeoHidePermission" + PARAM_BEGIN, "origin");
    }

    /**
     * This is called to apply the permission of geolaction.
     *
     * @param origin the origin of this event.
     * @param obj the jni object of this event.
     */
    public void firePageGeoPermission(Object obj) {
        this.nativeOnObjectEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onGeoPermission" + PARAM_BEGIN, "onGeoPermission", obj);
    }

    /**
     * This is called to common dialog event.
     *
     * @param dialogType the dialogType of this event.
     * @param obj the jni object of this event.
     */
    public void firePageOnCommonDialog(String dialogType, Object obj) {
        this.nativeOnObjectEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onCommonDialog" + PARAM_BEGIN, dialogType, obj);
    }

    /**
     * This is called to console message event.
     *
     * @param obj the jni object of this event.
     */
    public boolean firePageOnConsoleMessage(Object obj) {
        return this.nativeOnObjectEventWithBoolReturn(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onConsoleMessage" + PARAM_BEGIN, "onConsoleMessage", obj);
    }

    /**
     * This is called to file choose event.
     *
     * @param obj the jni object of this event.
     * @return true if the event is handled, false otherwise.
     */
    public boolean firePageOnShowFileChooser(Object obj) {
        return this.nativeOnObjectEventWithBoolReturn(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onShowFileChooser" + PARAM_BEGIN, "onShowFileChooser", obj);
    }

    /**
     * This is called to error receive event.
     *
     * @param obj the jni object of this event.
     */
    public void fireErrorReceive(Object obj) {
        ALog.i(LOG_TAG, "jcz OnErrorReceive");
        this.nativeOnObjectEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onErrorReceive" + PARAM_BEGIN, "onErrorReceive", obj);
    }

    /**
     * This is called to http error receive event.
     *
     * @param obj the jni object of this event.
     */
    public void fireHttpErrorReceive(Object obj) {
        this.nativeOnObjectEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onHttpErrorReceive" + PARAM_BEGIN, "onHttpErrorReceive", obj);
    }

    /**
     * This is called to http auth request receive event.
     *
     * @param obj the jni object of this event.
     */
    public void fireHttpAuthRequestReceive(Object obj) {
        this.nativeOnObjectEventWithBoolReturn(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onHttpAuthRequest" + PARAM_BEGIN, "onHttpAuthRequest", obj);
    }

    /**
     * This is called to permission request receive event.
     *
     * @param obj the jni object of this event.
     */
    public void firePermissionRequest(Object obj) {
        this.nativeOnObjectEventWithBoolReturn(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onPermissionRequest" + PARAM_BEGIN, "onPermissionRequest", obj);
    }

    /**
     * This is called to fire prompt event.
     *
     * @param obj the jni object of this event.
     * @return true if the event is handled, false otherwise.
     */
    public boolean fireJsPrompt(Object obj) {
        return this.nativeOnObjectEventWithBoolReturn(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onPrompt" + PARAM_BEGIN, "onPrompt", obj);
    }

    /**
     * This is called to fire alert event.
     *
     * @param obj the jni object of this event.
     * @return true if the event is handled, false otherwise.
     */
    public boolean fireJsAlert(Object obj) {
        return this.nativeOnObjectEventWithBoolReturn(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onAlert" + PARAM_BEGIN, "onAlert", obj);
    }

    /**
     * This is called to fire confirm event.
     *
     * @param obj the jni object of this event.
     * @return true if the event is handled, false otherwise.
     */
    public boolean fireJsConfirm(Object obj) {
        return this.nativeOnObjectEventWithBoolReturn(WEB_FLAG + id + EVENT + PARAM_EQUALS +
            "onConfirm" + PARAM_BEGIN, "onConfirm", obj);
    }

    /**
     * This is called to fire router event.
     *
     * @param url the specific url of the page.
     */
    public void fireRouterPush(String url) {
        callback.onEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS + "routerPush" + PARAM_BEGIN, url);
    }

    /**
     * This is called to fire beforeUnload event.
     *
     * @param obj the jni object of this event.
     * @return true if the event is handled, false otherwise.
     */
    public boolean fireJsBeforeUnload(Object obj) {
        return this.nativeOnObjectEventWithBoolReturn(makeEventHash("onBeforeUnload"), "onBeforeUnload", obj);
    }

    /**
     * This is called to fire post message event.
     *
     * @param message the specific message from web page.
     */
    public void firePostMessage(String message) {
        callback.onEvent(WEB_FLAG + id + EVENT + PARAM_EQUALS + "onMessage" + PARAM_BEGIN, message);
    }

    /**
     * This is called for activity resume.
     *
     */
    public abstract void onActivityResume();

    /**
     * This is called for activity pause.
     *
     */
    public abstract void onActivityPause();

    /**
     * Obtains the element type of the currently clicked region.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String getHitTest(Map<String, String> params);

    /**
     * This is called to clear the history of visited url(backward and forward).
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String clearHistory(Map<String, String> params);

    /**
     *  This is called for focus the webview.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String requestFocus(Map<String, String> params);

    /**
     * This is called to Let the Web active.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String onActive(Map<String, String> params);

    /**
     * This is called to Let the Web inactive.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String onInactive(Map<String, String> params);

    /**
     * This is called to stop loading the page.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String stopLoading(Map<String, String> params);

    /**
     * This is called to go back to the previous page.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String backward(Map<String, String> params);

    /**
     * This is called to go forward to the next page.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String forward(Map<String, String> params);

    /**
     * This is called to check whether we can go back to the previous url.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String accessBackward(Map<String, String> params);

    /**
     * This is called to check whether we can go forward to the next url.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String accessForward(Map<String, String> params);

    /**
     * This is called to load the given url.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String loadData(Map<String, String> params);

    /**
     * Given the steps, which is in the params, to check whether we can go forward or backward.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String accessStep(Map<String, String> params);

    /**
     * Performs a zoom operation in this WebView.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String zoom(Map<String, String> params);

    /**
     * Performs a zoomIn operation in this WebView.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String zoomIn();

    /**
     * Performs a zoomOut operation in this WebView.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String zoomOut();

    /**
     * Performs a getOriginalUrl operation in this WebView.
     *
     * @return result of call.
     */
    public abstract String getOriginalUrl();

    /**
     * Performs a pageUp operation in this WebView.
     *
     * @param top is boolean.
     */
    public abstract void pageUp(boolean top);

    /**
     * touch down.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String touchDown(Map<String, String> params);

    /**
     * touch up.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String touchUp(Map<String, String> params);

    /**
     * touch move.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String touchMove(Map<String, String> params);

    /**
     * touch cancel.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String touchCancel(Map<String, String> params);

    /**
     * update layout.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String updateLayout(Map<String, String> params);

    /**
     * zoomAccess.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String zoomAccess(Map<String, String> params);

    /**
     * onlineImageAccess
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String onlineImageAccess(Map<String, String> params);

    /**
     * geolocationAccess
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String geolocationAccess(Map<String, String> params);

    /**
     * blockNetwork
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String blockNetwork(Map<String, String> params);

    /**
     * mixedMode
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String mixedMode(Map<String, String> params);

    /**
     * domStorageAccess
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String domStorageAccess(Map<String, String> params);

    /**
     * cacheMode
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String cacheMode(Map<String, String> params);

    /**
     * imageAccess
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String imageAccess(Map<String, String> params);

    /**
     * javascriptAccess
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String javascriptAccess(Map<String, String> params);

    /**
     * minFontSize.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String minFontSize(Map<String, String> params);

    /**
     * horizontalScrollBarAccess.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String horizontalScrollBarAccess(Map<String, String> params);

    /**
     * verticalScrollBarAccess.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String verticalScrollBarAccess(Map<String, String> params);

    /**
     * backgroundColor.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String backgroundColor(Map<String, String> params);

    /**
     * mediaPlayGestureAccess.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String mediaPlayGestureAccess(Map<String, String> params);

    /**
     * loadUrl with header.
     *
     * @param url is url.
     * @param header is header.
     */
    public abstract void loadUrl(String url, Map<String, String> header);

    /**
     * Load the given URL.
     *
     * @return The result of the load operation.
     */
    public abstract String getUrl();

    /**
     * Evaluates the given JavaScript code in the context of the current page.
     *
     * @param script The JavaScript code to be evaluated.
     * @param asyncCallbackInfoId The ID of the asynchronous callback information.
     */
    public abstract void evaluateJavascript(String script, long asyncCallbackInfoId);

    /**
     * Evaluates the given JavaScript code in the context of the current page.
     *
     * @param script The JavaScript code to be evaluated.
     * @param asyncCallbackInfoId The ID of the asynchronous callback information.
     */
    public abstract void evaluateJavascriptExt(String script, long asyncCallbackInfoId);

    /**
     * Load the given URL.
     *
     * @param url The URL to load.
     */
    public abstract WebBackForwardList getBackForwardEntries();

    /**
     * Clear the cache.
     *
     * @param includeDiskFiles Whether to include disk files.
     */
    public abstract void clearCache(boolean includeDiskFiles);

    /**
     * Go back or forward in the history stack.
     *
     * @param steps The number of steps to go back or forward.
     */
    public abstract void goBackOrForward(int steps);

    /**
     * Get the title of the current page.
     *
     * @return The title of the current page.
     */
    public abstract String getTitle();

    /**
     * Get the content height of the current page.
     *
     * @return The content height of the current page.
     */
    public abstract int getContentHeight();

    /**
     * Scroll the page to the given position.
     *
     * @param params The position to scroll to.
     * @return The result of the scroll operation.
     */
    public abstract String scrollTo(Map<String, String> params);

    /**
     * Scroll the page by the given offset.
     *
     * @param params The offset to scroll by.
     * @return The result of the scroll operation.
     */
    public abstract String scrollBy(Map<String, String> params);

    /**
     * Set the user agent string.
     *
     * @param userAgent The user agent string to set.
     */
    public abstract void setUserAgentString(String userAgent);

    /**
     * Get the user agent string.
     *
     * @return The user agent string.
     */
    public abstract String getUserAgentString();

    private native void nativeOnObjectEvent(String id, String param, Object object);

    private native boolean nativeOnObjectEventWithBoolReturn(String id, String param, Object object);

    private native Object nativeOnObjectEventWithObjectReturn(String id, String param, Object object);

    /**
     * createWebMessagePorts.
     *
     * @return result of call.
     */
    public abstract String[] createWebMessagePorts();

    /**
     * postWebMessage.
     *
     * @param message is param String.
     * @param ports is param String[].
     * @param Uri is param String.
     * @return result of call.
     */
    public abstract void postWebMessage(String message, String[] ports, String Uri);

    /**
     * closeWebMessagePort.
     *
     * @param portHandle is param String.
     */
    public abstract void closeWebMessagePort(String portHandle);

    /**
     * postMessageEvent.
     *
     * @param portHandle is param String.
     * @param webMessageData is param String.
     * @return result of call.
     */
    public abstract int postMessageEvent(String portHandle, String webMessageData);

    /**
     * Send message to HTML5.
     *
     * @param portHandle Message port handle.
     * @param webMessageData The webMessageData is a message sent to H5.
     */
    public abstract int postMessageEventExt(String portHandle, String webMessageData);

    /**
     * onWebMessagePortEvent.
     *
     * @param id is param long.
     * @param portHandle is param String.
     * @return result of call.
     */
    public abstract int onWebMessagePortEvent(long id, String portHandle);

    /**
     * setWebDebuggingAccess.
     *
     * @param webDebuggingAccess is param boolean.
     * @return result of call.
     */
    public abstract void setWebDebuggingAccess(boolean webDebuggingAccess);

    /**
     * pageDown.
     *
     * @param bottom is param boolean.
     * @return result of call.
     */
    public abstract String pageDown(boolean bottom);

    /**
     * postUrl.
     *
     * @param url is param String.
     * @param postData is param byte[].
     * @return result of call.
     */
    public abstract String postUrl(String url, byte[] postData);

    /**
     * Start a download task with url.
     *
     * @param id Wevbiew id.
     * @param url The url of the download task.
     */
    public abstract void startDownload(long id, String url);

    /**
     * Change task download path.
     *
     * @param id Wevbiew id.
     * @param guid The unique identifier of the download task.
     * @param path The path of the download task.
     */
    public abstract void start(long id, String guid, String path);

    /**
     * Cancel download task.
     *
     * @param id Wevbiew id.
     * @param guid The unique identifier of the download task.
     */
    public abstract void cancel(long id, String guid);

    /**
     * Monitor messages sent by H5
     *
     * @param id Wevbiew id.
     * @param portHandle Message port handle.
     * @return result of call.
     */
    public abstract int onWebMessagePortEventExt(long id, String portHandle);

    /**
     * Registers a JavaScript proxy object with the specified methods.
     *
     * @param objectName The name of the JavaScript object.
     * @param methodList An array of method names to be exposed in the JavaScript object.
     * @param asyncMethodList An array of asynchronous method names to be exposed in the JavaScript object.
     * @param permission Configure JSBridge's permission control through this string.
     */
    public abstract void registerJavaScriptProxy(
        String objectName, String[] methodList, String[] asyncMethodList, String permission);

    /**
     * Deletes the JavaScript register with the specified object name.
     *
     * @param objectName The name of the JavaScript object to be deleted.
     */
    public abstract void deleteJavaScriptRegister(String objectName);
}
