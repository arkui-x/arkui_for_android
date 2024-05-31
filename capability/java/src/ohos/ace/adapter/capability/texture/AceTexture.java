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

package ohos.ace.adapter.capability.texture;

import java.util.HashMap;
import java.util.Map;

import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.SurfaceHolder;

import ohos.ace.adapter.AceTextureHolder;
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
    private static final String TEXTURE_SET_TEXTURE_SIZE_KEY = "setTextureSize";
    private static final String TEXTURE_UPDATE_TEXTURE_IMAGE_KEY = "updateTextureImage";
    private static final String TEXTURE_ATTACH_TO_GL_CONTEXT_KEY = "attachToGLContext";
    private static final String TEXTURE_INIT_PARAM_KEY = "initParam";
    private static final String TEXTURE_REGISTER_SURFACE_VALUE = "registerSurface";
    private static final String TEXTURE_ID_KEY = "textureId";
    private static final String TEXTURE_IS_ATTACH = "isAttach";

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

    private int instanceId = -1;
    
    private float[] transform = new float[16];

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
     * @param initParam initialization parameters
     */
    public AceTexture(int instanceId, long id, IAceTexture textureImpl, IAceOnResourceEvent callback, Map<String, String> initParam) {
        this.surfaceTexture = new SurfaceTexture(0);
        this.surfaceTexture.detachFromGLContext();
        this.surfaceTexture.setOnFrameAvailableListener(onFrameListener);
        this.instanceId = instanceId;
        this.id = id;
        this.textureImpl = textureImpl;
        this.callback = callback;
        this.callMethodMap = new HashMap<String, IAceOnCallResourceMethod>();
        IAceOnCallResourceMethod callSetTextureSize = new IAceOnCallResourceMethod() {

            /**
             * Set the size of the texture
             * @param params size params
             * @return result of setting texture size
             */
            public String onCall(Map<String, String> param) {
                return setTextureSize(param);
            }
        };
        this.callMethodMap.put("texture@" + id + METHOD + PARAM_EQUALS + TEXTURE_SET_TEXTURE_SIZE_KEY +
            PARAM_BEGIN, callSetTextureSize);

        IAceOnCallResourceMethod callAttachToGLContext = new IAceOnCallResourceMethod() {

            /**
             * attatch to glcontext
             * @param params size params
             * @return result of attatch
             */
            public String onCall(Map<String, String> param) {
                return attachToGLContext(param);
            }
        };
        this.callMethodMap.put("texture@" + id + METHOD + PARAM_EQUALS + TEXTURE_ATTACH_TO_GL_CONTEXT_KEY +
            PARAM_BEGIN, callAttachToGLContext);

        IAceOnCallResourceMethod callUpdateTextureImage = new IAceOnCallResourceMethod() {

            /**
             * update texture image
             * @param params size params
             * @return result of update
             */
            public String onCall(Map<String, String> param) {
                return updateTextureImage(param);
            }
        };
        this.callMethodMap.put("texture@" + id + METHOD + PARAM_EQUALS + TEXTURE_UPDATE_TEXTURE_IMAGE_KEY +
            PARAM_BEGIN, callUpdateTextureImage);

        registerSurface();

        AceTextureHolder.addSurfaceTexture(id, surfaceTexture);
    }

    /**
     * Get the call method map.
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

        setDefaultBufferSize(textureWidth, textureHeight);

        return SUCCESS;
    }

    /**
     * Update texture image
     *
     * @param params size params
     * @return result of update
     */
    public String updateTextureImage(Map<String, String> params) {
        if (surfaceTexture == null) {
            ALog.e(LOG_TAG, "updateTextureImage surfaceTexture is null.");
            return FALSE;
        }

        surfaceTexture.updateTexImage();
        surfaceTexture.getTransformMatrix(transform);
        long timestamp = surfaceTexture.getTimestamp();
        if (timestamp == 0) {
            ALog.e(LOG_TAG, "updateTextureImage etimestamp " + timestamp);
        }
        String param = "transform=" + "[" + transform[0];
        for (int i = 1; i < transform.length; i++) {
            param += ",";
            param += transform[i];
        }
        param += "]";

        return param;
    }

    /**
     * Attach to glcontext.
     *
     * @param params size params
     * @return result of attach
     */
    public String attachToGLContext(Map<String, String> params) {
        ALog.i(LOG_TAG, "attachToGLContext start.");
        if (!params.containsKey(TEXTURE_ID_KEY)) {
            ALog.e(LOG_TAG, "attachToGLContext texture id is empty.");
            return FALSE;
        }

        if (surfaceTexture == null) {
            ALog.e(LOG_TAG, "attachToGLContext surfaceTexture is null.");
            return FALSE;
        }

        int texName = Integer.parseInt(params.get(TEXTURE_ID_KEY));
        if (Integer.parseInt(params.get(TEXTURE_IS_ATTACH)) == 1){
            surfaceTexture.attachToGLContext(texName);
        } else {
            surfaceTexture.detachFromGLContext();
        }
        ALog.i(LOG_TAG, "attachToGLContext end." + texName);

        return SUCCESS;
    }

    /**
     * Get the surface texture.
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
            ALog.i(LOG_TAG, "registerSurface, id:" + this.id);
            this.textureImpl.registerSurface(this.id, surface);
            hasRegisterSurface = true;
        }
    }

    /**
     * Get the surface.
     *
     * @return Surface
     */
    public Surface getSurface() {
        return new Surface(surfaceTexture);
    }

    /**
     * Get the resource ID.
     *
     * @return resource ID
     */
    public long getId() {
        return id;
    }

    /**
     * Release the surface.
     *
     */
    public void release() {
        surfaceTexture.setOnFrameAvailableListener(null);
        textureImpl.unregisterTexture(id);
        textureImpl.unregisterSurface(id);
        AceTextureHolder.removeSurfaceTexture(id);
        if (surface != null) {
            surface.release();
        }
        surfaceTexture.release();
    }

    /**
     * Set the surface texture. This method applies to SurfaceView but not NativeView.
     *
     */
    public void setSurfaceTexture() {
        this.textureImpl.registerTexture(this.id, surfaceTexture);
    }

    /**
     * Mark the texture frame. The logic differs between SurfaceView and NativeView.
     *
     */
    public void markTextureFrame() {
        if (!hasRegisterTexture) {
            setSurfaceTexture();
            hasRegisterTexture = true;
        }

        this.textureImpl.markTextureFrameAvailable(id);
        String param = "instanceId=" + instanceId + "&textureId=" + id;
        callback.onEvent(TEXTURE_FLAG + id + EVENT + PARAM_EQUALS + "markTextureAvailable" + PARAM_BEGIN, param);
    }
}