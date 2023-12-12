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

import android.view.MotionEvent;
import android.view.KeyEvent;
import android.view.InputDevice;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * process for system touch/key events
 *
 * @since 1
 */
public class AceEventProcessorAosp {
    private static final int PONITER_FIELD_COUNT = 10;
    private static final int MOUSE_FIELD_COUNT = 15;

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

    private interface MouseActionType {
        int NONE = 0;
        int PRESS = 1;
        int RELEASE = 2;
        int MOVE = 3;
        int HOVER_ENTER = 4;
        int HOVER_MOVE = 5;
        int HOVER_EXIT = 6;
    };

    private interface MouseActionButton {
        int NONE_BUTTON = 0;
        int LEFT_BUTTON = 1;
        int RIGHT_BUTTON = 2;
        int MIDDLE_BUTTON = 4;
        int BACK_BUTTON = 8;
        int FORWARD_BUTTON = 16;
    };

    private interface KeySourceType {
        int NONE = 0;
        int MOUSE = 1;
        int TOUCH = 2;
        int TOUCH_PAD = 3;
        int KEYBOARD = 4;
    };

    private interface CtrlKeysBit {
        int CTRL = 1;
        int SHIFT = 2;
        int ALT = 4;
        int META = 8;
    };

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
        packet.putLong(eventSourceTransKeySource(event.getSource()));
        packet.putLong(event.getDeviceId());
    }

    /**
     * Process system motion events
     *
     * @param event motion event from system
     * @return whether the event is handled
     */
    public static ByteBuffer processMouseEvent(MotionEvent event, int actionKey, float lastX, float lastY) {
        if (event == null) {
            throw new AssertionError("event is null");
        }

        int pointerCount = event.getPointerCount();

        // Prepare data packet.
        ByteBuffer packet = ByteBuffer.allocateDirect(pointerCount * MOUSE_FIELD_COUNT * BYTES_PER_FIELD);
        packet.order(ByteOrder.LITTLE_ENDIAN);

        int actionMasked = actionKey & 15;
        int mouseKey = actionKey >> 4;
        int actionType = actionMaskedToMouseActionType(actionMasked);
        boolean upOrDown = actionMasked == MotionEvent.ACTION_DOWN || actionMasked == MotionEvent.ACTION_POINTER_DOWN
                || actionMasked == MotionEvent.ACTION_UP || actionMasked == MotionEvent.ACTION_POINTER_UP;
        if (upOrDown) {
            addMouseToBuffer(event, event.getActionIndex(), actionType, mouseKey, packet, lastX, lastY);
        } else {
            for (int index = 0; index < pointerCount; index++) {
                addMouseToBuffer(event, index, actionType, mouseKey, packet, lastX, lastY);
            }
        }

        // verify the size of packet.
        if (packet.position() % (MOUSE_FIELD_COUNT * BYTES_PER_FIELD) != 0) {
            throw new AssertionError("Packet position is not multiple of pointer length");
        }

        return packet;
    }

    /**
     * Process system motion events
     *
     * @param event motion event from system
     * @return whether the event is handled
     */
    public static ByteBuffer processMouseEvent(KeyEvent event, float x, float y) {
        if (event == null) {
            throw new AssertionError("event is null");
        }

        // Prepare data packet.
        ByteBuffer packet = ByteBuffer.allocateDirect(MOUSE_FIELD_COUNT * BYTES_PER_FIELD);
        packet.order(ByteOrder.LITTLE_ENDIAN);

        int action = event.getAction();
        int actionType;
        if (action == KeyEvent.ACTION_DOWN) {
            actionType = MouseActionType.PRESS;
        } if (action == KeyEvent.ACTION_UP) {
            actionType = MouseActionType.RELEASE;
        } else {
            actionType = MouseActionType.PRESS;
        }
        addMouseToBuffer(event, actionType, packet, x, y);

        // verify the size of packet.
        if (packet.position() % (MOUSE_FIELD_COUNT * BYTES_PER_FIELD) != 0) {
            throw new AssertionError("Packet position is not multiple of pointer length");
        }
        return packet;
    }

    private static void addMouseToBuffer(KeyEvent event, int actionType, ByteBuffer packet, float x, float y) {
        if (actionType == ActionType.UNKNOWN) {
            return;
        }

        long timeStamp = event.getEventTime() * 1000;
        packet.putDouble(x);
        packet.putDouble(y);
        packet.putDouble(0.0);
        packet.putDouble(0.0);
        packet.putDouble(0.0);
        packet.putDouble(0.0);
        packet.putDouble(0.0);
        packet.putDouble(0.0);
        packet.putDouble(0.0);
        packet.putLong(actionType);

        int keyCode = event.getKeyCode();
        int mouseActionButton = MouseActionButton.NONE_BUTTON;
        if (keyCode == KeyEvent.KEYCODE_FORWARD) {
            mouseActionButton = MouseActionButton.FORWARD_BUTTON;
        } else if (keyCode == KeyEvent.KEYCODE_BACK) {
            mouseActionButton = MouseActionButton.BACK_BUTTON;
        }
        packet.putLong(mouseActionButton);
        packet.putLong(mouseActionButton);
        packet.putLong(timeStamp);
        packet.putLong(event.getDeviceId());
        packet.putLong(eventSourceTransKeySource(event.getSource()));
    }

    private static void addMouseToBuffer(MotionEvent event, int actionIndex, int actionType, int mouseKey, ByteBuffer packet, float lastX, float lastY) {
        if (actionType == ActionType.UNKNOWN) {
            return;
        }
  
        long timeStamp = event.getEventTime() * 1000;
        packet.putDouble(event.getX(actionIndex));                      // physicalX;
        packet.putDouble(event.getY(actionIndex));                      // physicalY;
        packet.putDouble(0.0);                                          // physicalZ;
        packet.putDouble(event.getX(actionIndex) - lastX);              // deltaX;
        packet.putDouble(event.getY(actionIndex) - lastY);              // deltaY;
        packet.putDouble(0.0);                                          // deltaZ;
        packet.putDouble(0.0);                                          // scrollDeltaX;
        packet.putDouble(0.0);                                          // scrollDeltaY;
        packet.putDouble(0.0);                                          // scrollDeltaZ;
        packet.putLong(actionType);                                     // action;
        packet.putLong(mouseKey);                                       // actionButton;
        packet.putLong(event.getButtonState());                         // pressedButtons;
        packet.putLong(timeStamp);                                      // timeStamp;
        packet.putLong(event.getDeviceId());                            // deviceId;
        packet.putLong(eventSourceTransKeySource(event.getSource()));   // deviceSource
    }

    public static int eventSourceTransKeySource(int eventSource) {
        int keySource = KeySourceType.NONE;
        switch(eventSource) {
            case InputDevice.SOURCE_ANY: // 0xffffff00
                keySource = KeySourceType.NONE;
                break;
            case InputDevice.SOURCE_CLASS_NONE: // 0x00000000 
                keySource = KeySourceType.NONE;
                break;
            case InputDevice.SOURCE_KEYBOARD: // 0x00000101
                keySource = KeySourceType.KEYBOARD;
                break;
            case InputDevice.SOURCE_TOUCHSCREEN: // 0x00001002
                keySource = KeySourceType.TOUCH;
                break;
            case InputDevice.SOURCE_MOUSE: // 0x00002002
                keySource = KeySourceType.MOUSE;
                break;
            case InputDevice.SOURCE_TOUCHPAD: // 0x00100008
                keySource = KeySourceType.TOUCH_PAD;
                break;
            default: 
                break; 
        }
        return keySource;
    }

    public static int getModifierKeys(KeyEvent event) {
        boolean isCtrl = event.isCtrlPressed();
        boolean isShift = event.isShiftPressed();
        boolean isAlt = event.isAltPressed();
        boolean isMeta = event.isMetaPressed();
        int ctrlKeysBit = 0;
        if (isCtrl) {
            ctrlKeysBit |= CtrlKeysBit.CTRL;
        }
        if (isShift) {
            ctrlKeysBit |= CtrlKeysBit.SHIFT;
        }
        if (isAlt) {
            ctrlKeysBit |= CtrlKeysBit.ALT;
        }
        if (isMeta) {
            ctrlKeysBit |= CtrlKeysBit.META;
        }     
        return ctrlKeysBit;
    }

    private static int actionMaskedToMouseActionType(int actionMasked) {
        int actionType = MouseActionType.NONE;
        switch (actionMasked) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                actionType = MouseActionType.PRESS;
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                actionType = MouseActionType.RELEASE;
                break;
            case MotionEvent.ACTION_MOVE:
                actionType = MouseActionType.MOVE;
                break;
            case MotionEvent.ACTION_HOVER_ENTER:
                actionType = MouseActionType.HOVER_ENTER;
                break;
            case MotionEvent.ACTION_HOVER_EXIT:
                actionType = MouseActionType.HOVER_EXIT;
                break;
            case MotionEvent.ACTION_HOVER_MOVE:
                actionType = MouseActionType.HOVER_MOVE;
                break;
            default:
                break;
        }
        return actionType;
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
