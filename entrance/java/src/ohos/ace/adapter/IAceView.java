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
 * @since 1
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
        /**
         * the dpi of viewport
         */
        public float devicePixelRatio = 1.0f;

        /**
         * the physicial width of viewport
         */
        public int physicialWidth = 0;

        /**
         * the physicial height of viewport
         */
        public int physicialHeight = 0;

        /**
         * the physicial top padding of viewport
         */
        public int physicialPaddingTop = 0;

        /**
         * the physicial right padding of viewport
         */
        public int physicialPaddingRight = 0;

        /**
         * the physicial bottom padding of viewport
         */
        public int physicialPaddingBottom = 0;

        /**
         * the physicial left padding of viewport
         */
        public int physicialPaddingLeft = 0;

        /**
         * the physicial top view inset of viewport
         */
        public int physicialViewInsetTop = 0;

        /**
         * the physicial right view inset of viewport
         */
        public int physicialViewInsetRight = 0;

        /**
         * the physicial bottom view inset of viewport
         */
        public int physicialViewInsetBottom = 0;

        /**
         * the physicial left view inset of viewport
         */
        public int physicialViewInsetLeft = 0;

        /**
         * the system top gesture inset of viewport
         */
        public int systemGestureInsetTop = 0;

        /**
         * the system right gesture inset of viewport
         */
        public int systemGestureInsetRight = 0;

        /**
         * the system bottom gesture inset of viewport
         */
        public int systemGestureInsetBottom = 0;

        /**
         * the system left gesture inset of viewport
         */
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
     *
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
