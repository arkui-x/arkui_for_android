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

package ohos.ace.adapter;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * The environment for ACE runtime, the bridge between Java and C++
 *
 * @since 1
 */
public final class AceEnv {
    private static final String LOG_TAG = "AceEnv";

    /**
     * The type of platform, keep same with native
     */
    public static final int ACE_PLATFORM_ANDROID = 0;

    private static volatile AceEnv INSTANCE = null;

    private static final Object INSTANCE_LOCK = new Object();

    private IAceViewCreator creator;

    private int containerType = AceContainer.CONTAINER_TYPE_JS;

    private boolean isLoadSuccess = false;

    private AceEnv() {
        ALog.i(LOG_TAG, "AceEnv init start.");
        isLoadSuccess = LibraryLoader.loadJniLibrary();
        if (!isLoadSuccess) {
            ALog.e(LOG_TAG, "LoadLibrary failed");
        }
    }

    /**
     * Get instance of current env
     * @return instance of current env
     */
    public static AceEnv getInstance() {
        if (INSTANCE != null) {
            return INSTANCE;
        }

        synchronized (INSTANCE_LOCK) {
            if (INSTANCE != null) {
                return INSTANCE;
            }

            INSTANCE = new AceEnv();
            return INSTANCE;
        }
    }

    /**
     * Set library loaded flag
     *
     */
    public void setLibraryLoaded() {
        isLoadSuccess = true;
    }

    /**
     * Get whether the library is loaded
     *
     * @return the result for library status
     */
    public boolean isLibraryLoaded() {
        return isLoadSuccess;
    }

    /**
     * Set the view creator
     *
     * @param creator the creator to create AceView
     */
    public static void setViewCreator(IAceViewCreator creator) {
        getInstance().setViewCreatorInner(creator);
    }

    private void setViewCreatorInner(IAceViewCreator creator) {
        this.creator = creator;
    }

    /**
     * Set the type of container
     *
     * @param type the type to set
     */
    public static void setContainerType(int type) {
        getInstance().setContainerTypeInner(type);
    }

    private void setContainerTypeInner(int type) {
        if (type >= AceContainer.CONTAINER_TYPE_UNKNOWN && type <= AceContainer.CONTAINER_TYPE_DECLARATIVE_JS) {
            containerType = type;
        }
    }

    /**
     * Create a new container with parameters
     *
     * @param callback the callback interface the receive callback event
     * @param instanceId the id of container instance
     * @param name the name of the container
     * @return the new container
     */
    public static AceContainer createContainer(AceEventCallback callback, int instanceId, String name) {
        return getInstance().createContainerInner(callback, instanceId, name);
    }

    private AceContainer createContainerInner(AceEventCallback callback, int instanceId, String name) {
        if (creator == null) {
            return null;
        }
        return new AceContainer(instanceId, containerType, creator, callback, name);
    }

    /**
     * Destroy the container
     *
     * @param container the container to destroy
     */
    public static void destroyContainer(AceContainer container) {
        getInstance().destroyContainerInner(container);
    }

    private void destroyContainerInner(AceContainer container) {
        if (container == null) {
            return;
        }
        container.destroyContainer();
    }

    /**
     * Dump the debug information
     *
     * @param prefix prefix string of dump command
     * @param fd the file descriptor where to dump
     * @param writer the writer to write dump info
     * @param args the args of dump command
     */
    public static void dump(String prefix, FileDescriptor fd, PrintWriter writer, String[] args) {
        DumpHelper.dump(prefix, fd, writer, args);
    }

    /**
     * Set up first frame handler
     *
     * @param platform current platform type
     */
    public void setupFirstFrameHandler(int platform) {
        nativeSetupFirstFrameHandler(platform);
    }

    private native void nativeSetupFirstFrameHandler(int platform);
}
