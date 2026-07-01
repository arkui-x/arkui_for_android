/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.webkit.WebView;
import ohos.ace.adapter.ALog;
import java.util.Map;

/**
 * Object wrapper for onWindowNew event from Android WebChromeClient.onCreateWindow.
 *
 * Uses Android native WebView.WebViewTransport mechanism to bind the child WebView
 * to the parent's window.open() request, analogous to OH's SetNWebHandlerById which
 * lets the NWeb kernel handle navigation internally.
 *
 * Flow (matching OH two-phase pattern):
 *   Phase 1: onCreateWindow → store transport + resultMsg (don't send yet)
 *            → fire onWindowNew JS callback → user calls setWebController
 *   Phase 2: child Web initialized → completeWithChildWeb called
 *            → bind child WebView to transport → sendToTarget
 *            → Chromium natively navigates child to window.open target URL
 *
 * @since 26
 */
public class AceWebWindowNewObject {
    private static final String LOG_TAG = "AceWebWindowNewObject";
    private static final long POPUP_TIMEOUT_MS = 1000L;

    private final boolean isAlert;
    private final boolean isUserTrigger;
    private final String targetUrl;
    private WebView.WebViewTransport transport;
    private Message resultMsg;
    private volatile boolean isCompleted = false;
    private final Handler mainHandler;
    private final Runnable timeoutRunnable;

    /**
     * Constructor for AceWebWindowNewObject.
     * Stores transport and resultMsg for deferred binding. Does NOT send
     * resultMsg yet — waits for completeWithChildWeb to bind the child's
     * actual WebView, then Chromium drives the navigation natively.
     *
     * @param context The context (unused, kept for API compatibility).
     * @param isAlert Whether this is an alert-type window.
     * @param isUserTrigger Whether the window creation was triggered by user action.
     * @param targetUrl The requested popup target URL captured before dispatch.
     * @param transport The WebView transport object from onCreateWindow.
     * @param resultMsg The result message from onCreateWindow.
     */
    public AceWebWindowNewObject(
            Context context, boolean isAlert, boolean isUserTrigger, String targetUrl,
            WebView.WebViewTransport transport, Message resultMsg) {
        this.isAlert = isAlert;
        this.isUserTrigger = isUserTrigger;
                this.targetUrl = targetUrl == null ? "" : targetUrl;
        this.transport = transport;
        this.resultMsg = resultMsg;
        this.mainHandler = new Handler(Looper.getMainLooper());
        this.timeoutRunnable = () -> {
            ALog.w(LOG_TAG, "Popup timeout, auto cancel (setWebController not called in time)");
            cancelPopup();
        };
        this.mainHandler.postDelayed(timeoutRunnable, POPUP_TIMEOUT_MS);
    }

    /**
     * Indicates whether the current web window object represents an alert state.
     *
     * @return {@code true} if this object is in alert mode; {@code false} otherwise
     */
    public boolean isAlert() {
        return isAlert;
    }

    /**
     * Indicates whether the associated action or event was triggered directly by a user interaction.
     *
     * @return {@code true} if the action/event is user-triggered; {@code false} otherwise
     */
    public boolean isUserTrigger() {
        return isUserTrigger;
    }

    /**
     * Returns the popup target URL captured before the onWindowNew callback.
     *
     * @return popup target URL, or empty string when unavailable.
     */
    public String getTargetUrl() {
        return targetUrl;
    }

    /**
     * Complete the popup by binding the child WebView to the transport.
     * Chromium will then natively navigate the child to the window.open target URL.
     * This is analogous to OH's SetNWebHandlerById which binds the child NWeb
     * to the parent's popup handler, letting the kernel drive navigation.
     *
     * @param childWebId The web ID of the child web component.
     */
    public void completeWithChildWeb(long childWebId) {
        if (isCompleted) {
            ALog.w(LOG_TAG, "completeWithChildWeb already completed");
            return;
        }
        Map<Long, AceWebBase> objectMap = AceWebPluginBase.getObjectMap();
        if (objectMap == null) {
            ALog.e(LOG_TAG, "objectMap is null");
            cancelPopup();
            return;
        }
        AceWebBase childWeb = objectMap.get(childWebId);
        if (childWeb == null) {
            ALog.e(LOG_TAG, "child web not found for id: " + childWebId);
            cancelPopup();
            return;
        }
        if (!(childWeb instanceof AceWeb)) {
            ALog.e(LOG_TAG, "child web is not an AceWeb instance");
            cancelPopup();
            return;
        }
        final AceWeb childAceWeb = (AceWeb) childWeb;
        final WebView childWebView = childAceWeb.getWebview();
        if (childWebView == null) {
            ALog.e(LOG_TAG, "child WebView is null");
            cancelPopup();
            return;
        }

        isCompleted = true;
        mainHandler.removeCallbacks(timeoutRunnable);
        Runnable bindAction = () -> {
            if (transport != null && resultMsg != null) {
                transport.setWebView(childWebView);
                resultMsg.sendToTarget();
                transport = null;
                resultMsg = null;
            }
        };

        if (Looper.myLooper() == Looper.getMainLooper()) {
            bindAction.run();
        } else {
            mainHandler.post(bindAction);
        }
    }

    /**
     * Cancel the popup window creation.
     * Sends resultMsg without a valid WebView to tell Chromium to cancel,
     * analogous to OH's NotifyPopupWindowResult(false).
     */
    public void cancelPopup() {
        if (isCompleted) {
            return;
        }
        isCompleted = true;
        mainHandler.removeCallbacks(timeoutRunnable);
        Runnable cancelAction = () -> {
            if (resultMsg != null) {
                if (transport != null) {
                    transport.setWebView(null);
                }
                resultMsg.sendToTarget();
                transport = null;
                resultMsg = null;
            }
        };

        if (Looper.myLooper() == Looper.getMainLooper()) {
            cancelAction.run();
        } else {
            mainHandler.post(cancelAction);
        }
    }
}
