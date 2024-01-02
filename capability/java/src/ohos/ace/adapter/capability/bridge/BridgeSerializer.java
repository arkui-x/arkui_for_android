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
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import ohos.ace.adapter.ALog;

/**
 * Serializer used by Bridge.
 *
 * @since 10
 */
public class BridgeSerializer {
    private static final String LOG_TAG = "BridgeSerializer";

    private static final boolean IS_LITTLE_ENDIAN = ByteOrder.nativeOrder() == ByteOrder.LITTLE_ENDIAN;

    private static final String UTF8 = "UTF8";

    private static final int ALMOST = 254;

    private static final int SIZE_ERROR = -1;

    private static final byte T_NULL = 0;

    private static final byte T_TRUE = 1;

    private static final byte T_FALSE = 2;

    private static final byte T_INT32 = 3;

    private static final byte T_INT64 = 4;

    private static final byte T_DOUBLE = 5;

    private static final byte T_STRING = 6;

    private static final byte T_LIST_UINT8 = 7;

    private static final byte T_LIST_BOOL = 8;

    private static final byte T_LIST_INT32 = 9;

    private static final byte T_LIST_INT64 = 10;

    private static final byte T_LIST_DOUBLE = 11;

    private static final byte T_LIST_STRING = 12;

    private static final byte T_MAP = 13;

    private static final byte T_COMPOSITE_LIST = 14;

    private static int readSize(ByteBuffer buffer) {
        if (!buffer.hasRemaining()) {
            ALog.e("BridgeBinaryCodec", "No buffer left");
            return SIZE_ERROR;
        }
        int data = buffer.get() & 0xff;
        if (data < ALMOST) {
            return data;
        } else if (data == ALMOST) {
            return buffer.getChar();
        } else {
            return buffer.getInt();
        }
    }

    private static byte[] readBytes(ByteBuffer buffer) {
        int length = readSize(buffer);
        if (length == SIZE_ERROR) {
            return null;
        }
        byte[] bytes = new byte[length];
        buffer.get(bytes);
        return bytes;
    }

    private static void writeInt(ByteArrayOutputStream stream, int data) {
        if (IS_LITTLE_ENDIAN) {
            stream.write(data);
            stream.write(data >>> 8);
            stream.write(data >>> 16);
            stream.write(data >>> 24);
        } else {
            stream.write(data >>> 24);
            stream.write(data >>> 16);
            stream.write(data >>> 8);
            stream.write(data);
        }
    }

    private static void writeLong(ByteArrayOutputStream stream, long data) {
        if (IS_LITTLE_ENDIAN) {
            stream.write((byte) data);
            stream.write((byte) (data >>> 8));
            stream.write((byte) (data >>> 16));
            stream.write((byte) (data >>> 24));
            stream.write((byte) (data >>> 32));
            stream.write((byte) (data >>> 40));
            stream.write((byte) (data >>> 48));
            stream.write((byte) (data >>> 56));
        } else {
            stream.write((byte) (data >>> 56));
            stream.write((byte) (data >>> 48));
            stream.write((byte) (data >>> 40));
            stream.write((byte) (data >>> 32));
            stream.write((byte) (data >>> 24));
            stream.write((byte) (data >>> 16));
            stream.write((byte) (data >>> 8));
            stream.write((byte) data);
        }
    }

    private static void writeAlignment(ByteArrayOutputStream stream, int alignment) {
        int num = stream.size() % alignment;
        if (num != 0) {
            for (int i = 0; i < alignment - num; i++) {
                stream.write(0);
            }
        }
    }

    private static void writeDouble(ByteArrayOutputStream stream, double data) {
        writeLong(stream, Double.doubleToLongBits(data));
    }

    private static void writeString(ByteArrayOutputStream stream, Object data) {
        try {
            writeBytes(stream, data.toString().getBytes(UTF8));
        } catch (UnsupportedEncodingException e) {
            ALog.e(LOG_TAG, "writeString failed, UnsupportedEncodingException.");
        }
    }

    private static void writeChar(ByteArrayOutputStream stream, int data) {
        if (IS_LITTLE_ENDIAN) {
            stream.write(data);
            stream.write(data >>> 8);
        } else {
            stream.write(data >>> 8);
            stream.write(data);
        }
    }

    private static void writeSize(ByteArrayOutputStream stream, int data) {
        if (data < ALMOST) {
        stream.write(data);
        } else if (data <= 0xffff) {
            stream.write(ALMOST);
            writeChar(stream, data);
        } else {
            stream.write(255);
            writeInt(stream, data);
        }
    }

