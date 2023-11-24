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

import android.content.Context;

import java.util.ArrayList;
import java.util.List;

public class WebDataBaseManager {
    private static WebDataBaseManager instance;
    static final String LOG_TAG = "WebDataBaseManager";

    private WebDataBaseHttpAuthDao httpAuthDao;
    private WebDataBaseCredentialDao credentialDao;
    private WebDataBaseHelper db;

    private WebDataBaseManager() {}

    private WebDataBaseManager(WebDataBaseHelper db, WebDataBaseHttpAuthDao httpAuthDao,
        WebDataBaseCredentialDao credentialDao) {
        this.db = db;
        this.httpAuthDao = httpAuthDao;
        this.credentialDao = credentialDao;
    }

    public static WebDataBaseManager getInstance(Context context) {
        if (instance != null) {
            return instance;
        }
        WebDataBaseHelper db = new WebDataBaseHelper(context);
        instance = new WebDataBaseManager(db, new WebDataBaseHttpAuthDao(db), new WebDataBaseCredentialDao(db));
        return instance;
    }

    public List<WebDataBaseCredential> getHttpAuthCredentials(String host, String realm) {
        List<WebDataBaseCredential> credentials = new ArrayList<>();
        WebDataBaseHttpAuth httpAuth = httpAuthDao.find(host, realm);
        if (httpAuth != null) {
            credentials = credentialDao.getAllByHttpAuthId(httpAuth.getId());
        }
        return credentials;
    }

    public WebDataBaseCredential getHttpAuthCredential(String host, String realm) {
        List<WebDataBaseCredential> credentials = getHttpAuthCredentials(host, realm);
        WebDataBaseCredential credential = new WebDataBaseCredential("", "");
        if (!credentials.isEmpty()) {
            credential = credentials.get(credentials.size() - 1);
        }
        return credential;
    }

    public void deleteAllAuthCredentials() {
        db.clearAllTables(db.getWritableDatabase());
    }

    public void saveHttpAuthCredential(String host, String realm, String username, String password) {
        WebDataBaseHttpAuth httpAuth = httpAuthDao.find(host, realm);
        Long httpAuthId;
        if (httpAuth == null) {
            httpAuthId = httpAuthDao.insert(new WebDataBaseHttpAuth(null, host, realm));
        } else {
            httpAuthId = httpAuth.getId();
        }

        WebDataBaseCredential credential = credentialDao.find(username, password, httpAuthId);
        if (credential != null) {
            boolean needUpdate = false;
            if (!credential.getUsername().equals(username)) {
                credential.setUsername(username);
                needUpdate = true;
            }
            if (!credential.getPassword().equals(password)) {
                credential.setPassword(password);
                needUpdate = true;
            }
            if (needUpdate) {
                credentialDao.update(credential);
            }
        } else {
            credential = new WebDataBaseCredential(null, username, password, httpAuthId);
            credential.setId(credentialDao.insert(credential));
        }
    }

    public boolean existHttpAuthCredentials() {
        return credentialDao.exist();
    }
}