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

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.graphics.SurfaceTexture;
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
import ohos.ace.adapter.AceTextureHolder;
import ohos.ace.adapter.ALog;
import ohos.ace.adapter.IAceOnResourceEvent;

import java.io.File;
import java.io.IOException;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

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

    private static final String KEY_ISTEXTURE = "isTexture";

    private static final int SECOND_TO_MSEC = 1000;

    private static final AudioAttributes ATTR_VIDEO = new AudioAttributes.Builder().setUsage(
            AudioAttributes.USAGE_MEDIA).setContentType(AudioAttributes.CONTENT_TYPE_MOVIE).build();

    private volatile MediaPlayer mediaPlayer;

    private Lock mediaPlayerLock = new ReentrantLock();

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

    private boolean isPaused = false;

    private PlayState state = PlayState.IDLE;

    private boolean isTrueBack = false;

    private boolean isTexture= false;

    private boolean isStoped = false;

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
     * @param context  context of application
     * @param callback resource callback
     */
    public AceVideoAosp(long id, String name, Context context, IAceOnResourceEvent callback) {
        super(id, callback);
        this.instanceName = name;
        mediaPlayer = new MediaPlayer();
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
        } catch (IllegalStateException e) {
            ALog.e(LOG_TAG, "setAudioAttributes failed.");
        }
    }

    @Override
    public void release() {
        ALog.i(LOG_TAG, "media player will release.");
        isPaused = true;
        runAsync(() -> {
            mediaPlayerLock.lock();
            try {
                if (mediaPlayer == null) {
                    ALog.w(LOG_TAG, "media player is null.");
                    return;
                }
                try {
                    mediaPlayer.stop();
                    mediaPlayer.release();
                    mediaPlayer = null;
                } catch (IllegalStateException ignored) {
                    ALog.e(LOG_TAG, "mediaPlayer release failed, IllegalStateException.");
                }
            } finally {
                if (handlerThread != null) {
                    handlerThread.quitSafely();
                    handlerThread = null;
                }
                mediaPlayerLock.unlock();
            }
        });
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
        mediaPlayerLock.lock();
        try {
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
                reset();
                return FAIL;
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "initMediaPlayer failed, IllegalStateException.");
                reset();
                return FAIL;
            }
        } finally {
            mediaPlayerLock.unlock();
        }
        return SUCCESS;
    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        ALog.i(LOG_TAG, "onPrepared");
        mediaPlayerLock.lock();
        try {
            if (isStoped && state == PlayState.STOPPED) {
                mediaPlayer.stop();
                ALog.e(LOG_TAG, "media player is STOPPED.");
                return;
            }
            if (mp == null || mediaPlayer == null) {
                ALog.e(LOG_TAG, "onPrepared failed, MediaPlayer is null");
                return;
            }
            if (!isNeedResume && isAutoPlay()) {
                mediaPlayer.start();
                state = PlayState.STARTED;
                setKeepScreenOn(true);
                runOnUIThread(() -> {
                    firePlayStatusChange(true);
                });
            }
            resetFromParams(mediaPlayer);
            try {
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
                        if (isStoped) {
                            if (mediaPlayer.isPlaying()) {
                                mediaPlayer.stop();
                            }
                            state = PlayState.STOPPED;
                        }else{
                            if (mediaPlayer.isPlaying()) {
                                mediaPlayer.pause();
                            }
                            state = PlayState.PAUSED;
                        }
                    }
                    isNeedResume = false;
                    return;
                }
                if (!isAutoPlay() && mediaPlayer.isPlaying()) {
                    mediaPlayer.pause();
                    state = PlayState.PAUSED;
                }
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "run onPrepared, IllegalStateException.");
            }
        } finally {
            mediaPlayerLock.unlock();
        }
        runOnUIThread(
            new Runnable() {
                /**
                 * This is called to fire prepared event.
                 */
                public void run() {
                    mediaPlayerLock.lock();
                    try {
                        if (mediaPlayer != null) {
                            try {
                                firePrepared(mediaPlayer.getVideoWidth(), mediaPlayer.getVideoHeight(),
                                mediaPlayer.getDuration(), isAutoPlay(), false);
                            } catch (IllegalStateException ignored) {
                                ALog.e(LOG_TAG, "run firePrepared, IllegalStateException.");
                            }
                        }
                    }
                    finally {
                        mediaPlayerLock.unlock();
                    }
                }
            });
    }

    private void resetFromParams(MediaPlayer mp) {
        try {
            if (mp != null && isMute()) {
                mp.setVolume(0.0f, 0.0f);
            }

            if (mp != null && isLooping()) {
                mp.setLooping(true);
            }

            if (isNeedResume) {
                if (position > 0) {
                    mediaPlayer.seekTo(position);
                }
            }

            if (isSpeedChanged || isNeedResume) {
                setSpeedWithCheckVersion(getSpeed());
                isSpeedChanged = false;
            }
        } catch (IllegalArgumentException e) {
            ALog.e(LOG_TAG, "resetFromParams, IllegalArgumentException.");
        } catch (IllegalStateException e) {
            ALog.e(LOG_TAG, "resetFromParams, IllegalStateException.");
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

        runAsync(() -> {
            mediaPlayerLock.lock();
            isNeedResume = true;
            try {
                reset();
                if (!resume()) {
                    ALog.w(LOG_TAG, "media player resume failed.");
                    reset();
                }
            } finally {
                mediaPlayerLock.unlock();
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
                    mediaPlayerLock.lock();
                    try {
                        if (mediaPlayer != null) {
                            try {
                                fireSeekComplete(mediaPlayer.getCurrentPosition() / SECOND_TO_MSEC);
                            } catch (IllegalStateException ignored) {
                                ALog.e(LOG_TAG, "run failed, IllegalStateException.");
                            }
                        } 
                    } finally {
                        mediaPlayerLock.unlock();
                    }
                }
            });
    }

    @Override
    public void onBufferingUpdate(MediaPlayer mp, int percent) {}

    @Override
    public String start(Map<String, String> params) {
        ALog.i(LOG_TAG, "start param:" + params);
        mediaPlayerLock.lock();
        try {
            if (mediaPlayer == null) {
                ALog.w(LOG_TAG, "media player is null.");
                return FAIL;
            }
            isPaused = false;
            if (!isSetSurfaced) {
                Surface surface = getSurface();
                if (surface != null && mediaPlayer != null) {
                    mediaPlayer.setSurface(surface);
                    isSetSurfaced = true;
                }
            }
            if (state == PlayState.STOPPED) {
                try {
                    if (mediaPlayer != null) {
                        mediaPlayer.prepare();
                    }
                    state = PlayState.PREPARED;
                } catch (IOException ignored) {
                    ALog.e(LOG_TAG, "start failed, IOException");
                    return FAIL;
                } catch (IllegalStateException ignored) {
                    ALog.e(LOG_TAG, "start failed, IllegalStateException.");
                    return FAIL;
                }
            }
            if (mediaPlayer != null) {
                mediaPlayer.start();
                isStoped = false;
            }
        } finally {
            mediaPlayerLock.unlock();
        }
        state = PlayState.STARTED;
        setKeepScreenOn(true);
        return SUCCESS;
    }

    @Override
    public String pause(Map<String, String> params) {
        ALog.i(LOG_TAG, "pause param:" + params);
        mediaPlayerLock.lock();
        try {
            if (state == PlayState.STOPPED || state == PlayState.PREPARED) {
                ALog.w(LOG_TAG, "media player is not STARTED.");
                return SUCCESS;
            }
            if (mediaPlayer == null) {
                ALog.w(LOG_TAG, "media player is null.");
                return FAIL;
            }
            state = PlayState.PAUSED;
            isPaused = true;
            setKeepScreenOn(false);
            try {
                mediaPlayer.pause();
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "pause failed, IllegalStateException.");
                return FAIL;
            }
        } finally {
            mediaPlayerLock.unlock();
        }
        return SUCCESS;
    }

    @Override
    public String stop(Map<String, String> params) {
        ALog.i(LOG_TAG, "stop param:" + params);
        mediaPlayerLock.lock();
        try {
            isPaused = true;
            if (mediaPlayer == null) {
                ALog.w(LOG_TAG, "media player is null.");
                return FAIL;
            }
            try {
                mediaPlayer.stop();
                mediaPlayer.reset();
                mediaPlayer.release();
                mediaPlayer = new MediaPlayer();
                Surface surface = getSurface();
                if (surface != null) {
                    ALog.i(LOG_TAG, "MediaPlayer SetSurface");
                    mediaPlayer.setSurface(surface);
                    isSetSurfaced = true;
                }
                mediaPlayer.setAudioAttributes(ATTR_VIDEO);
                if (!source.isEmpty() && !setDataSource(source)) {
                    ALog.e(LOG_TAG, "setDataSource failed.");
                    return FAIL;
                }
                mediaPlayer.setOnPreparedListener(this);
                mediaPlayer.setOnErrorListener(this);
                mediaPlayer.setOnSeekCompleteListener(this);
                mediaPlayer.setOnCompletionListener(this);
                mediaPlayer.setOnBufferingUpdateListener(this);
                mediaPlayer.prepare();
            } catch (IOException ignored) {
                ALog.e(LOG_TAG, "start failed, IOException");
                return FAIL;
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "stop failed, IllegalStateException.");
                return FAIL;
            }
        } finally {
            mediaPlayerLock.unlock();
            state = PlayState.STOPPED;
            isStoped = true;
            setKeepScreenOn(false);
        }
        return SUCCESS;
    }

    @Override
    public String seekTo(Map<String, String> params) {
        mediaPlayerLock.lock();
        try {
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
            if (state == PlayState.STOPPED || state == PlayState.PREPARED) {
                ALog.w(LOG_TAG, "media player is not STARTED.");
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
        } finally {
            mediaPlayerLock.unlock();
        }
        return SUCCESS;
    }

    @Override
    public String setVolume(Map<String, String> params) {
        mediaPlayerLock.lock();
        try {
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
        } finally {
            mediaPlayerLock.unlock();
        }
        return SUCCESS;
    }

    @Override
    public String getPosition(Map<String, String> params) {
        // the current position in milliseconds.
        mediaPlayerLock.lock();
        try {
            if (mediaPlayer == null) {
                ALog.w(LOG_TAG, "media player is null.");
                return FAIL;
            }
            try {
                if (isNeedResume) {
                    ALog.i(LOG_TAG, "getPosition return:  "+"currentpos=" + position / SECOND_TO_MSEC);
                    return "currentpos=" + position / SECOND_TO_MSEC;
                } else {
                    position = mediaPlayer.getCurrentPosition();
                }
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "getCurrentPosition failed, IllegalStateException.");
                return FAIL;
            }
        } finally {
            mediaPlayerLock.unlock();
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
        mediaPlayerLock.lock();
        try {
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
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "IllegalStateException");
                return FAIL;
            } catch (IllegalArgumentException ignored) {
                ALog.e(LOG_TAG, "IllegalArgumentException");
                return FAIL;
            }
        } finally {
            mediaPlayerLock.unlock();
        }
    }

    @Override
    public String setSpeed(Map<String, String> params) {
        mediaPlayerLock.lock();
        try {
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
        } finally {
            mediaPlayerLock.unlock();
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
        mediaPlayerLock.lock();
        try {
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
                if (Integer.parseInt(params.get(KEY_ISTEXTURE)) == 1) {
                    isTexture = true;
                }
                ALog.i(LOG_TAG, "setSurface id:" + surfaceId);
                Surface surface = getSurface();
                if (surface != null && mediaPlayer != null) {
                    ALog.i(LOG_TAG, "MediaPlayer SetSurface");
                    mediaPlayer.setSurface(surface);
                    isSetSurfaced = true;
                }
            } catch (NumberFormatException ignored) {
                ALog.e(LOG_TAG, "NumberFormatException, setSurface failed. value = " + params.get(KEY_VALUE));
                return FAIL;
            } catch (IllegalArgumentException ignored) {
                ALog.e(LOG_TAG, "IllegalArgumentException, setSurface failed. value = " + params.get(KEY_VALUE));
                return FAIL;
            }
        } finally {
            mediaPlayerLock.unlock();
        }
        return FAIL;
    }

    @Override
    public String updateResource(Map<String, String> params) {
        ALog.i(LOG_TAG, "updateResource param:" + params);
        mediaPlayerLock.lock();
        try {
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
        } finally {
            mediaPlayerLock.unlock();
        }
        return SUCCESS;
    }

    @Override
    public void onActivityResume() {
        runAsync(() -> {
            mediaPlayerLock.lock();
            try {
                if(!isTrueBack) {
                    isNeedResume = true;
                    isResumePlaying = ((mediaPlayer != null && mediaPlayer.isPlaying()) || isAutoPlay()) && !isPaused;
                    reset();
                }
                if (!resume()) {
                    ALog.w(LOG_TAG, "media player resume failed.");
                    reset();
                }
            } finally {
                mediaPlayerLock.unlock();
            }
        });
    }

    @Override
    public void onActivityPause() {
        runAsync(() -> {
            mediaPlayerLock.lock();
            try {
                if (mediaPlayer != null) {
                    isTrueBack = true;
                    isNeedResume = true;
                    isResumePlaying = (mediaPlayer.isPlaying() || isAutoPlay()) && !isPaused;
                    reset();
                }
            } finally {
                mediaPlayerLock.unlock();
            }
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
        } catch (IllegalStateException e) {
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
        state = PlayState.IDLE;
        isSetSurfaced = false;
        isSpeedChanged = true;

        if (mediaPlayer == null) {
            ALog.w(LOG_TAG, "media player is null.");
            return;
        }
        try {
            position = mediaPlayer.getCurrentPosition();
            mediaPlayer.reset();
            mediaPlayer.release();
            mediaPlayer = null;
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "mediaPlayer.release, IllegalStateException.");
            mediaPlayer = null;
            return;
        }
        
    }

    private boolean resume() {
        ALog.i(LOG_TAG, "MediaPlayer resuming.");
        if (mediaPlayer != null && !isNeedResume) {
            ALog.i(LOG_TAG, "MediaPlayer no need to resume.");
            return true;
        }
        isTrueBack = false;
        mediaPlayer = new MediaPlayer();
        Surface surface = getSurface();
        if (surface != null) {
            ALog.i(LOG_TAG, "MediaPlayer SetSurface");
            mediaPlayer.setSurface(surface);
            isSetSurfaced = true;
        }
        try {
            mediaPlayer.setAudioAttributes(ATTR_VIDEO);
            if (!source.isEmpty() && !setDataSource(source)) {
                ALog.e(LOG_TAG, "setDataSource failed.");
                return false;
            }

            mediaPlayer.setOnPreparedListener(this);
            mediaPlayer.setOnErrorListener(this);
            mediaPlayer.setOnSeekCompleteListener(this);
            mediaPlayer.setOnCompletionListener(this);
            mediaPlayer.setOnBufferingUpdateListener(this);
            mediaPlayer.prepare();
            state = PlayState.PREPARED;
        } catch (IOException ignored) {
            ALog.e(LOG_TAG, "resume failed, IOException");
            return false;
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "resume failed, IllegalStateException.");
            return false;
        } catch (IllegalArgumentException ignored) {
            ALog.e(LOG_TAG, "resume failed, IllegalArgumentException.");
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

    private Surface getSurface() {
        Surface surface = null;
        if (isTexture) {
            SurfaceTexture surfaceTexture = AceTextureHolder.getSurfaceTexture(surfaceId);
            if (surfaceTexture != null) {
                surface = new Surface(surfaceTexture);
            }
        } else {
            surface = AceSurfaceHolder.getSurface(surfaceId);
        }
        return surface;
    }
}