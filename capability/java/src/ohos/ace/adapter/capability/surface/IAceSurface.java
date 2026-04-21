/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

package ohos.ace.adapter.capability.surface;

import java.nio.ByteBuffer;

/**
 * The Surface interface defined from ACE engine.
 *
 * @since 12
 */
public interface IAceSurface {
    /**
     * Attach surface to native and get the pointer.
     *
     * @param surface surface object
     * @return the native surface pointer.
     */
    long attachNaitveSurface(Object surface);

    /**
     * Create a direct byte buffer from a native pointer.
     *
     * @param pointer native memory pointer
     * @param bufferSize buffer size in bytes
     * @return direct byte buffer wrapping the native memory
     */
    ByteBuffer createDirectBufferFromPointer(long pointer, long bufferSize);
}