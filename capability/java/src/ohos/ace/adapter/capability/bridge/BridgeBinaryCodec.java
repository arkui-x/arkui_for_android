/**
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

package ohos.ace.adapter.capability.bridge;

import java.io.ByteArrayOutputStream;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import ohos.ace.adapter.ALog;

public class BridgeBinaryCodec implements BridgeBaseCodec<Object> {

    private static final Object INSTANCE_LOCK = new Object();

    private static volatile BridgeBinaryCodec INSTANCE = null;

    /**
     * Get BridgeBinaryCodec object.
     *
     * @return The BridgeBinaryCodec object.
     */
    public static BridgeBinaryCodec getInstance() {
        if (INSTANCE != null) {
            return INSTANCE;
        }

        synchronized (INSTANCE_LOCK) {
            if (INSTANCE != null) {
                return INSTANCE;
            }

            INSTANCE = new BridgeBinaryCodec();
            return INSTANCE;
        }
    }

    @Override
    /**
     * Encode data.
     *
     * @param data Data to be encoded.
     * @return Return encode data.
     */
    public ByteBuffer encodeData(Object data) {
        ByteArrayOutputStreamExposed stream = new ByteArrayOutputStreamExposed();
        BridgeSerializer.writeData(stream, data);
        ByteBuffer buffer = ByteBuffer.allocateDirect(stream.size());
        buffer.put(stream.buffer(), 0, stream.size());
        return buffer;
    }

    @Override
    /**
     * Decode data.
     *
     * @param data Data to be decoded.
     * @return Return decode data.
     */
    public Object decodeData(ByteBuffer byteBuffer) {
        if (byteBuffer == null) {
            return null;
        }
        byteBuffer.order(ByteOrder.nativeOrder());
        Object data = BridgeSerializer.readData(byteBuffer);
        if (byteBuffer.hasRemaining()) {
            ALog.e("BridgeBinaryCodec", "Buffer not fully resolved");
            return null;
        }
        return data;
    }
private static class ByteArrayOutputStreamExposed extends ByteArrayOutputStream {
    public byte[] buffer() {
        return buf;
    }
}
}