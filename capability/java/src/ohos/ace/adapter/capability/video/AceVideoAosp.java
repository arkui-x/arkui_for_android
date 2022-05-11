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

package ohos.ace.adapter.capability.video;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.media.AudioAttributes;
import android.media.MediaPlayer;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.view.Surface;
import android.view.Window;
import android.view.WindowManager;

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

    private final MediaPlayer mediaPlayer;

    private final Context context;

    private final String instanceName;

    private final Window window;

    private final Handler mainHandler;

    private Handler asyncHandler;

    private HandlerThread handlerThread;

    /**
     * constructor of AceVideo on AOSP platform
     *
     * @param id the id of plugin
     * @param name name of plugin
     * @param surface the surface to render video content
     * @param context context of application
     * @param callback resource callback
     */
    public AceVideoAosp(long id, String name, Surface surface, Context context, IAceOnResourceEvent callback) {
        super(id, callback);
        this.instanceName = name;
        mediaPlayer = new MediaPlayer();
        mediaPlayer.setSurface(surface);
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
        mediaPlayer.stop();
        mediaPlayer.release();
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

        try {
            if (param.startsWith(FILE_SCHEME)) {
                mediaPlayer.setDataSource(param.substring(FILE_SCHEME.length()));
            } else if (param.startsWith(HAP_SCHEME)) {
                AssetManager assetManage = context.getAssets();
                if (assetManage == null) {
                    ALog.e(LOG_TAG, "setDataSource null assetManager");
                    return false;
                }
                AssetFileDescriptor assetFd;
                try {
                    assetFd = assetManage.openFd(
                        "js" + File.separator + instanceName + File.separator + param.substring(HAP_SCHEME.length()));
                } catch (IOException ignored) {
                    ALog.i(LOG_TAG, "not found asset in instance path, now begin to search asset in share path");
                    assetFd = assetManage.openFd(
                        "js" + File.separator + "share" + File.separator + param.substring(HAP_SCHEME.length()));
                }
                mediaPlayer.setDataSource(assetFd.getFileDescriptor(), assetFd.getStartOffset(), assetFd.getLength());
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
        if (isAutoPlay()) {
            mp.start();
            setKeepScreenOn(true);
        }

        if (isMute()) {
            mp.setVolume(0.0f, 0.0f);
        }
        runOnUIThread(() -> {
            firePrepared(mp.getVideoWidth(), mp.getVideoHeight(), mp.getDuration(), isAutoPlay(), false);
        });
    }

    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        ALog.i(LOG_TAG, "onError");
        runOnUIThread(() -> {
            fireError();
        });
        setKeepScreenOn(false);
        return false;
    }

    @Override
    public void onCompletion(MediaPlayer mp) {
        runOnUIThread(() -> {
            fireCompletion();
        });
        setKeepScreenOn(false);
    }

    @Override
    public void onSeekComplete(MediaPlayer mp) {
        runOnUIThread(() -> {
            fireSeekComplete(mediaPlayer.getCurrentPosition() / SECOND_TO_MSEC);
        });
    }

    @Override
    public void onBufferingUpdate(MediaPlayer mp, int percent) {
        runOnUIThread(() -> {
            fireBufferingUpdate(percent);
        });
    }

    @Override
    public String start(Map<String, String> params) {
        ALog.i(LOG_TAG, "start param:" + params);
        mediaPlayer.start();
        setKeepScreenOn(true);
        return SUCCESS;
    }

    @Override
    public String pause(Map<String, String> params) {
        ALog.i(LOG_TAG, "pause param:" + params);
        mediaPlayer.pause();
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
        try {
            int msec = Integer.parseInt(params.get(KEY_VALUE)) * SECOND_TO_MSEC;
            mediaPlayer.seekTo(msec);
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "NumberFormatException, seek failed. value = " + params.get(KEY_VALUE));
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
        try {
            float volume = Float.parseFloat(params.get(KEY_VALUE));
            mediaPlayer.setVolume(volume, volume);
        } catch (NumberFormatException ignored) {
            ALog.e(LOG_TAG, "NumberFormatException, setVolume failed. value = " + params.get(KEY_VALUE));
            return FAIL;
        }
        return SUCCESS;
    }

    @Override
    public String getPosition(Map<String, String> params) {
        // the current position in milliseconds.
        int position = mediaPlayer.getCurrentPosition();
        runOnUIThread(() -> {
            fireGetCurrenttime(position / SECOND_TO_MSEC);
        });
        return "currentpos=" + position / SECOND_TO_MSEC;
    }

    @Override
    public String enableLooping(Map<String, String> params) {
        if (params == null) {
            return FAIL;
        }

        try {
            if (params.containsKey("loop")) {
                if (Integer.parseInt(params.get("loop")) == 1) {
                    mediaPlayer.setLooping(true);
                } else {
                    mediaPlayer.setLooping(false);
                }
            }
            return SUCCESS;
        } catch (NumberFormatException ignored) {
            ALog.w(LOG_TAG, "NumberFormatException");
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
            float speed = Float.parseFloat(params.get(KEY_VALUE));
            //mediaPlayer.setPlaybackParams(mediaPlayer.easyPlaybackParams(speed, 0));
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
    public void onActivityResume() {
    }

    @Override
    public void onActivityPause() {
        runAsync(() -> {
            if (mediaPlayer.isPlaying()) {
                mediaPlayer.pause();
                setKeepScreenOn(false);
                runOnUIThread(() -> {
                    firePlayStatusChange(false);
                });
            }
        });
    }

    private void setKeepScreenOn(boolean screenOn) {
        if (screenOn) {
            runOnUIThread(() -> {
                if (window != null) {
                    window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                }
            });
        } else {
            runOnUIThread(() -> {
                if (window != null) {
                    window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                }
            });
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

    @Override
    public String runAsync(Runnable runnable) {
        asyncHandler.post(runnable);
        return SUCCESS;
    }
}