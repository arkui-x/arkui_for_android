/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.texture;

import ohos.ace.adapter.AceResourcePlugin;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

/**
 * The class for creating Texture on Android platform.
 *
 * @since 1
 */
public class AceTexturePluginAosp extends AceResourcePlugin {
    private static final String LOG_TAG = "AceTexturePluginAosp";
    private static final AtomicLong NEXT_TEXTURE_ID = new AtomicLong(0L);

    private final IAceTexture textureImpl;

    private final Map<Long, AceTexture> objectMap;

    private int instanceId = -1;

    private AceTexturePluginAosp(int instanceId, IAceTexture textureImpl) {
        // plugin name is texture, version is 1.0.
        super("texture", 1.0f);
        this.instanceId = instanceId;
        this.textureImpl = textureImpl;
        this.objectMap = new HashMap<Long, AceTexture>();
    }

    /**
     * Create a texture resource register.
     *
     * @param instanceId instance id
     * @param textureImpl interface for texture interaction with the engine.
     * @return texture plugin
     */
    public static AceTexturePluginAosp createRegister(int instanceId, IAceTexture textureImpl) {
        return new AceTexturePluginAosp(instanceId, textureImpl);
    }

    /**
     * Create a texture.
     *
     * @param param calling param
     * @return texture id
     */
    @Override
    public long create(Map<String, String> param) {
        AceTexture aceTexture = new AceTexture(instanceId, NEXT_TEXTURE_ID.get(), textureImpl,
                                                getEventCallback(), param);
        objectMap.put(NEXT_TEXTURE_ID.get(), aceTexture);
        registerCallMethod(aceTexture.getCallMethod());
        return NEXT_TEXTURE_ID.getAndIncrement();
    }

    /**
     * Get AceTexture object by id.
     *
     * @param id id of object
     * @return object or null if id not found
     */
    @Override
    public Object getObject(long id) {
        return objectMap.get(id);
    }

    /**
     * Release AceTexture by id.
     *
     * @param id id of object
     * @return result of release
     */
    @Override
    public boolean release(long id) {
        if (objectMap.containsKey(id)) {
            AceTexture aceTexture = objectMap.get(id);
            unregisterCallMethod(aceTexture.getCallMethod());
            aceTexture.release();
            objectMap.remove(id);
            return true;
        }
        return false;
    }

    /**
     * Release all AceTexture objects.
     *
     */
    @Override
    public void release() {
        for (Map.Entry<Long, AceTexture> entry : objectMap.entrySet()) {
            entry.getValue().release();
        }
        objectMap.clear();
    }
}