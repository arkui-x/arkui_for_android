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

import android.content.Context;

/**
 * The class for creating an AceView on Android Platform.
 *
 * @since 1
 */
public class AceViewCreatorAosp implements IAceViewCreator {
    private Context context;

    /**
     * constructor of AceViewCreator on AOSP platform
     *
     * @param context context of application
     */
    public AceViewCreatorAosp(Context context) {
        this.context = context;
    }

    /**
     * Create View on AOSP platform
     *
     * @param instanceId instance ID of application
     * @param density density of application
     */
    @Override
    public IAceView createView(int instanceId, float density) {
        return new AceViewAosp(context, instanceId, density, false);
    }
}
