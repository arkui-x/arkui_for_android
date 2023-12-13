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
 * The common interface of resource event
 *
 * @since 1
 */
public interface IAceOnResourceEvent {
    /**
     * Called when a the resource event is send to Native
     *
     * @param eventId the id of the event
     * @param param the params of the event
     */
    void onEvent(String eventId, String param);
}
