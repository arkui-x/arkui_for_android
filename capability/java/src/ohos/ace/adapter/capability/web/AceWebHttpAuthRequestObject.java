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

import android.webkit.HttpAuthHandler;
import ohos.ace.adapter.ALog;
import android.content.Context;

public class AceWebHttpAuthRequestObject {
    private static final String LOG_TAG = "AceWebHttpAuthRequestObject";

    private HttpAuthHandler handler;
    private String host;
    private String realm;
    private Context context;

    public AceWebHttpAuthRequestObject(HttpAuthHandler handler, String host, String realm, Context context) {
        this.handler = handler;
        this.host = host;
        this.realm = realm;
        this.context = context;
    }

    private Context getContext() {
        return this.context;
    }

    public HttpAuthHandler getRequestUrl() {
        return this.handler;
    }

    public String getHost() {
        return this.host;
    }

    public String getRealm() {
        return this.realm;
    }

    public void cancel() {
        try {
            this.handler.cancel();
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call cancel method failed");
        }
        
    }

    public boolean proceed(String username, String password) {
        try {
            this.handler.proceed(username, password);
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call proceed method failed");
        }
        return true;
    }

    public boolean useHttpAuthUsernamePassword() {
        try {
            boolean result = this.handler.useHttpAuthUsernamePassword();
            if (result) {
                WebDataBaseManager instance = WebDataBaseManager.getInstance(getContext());
                if (instance != null) {
                    WebDataBaseCredential credential = instance.getHttpAuthCredential(getHost(), getRealm());
                    if (credential != null) {
                        proceed(credential.getUsername(), credential.getPassword());
                    }
                }
            }
            return result;
        } catch (Exception e) {
            ALog.e(LOG_TAG, "call useHttpAuthUsernamePassword method failed");
            return false;
        }
    }
}
