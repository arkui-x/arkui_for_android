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
package ohos.ace.adapter;

import android.app.Activity;
import android.util.Log;
import android.view.View;
import android.widget.PopupWindow;
import android.graphics.drawable.BitmapDrawable;

public class SubWindow {

    private static final String TAG = "SubWindow";

    public static class WindowParam {
        public int backgroundColor;
        public int width;
        public int height;
        public int x;
        public int y;

        public WindowParam() {
            backgroundColor = 0;
            width = 100;
            height = 100;
            x = 0;
            y = 0;
        }
    }

    public static enum WindowMode {
        WINDOW_MODE_UNDEFINED(0),
        WINDOW_MODE_FULLSCREEN(1),
        WINDOW_MODE_SPLIT_PRIMARY(100),
        WINDOW_MODE_SPLIT_SECONDARY(101),
        WINDOW_MODE_FLOATING(102),
        WINDOW_MODE_PIP(103);

        public int getMode() {
            return mode;
        }

        public String getName() {
            String name = "UNKONOW";
            switch (mode) {
                case 0:
                    name = "WINDOW_MODE_UNDEFINED";
                    break;
                case 1:
                    name = "WINDOW_MODE_FULLSCREEN";
                    break;
                case 100:
                    name = "WINDOW_MODE_SPLIT_PRIMARY";
                    break;
                case 101:
                    name = "WINDOW_MODE_SPLIT_SECONDARY";
                    break;
                case 102:
                    name = "WINDOW_MODE_FLOATING";
                    break;
                case 103:
                    name = "WINDOW_MODE_PIP";
                    break;
                default:
                    break;
            }
            return name;
        }

        private WindowMode(int mode) {
            this.mode = mode;
        }

        private int mode;
    }

    public static enum WindowType {
        WINDOW_TYPE_APP_MAIN_WINDOW(1),
        WINDOW_TYPE_MEDIA(1000),
        WINDOW_TYPE_APP_SUB_WINDOW(1001),
        WINDOW_TYPE_APP_COMPONENT(1002);

        public int getMode() {
            return mode;
        }

        public String getName() {
            String name = "UNKONOW";
            switch (mode) {
                case 1:
                    name = "WINDOW_TYPE_APP_MAIN_WINDOW";
                    break;
                case 1000:
                    name = "WINDOW_TYPE_MEDIA";
                    break;
                case 1001:
                    name = "WINDOW_TYPE_APP_SUB_WINDOW";
                    break;
                case 1002:
                    name = "WINDOW_TYPE_APP_COMPONENT";
                    break;
                default:
                    break;
            }
            return name;
        }

        private WindowType(int mode) {
            this.mode = mode;
        }

        private int mode;
        }


    public SubWindow(Activity activity, String name) {
        this.name = name;
        this.windowId = hashCode();
        rootActivity = activity;
        rootView = rootActivity.getWindow().getDecorView();
        windowParam = new WindowParam();
    }

    public void createSubWindow(int type, int mode, int tag, int parentId, int x, int y, int width, int height) {
        updateWindowParam(type, mode, tag, parentId, x, y, width, height);
        createSubWindow(windowParam);
    }

    public void createSubWindow(WindowParam param) {
        Log.d(TAG, "createSubwindow called. name=" + name);
        subWindowView = new PopupWindow();
        setContentView(new WindowView(rootActivity));
    }

    public void updateWindowParam(int type, int mode, int tag, int parentId, int x, int y, int width, int height) {
        this.windowType = type;
        this.windowMode = mode;
        this.windowTag = tag;
        this.parentId = parentId;
        this.windowParam.x = x;
        this.windowParam.y = y;

        if(width > 0)
            this.windowParam.width = width;

        if(height > 0)
            this.windowParam.height = height;
    }

    public void showWindow() {
        subWindowView.setWidth(windowParam.width);
        subWindowView.setHeight(windowParam.height);
        subWindowView.setOutsideTouchable(false);
        subWindowView.setFocusable(false);
        subWindowView.setElevation(0);
        subWindowView.setTouchable(true);
        subWindowView.setBackgroundDrawable(new BitmapDrawable());

        subWindowView.setContentView(contentView);

        subWindowView.showAsDropDown(rootView, windowParam.x, windowParam.y);
    }

    public void updateWindow() {
        subWindowView.setContentView(contentView);
        subWindowView.update(windowParam.x, windowParam.y, windowParam.width, windowParam.height, true);
    }

    public void showWindow(View anchor) {
        subWindowView.showAsDropDown(anchor);
    }

    public void showWindow(View anchor, int xoffset, int yoffset) {
        subWindowView.showAsDropDown(anchor, xoffset, yoffset);
    }

    public void resize(int width, int height) {
        windowParam.width = width;
        windowParam.height = height;
        updateWindow();
    }

    public void moveWindowTo(int x, int y) {
        windowParam.x = y;
        windowParam.y = y;
        updateWindow();
    }

    public void destroyWindow() {
        subWindowView.dismiss();
    }

    ///////////////////////////Members Getter & Setter/////////////////////////////////////////////
    private String name;
    private int windowId;
    private int parentId;
    private int windowMode;
    private int windowType;
    private WindowParam windowParam;
    private PopupWindow subWindowView;
    private View contentView;
    private Activity rootActivity;
    private View rootView;

    public int getWindowTag() {
        return windowTag;
    }

    public void setWindowTag(int windowTag) {
        this.windowTag = windowTag;
    }

    private int windowTag;

    public PopupWindow getSubWindowView() {
        return subWindowView;
    }

    public void setSubWindowView(PopupWindow subWindowView) {
        this.subWindowView = subWindowView;
    }


    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getWindowId() {
        return windowId;
    }

    public void setWindowId(int windowId) {
        this.windowId = windowId;
    }

    public int getParentId() {
        return parentId;
    }

    public void setParentId(int parentId) {
        this.parentId = parentId;
    }

    public int getWindowMode() {
        return windowMode;
    }

    public void setWindowMode(int windowMode) {
        this.windowMode = windowMode;
    }

    public int getWindowType() {
        return windowType;
    }

    public void setWindowType(int windowType) {
        this.windowType = windowType;
    }

    public View getContentView() {
        return contentView;
    }

    public void setContentView(View contentView) {
        this.contentView = contentView;
    }

    public WindowParam getWindowParam() {
        return windowParam;
    }

    public void setWindowParam(WindowParam param) {
        this.windowParam = param;
    }

    public Activity getRootActivity() {
        return rootActivity;
    }

    public void setRootActivity(Activity rootActivity) {
        this.rootActivity = rootActivity;
    }

    public View getRootView() {
        return rootView;
    }

    public void setRootView(View rootView) {
        this.rootView = rootView;
    }

}
