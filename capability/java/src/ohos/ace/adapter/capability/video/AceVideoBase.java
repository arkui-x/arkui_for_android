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

package ohos.ace.adapter.capability.video;

import ohos.ace.adapter.ALog;
import ohos.ace.adapter.IAceOnCallResourceMethod;
import ohos.ace.adapter.IAceOnResourceEvent;

import java.util.HashMap;
import java.util.Map;

/**
 * This class handles the lifecycle of a media player.
 *
 * @since 1
 */
public abstract class AceVideoBase {
    private static final String LOG_TAG = "AceVideoBase";

    private static final String PARAM_AND = "#HWJS-&-#";

    private static final String PARAM_EQUALS = "#HWJS-=-#";

    private static final String PARAM_BEGIN = "#HWJS-?-#";

    private static final String METHOD = "method";

    private static final String EVENT = "event";

    private static final String VIDEO_FLAG = "video@";

    private final long id;

    private final IAceOnResourceEvent callback;

    private boolean isAutoPlay;

    private boolean isMute;

    private float speed;

    private boolean isLooping;

    private final Map<String, IAceOnCallResourceMethod> callMethodMap;

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

    /**
     * base constructor of AceVideo
     *
     * @param id id of plugin
     * @param callback resource callback
     */
    public AceVideoBase(long id, IAceOnResourceEvent callback) {
        this.id = id;
        this.isAutoPlay = false;
        this.isMute = false;
        this.isLooping = false;
        speed = 1.0f;
        this.callback = callback;
        callMethodMap = new HashMap<>();

        IAceOnCallResourceMethod callInit = new IAceOnCallResourceMethod() {

            /**
             * This is called to init media player.
             *
             * @param params calling param
             * @return result of calling
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {

                    /**
                     * This is called to init media player.
                     */
                    public void run() {
                        initMediaPlayer(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "init" + PARAM_BEGIN, callInit);

        IAceOnCallResourceMethod callStart = new IAceOnCallResourceMethod() {

            /**
             * This is called to start player.
             *
             * @param param
             * @return unused
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {

                    /**
                     * This is called to start player.
                     */
                    public void run() {
                        start(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "start" + PARAM_BEGIN, callStart);

        IAceOnCallResourceMethod callPause = new IAceOnCallResourceMethod() {

            /**
             * This is called to pause player.
             *
             * @param param
             * @return unused
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {

                    /**
                     * This is called to pause player.
                     */
                    public void run() {
                        pause(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "pause" + PARAM_BEGIN, callPause);

        IAceOnCallResourceMethod callStop = (param) -> runAsync(() -> {
            stop(param);
        });
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "stop" + PARAM_BEGIN, callStop);

        IAceOnCallResourceMethod callGetPosition = new IAceOnCallResourceMethod() {

            /**
             * This is called to get video position.
             *
             * @param param
             * @return unused
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {

                    /**
                     * This is called to get video position.
                     */
                    public void run() {
                        getPosition(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "getposition" + PARAM_BEGIN, callGetPosition);

        IAceOnCallResourceMethod callSeekTo = new IAceOnCallResourceMethod() {

            /**
             * This is called to seek video.
             *
             * @param param
             * @return unused
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {

                    /**
                     * This is called to seek video.
                     */
                    public void run() {
                        seekTo(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "seekto" + PARAM_BEGIN, callSeekTo);

        IAceOnCallResourceMethod setVolume = new IAceOnCallResourceMethod() {

            /**
             * This is called to set volume.
             *
             * @param param
             * @return unused
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {

                    /**
                     * This is called to set volume.
                     */
                    public void run() {
                        setVolume(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "setvolume" + PARAM_BEGIN, setVolume);

        IAceOnCallResourceMethod enableLooping = new IAceOnCallResourceMethod() {

            /**
             * This is called to enable single looping.
             *
             * @param param
             * @return unused
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {

                    /**
                     * This is called to enable single looping.
                     */
                    public void run() {
                        enableLooping(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "enablelooping" + PARAM_BEGIN, enableLooping);

        IAceOnCallResourceMethod setSpeed = new IAceOnCallResourceMethod() {

            /**
             * This is called to set video speed.
             *
             * @param param
             * @return unused
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {

                    /**
                     * This is called to set video speed.
                     */
                    public void run() {
                        setSpeed(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "setspeed" + PARAM_BEGIN, setSpeed);

        IAceOnCallResourceMethod setDirection = new IAceOnCallResourceMethod() {

            /**
             * This is called to set video direction.
             *
             * @param param
             * @return unused
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {

                    /**
                     * This is called to set video direction.
                     */
                    public void run() {
                        setDirection(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "setdirection" + PARAM_BEGIN, setDirection);

        IAceOnCallResourceMethod setLandscape = new IAceOnCallResourceMethod() {

            /**
             * This is called to set video direction LANDSCAPE.
             *
             * @param param
             * @return unused
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {

                    /**
                     * This is called to set video direction LANDSCAPE.
                     */
                    public void run() {
                        setLandscape(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "setlandscape" + PARAM_BEGIN, setLandscape);

        IAceOnCallResourceMethod callSetSurface = new IAceOnCallResourceMethod() {

            /**
             * This is called to set video surface
             *
             * @param param
             * @return unused
             */
            public String onCall(Map<String, String> param) {
                runAsync(new InnerProcessor(param) {

                    /**
                     * This is called to set video surface
                     */
                    public void run() {
                        setSurface(param);
                    }
                });
                return "";
            }
        };
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "setsurface" + PARAM_BEGIN, callSetSurface);

        IAceOnCallResourceMethod callUpdateSrc = (param) -> runAsync(() -> {
            updateResource(param);
        });
        callMethodMap.put(VIDEO_FLAG + id + METHOD + PARAM_EQUALS + "updateresource" + PARAM_BEGIN, callUpdateSrc);
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
     * This is called to init media player.
     *
     * @param params calling param
     * @return result of calling
     */
    public String initMediaPlayer(Map<String, String> params) {
        try {
            if (params.containsKey("mute")) {
                if (Integer.parseInt(params.get("mute")) == 1) {
                    isMute = true;
                }
            }

            if (params.containsKey("autoplay")) {
                if (Integer.parseInt(params.get("autoplay")) == 1) {
                    isAutoPlay = true;
                }
            }

            if (params.containsKey("speed")) {
                speed = Float.parseFloat(params.get("speed"));
            }
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "NumberFormatException");
            return "fail";
        }
        return "success";
    }

    /**
     * This is called to get id.
     *
     * @return resource id.
     */
    public long getId() {
        return id;
    }

    /**
     * This is called to get autoplay.
     *
     * @return is video autoplay.
     */
    public boolean isAutoPlay() {
        return isAutoPlay;
    }

    /**
     * This is called to get mute.
     *
     * @return is video mute.
     */
    public boolean isMute() {
        return isMute;
    }

    /**
     * set mute.
     *
     * @param params is video mute.
     */
    public void setIsMute(boolean isMute) {
        this.isMute = isMute;
    }

    /**
     * This is called to get speed.
     *
     * @return is video speed.
     */
    public float getSpeed() {
        return speed;
    }

    /**
     * This is called to set speed.
     *
     * @param is video speed.
     */
    public void setSpeed(float speed) {
        this.speed = speed;
    }

    /**
     * This is called to get looping.
     *
     * @return is video looping.
     */
    public boolean isLooping() {
        return isLooping;
    }

    /**
     * This is called to set looping.
     *
     * @param is video looping.
     */
    public void setLooping(boolean isLooping) {
        this.isLooping = isLooping;
    }

    /**
     * This is called to start player.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String start(Map<String, String> params);

    /**
     * This is called to pause player.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String pause(Map<String, String> params);

    /**
     * This is called to stop player.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String stop(Map<String, String> params);

    /**
     * This is called to seek video.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String seekTo(Map<String, String> params);

    /**
     * This is called to set volume.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String setVolume(Map<String, String> params);

    /**
     * This is called to get video position.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String getPosition(Map<String, String> params);

    /**
     * This is called to enable single looping.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String enableLooping(Map<String, String> params);

    /**
     * This is called to set video speed.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String setSpeed(Map<String, String> params);

    /**
     * This is called to set video direction.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String setDirection(Map<String, String> params);

    /**
     * This is called to set video direction LANDSCAPE.
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String setLandscape(Map<String, String> params);

    /**
     * This is called to set video surface
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String setSurface(Map<String, String> params);

    /**
     * This is called to update video resource
     *
     * @param params is param map.
     * @return result of call.
     */
    public abstract String updateResource(Map<String, String> params);

    /**
     * This is called to fire prepared event.
     *
     * @param width width of video.
     * @param height height of video.
     * @param duration duration of video.
     * @param isPlaying is video playing.
     * @param needRefreshForce need refresh every frame.
     */
    public void firePrepared(int width, int height, int duration, boolean isPlaying, boolean needRefreshForce) {
        String param = "width=" + width + "&height=" + height + "&duration=" +
            duration + "&isplaying=" + (isPlaying ? 1 : 0) + "&needRefreshForce=" + (needRefreshForce ? 1 : 0);
        callback.onEvent(VIDEO_FLAG + id + EVENT + PARAM_EQUALS + "prepared" + PARAM_BEGIN, param);
    }

    /**
     * This is called to fire error event.
     *
     */
    public void fireError() {
        callback.onEvent(VIDEO_FLAG + id + EVENT + PARAM_EQUALS + "error" + PARAM_BEGIN, "");
    }

    /**
     * This is called to fire completion event.
     */
    public void fireCompletion() {
        callback.onEvent(VIDEO_FLAG + id + EVENT + PARAM_EQUALS + "completion" + PARAM_BEGIN, "");
    }

    /**
     * This is called to fire seek complete event.
     *
     * @param position video current time.
     */
    public void fireSeekComplete(int position) {
        callback.onEvent(VIDEO_FLAG + id + EVENT + PARAM_EQUALS + "seekcomplete" + PARAM_BEGIN,
            "currentpos=" + position);
    }

    /**
     * This is called to fire buffering update event.
     *
     * @param percent loading percent.
     */
    public void fireBufferingUpdate(int percent) {
        callback.onEvent(VIDEO_FLAG + id + EVENT + PARAM_EQUALS + "bufferingupdate" + PARAM_BEGIN,
            "percent=" + percent);
    }

    /**
     * This is called to fire play status change event.
     *
     * @param isPlaying is video playing.
     */
    public void firePlayStatusChange(boolean isPlaying) {
        callback.onEvent(VIDEO_FLAG + id + EVENT + PARAM_EQUALS + "onplaystatus" + PARAM_BEGIN,
            "isplaying=" + (isPlaying ? 1 : 0));
    }

    /**
     * This is called to fire play currenttime.
     *
     * @param value currenttime.
     */
    public void fireGetCurrenttime(int value) {
        callback.onEvent(VIDEO_FLAG + id + EVENT + PARAM_EQUALS + "ongetcurrenttime" + PARAM_BEGIN,
            "currentpos=" + value);
    }

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
     * @param Runnable runnable.
     * @return Result.
     */
    public abstract String runAsync(Runnable runnable);
}