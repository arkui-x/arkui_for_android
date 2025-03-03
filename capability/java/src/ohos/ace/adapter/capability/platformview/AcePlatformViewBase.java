/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import android.view.MotionEvent;

import ohos.ace.adapter.IAceOnCallResourceMethod;
import ohos.ace.adapter.IAceOnResourceEvent;

import java.util.HashMap;
import java.util.Map;

/**
 * This class handles the lifecycle of a PlatformView.
 *
 * @since 1
 */
public abstract class AcePlatformViewBase {
    private static final String LOG_TAG = "AcePlatformViewBase";

    private static final String PARAM_AND = "#HWJS-&-#";

    private static final String PARAM_EQUALS = "#HWJS-=-#";

    private static final String PARAM_BEGIN = "#HWJS-?-#";

    private static final String METHOD = "method";

    private static final String EVENT = "event";

    private static final String PLATFORM_VIEW_FLAG = "platformview@";

    private final long id;

    private final IAceOnResourceEvent callback;

    private final Map<String, IAceOnCallResourceMethod> callMethodMap;

    /**
     * base constructor of AcePlatformViewBase
     *
     * @param id       id of plugin
     * @param callback resource callback
     */
    public AcePlatformViewBase(long id, IAceOnResourceEvent callback) {
        this.id = id;
        this.callback = callback;
        callMethodMap = new HashMap<>();

        IAceOnCallResourceMethod callcreate = new IAceOnCallResourceMethod() {
            /**
             * This is called to create texture.
             *
             * @param params calling param
             * @return result of calling
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {
                    /**
                     * This is called to create texture.
                     */
                    public void run() {
                        registerPlatformView(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(PLATFORM_VIEW_FLAG + id + METHOD + PARAM_EQUALS + "registerPlatformView" + PARAM_BEGIN,
                callcreate);

        IAceOnCallResourceMethod callUpdateLayout = new IAceOnCallResourceMethod() {
            /**
             * This is called to update layout.
             *
             * @param params calling param
             * @return result of calling
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {
                    /**
                     * This is called to update layout.
                     */
                    public void run() {
                        updateLayout(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(PLATFORM_VIEW_FLAG + id + METHOD + PARAM_EQUALS + "updateLayout" + PARAM_BEGIN,
                callUpdateLayout);

        IAceOnCallResourceMethod callTouchDown = (param) -> touchDown(param);
        this.callMethodMap.put(PLATFORM_VIEW_FLAG + id + METHOD +
                PARAM_EQUALS +
                "touchDown" +
                PARAM_BEGIN,
                callTouchDown);
        IAceOnCallResourceMethod callTouchUp = (param) -> touchUp(param);
        this.callMethodMap.put(PLATFORM_VIEW_FLAG + id + METHOD +
                PARAM_EQUALS +
                "touchUp" +
                PARAM_BEGIN,
                callTouchUp);
        IAceOnCallResourceMethod callTouchMove = (param) -> touchMove(param);
        this.callMethodMap.put(PLATFORM_VIEW_FLAG + id + METHOD +
                PARAM_EQUALS +
                "touchMove" +
                PARAM_BEGIN,
                callTouchMove);
        IAceOnCallResourceMethod callTouchCancel = (param) -> touchCancel(param);
        this.callMethodMap.put(PLATFORM_VIEW_FLAG + id + METHOD +
                PARAM_EQUALS +
                "touchCancel" +
                PARAM_BEGIN,
                callTouchCancel);
        IAceOnCallResourceMethod callDispose = (param) -> dispose(param);
        this.callMethodMap.put(PLATFORM_VIEW_FLAG + id + METHOD +
                PARAM_EQUALS +
                "dispose" +
                PARAM_BEGIN,
                callDispose);
    }

    /**
     * This is called to release.
     *
     */
    public abstract void release();

    /**
     * This is called to get resource call methods.
     *
     * @return Map of call method
     */
    public Map<String, IAceOnCallResourceMethod> getCallMethod() {
        return callMethodMap;
    }

    /**
     * This is called to create texture.
     *
     * @param params calling param
     * @return result of calling
     */
    public abstract String registerPlatformView(Map<String, String> params);

    /**
     * This is called to get id.
     *
     * @return resource id.
     */
    public long getId() {
        return id;
    }

    /**
     * This is called to get platform view ready.
     *
     */
    public void platformViewReady() {
        callback.onEvent(PLATFORM_VIEW_FLAG + id + EVENT + PARAM_EQUALS + "platformViewReady" + PARAM_BEGIN, "");
    }

    /**
     * This is called to offset.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String updateLayout(Map<String, String> params);

    /**
     * This is called to set touch event.
     *
     * @param event is touch event.
     */
    public abstract void setTouchEvent(MotionEvent event);

    /**
     * touch down.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String touchDown(Map<String, String> params);

    /**
     * touch up.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String touchUp(Map<String, String> params);

    /**
     * touch move.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String touchMove(Map<String, String> params);

    /**
     * touch cancel.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String touchCancel(Map<String, String> params);

    /**
     * dispose.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String dispose(Map<String, String> params);

    /**
     * This is called for activity resume.
     *
     */
    public abstract void onActivityResume();

    /**
     * This is called for activity pause.
     *
     */
    public abstract void onActivityPause();

    /**
     * This is called to async runnable.
     *
     * @param runnable runnable.
     * @return Result.
     */
    public abstract String runAsync(Runnable runnable);

    /**
     * InnerProcessor.
     */
    public abstract class InnerProcessor implements Runnable {
        Map<String, String> param;

        /**
         * InnerProcessor to keep calling param value to member var :param;
         *
         * @param param calling param
         */
        public InnerProcessor(Map<String, String> param) {
            this.param = param;
        }
    };
}