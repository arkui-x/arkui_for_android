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

import android.net.http.SslError;
import android.webkit.SslErrorHandler;
import java.util.List;
import ohos.ace.adapter.capability.web.AceWebSslErrorHelperObject;

/**
 * The type Ace web all ssl error receive object for onSslErrorEvent.
 * This provides extended SSL error information compared to
 * AceWebSslErrorReceiveObject.
 *
 * @since 22
 */
public class AceWebAllSslErrorReceiveObject {
    private static final String LOG_TAG = "AceWebAllSslErrorReceiveObject";

    private final SslError error;
    private final SslErrorHandler handler;
    private final String referrer;
    private final boolean isMainFrame;

    /**
     * Instantiates a new Ace web all ssl error receive object.
     *
     * @param error       the ssl error
     * @param handler     the ssl error handler
     * @param referrer    the referrer URL
     * @param isMainFrame whether this error occurred in the main frame
     */
    public AceWebAllSslErrorReceiveObject(SslError error, SslErrorHandler handler, String referrer,
            boolean isMainFrame) {
        this.error = error;
        this.handler = handler;
        this.referrer = referrer != null ? referrer : "";
        this.isMainFrame = isMainFrame;
    }

    /**
     * Gets error code (primary error).
     *
     * @return the error code
     */
    public int getError() {
        return AceWebSslErrorHelperObject.convertErrorCode(this.error);
    }

    /**
     * Gets current request URL.
     *
     * @return the current URL
     */
    public String getUrl() {
        if (this.error == null) {
            return "";
        }
        String url = this.error.getUrl();
        return url != null ? url : "";
    }

    /**
     * Gets original request URL.
     *
     * @return the original URL
     */
    public String getOriginalUrl() {
        return getUrl();
    }

    /**
     * Gets referrer URL.
     *
     * @return the referrer URL
     */
    public String getReferrer() {
        return this.referrer;
    }

    /**
     * Gets whether this is a fatal error.
     *
     * @return true if fatal error, false otherwise
     */
    public boolean getIsFatalError() {
        return false;
    }

    /**
     * Gets whether error occurred in main frame.
     *
     * @return true if main frame, false otherwise
     */
    public boolean getIsMainFrame() {
        return this.isMainFrame;
    }

    /**
     * Confirm the SSL error (proceed).
     */
    public void confirm() {
        if (this.handler != null) {
            this.handler.proceed();
        }
    }

    /**
     * Cancel the request due to SSL error.
     */
    public void cancel() {
        if (this.handler != null) {
            this.handler.cancel();
        }
    }

    /**
     * Gets certificate chain data array.
     *
     * @return the certificate chain data as a array of strings
     */
    public String[] getCertChainDataArray() {
        List<String> list = AceWebSslErrorHelperObject.getCertChainData(this.error);
        return list.toArray(new String[0]);
    }
}
