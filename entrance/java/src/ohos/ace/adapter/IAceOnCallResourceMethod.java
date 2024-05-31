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

import java.util.Map;

/**
 * The common interface of resource onCall event
 *
 * @since 1
 */
public interface IAceOnCallResourceMethod {
    /**
     * Called when a native report resource event occurred
     *
     * @param param the params of the event
     * @return the result of of the event
     */
    String onCall(Map<String, String> param);
}
