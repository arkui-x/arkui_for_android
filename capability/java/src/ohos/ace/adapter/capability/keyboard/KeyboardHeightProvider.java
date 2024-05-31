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

package ohos.ace.adapter.capability.keyboard;

import java.util.HashMap;
import java.util.Map;

import android.app.Activity;
import android.content.res.Configuration;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.drawable.ColorDrawable;
import android.view.Gravity;
import android.view.View;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.WindowManager.LayoutParams;
import android.widget.PopupWindow;

/**
 * Keyboard Height Provider.
 *
 * @since 10
 */
public class KeyboardHeightProvider extends PopupWindow implements OnGlobalLayoutListener {
    private static final String LOG_TAG = "KeyboardHeightProvider";
    private KeyboardHeightObserver observer;
    private View popupView;
    private View parentView;
    private Activity activity;
    private int lastKeyboardHeight = 0;
    private int lastPortraitVisibleHeight = -1;
    private int navigationBarHeight = 0;
    private float navigationBarMaxHeightRate = 0.1f;
    private Map<Integer, Integer> bottomMaxMap = null;
    private Map<Integer, Integer> screenSizeYMap = null;

    /**
     * Constructor.
     * @param activity the activity
     */
    public KeyboardHeightProvider(Activity activity) {
        super(activity);
        this.activity = activity;
        this.popupView = new View(activity);
        this.bottomMaxMap = new HashMap<Integer, Integer>();
        this.screenSizeYMap = new HashMap<Integer, Integer>();

        setContentView(popupView);
        setSoftInputMode(LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        setInputMethodMode(PopupWindow.INPUT_METHOD_NEEDED);
        setWidth(0);
        setHeight(LayoutParams.MATCH_PARENT);
        popupView.getViewTreeObserver().addOnGlobalLayoutListener(this);
    }

    @Override
    public void onGlobalLayout() {
        if (popupView != null) {
            popupView.post(new Runnable() {
                @Override
                public void run() {
                    handleOnGlobalLayout();
                }
            });
        }
    }

    private int getScreenOrientation() {
        return activity.getResources().getConfiguration().orientation;
    }

    private void handleOnGlobalLayout() {
        Point screenSize = new Point();
        activity.getWindowManager().getDefaultDisplay().getSize(screenSize);
        int orientation = getScreenOrientation();

        Integer screenSizeY = screenSizeYMap.get(orientation);
        if (screenSizeY == null || screenSizeY.intValue() < screenSize.y) {
            if (screenSizeY != null) {
                navigationBarHeight = 0;
            }
            screenSizeY = screenSize.y;
            screenSizeYMap.put(orientation, screenSizeY);
        }

        int navigationBarMaxHeight = (int)(screenSizeY * navigationBarMaxHeightRate);
        if (screenSizeY.intValue() > screenSize.y) {
            if ((screenSizeY - screenSize.y) > navigationBarMaxHeight) {
                return;
            } else {
                navigationBarHeight = screenSizeY - screenSize.y;
            }
            screenSizeY = screenSize.y;
            screenSizeYMap.put(orientation, screenSizeY);
        }

        int bottomMax = 0;
        Integer bm = this.bottomMaxMap.get(orientation);
        if (bm != null) {
            bottomMax = bm.intValue();
        }

        Rect rect = new Rect();
        popupView.getWindowVisibleDisplayFrame(rect);
        if (rect.bottom < 0) {
            return;
        }

        if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
            if (rect.bottom > rect.right) {
                return;
            }
            if (bottomMax < rect.bottom) {
                bottomMax = rect.bottom;
                this.bottomMaxMap.put(orientation, bottomMax);
            }
            if (bottomMax < screenSizeY) {
                bottomMax = screenSizeY;
                this.bottomMaxMap.put(orientation, bottomMax);
            }
        } else {
            if (bottomMax < rect.bottom) {
                bottomMax = rect.bottom;
                this.bottomMaxMap.put(orientation, bottomMax);
            }
        }

        if (navigationBarHeight > 0 && ((bottomMax - rect.bottom) == navigationBarHeight)) {
            bottomMax = rect.bottom;
            this.bottomMaxMap.put(orientation, bottomMax);
        }

        int keyboardHeight = bottomMax - rect.bottom;
        if (keyboardHeight > screenSizeY) {
            return;
        }

        if (keyboardHeight < navigationBarMaxHeight) {
            bottomMax = bottomMax - keyboardHeight;
            this.bottomMaxMap.put(orientation, bottomMax);
            keyboardHeight = 0;
        }

        int visibleHeight = rect.bottom - rect.top;
        if (orientation == Configuration.ORIENTATION_PORTRAIT) {
            if ((keyboardHeight > bottomMax / 2) && (visibleHeight != popupView.getHeight())) {
                return;
            }
            if (visibleHeight <= lastPortraitVisibleHeight) {
                return;
            } else {
                lastPortraitVisibleHeight = -1;
            }
        }

        if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
            lastPortraitVisibleHeight = visibleHeight;
            if (lastPortraitVisibleHeight == 0) {
                lastPortraitVisibleHeight = popupView.getHeight();
            }
        }

        if (lastKeyboardHeight != keyboardHeight) {
            notifyKeyboardHeightChanged(keyboardHeight);
            lastKeyboardHeight = keyboardHeight;
        }
    }

    private void notifyKeyboardHeightChanged(int height) {
        if (observer != null) {
            observer.onKeyboardHeightChanged(height);
        }
    }

    /**
     * Set keyboard height observer.
     * @param observer the observer
     */
    public void setKeyboardHeightObserver(KeyboardHeightObserver observer) {
        this.observer = observer;
    }

    /**
     * Start the provider.
     */
    public void start() {
        parentView = activity.getWindow().getDecorView();
        if (!isShowing() && parentView.getWindowToken() != null) {
            setBackgroundDrawable(new ColorDrawable(0));
            parentView.post(new Runnable() {
                @Override
                public void run() {
                    showAtLocation(parentView, Gravity.NO_GRAVITY, 0, 0);
                }
            });
        }
    }

    /**
     * Close the provider.
     */
    public void close() {
        this.observer = null;
        dismiss();
    }
}