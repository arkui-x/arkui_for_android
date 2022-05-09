/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

import java.util.HashMap;
import java.util.Map;

import android.graphics.SurfaceTexture;
import android.view.Surface;

import ohos.ace.adapter.ALog;
import ohos.ace.adapter.IAceOnCallResourceMethod;
import ohos.ace.adapter.IAceOnResourceEvent;

/**
 * This class handles the lifecycle of a surface texture.
 *
 * @since 1
 */
public class AceTexture {
    private static final String LOG_TAG = "AceTexture";

    private static final String SUCCESS = "success";
    private static final String FALSE = "false";

    private static final String PARAM_EQUALS = "#HWJS-=-#";
    private static final String PARAM_BEGIN = "#HWJS-?-#";
    private static final String METHOD = "method";
    private static final String EVENT = "event";
    private static final String TEXTURE_FLAG = "texture@";

    private static final String TEXTURE_WIDTH_KEY = "textureWidth";
    private static final String TEXTURE_HEIGHT_KEY = "textureHeight";
    private static final String TEXTURE_SET_DEFAULT_SIZE_KEY = "setDefaultSize";
    private static final String TEXTURE_INIT_PARAM_KEY = "initParam";
    private static final String TEXTURE_REGISTER_SURFACE_VALUE = "registerSurface";

    /**
     * IAceTexture.
     */
    protected final IAceTexture textureImpl;

    /**
     * SurfaceTexture.
     */
    protected final SurfaceTexture surfaceTexture;

    /**
     * surface.
     */
    protected Surface surface = null;

    /**
     * hasRegisterSurface.
     */
    protected boolean hasRegisterSurface = false;

    /**
     * id.
     */
    protected final long id;

    private final IAceOnResourceEvent callback;

    private boolean hasRegisterTexture = false;

    private Map<String, IAceOnCallResourceMethod> callMethodMap;

    private int textureWidth = 0;

    private int textureHeight = 0;

    private SurfaceTexture.OnFrameAvailableListener onFrameListener = new SurfaceTexture.OnFrameAvailableListener() {
        @Override
        public void onFrameAvailable(SurfaceTexture texture) {
            markTextureFrame();
        }
    };

    /**
     * constructor of AceTexture
     * 
     * @param id id of texture
     * @param textureImpl texture object
     * @param callback resource callback
     * @param initParam initialize parameters
     */
    public AceTexture(long id, IAceTexture textureImpl, IAceOnResourceEvent callback, Map<String, String> initParam) {
        this.surfaceTexture = new SurfaceTexture(0);
        this.surfaceTexture.detachFromGLContext();
        this.surfaceTexture.setOnFrameAvailableListener(onFrameListener);

        this.id = id;
        this.textureImpl = textureImpl;
        this.callback = callback;
        this.callMethodMap = new HashMap<String, IAceOnCallResourceMethod>();
        IAceOnCallResourceMethod callSetTextureSize = (param) -> setTextureSize(param);
        this.callMethodMap.put("texture@" + id + METHOD + PARAM_EQUALS + "setTextureSize" + PARAM_BEGIN, callSetTextureSize);

        if (initParam.containsKey(TEXTURE_INIT_PARAM_KEY)
                && initParam.get(TEXTURE_INIT_PARAM_KEY).equals(TEXTURE_REGISTER_SURFACE_VALUE)) {
            registerSurface();
        }
    }

    /**
     * This is called to get call MethodMap
     *
     * @return Map
     */
    public Map<String, IAceOnCallResourceMethod> getCallMethod() {
        return callMethodMap;
    }

    /**
     * Set the size of the texture
     *
     * @param params size params
     * @return result of setting texture size
     */
    public String setTextureSize(Map<String, String> params) {
        if (!params.containsKey(TEXTURE_WIDTH_KEY) || !params.containsKey(TEXTURE_HEIGHT_KEY)) {
            return FALSE;
        }
        try {
            textureWidth = Integer.parseInt(params.get(TEXTURE_WIDTH_KEY));
            textureHeight = Integer.parseInt(params.get(TEXTURE_HEIGHT_KEY));
        } catch (NumberFormatException e) {
            ALog.e(LOG_TAG, "NumberFormatException, setTextureSize failed");
            return FALSE;
        }

        if (params.containsKey(TEXTURE_SET_DEFAULT_SIZE_KEY)) {
            setDefaultBufferSize(textureWidth, textureHeight);
        }
        return SUCCESS;
    }

    /**
     * This is called to get surface texture.
     *
     * @return SurfaceTexture
     */
    public SurfaceTexture getSurfaceTexture() {
        return surfaceTexture;
    }

    /**
     * Set default buffer size.
     *
     * @param width The surface width.
     * @param height The surface height.
     */
    public void setDefaultBufferSize(int width, int height) {
        if (this.surfaceTexture != null) {
            this.surfaceTexture.setDefaultBufferSize(width, height);
        }
    }

    /**
     * Create and register new surface
     * 
     */
    public void registerSurface() {
        if (surface == null) {
            surface = new Surface(this.surfaceTexture);
        }

        if (!hasRegisterSurface) {
            this.textureImpl.registerSurface(this.id, surface);
            hasRegisterSurface = true;
        }
    }

    /**
     * This is called to get surface.
     *
     * @return Surface
     */
    public Surface getSurface() {
        return new Surface(surfaceTexture);
    }

    /**
     * This is called to get resource id.
     *
     * @return resource id
     */
    public long getId() {
        return id;
    }

    /**
     * This is called to release.
     *
     */
    public void release() {
        surfaceTexture.setOnFrameAvailableListener(null);
        textureImpl.unregisterTexture(id);
        textureImpl.unregisterSurface(id);
        if (surface != null) {
            surface.release();
        }
        surfaceTexture.release();
    }

    /**
     * SurfaceView do it and nativeView not.
     *
     */
    public void setSurfaceTexture() {
        this.textureImpl.registerTexture(this.id, surfaceTexture);
    }

    /**
     * There is a different logic between SurfaceView and NativeView.
     *
     */
    public void markTextureFrame() {
        if (!hasRegisterTexture) {
            setSurfaceTexture();
            hasRegisterTexture = true;
        }

        this.textureImpl.markTextureFrameAvailable(id);
        callback.onEvent(TEXTURE_FLAG + id + EVENT + PARAM_EQUALS + "markTextureFrameAvailable" + PARAM_BEGIN, "");
    }
}

