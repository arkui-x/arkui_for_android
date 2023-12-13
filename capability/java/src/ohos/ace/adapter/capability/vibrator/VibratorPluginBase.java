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

package ohos.ace.adapter.capability.vibrator;

/**
 * VibratorPluginBase
 *
 * @since 1
 */
public abstract class VibratorPluginBase {
    /**
     * Run the vibrator with the specified duration.
     *
     * @param duration Duration which set to vibrator.
     */
    public abstract void vibrate(int duration);

    /**
     * Run the vibrator with the specified effect ID.
     *
     * @param effectId Effect ID which set to vibrator.
     */
    public abstract void vibrate(String effectId);

    /**
     * native func for Init.
     */
    protected native void nativeInit();
}