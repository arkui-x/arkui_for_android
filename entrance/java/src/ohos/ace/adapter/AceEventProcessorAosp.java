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

import android.view.MotionEvent;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * process for system touch/key events
 *
 * @since 1
 */
public class AceEventProcessorAosp {
    private static final int PONITER_FIELD_COUNT = 10;

    private static final int BYTES_PER_FIELD = 8;

    private interface ActionType {
        int UNKNOWN = -1;
        int CANCEL = 0;
        int ADD = 1;
        int REMOVE = 2;
        int HOVER = 3;
        int DOWN = 4;
        int MOVE = 5;
        int UP = 6;
    }

    private AceEventProcessorAosp() {
    }

    /**
     * Process system motion events
     *
     * @param event motion event from system
     * @return whether the event is handled
     */
    public static ByteBuffer processTouchEvent(MotionEvent event) {
        if (event == null) {
            throw new AssertionError("event is null");
        }

        int pointerCount = event.getPointerCount();

        // Prepare data packet.
        ByteBuffer packet = ByteBuffer.allocateDirect(pointerCount * PONITER_FIELD_COUNT * BYTES_PER_FIELD);
        packet.order(ByteOrder.LITTLE_ENDIAN);

        int actionMasked = event.getActionMasked();
        int actionType = actionMaskedToActionType(actionMasked);

        boolean upOrDown = actionMasked == MotionEvent.ACTION_DOWN || actionMasked == MotionEvent.ACTION_POINTER_DOWN
                || actionMasked == MotionEvent.ACTION_UP || actionMasked == MotionEvent.ACTION_POINTER_UP;
        if (upOrDown) {
            addEventToBuffer(event, event.getActionIndex(), actionType, packet);
        } else {
            for (int index = 0; index < pointerCount; index++) {
                addEventToBuffer(event, index, actionType, packet);
            }
        }

        // verify the size of packet.
        if (packet.position() % (PONITER_FIELD_COUNT * BYTES_PER_FIELD) != 0) {
            throw new AssertionError("Packet position is not multiple of pointer length");
        }

        return packet;
    }

    private static void addEventToBuffer(MotionEvent event, int actionIndex, int actionType, ByteBuffer packet) {
        if (actionType == ActionType.UNKNOWN) {
            return;
        }
        long timeStamp = event.getEventTime() * 1000;

        packet.putLong(timeStamp);
        packet.putLong(actionType);
        packet.putLong(event.getPointerId(actionIndex));
        packet.putDouble(event.getX(actionIndex));
        packet.putDouble(event.getY(actionIndex));

        packet.putDouble(event.getPressure(actionIndex));
        packet.putDouble(0.0);
        packet.putDouble(event.getSize(actionIndex));
        packet.putLong(event.getSource());
        packet.putLong(event.getDeviceId());
    }

    private static int actionMaskedToActionType(int actionMasked) {
        int actionType = ActionType.UNKNOWN;
        switch (actionMasked) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                actionType = ActionType.DOWN;
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                actionType = ActionType.UP;
                break;
            case MotionEvent.ACTION_MOVE:
                actionType = ActionType.MOVE;
                break;
            case MotionEvent.ACTION_HOVER_MOVE:
            case MotionEvent.ACTION_SCROLL:
                actionType = ActionType.HOVER;
                break;
            case MotionEvent.ACTION_CANCEL:
                actionType = ActionType.CANCEL;
                break;
            default:
                break;
        }
        return actionType;
    }
}