    private static void writeBytes(ByteArrayOutputStream stream, byte[] bytes) {
        writeSize(stream, bytes.length);
        stream.write(bytes, 0, bytes.length);
    }

    private static void writeByteBuffer(ByteArrayOutputStream stream, ByteBuffer data) {
        stream.write(T_LIST_UINT8);
        data.rewind();
        byte[] bytes = new byte[data.remaining()];
        data.get(bytes);
        writeBytes(stream, bytes);
    }

    private static void writeBoolArray(ByteArrayOutputStream stream, Object data) {
        stream.write(T_LIST_BOOL);
        List<Boolean> list =  new ArrayList<>();
        for (boolean boolIt : (boolean[]) data) {
            list.add(boolIt);
        }
        writeSize(stream, list.size());
        for (boolean iter : list) {
            writeData(stream, iter);
        }
    }

    private static void writeIntArray(ByteArrayOutputStream stream, Object data) {
        stream.write(T_LIST_INT32);
        int[] array = (int[]) data;
        writeSize(stream, array.length);
        writeAlignment(stream, 4);
        for (int iter : array) {
            writeInt(stream, iter);
        }
    }

    private static void writeLongArray(ByteArrayOutputStream stream, Object data) {
        stream.write(T_LIST_INT64);
        long[] array = (long[]) data;
        writeSize(stream, array.length);
        writeAlignment(stream, 8);
        for (long iter : array) {
            writeLong(stream, iter);
        }
    }

    private static void writeDoubleArray(ByteArrayOutputStream stream, Object data) {
        stream.write(T_LIST_DOUBLE);
        double[] array = (double[]) data;
        writeSize(stream, array.length);
        writeAlignment(stream, 8);
        for (double iter : array) {
            writeDouble(stream, iter);
        }
    }

    private static void writeStringArray(ByteArrayOutputStream stream, Object data) {
        stream.write(T_LIST_STRING);
        List<String> list = Arrays.asList((String[])data);
        writeSize(stream, list.size());
        for (String iter : list) {
            writeString(stream, iter);
        }
    }

    private static void writeMap(ByteArrayOutputStream stream, Object data) {
        stream.write(T_MAP);
        Map<?, ?> map = (Map) data;
        writeSize(stream, map.size());
        for (Entry<?, ?> entry : map.entrySet()) {
            writeData(stream, entry.getKey());
            writeData(stream, entry.getValue());
        }
    }

    private static void writeList(ByteArrayOutputStream stream, Object data) {
        stream.write(T_COMPOSITE_LIST);
        List<?> list = (List) data;
        writeSize(stream, list.size());
        for (Object iter : list) {
            writeData(stream, iter);
        }
    }

    private static void writeObjectArray(ByteArrayOutputStream stream, Object data) {
        stream.write(T_COMPOSITE_LIST);
        Object[] objArray = (Object[]) data;
        List<Object> list = Arrays.asList(objArray);
        writeSize(stream, list.size());
        for (Object iter : list) {
            writeData(stream, iter);
        }
    }

    /**
     * Write data.
     *
     * @param data Data to be written.
     * @param stream To be deposited into stream.
     */
    public static void writeData(ByteArrayOutputStream stream, Object data) {
        if (data == null) {
            stream.write(T_NULL);
        } else if (data instanceof Boolean) {
            stream.write(((Boolean) data).booleanValue() ? T_TRUE : T_FALSE);
        } else if (data instanceof Short || data instanceof Integer) {
            stream.write(T_INT32);
            writeInt(stream, (int) data);
        } else if (data instanceof Long) {
            stream.write(T_INT64);
            writeLong(stream, (long) data);
        } else if (data instanceof Float || data instanceof Double) {
            stream.write(T_DOUBLE);
            writeAlignment(stream, 8);
            writeDouble(stream, (double) data);
        } else if (data instanceof String) {
            stream.write(T_STRING);
            writeString(stream, data);
        } else if (data instanceof ByteBuffer) {
            writeByteBuffer(stream, (ByteBuffer) data);
        } else if (data instanceof byte[]) {
            stream.write(T_LIST_UINT8);
            writeBytes(stream, (byte[]) data);
        } else if (data instanceof boolean[]) {
            writeBoolArray(stream, data);
        } else if (data instanceof int[]) {
            writeIntArray(stream, data);
        } else if (data instanceof long[]) {
            writeLongArray(stream, data);
        } else if (data instanceof double[]) {
            writeDoubleArray(stream, data);
        } else if (data instanceof String[]) {
            writeStringArray(stream, data);
        } else if (data instanceof Map) {
            writeMap(stream, data);
        } else if (data instanceof List) {
            writeList(stream, data);
        } else if (data instanceof Object[]) {
            writeObjectArray(stream, data);
        }
    }

