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
import android.graphics.Color;
import android.util.Log;
import android.view.View;
import android.widget.PopupWindow;

/**
 * The type Sub window.
 */
public class SubWindow {

    private static final String TAG = "SubWindow";

    /**
     * The type Window param.
     */
    public static class WindowParam {
        /**
         * The Background color.
         */
        public int backgroundColor;
        /**
         * The Width.
         */
        public int width;
        /**
         * The Height.
         */
        public int height;
        /**
         * The X.
         */
        public int x;
        /**
         * The Y.
         */
        public int y;

        /**
         * Instantiates a new Window param.
         */
        public WindowParam() {
            backgroundColor = 0;
            width = 100;
            height = 100;
            x = 0;
            y = 0;
        }
    }


    /**
     * Instantiates a new Sub window.
     *
     * @param activity the activity
     * @param name     the name
     */
    public SubWindow(Activity activity, String name) {
        this.name = name;
        this.windowId = hashCode();
        rootActivity = activity;
        rootView = rootActivity.getWindow().getDecorView();
        windowParam = new WindowParam();
    }

    /**
     * Create sub window.
     *
     * @param type     the type
     * @param mode     the mode
     * @param tag      the tag
     * @param parentId the parent id
     * @param x        the x
     * @param y        the y
     * @param width    the width
     * @param height   the height
     */
    public void createSubWindow(int type, int mode, int tag, int parentId, int x, int y, int width, int height) {
        updateWindowParam(type, mode, tag, parentId, x, y, width, height);
        createSubWindow(windowParam);
    }

    /**
     * Create sub window.
     *
     * @param param the param
     */
    public void createSubWindow(WindowParam param) {
        Log.d(TAG, "createSubwindow called. name=" + name);
        subWindowView = new PopupWindow();
        setContentView(new WindowView(rootActivity));
    }

    /**
     * Update window param.
     *
     * @param type     the type
     * @param mode     the mode
     * @param tag      the tag
     * @param parentId the parent id
     * @param x        the x
     * @param y        the y
     * @param width    the width
     * @param height   the height
     */
    public void updateWindowParam(int type, int mode, int tag, int parentId, int x, int y, int width, int height) {
        this.windowType = type;
        this.windowMode = mode;
        this.windowTag = tag;
        this.parentId = parentId;
        this.windowParam.x = x;
        this.windowParam.y = y;

        if (width > 0) {
            this.windowParam.width = width;
        }

        if (height > 0) {
            this.windowParam.height = height;
        }
    }

    /**
     * Show window.
     */
    public void showWindow() {
        subWindowView.setWidth(windowParam.width);
        subWindowView.setHeight(windowParam.height);
        subWindowView.setOutsideTouchable(false);
        subWindowView.setFocusable(false);
        subWindowView.setElevation(0);
        subWindowView.setTouchable(true);
        contentView.setBackgroundColor(Color.GRAY);

        subWindowView.setContentView(contentView);

        subWindowView.showAsDropDown(rootView, windowParam.x, windowParam.y);
    }

    /**
     * Update window.
     */
    public void updateWindow() {
        subWindowView.setContentView(contentView);
        subWindowView.update(windowParam.x, windowParam.y, windowParam.width, windowParam.height, true);
    }

    /**
     * Show window.
     *
     * @param anchor the anchor
     */
    public void showWindow(View anchor) {
        subWindowView.showAsDropDown(anchor);
    }

    /**
     * Show window.
     *
     * @param anchor  the anchor
     * @param xoffset the xoffset
     * @param yoffset the yoffset
     */
    public void showWindow(View anchor, int xoffset, int yoffset) {
        subWindowView.showAsDropDown(anchor, xoffset, yoffset);
    }

    /**
     * Resize.
     *
     * @param width  the width
     * @param height the height
     */
    public void resize(int width, int height) {
        windowParam.width = width;
        windowParam.height = height;
        updateWindow();
    }

    /**
     * Move window to.
     *
     * @param x the x
     * @param y the y
     */
    public void moveWindowTo(int x, int y) {
        windowParam.x = y;
        windowParam.y = y;
        updateWindow();
    }

    /**
     * Destroy window.
     */
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

    /**
     * Gets window tag.
     *
     * @return the window tag
     */
    public int getWindowTag() {
        return windowTag;
    }

    /**
     * Sets window tag.
     *
     * @param windowTag the window tag
     */
    public void setWindowTag(int windowTag) {
        this.windowTag = windowTag;
    }

    private int windowTag;

    /**
     * Gets sub window view.
     *
     * @return the sub window view
     */
    public PopupWindow getSubWindowView() {
        return subWindowView;
    }

    /**
     * Sets sub window view.
     *
     * @param subWindowView the sub window view
     */
    public void setSubWindowView(PopupWindow subWindowView) {
        this.subWindowView = subWindowView;
    }

    /**
     * Gets name.
     *
     * @return the name
     */
    public String getName() {
        return name;
    }

    /**
     * Sets name.
     *
     * @param name the name
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * Gets window id.
     *
     * @return the window id
     */
    public int getWindowId() {
        return windowId;
    }

    /**
     * Sets window id.
     *
     * @param windowId the window id
     */
    public void setWindowId(int windowId) {
        this.windowId = windowId;
    }

    /**
     * Gets parent id.
     *
     * @return the parent id
     */
    public int getParentId() {
        return parentId;
    }

    /**
     * Sets parent id.
     *
     * @param parentId the parent id
     */
    public void setParentId(int parentId) {
        this.parentId = parentId;
    }

    /**
     * Gets window mode.
     *
     * @return the window mode
     */
    public int getWindowMode() {
        return windowMode;
    }

    /**
     * Sets window mode.
     *
     * @param windowMode the window mode
     */
    public void setWindowMode(int windowMode) {
        this.windowMode = windowMode;
    }

    /**
     * Gets window type.
     *
     * @return the window type
     */
    public int getWindowType() {
        return windowType;
    }

    /**
     * Sets window type.
     *
     * @param windowType the window type
     */
    public void setWindowType(int windowType) {
        this.windowType = windowType;
    }

    /**
     * Gets content view.
     *
     * @return the content view
     */
    public View getContentView() {
        return contentView;
    }

    /**
     * Sets content view.
     *
     * @param contentView the content view
     */
    public void setContentView(View contentView) {
        this.contentView = contentView;
    }

    /**
     * Gets window param.
     *
     * @return the window param
     */
    public WindowParam getWindowParam() {
        return windowParam;
    }

    /**
     * Sets window param.
     *
     * @param param the param
     */
    public void setWindowParam(WindowParam param) {
        this.windowParam = param;
    }

    /**
     * Gets root activity.
     *
     * @return the root activity
     */
    public Activity getRootActivity() {
        return rootActivity;
    }

    /**
     * Sets root activity.
     *
     * @param rootActivity the root activity
     */
    public void setRootActivity(Activity rootActivity) {
        this.rootActivity = rootActivity;
    }

    /**
     * Gets root view.
     *
     * @return the root view
     */
    public View getRootView() {
        return rootView;
    }

    /**
     * Sets root view.
     *
     * @param rootView the root view
     */
    public void setRootView(View rootView) {
        this.rootView = rootView;
    }

}
