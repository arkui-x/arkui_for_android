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

/**
 * AceWebScaleObject class is used to store the scale information.
 *
 * @since 2024-05-31
 */
public class AceWebScaleObject {
    private float oldScale;
    private float newScale;

    public AceWebScaleObject(float oldScale, float newScale) {
        this.oldScale = oldScale;
        this.newScale = newScale;
    }

    public float getOldScale() {
        return this.oldScale;
    }

    public float getNewScale() {
        return this.newScale;
    }
}
