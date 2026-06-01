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

    private static final String INSTANCE_SEPARATOR = ":";

    private static final int INSTANCE_ARRAY_NUM_MAX = 3;

    private static final int INSTANCE_ARRAY_MODULE = 1;

    private static final int INSTANCE_ARRAY_ID = 3;

    private static final int SECOND_TO_MSEC = 1000;

    private static final AudioAttributes ATTR_VIDEO = new AudioAttributes.Builder().setUsage(
            AudioAttributes.USAGE_MEDIA).setContentType(AudioAttributes.CONTENT_TYPE_MOVIE).build();

    private volatile MediaPlayer mediaPlayer;

    private Lock mediaPlayerLock = new ReentrantLock();

    private final Context context;

    private final String instanceName;

    private String moduleName = "";

    private int instanceId = -1;

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

    private boolean isTexture = false;

    private boolean isStoped = false;

    private boolean isShowFirstFrame = false;

    private boolean isInternalFirstFrameSeeking = false;

    private boolean isMediaPrepared = false;

    private boolean isPendingRenderFirstFrameUpdate = false;

    private boolean shouldDelaySurfaceAttachForReset = false;

    private boolean shouldDelaySurfaceAttachForSourceSwitch = false;

    private boolean isManualStartFromStopped = false;

    private boolean shouldNotifyCurrentTimeOnPrepared = false;

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
        String[] nameArray = instanceName.split(INSTANCE_SEPARATOR);
        if (nameArray.length >= INSTANCE_ARRAY_NUM_MAX) {
            this.moduleName = nameArray[INSTANCE_ARRAY_MODULE];
            this.instanceId = Integer.parseInt(nameArray[INSTANCE_ARRAY_ID]);
        } else {
            ALog.w(LOG_TAG, "nameArray is out of max length.");
        }
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

    /**
     * constructor of AceVideoAosp
     */
    public enum PlayState {
        IDLE,
        PREPARED,
        STARTED,
        PAUSED,
        STOPPED,
        PLAYBACK_COMPLETE
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
        if (param == null || param.isEmpty()) {
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
                isMediaPrepared = false;
                mediaPlayer.prepareAsync();
                state = PlayState.PREPARED;
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
        boolean isSkipPauseForManualStart = false;
        boolean isManualStartFromStoppedPrepared = false;
        boolean shouldNotifyCurrentTime = false;
        MediaPlayer restoreSurfaceTarget = null;
        mediaPlayerLock.lock();
        try {
            if (handleStoppedOnPrepared()) {
                return;
            }
            if (isMediaPlayerInvalid(mp)) {
                return;
            }
            isMediaPrepared = true;
            attachSurfaceIfNeededForSeek();
            try {
                boolean isManualStartPlaying = mediaPlayer.isPlaying() && !isPaused;
                isManualStartFromStoppedPrepared = isManualStartFromStopped;
                if (isManualStartFromStopped || isManualStartPlaying) {
                    isSkipPauseForManualStart = true;
                    isManualStartFromStopped = false;
                }
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "onPrepared check isPlaying failed, IllegalStateException.");
            }
            if (shouldAutoPlayOnPrepared(isManualStartFromStoppedPrepared)) {
                startPlaybackAndNotify();
            }
            resetFromParams(mediaPlayer);
            try {
                if (handleResumeOnPreparedInternal()) {
                    return;
                }
                handlePauseWhenNotAutoPlay(isSkipPauseForManualStart);
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "run onPrepared, IllegalStateException.");
            }
            shouldNotifyCurrentTime = shouldNotifyCurrentTimeOnPrepared;
            if (shouldDelaySurfaceAttachForReset && isShowFirstFrame && !isSetSurfaced) {
                restoreSurfaceTarget = mediaPlayer;
                shouldDelaySurfaceAttachForReset = false;
            }
            shouldNotifyCurrentTimeOnPrepared = false;
        } finally {
            mediaPlayerLock.unlock();
        }

        boolean finalShouldNotifyCurrentTime = shouldNotifyCurrentTime;
        MediaPlayer finalRestoreSurfaceTarget = restoreSurfaceTarget;
        runOnUIThread(() -> {
            if (finalRestoreSurfaceTarget != null) {
                restoreSurfaceAfterResetIfNeeded(finalRestoreSurfaceTarget);
            }
            if (finalShouldNotifyCurrentTime) {
                fireGetCurrenttime(0);
            }
            handlePreparedOnUiThread();
            updateFirstFrameVisibility();
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

    private boolean handleStoppedOnPrepared() {
        if (isStoped && state == PlayState.STOPPED) {
            mediaPlayer.stop();
            ALog.e(LOG_TAG, "media player is STOPPED.");
            return true;
        }
        return false;
    }

    private boolean isMediaPlayerInvalid(MediaPlayer mp) {
        if (mp == null || mediaPlayer == null) {
            ALog.e(LOG_TAG, "onPrepared failed, MediaPlayer is null");
            return true;
        }
        return false;
    }

    private boolean shouldAutoPlayOnPrepared(boolean isManualStartFromStoppedPrepared) {
        return !isNeedResume && isAutoPlay() && !isManualStartFromStoppedPrepared;
    }

    private void startPlaybackAndNotify() {
        mediaPlayer.start();
        state = PlayState.STARTED;
        setKeepScreenOn(true);
        runOnUIThread(() -> firePlayStatusChange(true));
    }

    private boolean handleResumeOnPreparedInternal() {
        if (!isNeedResume) {
            return false;
        }
        if (isResumePlaying) {
            if (!mediaPlayer.isPlaying()) {
                mediaPlayer.start();
            }
            state = PlayState.STARTED;
            setKeepScreenOn(true);
            runOnUIThread(() -> firePlayStatusChange(true));
            isResumePlaying = false;
        } else if (isStoped) {
            if (mediaPlayer.isPlaying()) {
                mediaPlayer.stop();
            }
            state = PlayState.STOPPED;
        } else {
            if (mediaPlayer.isPlaying()) {
                mediaPlayer.pause();
            }
            state = PlayState.PAUSED;
        }
        isNeedResume = false;
        return true;
    }

    private void handlePauseWhenNotAutoPlay(boolean isSkipPauseForManualStart) {
        if (!isAutoPlay() && mediaPlayer.isPlaying() && !isSkipPauseForManualStart) {
            mediaPlayer.pause();
            state = PlayState.PAUSED;
        }
    }

    private void handlePreparedOnUiThread() {
        mediaPlayerLock.lock();
        try {
            if (mediaPlayer == null) {
                return;
            }
            try {
                int videoWidth = mediaPlayer.getVideoWidth();
                int videoHeight = mediaPlayer.getVideoHeight();
                int duration = mediaPlayer.getDuration();
                boolean isPlaying = mediaPlayer.isPlaying();
                firePrepared(videoWidth, videoHeight, duration, isPlaying, false);
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "run firePrepared, IllegalStateException.");
            }
        } finally {
            mediaPlayerLock.unlock();
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
            isMediaPrepared = false;
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
        boolean isInternalFirstFrameSeek = false;
        mediaPlayerLock.lock();
        try {
            if (isInternalFirstFrameSeeking) {
                isInternalFirstFrameSeek = true;
                isInternalFirstFrameSeeking = false;
            }
        } finally {
            mediaPlayerLock.unlock();
        }

        if (isInternalFirstFrameSeek) {
            return;
        }

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
                                int currentPosition = mediaPlayer.getCurrentPosition() / SECOND_TO_MSEC;
                                fireGetCurrenttime(currentPosition);
                                fireSeekComplete(currentPosition);
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
            if (isPendingRenderFirstFrameUpdate) {
                isPendingRenderFirstFrameUpdate = false;
            }
            if (!isSetSurfaced) {
                Surface surface = getSurface();
                if (surface != null && mediaPlayer != null) {
                    mediaPlayer.setSurface(surface);
                    isSetSurfaced = true;
                    shouldDelaySurfaceAttachForReset = false;
                    shouldDelaySurfaceAttachForSourceSwitch = false;
                }
            }
            if (state == PlayState.STOPPED) {
                if (!preparePlayerFromStopped(true, "start")) {
                    return FAIL;
                }
            }
            if (isSpeedChanged) {
                setSpeedWithCheckVersion(getSpeed());
                isSpeedChanged = false;
            }
            if (mediaPlayer != null) {
                isPaused = false;
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
        boolean isStopSuccess = false;
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
                isSetSurfaced = false;
                Surface surface = getSurface();
                if (surface != null) {
                    ALog.i(LOG_TAG, "MediaPlayer SetSurface");
                    mediaPlayer.setSurface(surface);
                    isSetSurfaced = true;
                }
                mediaPlayer.setAudioAttributes(ATTR_VIDEO);
                if (!setDataSource(source)) {
                    ALog.e(LOG_TAG, "setDataSource failed.");
                    return FAIL;
                }
                mediaPlayer.setOnPreparedListener(this);
                mediaPlayer.setOnErrorListener(this);
                mediaPlayer.setOnSeekCompleteListener(this);
                mediaPlayer.setOnCompletionListener(this);
                mediaPlayer.setOnBufferingUpdateListener(this);
                isMediaPrepared = false;
                isStopSuccess = true;
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "stop failed, IllegalStateException.");
                return FAIL;
            }
        } finally {
            mediaPlayerLock.unlock();
            state = PlayState.STOPPED;
            isStoped = true;
            isManualStartFromStopped = false;
            setKeepScreenOn(false);
        }
        if (isStopSuccess) {
            runOnUIThread(() -> fireStop());
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
            if (state == PlayState.STOPPED) {
                ALog.i(LOG_TAG, "seekTo ignored because player is STOPPED.");
                return SUCCESS;
            }
            try {
                int msec = Integer.parseInt(params.get(KEY_VALUE));
                attachSurfaceIfNeededForSeek();
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
                if (Math.abs(volume) < 0.00001) {
                    mediaPlayer.setVolume(0.0f, 0.0f);
                    setIsMute(true);
                } else {
                    mediaPlayer.setVolume(volume, volume);
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
                    ALog.i(LOG_TAG, "getPosition return:  " + "currentpos=" + position / SECOND_TO_MSEC);
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
        if (!isSetSurfaced && surfaceId > 0) {
            attachSurfaceIfNeededForSeek();
        }
        if (isShowFirstFrame || isPendingRenderFirstFrameUpdate) {
            updateFirstFrameVisibility();
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
                if (shouldDelaySurfaceAttachForReset || shouldDelaySurfaceAttachForSourceSwitch ||
                    isPendingRenderFirstFrameUpdate) {
                    isSetSurfaced = false;
                    return SUCCESS;
                }
                Surface surface = getSurface();
                if (surface == null || mediaPlayer == null) {
                    isSetSurfaced = false;
                    ALog.w(LOG_TAG, "setSurface deferred: surface or mediaPlayer not ready yet.");
                    return SUCCESS;
                }
                mediaPlayer.setSurface(surface);
                isSetSurfaced = true;
            } catch (NumberFormatException ignored) {
                ALog.e(LOG_TAG, "NumberFormatException, setSurface failed. value = " + params.get(KEY_VALUE));
                return FAIL;
            } catch (IllegalArgumentException ignored) {
                ALog.e(LOG_TAG, "IllegalArgumentException, setSurface failed. value = " + params.get(KEY_VALUE));
                return FAIL;
            } catch (IllegalStateException ignored) {
                ALog.e(LOG_TAG, "IllegalStateException, setSurface failed. value = " + params.get(KEY_VALUE));
                return FAIL;
            }
        } finally {
            mediaPlayerLock.unlock();
        }
        updateFirstFrameVisibility();
        return SUCCESS;
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
            return updateResourceInternal(params);
        } finally {
            mediaPlayerLock.unlock();
        }
    }

    @Override
    public String setRenderFirstFrame(Map<String, String> params) {
        if (params == null) {
            return FAIL;
        }
        String showFirstFrameValue = params.get("showFirstFrame");
        if (showFirstFrameValue == null) {
            return FAIL;
        }
        try {
            isShowFirstFrame = Integer.parseInt(showFirstFrameValue) != 0;
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "setRenderFirstFrame failed: invalid showFirstFrame value.");
            return FAIL;
        }
        return handleSetRenderFirstFrame();
    }

    private void updateFirstFrameVisibility() {
        if (isPendingRenderFirstFrameUpdate) {
            return;
        }
        if (!isShowFirstFrame) {
            return;
        }
        if (!isMediaPrepared) {
            return;
        }
        if (!isSetSurfaced) {
            attachSurfaceIfNeededForSeek();
            if (!isSetSurfaced) {
                return;
            }
        }
        MediaPlayer currentPlayer = null;
        mediaPlayerLock.lock();
        try {
            if (mediaPlayer != null && !mediaPlayer.isPlaying()) {
                if (mediaPlayer.getCurrentPosition() > 0) {
                    return;
                }
                isInternalFirstFrameSeeking = true;
                currentPlayer = mediaPlayer;
            }
        } catch (IllegalStateException e) {
            isInternalFirstFrameSeeking = false;
            ALog.e(LOG_TAG, "updateFirstFrameVisibility seekTo failed, IllegalStateException.");
        } finally {
            mediaPlayerLock.unlock();
        }
        if (currentPlayer == null) {
            return;
        }
        try {
            currentPlayer.seekTo(0);
        } catch (IllegalStateException e) {
            mediaPlayerLock.lock();
            try {
                if (mediaPlayer == currentPlayer) {
                    isInternalFirstFrameSeeking = false;
                }
            } finally {
                mediaPlayerLock.unlock();
            }
            ALog.e(LOG_TAG, "updateFirstFrameVisibility seekTo failed, IllegalStateException.");
        }
    }

    @Override
    public void onActivityResume() {
        runAsync(() -> {
            if (isResumePlaying) {
                start(null);
                isResumePlaying = false;
                runOnUIThread(() -> {
                    firePlayStatusChange(true);
                });
            }
        });
    }

    @Override
    public void onActivityPause() {
        runAsync(() -> {
            isResumePlaying = (mediaPlayer != null) && mediaPlayer.isPlaying() && !isPaused;
            pause(null);
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
        isMediaPrepared = false;
        isPendingRenderFirstFrameUpdate = false;
        isInternalFirstFrameSeeking = false;
        shouldDelaySurfaceAttachForReset = false;
        shouldDelaySurfaceAttachForSourceSwitch = false;
        isManualStartFromStopped = false;

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
        isSetSurfaced = false;
        Surface surface = getSurface();
        if (surface != null) {
            ALog.i(LOG_TAG, "MediaPlayer SetSurface");
            mediaPlayer.setSurface(surface);
            isSetSurfaced = true;
        }
        try {
            mediaPlayer.setAudioAttributes(ATTR_VIDEO);
            if (!setDataSource(source)) {
                ALog.e(LOG_TAG, "setDataSource failed.");
                return false;
            }

            mediaPlayer.setOnPreparedListener(this);
            mediaPlayer.setOnErrorListener(this);
            mediaPlayer.setOnSeekCompleteListener(this);
            mediaPlayer.setOnCompletionListener(this);
            mediaPlayer.setOnBufferingUpdateListener(this);
            isMediaPrepared = false;
            mediaPlayer.prepareAsync();
            state = PlayState.PREPARED;
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
                    String filePath = "arkui-x" + File.separator + moduleName + File.separator + "ets" + param;
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
                            "js" + File.separator + moduleName + File.separator
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

    private boolean preparePlayerFromStopped(boolean isManualStart, String action) {
        try {
            isManualStartFromStopped = isManualStart;
            isStoped = false;
            if (mediaPlayer != null) {
                mediaPlayer.prepare();
                isMediaPrepared = true;
                state = PlayState.PREPARED;
            }
            return true;
        } catch (IOException ignored) {
            isManualStartFromStopped = false;
            isStoped = true;
            state = PlayState.STOPPED;
            ALog.e(LOG_TAG, action + " failed, IOException");
            return false;
        } catch (IllegalStateException ignored) {
            isManualStartFromStopped = false;
            isStoped = true;
            state = PlayState.STOPPED;
            ALog.e(LOG_TAG, action + " failed, IllegalStateException.");
            return false;
        }
    }

    private String updateResourceInternal(Map<String, String> params) {
        try {
            boolean isResetRequest = isResetRequest(params);
            prepareUpdateFlags(isResetRequest);
            detachSurfaceBeforeUpdate(isResetRequest);
            if (!resetAndPreparePlayer(params)) {
                resetUpdateFlags();
                return FAIL;
            }
            runOnUIThread(() -> handlePreparedOnUiThread());
            if (isResetRequest) {
                return keepFrameAfterReset();
            }
            return handleSourceSwitchAfterPrepare();
        } catch (IOException ignored) {
            resetUpdateFlags();
            ALog.e(LOG_TAG, "updateResource failed, IOException");
            return FAIL;
        } catch (IllegalStateException ignored) {
            resetUpdateFlags();
            ALog.e(LOG_TAG, "updateResource failed, IllegalStateException.");
            return FAIL;
        }
    }

    private void resetUpdateFlags() {
        isPendingRenderFirstFrameUpdate = false;
        shouldDelaySurfaceAttachForReset = false;
        shouldDelaySurfaceAttachForSourceSwitch = false;
        shouldNotifyCurrentTimeOnPrepared = false;
    }

    private boolean isResetRequest(Map<String, String> params) {
        if (!params.containsKey("reset")) {
            return false;
        }
        try {
            return Integer.parseInt(params.get("reset")) != 0;
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "isResetRequest failed: invalid reset value.");
            return false;
        }
    }

    private void prepareUpdateFlags(boolean isResetRequest) {
        isPendingRenderFirstFrameUpdate = !isResetRequest;
        shouldDelaySurfaceAttachForReset = isResetRequest;
        shouldDelaySurfaceAttachForSourceSwitch = !isResetRequest && !isShowFirstFrame;
        shouldNotifyCurrentTimeOnPrepared = isResetRequest;
    }

    private void detachSurfaceBeforeUpdate(boolean isResetRequest) {
        if (!isSetSurfaced) {
            return;
        }
        try {
            mediaPlayer.setSurface(null);
            isSetSurfaced = false;
        } catch (IllegalArgumentException ignored) {
            ALog.e(LOG_TAG, isResetRequest
                ? "updateResource detach current surface before reset failed, IllegalArgumentException."
                : "updateResource detach current surface before source switch failed, IllegalArgumentException.");
        }
    }

    private boolean resetAndPreparePlayer(Map<String, String> params) throws IOException {
        mediaPlayer.reset();
        state = PlayState.IDLE;
        position = 0;
        isSetSurfaced = false;
        if (!params.containsKey(KEY_SOURCE) || !setDataSource(params.get(KEY_SOURCE))) {
            return false;
        }
        mediaPlayer.prepare();
        isMediaPrepared = true;
        state = PlayState.PREPARED;
        return true;
    }

    private String keepFrameAfterReset() {
        setKeepScreenOn(false);
        return SUCCESS;
    }

    private void restoreSurfaceAfterResetIfNeeded(MediaPlayer targetPlayer) {
        if (targetPlayer == null || mediaPlayer != targetPlayer || isSetSurfaced) {
            return;
        }
        try {
            bindSurfaceForPlayer(targetPlayer);
        } catch (IllegalArgumentException ignored) {
            ALog.e(LOG_TAG, "reset restore surface failed, IllegalArgumentException.");
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "reset restore surface failed, IllegalStateException.");
        }
    }

    private String handleSourceSwitchAfterPrepare() {
        if (isShowFirstFrame) {
            return renderFirstFrameAfterSourceSwitch();
        }
        shouldDelaySurfaceAttachForSourceSwitch = false;
        isPendingRenderFirstFrameUpdate = false;
        bindSurfaceForCurrentPlayer();
        setKeepScreenOn(false);
        return SUCCESS;
    }

    private String renderFirstFrameAfterSourceSwitch() {
        shouldDelaySurfaceAttachForSourceSwitch = false;
        bindSurfaceForCurrentPlayer();
        if (!isSetSurfaced) {
            return SUCCESS;
        }
        isPendingRenderFirstFrameUpdate = false;
        mediaPlayer.start();
        state = PlayState.STARTED;
        mediaPlayer.pause();
        state = PlayState.PAUSED;
        setKeepScreenOn(false);
        return SUCCESS;
    }

    private String handleSetRenderFirstFrame() {
        if (shouldDelaySurfaceAttachForReset) {
            return SUCCESS;
        }
        if (shouldDelaySurfaceAttachForSourceSwitch && !isShowFirstFrame) {
            return SUCCESS;
        }
        if (shouldDelaySurfaceAttachForSourceSwitch) {
            shouldDelaySurfaceAttachForSourceSwitch = false;
        }
        boolean isSourceUpdatePending = isPendingRenderFirstFrameUpdate;
        isPendingRenderFirstFrameUpdate = false;
        maybeRestoreSurfaceForFirstFrame(isSourceUpdatePending);
        updateFirstFrameVisibility();
        return SUCCESS;
    }

    private void maybeRestoreSurfaceForFirstFrame(boolean isSourceUpdatePending) {
        if (!isShowFirstFrame || isSetSurfaced) {
            return;
        }
        mediaPlayerLock.lock();
        try {
            if (mediaPlayer == null) {
                return;
            }
            bindSurfaceForCurrentPlayer();
            refreshSourceUpdateFirstFrame(isSourceUpdatePending);
        } catch (IllegalArgumentException ignored) {
            ALog.e(LOG_TAG, "setRenderFirstFrame restore surface failed, IllegalArgumentException.");
        } catch (IllegalStateException ignored) {
            ALog.e(LOG_TAG, "setRenderFirstFrame refresh first frame failed, IllegalStateException.");
        } finally {
            mediaPlayerLock.unlock();
        }
    }

    private void refreshSourceUpdateFirstFrame(boolean isSourceUpdatePending) {
        if (!isSourceUpdatePending || !isShowFirstFrame || !isMediaPrepared || !isSetSurfaced) {
            return;
        }
        mediaPlayer.start();
        state = PlayState.STARTED;
        mediaPlayer.pause();
        state = PlayState.PAUSED;
        setKeepScreenOn(false);
    }

    private void bindSurfaceForCurrentPlayer() {
        Surface surface = getSurface();
        if (surface == null || mediaPlayer == null) {
            return;
        }
        mediaPlayer.setSurface(surface);
        isSetSurfaced = true;
    }

    private void bindSurfaceForPlayer(MediaPlayer targetPlayer) {
        Surface surface = getSurface();
        if (surface == null || targetPlayer == null) {
            return;
        }
        targetPlayer.setSurface(surface);
        if (mediaPlayer == targetPlayer) {
            isSetSurfaced = true;
        }
    }

    private void attachSurfaceIfNeededForSeek() {
        if (isSetSurfaced) {
            return;
        }
        Surface surface = getSurface();
        if (surface == null || mediaPlayer == null) {
            return;
        }
        mediaPlayer.setSurface(surface);
        isSetSurfaced = true;
        shouldDelaySurfaceAttachForReset = false;
        shouldDelaySurfaceAttachForSourceSwitch = false;
    }

    @Override
    public String runAsync(Runnable runnable) {
        asyncHandler.post(runnable);
        return SUCCESS;
    }

    private Surface getSurface() {
        Surface surface = null;
        if (isTexture) {
            surface = AceTextureHolder.getSurface(surfaceId);
        } else {
            surface = AceSurfaceHolder.getSurface(instanceId, surfaceId);
        }
        if (surface != null && !surface.isValid()) {
            ALog.w(LOG_TAG, "getSurface returns an invalid(released) surface, ignore.");
            return null;
        }
        return surface;
    }
}