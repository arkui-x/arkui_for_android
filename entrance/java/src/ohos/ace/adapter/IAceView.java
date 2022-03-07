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

package ohos.ace.adapter;

/**
 * The common interface of the AceView
 * 
 */
public interface IAceView {
    /**
     * The initial state of surface
     */
    int SURFACE_STATE_UNINITIALIZED = 0;

    /**
     * When surface is created
     */
    int SURFACE_STATE_CREATED = 1;

    /**
     * When surface is being shown
     */
    int SURFACE_STATE_SHOWING = 2;

    /**
     * When surface is set as hidden by system
     */
    int SURFACE_STATE_HIDDEN = 3;

    /**
     * Image cache path
     */
    String CACHE_IMAGE_PATH_NAME = "cache_images";

    /**
     * File cache path
     */
    String CACHE_FILE_NAME = "cache_files";

    /**
     * view port metrics
     */
    class ViewPortMetrics {
        public float devicePixelRatio = 1.0f;

        public int physicialWidth = 0;

        public int physicialHeight = 0;

        public int physicialPaddingTop = 0;

        public int physicialPaddingRight = 0;

        public int physicialPaddingBottom = 0;

        public int physicialPaddingLeft = 0;

        public int physicialViewInsetTop = 0;

        public int physicialViewInsetRight = 0;

        public int physicialViewInsetBottom = 0;

        public int physicialViewInsetLeft = 0;

        public int systemGestureInsetTop = 0;

        public int systemGestureInsetRight = 0;

        public int systemGestureInsetBottom = 0;

        public int systemGestureInsetLeft = 0;
    }

    /**
     * Get native pointer of aceView
     * 
     * @return the NativePtr of AceView
     */
    long getNativePtr();

    /**
     * Release native view resources.
     * 
     */
    void releaseNativeView();

    /**
     * Called when view hide.
     * 
     */
    void onHide();

    /**
     * Called when view show.
     * 
     */
    void onShow();

    /**
     * Called for init device type.
     */
    void initDeviceType();

    /**
     * Called then aceView created.
     * 
     */
    void viewCreated();

    /**
     * Called to add resource plugin.
     * 
     * @param plugin the plugin
     */
    void addResourcePlugin(AceResourcePlugin plugin);
}
