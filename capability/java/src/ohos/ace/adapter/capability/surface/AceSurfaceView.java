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

package ohos.ace.adapter.capability.surface;

import java.util.HashMap;
import java.util.Map;

import android.app.Activity;
import android.content.Context;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import ohos.ace.adapter.AceSurfaceHolder;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.IAceOnCallResourceMethod;
import ohos.ace.adapter.IAceOnResourceEvent;

/**
 * This class handles the lifecycle of a surface texture.
 *
 * @since 1
 */
public class AceSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    private static final String LOG_TAG = "AceSurfaceView";

    private static final String SUCCESS = "success";
    private static final String FALSE = "false";

    private static final String PARAM_EQUALS = "#HWJS-=-#";
    private static final String PARAM_BEGIN = "#HWJS-?-#";
    private static final String METHOD = "method";
    private static final String EVENT = "event";
    private static final String SURFACE_FLAG = "surface@";

    private static final String SURFACE_LEFT_KEY = "surfaceLeft";
    private static final String SURFACE_TOP_KEY = "surfaceTop";
    private static final String SURFACE_WIDTH_KEY = "surfaceWidth";
    private static final String SURFACE_HEIGHT_KEY = "surfaceHeight";
    private static final String SURFACE_SET_BOUNDS = "setSurfaceBounds";

    /**
     * surface.
     */
    protected Surface surface = null;

    /**
     * id.
     */
    protected final long id;

    private final IAceOnResourceEvent callback;

    private Map<String, IAceOnCallResourceMethod> callMethodMap;

    private Context context;

    private int surfaceLeft = 0;
    private int surfaceTop = 0;
    private int surfaceWidth = 0;
    private int surfaceHeight = 0;

    private boolean viewAdded = false;

    /**
     * constructor of AceSurfaceView
     *
     * @param context   context of host activity
     * @param id        id of surface
     * @param callback  resource callback
     * @param initParam initialization parameters
     */
    public AceSurfaceView(Context context, long id, IAceOnResourceEvent callback, Map<String, String> initParam) {
        super(context);
        this.id = id;
        this.callback = callback;
        this.callMethodMap = new HashMap<String, IAceOnCallResourceMethod>();
        this.context = context;
        getHolder().addCallback(this);

        IAceOnCallResourceMethod callSetSurfaceSize = (param) -> setSurfaceBounds(param);

        this.callMethodMap.put("surface@" + id + METHOD + PARAM_EQUALS + "setSurfaceBounds" + PARAM_BEGIN,
                callSetSurfaceSize);
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
    public String setSurfaceBounds(Map<String, String> params) {
        if (!params.containsKey(SURFACE_WIDTH_KEY) || !params.containsKey(SURFACE_HEIGHT_KEY)) {
            return FALSE;
        }
        try {
            surfaceLeft = Integer.parseInt(params.get(SURFACE_LEFT_KEY));
            surfaceTop = Integer.parseInt(params.get(SURFACE_TOP_KEY));
            surfaceWidth = Integer.parseInt(params.get(SURFACE_WIDTH_KEY));
            surfaceHeight = Integer.parseInt(params.get(SURFACE_HEIGHT_KEY));
            ALog.i(LOG_TAG, "setSurfaceBounds (" + surfaceLeft + ", " + surfaceTop + ") - (" + surfaceWidth + " x "
                    + surfaceHeight + ")");
            FrameLayout.LayoutParams layoutParams = buildLayoutParams(surfaceLeft, surfaceTop, surfaceWidth,
                    surfaceHeight);
            if (viewAdded) {
                this.setLayoutParams(layoutParams);
                this.invalidate();
            } else {
                Activity activity = (Activity) context;
                activity.addContentView(this, layoutParams);
                viewAdded = true;
                ALog.i(LOG_TAG, "AceSurfaceView added");
            }
        } catch (NumberFormatException e) {
            ALog.e(LOG_TAG, "NumberFormatException, setSurfaceSize failed");
            return FALSE;
        }
        return SUCCESS;
    }

    /**
     * Get the surface.
     *
     * @return Surface
     */
    public Surface getSurface() {
        return surface;
    }

    /**
     * Get the resource ID.
     *
     * @return resource ID
     */
    public long getResId() {
        return id;
    }

    /**
     * Release the surface.
     *
     */
    public void release() {
        if (viewAdded) {
            if (getParent() instanceof ViewGroup) {
                ((ViewGroup) getParent()).removeView(this);
            }
            viewAdded = false;
            ALog.i(LOG_TAG, "AceSurfaceView removed");
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        ALog.i(LOG_TAG, "Surface Created");
        surface = holder.getSurface();
        callback.onEvent(SURFACE_FLAG + id + EVENT + PARAM_EQUALS + "onCreate" + PARAM_BEGIN, "");
        AceSurfaceHolder.addSurface(id, surface);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        surfaceWidth = width;
        surfaceHeight = height;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        surface = null;
        AceSurfaceHolder.removeSurface(id);
    }

    private FrameLayout.LayoutParams buildLayoutParams(int left, int top, int width, int height) {
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(width, height);
        params.setMargins(left, top, 0, 0);
        return params;
    }
}