    private static void readAlignment(ByteBuffer buffer, int alignment) {
        int num = buffer.position() % alignment;
        if (num != 0) {
            buffer.position(buffer.position() + alignment - num);
        }
    }

    private static Object readString(ByteBuffer data) {
        byte[] bytes = readBytes(data);
        if (bytes == null) {
            return null;
        }
        String result = "Waiting for reading";
        try {
            result = new String(bytes, UTF8);
        } catch (UnsupportedEncodingException e) {
            ALog.e(LOG_TAG, "readString failed, UnsupportedEncodingException.");
            return null;
        }
        return result;
    }

    private static ByteBuffer readByteBuffer(ByteBuffer data) {
        byte[] bytes = readBytes(data);
        if (bytes == null) {
            return null;
        }
        ByteBuffer result = ByteBuffer.allocateDirect(bytes.length);
        result.put(bytes);
        return result;
    }

    private static Object readBoolArray(ByteBuffer data) {
        int length = readSize(data);
        if (length == SIZE_ERROR) {
            return null;
        }
        List<Object> list = new ArrayList<>(length);
        for (int i = 0; i < length; i++) {
            list.add(readData(data));
        }
        boolean[] result= new boolean[list.size()];
        int count = 0;
        for (Object iter : list) {
            result[count] = (boolean) iter;
            ++count;
        }
        return result;
    }

    private static Object readIntArray(ByteBuffer data) {
        int length = readSize(data);
        if (length == SIZE_ERROR) {
            return null;
        }
        int[] result = new int[length];
        readAlignment(data, 4);
        data.asIntBuffer().get(result);
        data.position(data.position() + 4 * length);
        return result;
    }

    private static Object readLongArray(ByteBuffer data) {
        int length = readSize(data);
        if (length == SIZE_ERROR) {
            return null;
        }
        long[] result = new long[length];
        readAlignment(data, 8);
        data.asLongBuffer().get(result);
        data.position(data.position() + 8 * length);
        return result;
    }

    private static Object readDoubleArray(ByteBuffer data) {
        int length = readSize(data);
        if (length == SIZE_ERROR) {
            return null;
        }
        double[] result = new double[length];
        readAlignment(data, 8);
        data.asDoubleBuffer().get(result);
        data.position(data.position() + 8 * length);
        return result;
    }

    private static Object readStringArray(ByteBuffer data) {
        int length = readSize(data);
        if (length == SIZE_ERROR) {
            return null;
        }
        List<Object> result = new ArrayList<>(length);
        for (int i = 0; i < length; i++) {
            result.add(readString(data));
        }
        return result.toArray(new String[result.size()]);
    }

    private static Object readMap(ByteBuffer data) {
        int length = readSize(data);
        if (length == SIZE_ERROR) {
            return null;
        }
        Map<Object, Object> result = new HashMap<>();
        for (int i = 0; i < length; i++) {
            result.put(readData(data), readData(data));
        }
        return result;
    }

    private static Object readList(ByteBuffer data) {
        int size = readSize(data);
        List<Object> result = new ArrayList<>(size);
        for (int i = 0; i < size; i++) {
            result.add(readData(data));
        }
        return result;
    }

    /**
     * Read data.
     *
     * @param buffer Data to be read.
     * @return Return encode data.
     */
    public static Object readData(ByteBuffer buffer) {
        byte type = buffer.get();
        if (type == T_NULL) {
            return null;
        } else if (type == T_TRUE) {
            return true;
        } else if (type == T_FALSE) {
            return false;
        } else if (type == T_INT32) {
            return buffer.getInt();
        } else if (type == T_INT64) {
            return buffer.getLong();
        } else if (type == T_DOUBLE) {
            readAlignment(buffer, 8);
            return buffer.getDouble();
        } else if (type == T_STRING) {
            return readString(buffer);
        } else if (type == T_LIST_UINT8) {
            return readByteBuffer(buffer);
        } else if (type == T_LIST_BOOL) {
            return readBoolArray(buffer);
        } else if (type == T_LIST_INT32) {
            return readIntArray(buffer);
        } else if (type == T_LIST_INT64) {
            return readLongArray(buffer);
        } else if (type == T_LIST_DOUBLE) {
            return readDoubleArray(buffer);
        } else if (type == T_LIST_STRING) {
            return readStringArray(buffer);
        } else if (type == T_MAP) {
            return readMap(buffer);
        } else if (type == T_COMPOSITE_LIST) {
            return readList(buffer);
        } else {
            return null;
        }
    }
}