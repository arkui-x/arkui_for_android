/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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

import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;

import ohos.ace.adapter.ALog;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

/**
 * AceWebSchemeHandler is a class that handles web scheme requests.
 *
 * @since 22
 */
public class AceWebSchemeHandler {
    private static final String LOG_TAG = "AceWebSchemeHandler";

    private static final int WEB_ERROR_CODE = -104;

    private String requestUrl = "";

    private String requestMethod = "";

    private boolean isRequestGesture = false;

    private boolean isMainFrame = false;

    private boolean isRedirect = false;

    private Map<String, String> requestHeaders;

    private WebResourceResponse response = null;

    private int errorCode = 0;

    private String errorInfo = "";

    private String responseUrl = "";

    private String responseMimeType = "";

    private String responseEncoding = "";

    private int responseStatusCode = 0;

    private String responseReason = "";

    private Map<String, String> responseHeaders;

    private String responseData = "";

    private boolean isFinished = false;

    private boolean isResponseFail = false;

    private int responseErrorCode = 0;

    private String responseErrorInfo = "";

    private boolean isReceiveResponseFinished = false;

    /**
     * Constructs an instance of {@code AceWebSchemeHandler} using the provided {@link WebResourceRequest}.
     * <p>
     * Initializes the following fields based on the request:
     * <ul>
     *   <li>{@code requestUrl}: The URL of the request as a string, or an empty string if the URL is null.</li>
     *   <li>{@code requestMethod}: The HTTP method of the request as a string,
     * or an empty string if the method is null.</li>
     *   <li>{@code isRequestGesture}: {@code true} if the request was initiated by a user gesture;
     * {@code false} otherwise.</li>
     *   <li>{@code isMainFrame}: {@code true} if the request is for the main frame; {@code false} otherwise.</li>
     *   <li>{@code isRedirect}: {@code true} if the request is a redirect; {@code false} otherwise.</li>
     *   <li>{@code requestHeaders}: The request headers as a map. If the headers are null, an empty map is used.</li>
     * </ul>
     *
     * @param request The {@link WebResourceRequest} containing the details of the web resource request.
     */
    public AceWebSchemeHandler(WebResourceRequest request) {
        if (request == null) {
            ALog.e(LOG_TAG, "AceWebSchemeHandler: request is null");
            return;
        }
        this.requestUrl = request.getUrl() != null ? request.getUrl().toString() : "";
        this.requestMethod = request.getMethod() != null ? request.getMethod().toString() : "";
        this.isRequestGesture = request.hasGesture();
        this.isMainFrame = request.isForMainFrame();
        this.isRedirect = request.isRedirect();
        this.requestHeaders =
            request.getRequestHeaders() != null ? request.getRequestHeaders() : new HashMap<String, String>();
    }

    /**
     * Retrieves the URL of the request.
     *
     * @return string representing the URL of the request. If the URL is null, an
     *         empty string is returned.
     */
    public String getRequestUrl() {
        return this.requestUrl;
    }

    /**
     * Retrieves the HTTP method of the request.
     *
     * @return A string representing the HTTP method, or an empty string if the
     *         method is not available.
     */
    public String getMethod() {
        return this.requestMethod;
    }

    /**
     * Checks if the request has a gesture.
     *
     * @return true if the request has a gesture, false otherwise.
     */
    public boolean isRequestGesture() {
        return this.isRequestGesture;
    }

    /**
     * Checks if the request is for the main frame.
     *
     * @return true if the request is for the main frame, false otherwise.
     */
    public boolean isMainFrame() {
        return this.isMainFrame;
    }

    /**
     * Checks if the request is a redirect.
     *
     * @return true if the request is a redirect, false otherwise.
     */
    public boolean isRedirect() {
        return this.isRedirect;
    }

    /**
     * Retrieves the request headers as a map.
     *
     * @return A map of request headers, or an empty map if the headers are not
     *         available.
     */
    public Map<String, String> getRequestHeader() {
        return this.requestHeaders;
    }

    /**
     * Retrieves the current WebResourceResponse associated with this handler.
     *
     * @return the WebResourceResponse object that contains the response data
     * for a web resource request.
     */
    public WebResourceResponse getResponse() {
        try {
            if (this.responseErrorCode != 0) {
                this.isResponseFail = true;
                this.response = new WebResourceResponse("text/html", "UTF-8",
                    new ByteArrayInputStream("".getBytes(StandardCharsets.UTF_8)));
                return this.response;
            }
            InputStream dataStream = new ByteArrayInputStream(this.responseData.getBytes(StandardCharsets.UTF_8));
            this.response = new WebResourceResponse(this.responseMimeType, this.responseEncoding,
                this.responseStatusCode, this.responseReason, this.responseHeaders, dataStream);
        } catch (IllegalArgumentException e) {
            ALog.e(LOG_TAG, "getResponse throw IllegalArgumentException " + e);
            this.isResponseFail = true;
            this.response = new WebResourceResponse("text/html", "UTF-8",
                new ByteArrayInputStream("".getBytes(StandardCharsets.UTF_8)));
        }
        return this.response;
    }

