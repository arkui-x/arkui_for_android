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

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.media.AudioAttributes;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.view.Surface;
import android.view.Window;
import android.view.WindowManager;

import ohos.ace.adapter.AceSurfaceHolder;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.IAceOnResourceEvent;

import java.io.File;
import java.io.IOException;
import java.util.Map;

/**
 * This class handles the lifecycle of a media player.
 *
 * @since 1
 */
public class AceVideoAosp extends AceVideoBase
        implements MediaPlayer.OnPreparedListener, MediaPlayer.OnErrorListener, MediaPlayer.OnSeekCompleteListener,
        MediaPlayer.OnCompletionListener, MediaPlayer.OnBufferingUpdateListener {
    private static final String LOG_TAG = "AceVideoAosp";

    private static final String SUCCESS = "success";

    private static final String FAIL = "fail";

    private static final String KEY_SOURCE = "src";

    private static final String KEY_VALUE = "value";

    private static final String FILE_SCHEME = "file://";

    private static final String HAP_SCHEME = "/";

    private static final int SECOND_TO_MSEC = 1000;

    private static final AudioAttributes ATTR_VIDEO = new AudioAttributes.Builder().setUsage(
            AudioAttributes.USAGE_MEDIA).setContentType(AudioAttributes.CONTENT_TYPE_MOVIE).build();

    private volatile MediaPlayer mediaPlayer;

    private final Context context;

    private final String instanceName;

    private final Window window;

    private final Handler mainHandler;

    private Handler asyncHandler;

    private HandlerThread handlerThread;

    private boolean stageMode = true;

    private boolean isSpeedChanged = false;

    private boolean isSetSurfaced = false;

    private String source;

    private long surfaceId = 0L;

    private int position = 0;

    private boolean isResumePlaying = false;

    private boolean isNeedResume = false;

    private PlayState state = PlayState.IDLE;

    public enum PlayState {
        IDLE,
        PREPARED,
        STARTED,
        PAUSED,
        STOPPED,
        PLAYBACK_COMPLETE
    }

    /**
     * constructor of AceVideo on AOSP platform
     *
     * @param id       the id of plugin
     * @param name     name of plugin
     * @param surface  the surface to render video content
     * @param context  context of application
     * @param callback resource callback
     */
    public AceVideoAosp(long id, String name, Surface surface, Context context, IAceOnResourceEvent callback) {
        super(id, callback);
        this.instanceName = name;
        mediaPlayer = new MediaPlayer();
        if (surface != null) {
            mediaPlayer.setSurface(surface);
            isSetSurfaced = true;
            stageMode = false;
        }
        this.context = context;
        window = getWindow();
        mainHandler = new Handler(Looper.getMainLooper());
        handlerThread = new HandlerThread("video-" + id);
        handlerThread.start();
        Looper looper = handlerThread.getLooper();
        if (looper != null) {
            asyncHandler = new Handler(looper);
        } else {
            asyncHandler = new Handler();
        }
        try {
            mediaPlayer.setAudioAttributes(ATTR_VIDEO);
        } catch (IllegalArgumentException e) {
            ALog.e(LOG_TAG, "setAudioAttributes failed.");
        }
    }

    @Override
    public void release() {
        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return;
        }
        try {
            mediaPlayer.stop();
            mediaPlayer.release();
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "mediaPlayer release failed, IllegalStateException.");
        }
        if (handlerThread != null) {
            handlerThread.quitSafely();
            handlerThread = null;
        }
    }

    private String getUrl(String param) {
        if (param.indexOf(FILE_SCHEME) == 0) {
            return param.substring(FILE_SCHEME.length());
        }
        return param;
    }

    private boolean setDataSource(String param) {
        ALog.i(LOG_TAG, "setDataSource param:" + param);
        if (param == null) {
            ALog.e(LOG_TAG, "param is null");
            return false;
        }
        source = param;
        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return false;
        }
        try {
            if (param.startsWith(FILE_SCHEME)) {
                mediaPlayer.setDataSource(param.substring(FILE_SCHEME.length()));
            } else if (param.startsWith(HAP_SCHEME)) {
                return setDataSourceWithFd(param);
            } else {
                mediaPlayer.setDataSource(param);
            }
        } catch (IOException ignored) {
            ALog.e(LOG_TAG, "setDataSource failed, IOException");
            return false;
        }
        return true;
    }

    @Override
    public String initMediaPlayer(Map<String, String> param) {
        ALog.i(LOG_TAG, "initMediaPlayer param:" + param);
        if (param == null) {
            ALog.e(LOG_TAG, "initMediaPlayer failed, param is null");
            return FAIL;
        }
        super.initMediaPlayer(param);
        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return FAIL;
        }
        try {
            if (!param.containsKey(KEY_SOURCE) || !setDataSource(param.get(KEY_SOURCE))) {
                return FAIL;
            }
            mediaPlayer.setOnPreparedListener(this);
            mediaPlayer.setOnErrorListener(this);
            mediaPlayer.setOnSeekCompleteListener(this);
            mediaPlayer.setOnCompletionListener(this);
            mediaPlayer.setOnBufferingUpdateListener(this);
            mediaPlayer.prepare();
            state = PlayState.PREPARED;
        } catch (IOException ignored) {
            ALog.e(LOG_TAG, "initMediaPlayer failed, IOException");
            return FAIL;
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "initMediaPlayer failed, IllegalStateException.");
            return FAIL;
        }
        return SUCCESS;
    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        ALog.i(LOG_TAG, "onPrepared");
        if (mp == null) {
            ALog.e(LOG_TAG, "onPrepared failed, MediaPlayer is null");
            return;
        }
        if (!isNeedResume && isAutoPlay()) {
            mp.start();
            state = PlayState.STARTED;
            setKeepScreenOn(true);
            runOnUIThread(() -> {
                firePlayStatusChange(true);
            });
        }
        resetFromParams(mp);
        if (isNeedResume) {
            if (isResumePlaying) {
                if (!mediaPlayer.isPlaying()) {
                    mediaPlayer.start();
                }
                state = PlayState.STARTED;
                setKeepScreenOn(true);
                runOnUIThread(() -> {
                    firePlayStatusChange(true);
                });
                isResumePlaying = false;
            } else {
                if (mediaPlayer.isPlaying()) {
                    mediaPlayer.pause();
                }
                state = PlayState.PAUSED;
            }
            isNeedResume = false;
            return;
        }
        if (!isAutoPlay() && mediaPlayer.isPlaying()) {
            mediaPlayer.pause();
            state = PlayState.PAUSED;
        }
        runOnUIThread(
            new Runnable() {
                /**
                 * This is called to fire prepared event.
                 */
                public void run() {
                    firePrepared(mp.getVideoWidth(), mp.getVideoHeight(), mp.getDuration(), isAutoPlay(), false);
                }
            });
    }

    private void resetFromParams(MediaPlayer mp) {
        if (isMute()) {
            mp.setVolume(0.0f, 0.0f);
        }

        if (isLooping()) {
            mp.setLooping(true);
        }

        if (isSpeedChanged || isNeedResume) {
            setSpeedWithCheckVersion(getSpeed());
            isSpeedChanged = false;
        }
    }

    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        ALog.i(LOG_TAG, "onError");
        runOnUIThread(
            new Runnable() {

                /**
                 * This is called to fire error event.
                 */
                public void run() {
                    fireError();
                }
            });

        setKeepScreenOn(false);
        try {
            reset();
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "reset failed, IllegalStateException.");
            return false;
        }
        runAsync(() -> {
            if (!resume()) {
                ALog.w(LOG_TAG, "media player resume failed.");
            }
        });
        return false;
    }

    @Override
    public void onCompletion(MediaPlayer mp) {
        runOnUIThread(
            new Runnable() {

                /**
                 * This is called to fire completion event.
                 */
                public void run() {
                    fireCompletion();
                }
            });
        state = PlayState.PLAYBACK_COMPLETE;
        setKeepScreenOn(false);
    }

    @Override
    public void onSeekComplete(MediaPlayer mp) {
        runOnUIThread(
            new Runnable() {

                /**
                 * This is called to fire seek complete event.
                 */
                public void run() {
                    try {
                        fireSeekComplete(mediaPlayer.getCurrentPosition() / SECOND_TO_MSEC);
                    } catch (IllegalStateException ignored) {
                        ALog.e(LOG_TAG, "run failed, IllegalStateException.");
                    }
                }
            });
    }

    @Override
    public void onBufferingUpdate(MediaPlayer mp, int percent) {
        runOnUIThread(
            new Runnable() {

                /**
                 * This is called to fire buffering update event.
                 */
                public void run() {
                    fireBufferingUpdate(percent);
                }
            });
    }

    @Override
    public String start(Map<String, String> params) {
        ALog.i(LOG_TAG, "start param:" + params);
        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return FAIL;
        }
        if (!isSetSurfaced) {
            Surface surface = AceSurfaceHolder.getSurface(surfaceId);
            if (surface != null && mediaPlayer != null) {
                mediaPlayer.setSurface(surface);
                isSetSurfaced = true;
            }
        }
        if (state == PlayState.STOPPED) {
            try {
                mediaPlayer.prepare();
                state = PlayState.PREPARED;
            } catch (IOException ignored) {
                ALog.e(LOG_TAG, "start failed, IOException");
                return FAIL;
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "start failed, IllegalStateException.");
                return FAIL;
            }
        }
        mediaPlayer.start();
        state = PlayState.STARTED;
        setKeepScreenOn(true);
        return SUCCESS;
    }

    @Override
    public String pause(Map<String, String> params) {
        ALog.i(LOG_TAG, "pause param:" + params);
        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return FAIL;
        }
        try {
            mediaPlayer.pause();
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "pause failed, IllegalStateException.");
            return FAIL;
        }
        state = PlayState.PAUSED;
        setKeepScreenOn(false);
        return SUCCESS;
    }

    @Override
    public String stop(Map<String, String> params) {
        ALog.i(LOG_TAG, "stop param:" + params);
        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return FAIL;
        }
        try {
            mediaPlayer.stop();
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "stop failed, IllegalStateException.");
            return FAIL;
        }
        state = PlayState.STOPPED;
        setKeepScreenOn(false);
        return SUCCESS;
    }

    @Override
    public String seekTo(Map<String, String> params) {
        if (params == null) {
            ALog.e(LOG_TAG, "seekTo failed: params is null");
            return FAIL;
        }
        if (!params.containsKey(KEY_VALUE)) {
            ALog.e(LOG_TAG, "seekTo failed: value is illegal");
            return FAIL;
        }
        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return FAIL;
        }
        if (state == PlayState.STOPPED) {
            ALog.w(LOG_TAG, "media player has stopped.");
            return SUCCESS;
        }
        try {
            int msec = Integer.parseInt(params.get(KEY_VALUE));
            if (params.containsKey("seekMode")) {
                int mode = Integer.parseInt(params.get("seekMode"));
                mediaPlayer.seekTo(msec, mode);
            } else {
                mediaPlayer.seekTo(msec);
            }
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "NumberFormatException, seek failed. value = " + params.get(KEY_VALUE));
            return FAIL;
        }
        return SUCCESS;
    }

    @Override
    public String setVolume(Map<String, String> params) {
        if (params == null) {
            ALog.e(LOG_TAG, "setVolume failed: params is null");
            return FAIL;
        }
        if (!params.containsKey(KEY_VALUE)) {
            ALog.e(LOG_TAG, "setVolume failed: value is illegal");
            return FAIL;
        }
        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return FAIL;
        }
        try {
            float volume = Float.parseFloat(params.get(KEY_VALUE));
            mediaPlayer.setVolume(volume, volume);
            if (Math.abs(volume) < 0.00001) {
                setIsMute(true);
            } else {
                setIsMute(false);
            }
            ALog.i(LOG_TAG, "setVolume ." + volume);
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "NumberFormatException, setVolume failed. value = " + params.get(KEY_VALUE));
            return FAIL;
        }
        return SUCCESS;
    }

    @Override
    public String getPosition(Map<String, String> params) {
        // the current position in milliseconds.
        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return FAIL;
        }
        try {
            position = mediaPlayer.getCurrentPosition();
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "getCurrentPosition failed, IllegalStateException.");
            return FAIL;
        }
        runOnUIThread(
            new Runnable() {

                /**
                 * This is called to fire play currenttime.
                 */
                public void run() {
                    fireGetCurrenttime(position / SECOND_TO_MSEC);
                }
            });

        return "currentpos=" + position / SECOND_TO_MSEC;
    }

    @Override
    public String enableLooping(Map<String, String> params) {
        if (params == null) {
            return FAIL;
        }
        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return FAIL;
        }
        try {
            if (params.containsKey("loop")) {
                if (Integer.parseInt(params.get("loop")) == 1) {
                    mediaPlayer.setLooping(true);
                    setLooping(true);
                } else {
                    mediaPlayer.setLooping(false);
                    setLooping(false);
                }
            }
            return SUCCESS;
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "NumberFormatException");
            return FAIL;
        }
    }

    @Override
    public String setSpeed(Map<String, String> params) {
        if (params == null) {
            ALog.e(LOG_TAG, "setSpeed failed: params is null");
            return FAIL;
        }
        if (!params.containsKey(KEY_VALUE)) {
            ALog.e(LOG_TAG, "setSpeed failed: value is illegal");
            return FAIL;
        }
        try {
            setSpeed(Float.parseFloat(params.get(KEY_VALUE)));
            isSpeedChanged = true;
            if (state == PlayState.STARTED) {
                setSpeedWithCheckVersion(getSpeed());
            } else if (state == PlayState.PAUSED) {
                setSpeedWithCheckVersion(getSpeed());
                if (mediaPlayer != null && mediaPlayer.isPlaying()) {
                    mediaPlayer.pause();
                }
            } else {
                ALog.w(LOG_TAG, "If the speed is greater than 0, the video will start playing.");
            }
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "NumberFormatException, setSpeed failed. value = " + params.get(KEY_VALUE));
            return FAIL;
        }
        return SUCCESS;
    }

    @Override
    public String setDirection(Map<String, String> params) {
        return SUCCESS;
    }

    @Override
    public String setLandscape(Map<String, String> params) {
        return SUCCESS;
    }

    @Override
    public String setSurface(Map<String, String> params) {
        if (params == null) {
            ALog.e(LOG_TAG, "setSurface failed: params is null");
            return FAIL;
        }
        if (!params.containsKey(KEY_VALUE)) {
            ALog.e(LOG_TAG, "setSurface failed: value is illegal");
            return FAIL;
        }
        try {
            surfaceId = Integer.parseInt(params.get(KEY_VALUE));
            ALog.i(LOG_TAG, "setSurface id:" + surfaceId);
            Surface surface = AceSurfaceHolder.getSurface(surfaceId);
            if (surface != null && mediaPlayer != null) {
                ALog.i(LOG_TAG, "MediaPlayer SetSurface");
                mediaPlayer.setSurface(surface);
                isSetSurfaced = true;
            }
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "NumberFormatException, setSurface failed. value = " + params.get(KEY_VALUE));
            return FAIL;
        }
        return FAIL;
    }

    @Override
    public String updateResource(Map<String, String> params) {
        ALog.i(LOG_TAG, "updateResource param:" + params);
        if (params == null || mediaPlayer == null) {
            ALog.e(LOG_TAG, "updateResource failed, param is null");
            return FAIL;
        }
        try {
            ALog.i(LOG_TAG, "updateResource state:" + state.toString());
            mediaPlayer.reset();
            state = PlayState.IDLE;
            if (!params.containsKey(KEY_SOURCE) || !setDataSource(params.get(KEY_SOURCE))) {
                return FAIL;
            }
            mediaPlayer.prepare();
            state = PlayState.PREPARED;
            mediaPlayer.start();
            state = PlayState.STARTED;
            mediaPlayer.pause();
            state = PlayState.PAUSED;
            setKeepScreenOn(false);
        } catch (IOException ignored) {
            ALog.e(LOG_TAG, "updateResource failed, IOException");
            return FAIL;
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "updateResource failed, IllegalStateException.");
            return FAIL;
        }

        return SUCCESS;
    }

    @Override
    public void onActivityResume() {
        runAsync(() -> {
            if (!resume()) {
                ALog.w(LOG_TAG, "media player resume failed.");
            }
        });
    }

    @Override
    public void onActivityPause() {
        runAsync(() -> {
            if (mediaPlayer != null) {
                isNeedResume = true;
                isResumePlaying = mediaPlayer.isPlaying();
                reset();
                setKeepScreenOn(false);
                runAsync(
                    new Runnable() {

                        /**
                         * This is called to fire play currenttime.
                         */
                        public void run() {
                            runOnUIThread(
                                new Runnable() {
                                    /**
                                     * This is called to fire play status change event.
                                     */
                                    public void run() {
                                        firePlayStatusChange(false);
                                    }
                                });
                        }
                    });
            }
        });
    }

    private void setKeepScreenOn(boolean screenOn) {
        if (screenOn) {
            runOnUIThread(
                new Runnable() {

                    /**
                     * Add flags if window is not null.
                     */
                    public void run() {
                        if (window != null) {
                            window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                        }
                    }
                });
        } else {
            runOnUIThread(
                new Runnable() {

                    /**
                     * Clear flags if window is not null.
                     */
                    public void run() {
                        if (window != null) {
                            window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                        }
                    }
                });
        }
    }

    private void setSpeedWithCheckVersion(Float speed) {
        try {
            if (mediaPlayer == null) {
                ALog.w(LOG_TAG, "media player is null.");
                return;
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                mediaPlayer.setPlaybackParams(mediaPlayer.getPlaybackParams().setSpeed(speed));
            } else {
                ALog.w(LOG_TAG, "build version not suporrt set speed.");
            }
        } catch (IllegalArgumentException e) {
            ALog.e(LOG_TAG, "setSpeedWithCheckVersion failed.");
        }
    }

    private Window getWindow() {
        if (!(context instanceof Activity)) {
            ALog.e(LOG_TAG, "Cannot get activity.");
            return null;
        }
        Activity activity = (Activity) context;
        return activity.getWindow();
    }

    // Just for ensure that task is running from UI thread.
    private void runOnUIThread(Runnable task) {
        if (Looper.myLooper() == Looper.getMainLooper()) {
            task.run();
        } else {
            mainHandler.post(task);
        }
    }

    private void reset() {
        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return;
        }
        try {
            position = mediaPlayer.getCurrentPosition();
            mediaPlayer.reset();
            mediaPlayer.release();
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "mediaPlayer.release, IllegalStateException.");
            return;
        }
        mediaPlayer = null;
        state = PlayState.IDLE;
        isSetSurfaced = false;
        isSpeedChanged = true;
    }

    private boolean resume() {
        ALog.i(LOG_TAG, "MediaPlayer resuming");
        mediaPlayer = new MediaPlayer();
        Surface surface = AceSurfaceHolder.getSurface(surfaceId);
        if (surface != null) {
            ALog.i(LOG_TAG, "MediaPlayer SetSurface");
            mediaPlayer.setSurface(surface);
            isSetSurfaced = true;
        }
        try {
            mediaPlayer.setAudioAttributes(ATTR_VIDEO);
        } catch (IllegalArgumentException e) {
            ALog.e(LOG_TAG, "setAudioAttributes failed.");
            return false;
        }
        if (!source.isEmpty() && !setDataSource(source)) {
            ALog.e(LOG_TAG, "setDataSource failed.");
            return false;
        }
        try {
            mediaPlayer.setOnPreparedListener(this);
            mediaPlayer.setOnErrorListener(this);
            mediaPlayer.setOnSeekCompleteListener(this);
            mediaPlayer.setOnCompletionListener(this);
            mediaPlayer.setOnBufferingUpdateListener(this);
            mediaPlayer.prepare();
            state = PlayState.PREPARED;
            if (position > 0) {
                mediaPlayer.seekTo(position);
            }
        } catch (IOException ignored) {
            ALog.e(LOG_TAG, "initMediaPlayer failed, IOException");
            return false;
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "initMediaPlayer failed, IllegalStateException.");
            return false;
        }
        ALog.i(LOG_TAG, "MediaPlayer resumed.");
        return true;
    }

    private boolean setDataSourceWithFd(String param) {
        AssetManager assetManage = context.getAssets();
        if (assetManage == null) {
            ALog.e(LOG_TAG, "setDataSource null assetManager");
            return false;
        }
        AssetFileDescriptor assetFd;
        try {
            if (stageMode) {
                try {
                    String filePath = "arkui-x" + File.separator + instanceName + File.separator + "ets" + param;
                    ALog.i(LOG_TAG, "setDataSourc hapPath:" + filePath);
                    assetFd = assetManage.openFd(filePath);
                } catch (IOException ignored) {
                    ALog.e(LOG_TAG, "not found asset in instance path, now begin to search asset in share path");
                    return false;
                }
                mediaPlayer.setDataSource(assetFd.getFileDescriptor(), assetFd.getStartOffset(),
                            assetFd.getLength());
            } else {
                try {
                    assetFd = assetManage.openFd(
                            "js" + File.separator + instanceName + File.separator
                                    + param.substring(HAP_SCHEME.length()));
                } catch (IOException ignored) {
                    ALog.e(LOG_TAG, "not found asset in instance path, now begin to search asset in share path");
                    assetFd = assetManage.openFd(
                            "js" + File.separator + "share" + File.separator
                                    + param.substring(HAP_SCHEME.length()));
                    return false;
                }
                mediaPlayer.setDataSource(assetFd.getFileDescriptor(), assetFd.getStartOffset(),
                        assetFd.getLength());
            }
        } catch (IOException ignored) {
            ALog.e(LOG_TAG, "setDataSource failed, IOException");
            return false;
        }
        return true;
    }

    @Override
    public String runAsync(Runnable runnable) {
        asyncHandler.post(runnable);
        return SUCCESS;
    }
}