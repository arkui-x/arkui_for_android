/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import android.app.DownloadManager;
import java.io.File;
import java.net.HttpURLConnection;
import java.util.ArrayList;
import java.util.List;

/**
 * Download utility class, providing methods related to file downloading.
 *
 * @since 2025-01-16
 */
public class AceWebDownloadHelperObject {
    private static final int MAX_INCREMENT_ID = 0x1000000;
    private static final int WEB_ERROR_UNKNOWN = 0;
    private static final int WEB_FILE_FAILED = 1;
    private static final int WEB_FILE_NO_SPACE = 3;
    private static final int WEB_FILE_TRANSIENT_ERROR = 10;
    private static final int WEB_FILE_TOO_SHORT = 13;
    private static final int WEB_FILE_SAME_AS_SOURCE = 15;
    private static final int WEB_NETWORK_FAILED = 20;
    private static final int WEB_NETWORK_DISCONNECTED = 22;
    private static final int WEB_SERVER_FAILED = 30;
    private static final int WEB_SERVER_BAD_CONTENT = 33;
    private static final int WEB_SERVER_FORBIDDEN = 36;

    private static int incrementId_ = 0;
    private static List<String> webDownloadFilePathList_ = new ArrayList<>();
    private static final Object WEB_TOOL_LOCK = new Object();

    /**
     * Remove existed path from list.
     *
     * @param path Download path.
     */
    public static void removeFilePathFromList(String path) {
        synchronized (WEB_TOOL_LOCK) {
            webDownloadFilePathList_.remove(path);
        }
    }

    /**
     * Add record existed path to list.
     *
     * @param path Download path.
     */
    public static void addFilePathToList(String path) {
        synchronized (WEB_TOOL_LOCK) {
            webDownloadFilePathList_.add(path);
        }
    }

    /**
     * Create a unique identifier.
     *
     * @return Unique identifier.
     */
    public static final String createDownloadGuid() {
        long timestamp = System.currentTimeMillis();
        if (++incrementId_ >= MAX_INCREMENT_ID) {
            incrementId_ = 0;
        }
        return Long.toHexString(timestamp) + Integer.toHexString(incrementId_ + MAX_INCREMENT_ID);
    }

    /**
     * Check if the path is Legal.
     *
     * @param path Download path.
     * @return The result of whether path is Legal.
     */
    public static final boolean isLegalPath(String path) {
        File pathDir = new File(path);
        if (pathDir == null) {
            return false;
        }
        File parentDir = pathDir.getParentFile();
        if (parentDir != null && ((parentDir.exists() || (!parentDir.exists() && parentDir.mkdirs())) &&
            parentDir.canWrite() && parentDir.canRead())) {
            return true;
        }
        return false;
    }

    /**
     * Get non duplicate path.
     *
     * @param path Download path.
     * @return The non duplicate path.
     */
    public static final String getNonDuplicatePath(String path) {
        File createFile = new File(path);
        if (createFile == null) {
            return "";
        }
        File parentDir = createFile.getParentFile();
        if (parentDir != null && !parentDir.exists()) {
            parentDir.mkdirs();
        }
        int lastIndex = path.lastIndexOf('.');
        int fileIndex = 1;
        String finalPath = path;
        String fileName = "";
        String fileSuffix = "";
        while (createFile != null && createFile.exists() || webDownloadFilePathList_.contains(finalPath)) {
            if (lastIndex != -1) {
                fileName = path.substring(0, lastIndex);
                fileSuffix = path.substring(lastIndex);
                finalPath = fileName + '(' + fileIndex + ')' + fileSuffix;
            } else {
                finalPath = finalPath + '(' + fileIndex + ')';
            }
            createFile = new File(finalPath);
            if (createFile == null) {
                return "";
            }
            fileIndex++;
        }
        addFilePathToList(finalPath);
        return finalPath;
    }

    /**
     * Convert http or downloadmanager error code.
     *
     * @param code Http or downloadmanager error code.
     * @return Adaptor error code.
     */
    public static final int convertsHttpErrorCode(int code) {
        int ret = WEB_ERROR_UNKNOWN;
        switch (code) {
            case DownloadManager.ERROR_UNKNOWN:
                ret = WEB_ERROR_UNKNOWN;
                break;
            case DownloadManager.PAUSED_WAITING_TO_RETRY:
                ret = WEB_SERVER_FAILED;
                break;
            case DownloadManager.PAUSED_WAITING_FOR_NETWORK:
                ret = WEB_NETWORK_DISCONNECTED;
                break;
            case HttpURLConnection.HTTP_FORBIDDEN:
                ret = WEB_SERVER_FORBIDDEN;
                break;
            case HttpURLConnection.HTTP_NOT_FOUND:
                ret = WEB_SERVER_BAD_CONTENT;
                break;
            case HttpURLConnection.HTTP_INTERNAL_ERROR:
                ret = WEB_SERVER_FAILED;
                break;
            case DownloadManager.ERROR_FILE_ERROR:
                ret = WEB_FILE_FAILED;
                break;
            case DownloadManager.ERROR_UNHANDLED_HTTP_CODE:
                ret = WEB_NETWORK_FAILED;
                break;
            case DownloadManager.ERROR_INSUFFICIENT_SPACE:
                ret = WEB_FILE_NO_SPACE;
                break;
            case DownloadManager.ERROR_DEVICE_NOT_FOUND:
                ret = WEB_FILE_TRANSIENT_ERROR;
                break;
            case DownloadManager.ERROR_CANNOT_RESUME:
                ret = WEB_FILE_TOO_SHORT;
                break;
            case DownloadManager.ERROR_FILE_ALREADY_EXISTS:
                ret = WEB_FILE_SAME_AS_SOURCE;
                break;
            default:
                ret = WEB_ERROR_UNKNOWN;
        }
        return ret;
    }
}
