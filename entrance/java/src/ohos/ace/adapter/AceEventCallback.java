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

/**
 * The event callback interface define from native
 *
 * @since 1
 */
public interface AceEventCallback {
    /**
     * Event from native with event info
     *
     * @param pageId the page id of the event
     * @param eventId the unique event id
     * @param params the extra params with json format
     * @return the event consumption result with json format
     */
    String onEvent(int pageId, String eventId, String params);

    /**
     * Called by native when pipeline is finished
     *
     */
    void onFinish();
    
    /**
     * called when status bar background color has changed.
     *
     * @param color new status bar backgournd color
     */
    public void onStatusBarBgColorChanged(int color);
}
