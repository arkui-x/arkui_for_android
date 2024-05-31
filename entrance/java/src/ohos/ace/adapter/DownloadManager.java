/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

package ohos.ace.adapter;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import java.net.HttpURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

/**
 * a simple download manager to request bytes via http
 *
 * @since 1
 */
public class DownloadManager {
    private static final String LOG_TAG = "DownloadManager";

    private static final int DEFAULT_TIME_OUT = 5000;

    private static final int UNIT_BYTE_LENGTH = 1024;

    private static final Object LOCK = new Object();

    private static final String DEFAULT_CHARSET = "UTF-8";

    private static final byte[] NULL_BYTE = new byte[0];

    private static volatile boolean isSslInited = false;

    private static class AceX509TrustManager implements X509TrustManager {
        @Override
        public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
        }

        @Override
        public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
        }

        @Override
        public X509Certificate[] getAcceptedIssuers() {
            return new X509Certificate[0];
        }
    }

    private DownloadManager() {
    }

    /**
     * Start download with url
     *
     * @param urlStr the download url
     * @return the downloaded content
     */
    public static byte[] download(String urlStr) {
        initSsl();
        byte[] buff = new byte[UNIT_BYTE_LENGTH];
        int len = 0;
        HttpURLConnection conn = null;

        try {
            URL url = new URL(urlStr);
            URLConnection connection = url.openConnection();
            if (!(connection instanceof HttpURLConnection)) {
                ALog.e(LOG_TAG, "not http or https url");
                return NULL_BYTE;
            }
            conn = (HttpURLConnection) connection;
            conn.setRequestProperty("Content-Type", "plain/text;charset=" + DEFAULT_CHARSET);
            conn.setRequestProperty("charset", DEFAULT_CHARSET);
            conn.setRequestProperty("User-agent", "Ace");
            conn.setDoInput(true);
            conn.setConnectTimeout(DEFAULT_TIME_OUT);
            conn.setReadTimeout(DEFAULT_TIME_OUT);
            conn.connect();
        } catch (IOException error) {
            ALog.e(LOG_TAG, "request data err:" + error.getMessage());
            return NULL_BYTE;
        } catch (SecurityException error) {
            ALog.e(LOG_TAG, "security err:" + error.getMessage());
            return NULL_BYTE;
        }

        InputStream inputStream = null;
        try (ByteArrayOutputStream outputStream = new ByteArrayOutputStream()) {
            inputStream = conn.getInputStream();
            while ((len = inputStream.read(buff)) != -1) {
                outputStream.write(buff, 0, len);
            }
            return outputStream.toByteArray();
        } catch (IOException error) {
            ALog.e(LOG_TAG, "read or write data err:" + error.getMessage());
            return NULL_BYTE;
        } finally {
            if (inputStream != null) {
                try {
                    inputStream.close();
                } catch (IOException error) {
                    ALog.e(LOG_TAG, "InputStream close err:" + error.getMessage());
                }
            }
        }
    }

    private static void initSsl() {
        if (isSslInited) {
            return;
        }

        synchronized (LOCK) {
            if (isSslInited) {
                return;
            }

            try {
                SSLContext sslContext = SSLContext.getInstance("SSL");
                sslContext.init(null, new TrustManager[] { new AceX509TrustManager() }, new SecureRandom());
                HostnameVerifier ignoreHostnameVerifier = new HostnameVerifier() {
                    public boolean verify(String host, SSLSession sslSession) {
                        if (host == null || host.isEmpty()) {
                            return false;
                        }
                        return true;
                    }
                };
                HttpsURLConnection.setDefaultHostnameVerifier(ignoreHostnameVerifier);
                HttpsURLConnection.setDefaultSSLSocketFactory(sslContext.getSocketFactory());
                isSslInited = true;
            } catch (NoSuchAlgorithmException | KeyManagementException error) {
                ALog.e(LOG_TAG, "init SSL error:" + error.getMessage());
            }
        }
    }
}
