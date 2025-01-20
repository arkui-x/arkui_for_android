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

/**
 * Download item object, providing methods related to deal download process data.
 *
 * @since 2025-01-16
 */
public class AceWebDownloadItemObject {
    private long id_ = 0L;
    private String guid_ = "";
    private long downloadId_ = -1L;
    private long lastBytes_ = 0L;
    private long receivedBytes_ = 0L;
    private long totalSize_ = 0L;
    private long currentSpeed_ = 0L;
    private int state_ = 0;
    private int errCode_ = 0;
    private int percentComplete_ = 0;
    private String method_ = "";
    private String mimeType_ = "";
    private String url_ = "";
    private String suggestedFilename_ = "";
    private String fullPath_ = "";
    private String realName_ = "";
    private String realPath_ = "";

    /**
     * Construct.
     *
     * @param id Webview id.
     * @param guid Unique identifier.
     * @param suggestedFilename Suggested filename.
     * @param state Download status.
     * @param url The url of download.
     */
    public AceWebDownloadItemObject(long id, String guid, String suggestedFilename, int state, String url) {
        this.id_ = id;
        this.guid_ = guid;
        this.suggestedFilename_ = suggestedFilename;
        this.state_ = state;
        this.url_ = url;
    }

    /**
     * Get webview id.
     *
     * @return Webview id.
     */
    public synchronized long getWebId() {
        return this.id_;
    }

    /**
     * Set unique identifier.
     *
     * @param guid Unique identifier.
     */
    public synchronized void setGuid(String guid) {
        this.guid_ = guid;
    }

    /**
     * Get unique identifier.
     *
     * @return Unique identifier.
     */
    public synchronized String getGuid() {
        return this.guid_;
    }

    /**
     * Set download id.
     *
     * @param downloadId Download id.
     */
    public synchronized void setDownloadId(long downloadId) {
        this.downloadId_ = downloadId;
    }

    /**
     * Get download id.
     *
     * @return Download id.
     */
    public synchronized long getDownloadId() {
        return this.downloadId_;
    }

    /**
     * Set download speed.
     *
     * @param speed Download speed.
     */
    public synchronized void setCurrentSpeed(long speed) {
        this.currentSpeed_ = speed;
    }

    /**
     * Get download speed.
     *
     * @return Download speed.
     */
    public synchronized long getCurrentSpeed() {
        return this.currentSpeed_;
    }

    /**
     * Set download percent.
     *
     * @param percent Download percent.
     */
    public synchronized void setPercentComplete(int percent) {
        this.percentComplete_ = percent;
    }

    /**
     * Get download percent.
     *
     * @return Download percent.
     */
    public synchronized int getPercentComplete() {
        return this.percentComplete_;
    }

    /**
     * Set download totalSize.
     *
     * @param totalSize Download totalSize.
     */
    public synchronized void setTotalBytes(long totalSize) {
        this.totalSize_ = totalSize;
    }

    /**
     * Get download total bytes.
     *
     * @return Download total bytes.
     */
    public synchronized long getTotalBytes() {
        return this.totalSize_;
    }

    /**
     * Set download state.
     *
     * @param state Download state.
     */
    public synchronized void setState(int state) {
        this.state_ = state;
    }

    /**
     * Get download state.
     *
     * @return Download state.
     */
    public synchronized int getState() {
        return this.state_;
    }

    /**
     * Set download error code.
     *
     * @param err Download error code.
     */
    public synchronized void setLastErrorCode(int err) {
        this.errCode_ = err;
    }

    /**
     * Get download error code.
     *
     * @return Download error code.
     */
    public synchronized int getLastErrorCode() {
        return this.errCode_;
    }

    /**
     * Set download request method.
     *
     * @param method Download request method.
     */
    public synchronized void setMethod(String method) {
        this.method_ = method;
    }

    /**
     * Get download request method.
     *
     * @return Download request method.
     */
    public synchronized String getMethod() {
        return this.method_;
    }

    /**
     * Set download mimeType.
     *
     * @param mimeType Download mimeType.
     */
    public synchronized void setMimeType(String mimeType) {
        this.mimeType_ = mimeType;
    }

    /**
     * Get download mimeType.
     *
     * @return Download mimeType.
     */
    public synchronized String getMimeType() {
        return this.mimeType_;
    }

    /**
     * Set download url.
     *
     * @param url Download url.
     */
    public synchronized void setUrl(String url) {
        this.url_ = url;
    }

    /**
     * Get download url.
     *
     * @return Download url.
     */
    public synchronized String getUrl() {
        return this.url_;
    }

    /**
     * Set download suggested filename.
     *
     * @param suggestedFilename Download suggested filename.
     */
    public synchronized void setSuggestedFileName(String suggestedFilename) {
        this.suggestedFilename_ = suggestedFilename;
    }

    /**
     * Get download suggested file name.
     *
     * @return Suggested file name.
     */
    public synchronized String getSuggestedFileName() {
        return this.suggestedFilename_;
    }

    /**
     * Set download real file name.
     *
     * @param realName Download real file name.
     */
    public synchronized void setRealName(String realName) {
        this.realName_ = realName;
    }

    /**
     * Get download real file name.
     *
     * @return Download real file name.
     */
    public synchronized String getRealName() {
        return this.realName_;
    }

    /**
     * Set download real path.
     *
     * @param realPath Download real path.
     */
    public synchronized void setRealPath(String realPath) {
        this.realPath_ = realPath;
    }

    /**
     * Get download real file path.
     *
     * @return Download real file path.
     */
    public synchronized String getRealPath() {
        return this.realPath_;
    }

    /**
     * Set download last recevied bytes.
     *
     * @param lastBytes Download last receveid bytes.
     */
    public synchronized void setLastBytes(long lastBytes) {
        this.lastBytes_ = lastBytes;
    }

    /**
     * Get download last recevied bytes.
     *
     * @return Download last recevied bytes.
     */
    public synchronized long getLastBytes() {
        return this.lastBytes_;
    }

    /**
     * Set download received bytes.
     *
     * @param bytes Download received bytes.
     */
    public synchronized void setReceivedBytes(long bytes) {
        this.receivedBytes_ = bytes;
    }

    /**
     * Get download received bytes.
     *
     * @return Download received bytes.
     */
    public synchronized long getReceivedBytes() {
        return this.receivedBytes_;
    }

    /**
     * Set download full path.
     *
     * @param path Download full path.
     */
    public synchronized void setFullPath(String path) {
        this.fullPath_ = path;
    }

    /**
     * Get download full path.
     *
     * @return Download full path.
     */
    public synchronized String getFullPath() {
        return this.fullPath_;
    }
}
