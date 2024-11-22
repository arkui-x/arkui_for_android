/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.video;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.Surface;

import ohos.ace.adapter.capability.texture.AceTexture;
import ohos.ace.adapter.ALog;

import java.util.Map;

/**
 * The class for creating VideoPlayer on Android platform.
 *
 * @since 1
 */
public class AceVideoPluginAosp extends AceVideoPluginBase {
    private static final String LOG_TAG = "AceVideoPluginAosp";

    private static final String KEY_TEXTURE = "texture";

    private final Context context;

    private String instanceName;

    private AceVideoPluginAosp(Context context, String instanceName) {
        this.context = context;
        this.instanceName = instanceName;
    }

    /**
     * This is called to create AceVideoAosp register.
     *
     * @param context the app context
     * @param name name of instance
     * @return video register plugin
     */
    public static AceVideoPluginAosp createRegister(Context context, String name) {
        return new AceVideoPluginAosp(context, name);
    }

    @Override
    public long create(Map<String, String> param) {
        try {
            long id = getAtomicId();
            AceVideoAosp video = new AceVideoAosp(id, instanceName, context, getEventCallback());
            addResource(id, video);
            return id;
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "NumberFormatException texture:" + param.get(KEY_TEXTURE));
        }
        return -1L;
    }
}