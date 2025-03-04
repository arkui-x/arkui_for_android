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

import android.provider.BaseColumns;

/**
 * Contains database column and table definitions for web-related data storage.
 *
 * @since 2024-05-31
 */
public class WebDataBaseColumns {
    private WebDataBaseColumns() {}

    /**
     * Database table and column names for web-related entries.
     */
    public static class EntryColumns implements BaseColumns {
        /**
         * credential table name.
         */
        public static final String TABLE_NAME_CREDENTIAL = "credential";

        /**
         * username column name
         */
        public static final String COLUMN_NAME_USERNAME = "username";

        /**
         * password column name
         */
        public static final String COLUMN_NAME_USERPASS = "password";

        /**
         * http_auth_id column name
         */
        public static final String COLUMN_NAME_HTTP_AUTH_ID = "http_auth_id";

        /**
         * http_auth table name.
         */
        public static final String TABLE_NAME_HTTPAUTH = "http_auth";

        /**
         * host column name
         */
        public static final String COLUMN_NAME_HOST = "host";

        /**
         * port column name
         */
        public static final String COLUMN_NAME_REALM = "realm";

        /**
         * geolocation table name.
         */
        public static final String TABLE_NAME_GEOLOCATION = "geolocation";

        /**
         * origin column name
         */
        public static final String COLUMN_NAME_ORIGIN = "origin";

        /**
         * result column name
         */
        public static final String COLUMN_NAME_RESULT = "result";
    }
}
