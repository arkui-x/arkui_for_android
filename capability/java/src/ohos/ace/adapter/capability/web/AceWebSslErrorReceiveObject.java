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
 * The type Ace web ssl error receive object.
 *
 * @since 22
 */
public class AceWebSslErrorReceiveObject {
    private final SslError error;
    private final SslErrorHandler handler;

    /**
     * Instantiates a new Ace web ssl error receive object.
     *
     * @param error the ssl error
     * @param handler the ssl error handler
     */
    public AceWebSslErrorReceiveObject(SslError error, SslErrorHandler handler) {
        this.error = error;
        this.handler = handler;
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
     * Gets certificate chain data array.
     *
     * @return the certificate chain data as a array of strings
     */
    public String[] getCertChainDataArray() {
        List<String> list = AceWebSslErrorHelperObject.getCertChainData(this.error);
        return list.toArray(new String[0]);
    }

    /**
     * Proceed with the SSL error (ignore the error and continue).
     */
    public void proceed() {
        if (handler != null) {
            handler.proceed();
        }
    }

    /**
     * Cancel the request due to SSL error.
     */
    public void cancel() {
        if (handler != null) {
            handler.cancel();
        }
    }
}
