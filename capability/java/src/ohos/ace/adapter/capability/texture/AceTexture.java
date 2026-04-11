/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
    private static final String TEXTURE_SET_TEXTURE_SIZE_KEY = "setTextureSize";
    private static final String TEXTURE_UPDATE_TEXTURE_IMAGE_KEY = "updateTextureImage";
    private static final String TEXTURE_ATTACH_TO_GL_CONTEXT_KEY = "attachToGLContext";
    private static final String TEXTURE_GET_TEXTURE_ID_KEY = "getTextureId";
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
    protected final IAceSurfaceTexture surfaceTexture;

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

    private boolean useImageReaderMode = false;

    private IAceSurfaceTexture.OnFrameAvailableListener onFrameListener =
        new IAceSurfaceTexture.OnFrameAvailableListener() {
        @Override
        public void onFrameAvailable(IAceSurfaceTexture texture) {
            markTextureFrame();
        }
    };

    /**
     * constructor of AceTexture
     *
     * @param instanceId instance id
     * @param id id of texture
     * @param textureImpl texture object
     * @param callback resource callback
     * @param initParam initialization parameters
     */
    public AceTexture(int instanceId, long id, IAceTexture textureImpl, IAceOnResourceEvent callback,
            Map<String, String> initParam) {
        this.instanceId = instanceId;
        this.id = id;
        this.textureImpl = textureImpl;
        this.callback = callback;
        this.callMethodMap = new HashMap<String, IAceOnCallResourceMethod>();
        registerCallMethods();
        if (getTextureMode(initParam)) {
            ALog.i(LOG_TAG, "AceTexture: Using ImageReader mode for textureId=" + id);
            surfaceTexture = new AceImageTexture(instanceId, id, textureImpl);
        } else {
            ALog.i(LOG_TAG, "AceTexture: Using SurfaceTexture mode for textureId=" + id);
            this.surfaceTexture = new AceSurfaceTexture(instanceId, id, textureImpl);
        }
        surfaceTexture.setOnFrameAvailableListener(onFrameListener);
    }

    private boolean getTextureMode(Map<String, String> initParam) {
        if (initParam != null && initParam.containsKey("useImageReader")) {
            String useImageReader = initParam.get("useImageReader");
            useImageReaderMode = "true".equalsIgnoreCase(useImageReader);
        }
        return useImageReaderMode;
    }

    private void registerCallMethods() {
        IAceOnCallResourceMethod callSetTextureSize = new IAceOnCallResourceMethod() {
            /**
             * Set the size of the texture
             *
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
             *
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
             *
             * @param params size params
             * @return result of update
             */
            public String onCall(Map<String, String> param) {
                return updateTextureImage(param);
            }
        };
        this.callMethodMap.put("texture@" + id + METHOD + PARAM_EQUALS + TEXTURE_UPDATE_TEXTURE_IMAGE_KEY +
            PARAM_BEGIN, callUpdateTextureImage);

        IAceOnCallResourceMethod callAttachNativeWindow = new IAceOnCallResourceMethod() {
            /**
             * Attach native window
             *
             * @param param params
             * @return result of attach native window.
             */
            public String onCall(Map<String, String> param) {
                return attachNativeWindow(param);
            }
        };

        this.callMethodMap.put("texture@" + id + METHOD + PARAM_EQUALS + "attachNativeWindow" + PARAM_BEGIN,
                callAttachNativeWindow);
        IAceOnCallResourceMethod callGetTextureId = new IAceOnCallResourceMethod() {
            /**
             * get texture id
             *
             * @param params size params
             * @return result of get texture id
             */
            public String onCall(Map<String, String> param) {
                return getTextureId(param);
            }
        };
        this.callMethodMap.put("texture@" + id + METHOD + PARAM_EQUALS + TEXTURE_GET_TEXTURE_ID_KEY +
                PARAM_BEGIN, callGetTextureId);
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
     * Retrieves the texture ID from the current SurfaceTexture instance.
     *
     * @param params A map of parameters (currently unused).
     * @return A string in the format "textureId:{id}" containing the texture ID,
     *         or an empty string if the SurfaceTexture is null.
     */
    public String getTextureId(Map<String, String> params) {
        if (surfaceTexture == null) {
            ALog.e(LOG_TAG, "surfaceTexture is null.");
            return "";
        }

        long textureId = surfaceTexture.getTextureId();
        return "textureId:" + textureId;
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

        surfaceTexture.setTextureSize(textureWidth, textureHeight);
        String param = "textureWidth=" + textureWidth + "&textureHeight=" + textureHeight;
        callback.onEvent(TEXTURE_FLAG + id + EVENT + PARAM_EQUALS + "onChanged" + PARAM_BEGIN, param);
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
        transform = surfaceTexture.getTransformMatrix();
        if (transform == null || transform.length < 16) {
            return FALSE;
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
        if (Integer.parseInt(params.get(TEXTURE_IS_ATTACH)) == 1) {
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
        if (surfaceTexture == null) {
            return null;
        }
        return surfaceTexture.getTexture();
    }

    /**
     * Attach the native window.
     *
     * @param params params
     * @return result of attach the native window.
     */
    private String attachNativeWindow(Map<String, String> params) {
        ALog.d(LOG_TAG, "attachNativeWindow called.");
        Surface surface = getSurface();
        if (surface == null) {
            ALog.e(LOG_TAG, "surface is null, attachNativeWindow failed");
            return FALSE;
        }
        long nativeWindow = textureImpl.attachNaitveSurface(surface);
        ALog.d(LOG_TAG, "Surface attach:" + nativeWindow);
        return "nativeWindow=" + nativeWindow;
    }

    /**
     * Get the surface.
     *
     * @return Surface
     */
    public Surface getSurface() {
        if (surfaceTexture == null) {
            return null;
        }
        return surfaceTexture.getSurface();
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
        surfaceTexture.release();
    }

    /**
     * Mark the texture frame. The logic differs between SurfaceView and NativeView.
     *
     */
    public void markTextureFrame() {
        this.textureImpl.markTextureFrameAvailable(id);
        String param = "instanceId=" + instanceId + "&textureId=" + id;
        callback.onEvent(TEXTURE_FLAG + id + EVENT + PARAM_EQUALS + "markTextureAvailable" + PARAM_BEGIN, param);
    }
}