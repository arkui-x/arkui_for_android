/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.platformview;
import android.content.Context;

import ohos.ace.adapter.capability.platformview.IPlatformView;
import ohos.ace.adapter.capability.platformview.PlatformViewFactory;
import ohos.ace.adapter.ALog;

import java.util.Map;
import java.util.HashMap;
/**
 * The class for creating  PlatformView on Android platform.
 *
 * @since 1
 */
public class AcePlatformViewPluginAosp extends AcePlatformViewPluginBase {
    private static final String LOG_TAG = "AcePlatformViewPluginAosp";
    private static final String KEY_TEXTURE = "texture";
    private static final String KEY_VIEWTAG = "viewTag";
    private static final String KEY_DATATAG = "dataTag";
    private static final String KEY_DATAEMPTYTAG = "dataEmptyTag";
    private static final int IS_EMPTY = 1;

    PlatformViewFactory platformViewFactory;

    private final Context context;

    private Map<String, IPlatformView> platformViewMap;

    private AcePlatformViewPluginAosp(Context context) {
        this.context = context;
        this.platformViewMap = new HashMap<String, IPlatformView>();
    }

    /**
     * This is called to create AcePlatformViewAosp register.
     *
     * @param context the app context
     * @param name name of instance
     * @return PlatformView register plugin
     */
    public static AcePlatformViewPluginAosp createRegister(Context context) {
        return new AcePlatformViewPluginAosp(context);
    }

    @Override
    public long create(Map<String, String> param) {
        try {
            if (!param.containsKey(KEY_VIEWTAG)) {
                ALog.e(LOG_TAG, "createTexture failed: Viewtag is illegal");
                return -1L;
            }
            String viewTag = param.get(KEY_VIEWTAG);
            IPlatformView platformView;
            if (param.containsKey(KEY_DATAEMPTYTAG)) {
                boolean isDataEmpty = Integer.parseInt(param.get(KEY_DATAEMPTYTAG)) == IS_EMPTY;
                platformView = this.platformViewFactory.getPlatformView(viewTag,
                        isDataEmpty ? "" : param.get(KEY_DATATAG));
            } else {
                platformView = this.platformViewFactory.getPlatformView(viewTag);
            }
            if (platformView == null) {
                ALog.e(LOG_TAG, "PlatformView == null." + viewTag);
                return -1L;
            }
            long id = getAtomicId();
            AcePlatformViewAosp acePlatformView = new AcePlatformViewAosp(id, context, getEventCallback());
            acePlatformView.setPlatformView(platformView);
            addResource(id, acePlatformView);
            return id;
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "NumberFormatException texture:" + param.get(KEY_TEXTURE));
        }
        ALog.e(LOG_TAG, "Create fail.");
        return -1L;
    }

    @Override
    public void registerPlatformViewFactory(PlatformViewFactory platformViewFactory) {
        this.platformViewFactory = platformViewFactory;
    }
}