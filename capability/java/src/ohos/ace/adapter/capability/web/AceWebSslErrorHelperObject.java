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

import android.net.http.SslCertificate;
import android.net.http.SslError;
import java.lang.reflect.Method;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateEncodingException;
import java.util.List;

/**
 * This provides helper method to ssl error
 * AceWebSslErrorHelperObject.
 *
 * @since 22
 */
public class AceWebSslErrorHelperObject {
    private static final String LOG_TAG = "AceWebSslErrorHelperObject";
    private static final int EXTERNAL_ERROR_INVALID = 0;
    private static final int EXTERNAL_ERROR_HOST_MISMATCH = 1;
    private static final int EXTERNAL_ERROR_DATE_INVALID = 2;
    private static final int EXTERNAL_ERROR_UNTRUSTED = 3;

    /**
     * Converts the given SslError's primary error code to an external error code.
     *
     * @param error the ssl error
     * @return the external error code
     */
    public static int convertErrorCode(SslError error) {
        int errorCode = EXTERNAL_ERROR_UNTRUSTED;
        if (error == null) {
            return errorCode;
        }
        int primaryError = error.getPrimaryError();
        switch (primaryError) {
            case android.net.http.SslError.SSL_NOTYETVALID:
            case android.net.http.SslError.SSL_EXPIRED:
            case android.net.http.SslError.SSL_DATE_INVALID:
                errorCode = EXTERNAL_ERROR_DATE_INVALID;
                break;
            case android.net.http.SslError.SSL_IDMISMATCH:
                errorCode = EXTERNAL_ERROR_HOST_MISMATCH;
                break;
            case android.net.http.SslError.SSL_UNTRUSTED:
                errorCode = EXTERNAL_ERROR_UNTRUSTED;
                break;
            case android.net.http.SslError.SSL_INVALID:
                errorCode = EXTERNAL_ERROR_INVALID;
                break;
            default:
                errorCode = EXTERNAL_ERROR_UNTRUSTED;
                break;
        }
        return errorCode;
    }

    /**
     * Gets certificate chain data lists.
     *
     * @param error error
     * @return the certificate chain data as a list of strings
     */
    public static java.util.List<String> getCertChainData(SslError error) {
        java.util.List<String> certList = new java.util.ArrayList<>();

        android.net.http.SslCertificate sslCertificate = error.getCertificate();
        if (sslCertificate == null) {
            return certList;
        }

        X509Certificate x509Certificate = null;
        try {
            Method method = sslCertificate.getClass().getMethod("getX509Certificate");
            x509Certificate = (X509Certificate) method.invoke(sslCertificate);
        } catch (NoSuchMethodException | IllegalAccessException | java.lang.reflect.InvocationTargetException e) {
            return certList;
        }

        if (x509Certificate != null) {
            try {
                byte[] derEncoded = x509Certificate.getEncoded();

                StringBuilder hexSb = new StringBuilder();
                for (byte b : derEncoded) {
                    int unsignedByte = b & 0xFF;
                    hexSb.append(String.format("%02X", unsignedByte));
                }

                certList.add(hexSb.toString());
            } catch (CertificateEncodingException e) {
                return certList;
            }
        }
        return certList;
    }
}