    /**
     * Handles the failure of a web resource request.
     *
     * This method records the error code and error information when a web
     * resource request fails. It logs the completion of the failure handling.
     *
     * @param errorCode An integer representing the error code of the failure.
     * @param errorInfo A string providing additional information about the error.
     * @param isCompleteAndNullResponse A boolean indicating whether to complete when no response is received.
     */
    public void didFail(int errorCode, String errorInfo, boolean isCompleteAndNullResponse) {
        this.errorCode = errorCode;
        this.errorInfo = errorInfo != null ? errorInfo : "";
        if (this.errorCode != 0) {
            this.isResponseFail = true;
        }
        if (!this.isReceiveResponseFinished && isCompleteAndNullResponse) {
            ALog.e(LOG_TAG, "getResponse didReceiveResponse is not finished");
            this.errorCode = WEB_ERROR_CODE;
            this.errorInfo = "ERR_CONNECTION_FAILED";
            this.isResponseFail = true;
        }
    }

    /**
     * Retrieves the error code associated with the current instance.
     *
     * @return the error code as an integer.
     */
    public int getErrorCode() {
        return this.errorCode;
    }

    /**
     * Retrieves the error information.
     *
     * @return A string containing the error information.
     */
    public String getErrorInfo() {
        return this.errorInfo;
    }

    /**
     * Sets the response URL for the handler.
     *
     * @param url the URL to set as the response. If the provided URL is null,
     * an empty string will be set instead.
     */
    public void setResponseUrl(String url) {
        this.responseUrl = url != null ? url : "";
    }

    /**
     * Retrieves the response URL.
     *
     * @return A string representing the response URL. If the response URL is
     *         null, an empty string is returned.
     */
    public String getResponseUrl() {
        return this.responseUrl;
    }

    /**
     * Sets the HTTP response details for the web scheme handler.
     *
     * @param mimeType      the MIME type of the response content (e.g., "text/html")
     * @param encoding      the character encoding of the response (e.g., "UTF-8")
     * @param statusCode    the HTTP status code to be returned (e.g., 200 for OK)
     * @param reasonPhrase  the reason phrase associated with the status code (e.g., "OK")
     * @param headers       a map containing HTTP response headers as key-value pairs
     * @param data          the response body data as a string
     */
    public void setResponse(String mimeType, String encoding, int statusCode,
        String reasonPhrase, HashMap<String, String> headers, String data) {
        this.responseMimeType = mimeType;
        this.responseEncoding = encoding;
        this.responseStatusCode = statusCode;
        this.responseReason = reasonPhrase;
        this.responseData = data;
        this.responseHeaders = headers;
        this.isReceiveResponseFinished = true;
    }

    /**
     * Sets the response data to be used by this handler.
     *
     * @param data The response data as a String.
     */
    public void setResponseData(String data) {
        this.responseData = data != null ? data : "";
    }

    /**
     * Sets the response finish status.
     *
     * @param isFinished true if the response has finished processing; false otherwise.
     */
    public void setResponseFinish(boolean isFinished) {
        this.isFinished = isFinished;
    }

    /**
     * Checks whether the response has finished processing.
     *
     * @return {@code true} if the response is finished; {@code false} otherwise.
     */
    public boolean getResponseFinish() {
        return this.isFinished;
    }

    /**
     * Returns whether the web response has failed.
     *
     * @return {@code true} if the response has failed; {@code false} otherwise.
     */
    public boolean getResponseFail() {
        return this.isResponseFail;
    }

    /**
     * Sets the response as failed with the specified error code and error information.
     * Updates the internal state to indicate a failure if the error code is non-zero.
     *
     * @param errorCode the error code representing the failure reason
     * @param errorInfo the error information message; if null, an empty string is used
     */
    public void setResponseFail(int errorCode, String errorInfo) {
        this.responseErrorCode = errorCode;
        this.responseErrorInfo = errorInfo != null ? errorInfo : "";
        if (this.responseErrorCode != 0) {
            this.isResponseFail = true;
        }
    }

    /**
     * Retrieves the error code associated with the web response.
     *
     * @return the response error code as an integer
     */
    public int getResponseErrCode() {
        return this.responseErrorCode;
    }

    /**
     * Retrieves the error information associated with the web response.
     *
     * @return a {@code String} containing the response error information, or {@code null} if no error has occurred.
     */
    public String getResponseErrInfo() {
        return this.responseErrorInfo;
    }
}
