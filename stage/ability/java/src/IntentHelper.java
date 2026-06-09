/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

package ohos.stage.ability.adapter;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.webkit.MimeTypeMap;

import java.util.Locale;
import java.util.Map;

/**
 * Helper class for Intent-related operations.
 * Provides common methods for resolving actions, handling categories, and detecting MIME types.
 *
 * @since 1
 */
public final class IntentHelper {
    private static final String LOG_TAG = "IntentHelper";

    private static final Map<String, String> ACTION_MAPPING = Map.of(
            "ohos.want.action.viewData", Intent.ACTION_VIEW
    );

    private static final Map<String, String> ENTITY_MAPPING = Map.of(
            "entity.system.browsable", Intent.CATEGORY_BROWSABLE
    );

    private IntentHelper() {
        // Private constructor to prevent instantiation
    }

    /**
     * Resolve the action string to Android Intent action.
     *
     * @param action the action string to resolve
     * @return the corresponding Android Intent action, or null if not found
     */
    public static String resolveAction(String action) {
        if (action == null || action.isEmpty()) {
            return null;
        }
        return ACTION_MAPPING.get(action);
    }

    /**
     * Add categories from entities to the Intent.
     *
     * @param intent   the intent to add categories to
     * @param entities the entity strings to map to categories
     */
    public static void addCategoriesFromEntities(Intent intent, String[] entities) {
        if (intent == null || entities == null) {
            return;
        }
        for (String entity : entities) {
            if (entity == null) {
                continue;
            }
            String mappedCategory = ENTITY_MAPPING.get(entity);
            if (mappedCategory != null) {
                intent.addCategory(mappedCategory);
            }
        }
    }

    /**
     * Get the MIME type of a URI.
     *
     * @param context the context
     * @param uri     the URI to get MIME type for
     * @return the MIME type, or null if cannot be determined
     */
    public static String getMimeType(Context context, Uri uri) {
        if (context == null || uri == null) {
            return null;
        }
        String mimeType = null;
        String scheme = uri.getScheme();
        if (ContentResolver.SCHEME_CONTENT.equals(scheme)) {
            mimeType = getMimeTypeFromContent(context, uri);
        } else if (ContentResolver.SCHEME_FILE.equals(scheme)) {
            mimeType = getMimeTypeFromFilePath(uri.toString());
        }
        return mimeType;
    }

    /**
     * Get MIME type from content URI.
     *
     * @param context the context
     * @param uri     the content URI
     * @return the MIME type, or null if cannot be determined
     */
    public static String getMimeTypeFromContent(Context context, Uri uri) {
        if (context == null || uri == null) {
            return null;
        }
        ContentResolver contentResolver = context.getContentResolver();
        String mimeType = contentResolver.getType(uri);
        if (mimeType == null) {
            mimeType = getMimeTypeFromFilePath(uri.toString());
        }
        return mimeType;
    }

    /**
     * Get MIME type from file path.
     *
     * @param uri the file path URI string
     * @return the MIME type, or null if cannot be determined
     */
    public static String getMimeTypeFromFilePath(String uri) {
        if (uri == null || uri.isEmpty()) {
            return null;
        }
        String fileExtension = MimeTypeMap.getFileExtensionFromUrl(uri);
        if (fileExtension == null || fileExtension.isEmpty()) {
            return null;
        }
        return MimeTypeMap.getSingleton().getMimeTypeFromExtension(
                fileExtension.toLowerCase(Locale.ROOT));
    }
}
