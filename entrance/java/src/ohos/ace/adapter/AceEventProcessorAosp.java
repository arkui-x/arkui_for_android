/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
    private static final int PONITER_FIELD_COUNT = 13;
    private static final int MOUSE_FIELD_COUNT = 15;
    private static final double TOUCH_ATCION_MULTI = 2.0D;
    private static final long TOUCH_EVENT_TIMEUNIT = 1000L;
    private static final int BYTES_PER_FIELD = 8;

    private AceEventProcessorAosp() {
    }

    private interface ActionType {
        /**
         * unknown action
         */
        int UNKNOWN = -1;

        /**
         * cancel action
         */
        int CANCEL = 0;

        /**
         * down action
         */
        int ADD = 1;

        /**
         * remove action
         */
        int REMOVE = 2;

        /**
         * move action
         */
        int HOVER = 3;

        /**
         * down action
         */
        int DOWN = 4;

        /**
         * up action
         */
        int MOVE = 5;

        /**
         * up action
         */
        int UP = 6;

        /**
         * hover enter action
         */
        int HOVER_ENTER = 7;

        /**
         * hover move action
         */
        int HOVER_MOVE = 8;

        /**
         * hover exit action
         */
        int HOVER_EXIT = 9;
    }

    private interface MouseActionType {
        /**
         * none action
         */
        int NONE = 0;

        /**
         * press action
         */
        int PRESS = 1;

        /**
         * release action
         */
        int RELEASE = 2;

        /**
         * move action
         */
        int MOVE = 3;

        /**
         * hover enter action
         */
        int HOVER_ENTER = 4;

        /**
         * hover move action
         */
        int HOVER_MOVE = 5;

        /**
         * hover exit action
         */
        int HOVER_EXIT = 6;
    };

    private interface MouseActionButton {
        /**
         * none button
         */
        int NONE_BUTTON = 0;

        /**
         * left button
         */
        int LEFT_BUTTON = 1;

        /**
         * right button
         */
        int RIGHT_BUTTON = 2;

        /**
         * middle button
         */
        int MIDDLE_BUTTON = 4;

        /**
         * back button
         */
        int BACK_BUTTON = 8;

        /**
         * forward button
         */
        int FORWARD_BUTTON = 16;
    };

    private interface KeySourceType {
        /**
         * none
         */
        int NONE = 0;

        /**
         * keyboard
         */
        int MOUSE = 1;

        /**
         * touch
         */
        int TOUCH = 2;

        /**
         * touchpad
         */
        int TOUCH_PAD = 3;

        /**
         * keyboard
         */
        int KEYBOARD = 4;
    };

    private interface CtrlKeysBit {
        /**
         * CTRL
         */
        int CTRL = 1;

        /**
         * SHIFT
         */
        int SHIFT = 2;

        /**
         * ALT
         */
        int ALT = 4;

        /**
         * META
         */
        int META = 8;
    };

    private interface SourceTool {
        /**
         * unknow
         */
        int UNKNOWN = -1;

        /**
         * finger
         */
        int FINGER = 0;

        /**
         * pen
         */
        int PEN = 1;

        /**
         * rubber
         */
        int RUBBER = 2;

        /**
         * brush
         */
        int BRUSH = 3;

        /**
         * pencil
         */
        int PENCIL = 4;

        /**
         * airbrush
         */
        int AIRBRUSH = 5;

        /**
         * mouse
         */
        int MOUSE = 6;

        /**
         * lens
         */
        int LENS = 7;

        /**
         * touchpad
         */
        int TOUCHPAD = 8;
    };

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

        for (int index = 0; index < pointerCount; index++) {
            addEventToBuffer(event, index, actionType, packet);
        }
        // verify the size of packet.
        if (packet.position() % (PONITER_FIELD_COUNT * BYTES_PER_FIELD) != 0) {
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
    public static ByteBuffer processHoverTouchEvent(MotionEvent event) {
        if (event == null) {
            throw new AssertionError("event is null");
        }

        int pointerCount = event.getPointerCount();

        // Prepare data packet.
        ByteBuffer packet = ByteBuffer.allocateDirect(pointerCount * PONITER_FIELD_COUNT * BYTES_PER_FIELD);
        packet.order(ByteOrder.LITTLE_ENDIAN);

        int actionMasked = event.getActionMasked();
        int actionType = actionMaskToHoverActionType(actionMasked);

        for (int index = 0; index < pointerCount; index++) {
            addHoverEventToBuffer(event, index, actionType, packet);
        }

        // verify the size of packet.
        if (packet.position() % (PONITER_FIELD_COUNT * BYTES_PER_FIELD) != 0) {
            throw new AssertionError("Packet position is not multiple of pointer length");
        }

        return packet;
    }

    private static void addHoverEventToBuffer(MotionEvent event, int actionIndex, int actionType, ByteBuffer packet) {
        if (actionType == ActionType.UNKNOWN) {
            return;
        }

        packet.putLong(event.getPointerId(actionIndex));
        packet.putLong(actionType);
        packet.putDouble(event.getX(actionIndex));
        packet.putDouble(event.getY(actionIndex));
        packet.putDouble(event.getTouchMinor(actionIndex) * TOUCH_ATCION_MULTI);
        packet.putDouble(event.getTouchMajor(actionIndex) * TOUCH_ATCION_MULTI);
        packet.putDouble(event.getPressure(actionIndex));
        packet.putLong(event.getDeviceId());
        packet.putLong(event.getDownTime() * TOUCH_EVENT_TIMEUNIT);
        packet.putLong(event.getEventTime() * TOUCH_EVENT_TIMEUNIT);
        packet.putLong(eventSourceTransKeySource(event.getSource()));
        packet.putLong(toolTypeTransSourceTool(event.getToolType(actionIndex)));
        int actionPoint = 1;
        if ((actionType == ActionType.UP || actionType == ActionType.DOWN) &&
            actionIndex != event.getActionIndex()) {
            actionPoint = 0;
        }
        packet.putLong(actionPoint);
    }
    private static void addEventToBuffer(MotionEvent event, int actionIndex, int actionType, ByteBuffer packet) {
        if (actionType == ActionType.UNKNOWN) {
            return;
        }

        packet.putLong(event.getPointerId(actionIndex));
        packet.putLong(actionType);
        packet.putDouble(event.getX(actionIndex));
        packet.putDouble(event.getY(actionIndex));
        packet.putDouble(event.getTouchMinor(actionIndex) * 2.0);
        packet.putDouble(event.getTouchMajor(actionIndex) * 2.0);
        packet.putDouble(event.getPressure(actionIndex));
        packet.putLong(event.getDeviceId());
        packet.putLong(event.getDownTime() * 1000);
        packet.putLong(event.getEventTime() * 1000);
        packet.putLong(eventSourceTransKeySource(event.getSource()));
        packet.putLong(toolTypeTransSourceTool(event.getToolType(actionIndex)));
        int actionPoint = 1;
        if ((actionType == ActionType.UP || actionType == ActionType.DOWN) &&
            actionIndex != event.getActionIndex()) {
            actionPoint = 0;
        }
        packet.putLong(actionPoint);
    }

    /**
     * Process system motion events
     *
     * @param event motion event from system
     * @param actionKey action key
     * @param lastX last x position
     * @param lastY last y position
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
     * @param x x position
     * @param y y position
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

    private static void addMouseToBuffer(MotionEvent event, int actionIndex, int actionType, int mouseKey,
            ByteBuffer packet, float lastX, float lastY) {
        if (actionType == ActionType.UNKNOWN) {
            return;
        }

        long timeStamp = event.getEventTime() * 1000;
        packet.putDouble(event.getX(actionIndex));
        packet.putDouble(event.getY(actionIndex));
        packet.putDouble(0.0);
        packet.putDouble(event.getX(actionIndex) - lastX);
        packet.putDouble(event.getY(actionIndex) - lastY);
        packet.putDouble(0.0);
        packet.putDouble(0.0);
        packet.putDouble(0.0);
        packet.putDouble(0.0);
        packet.putLong(actionType);
        packet.putLong(mouseKey);
        packet.putLong(event.getButtonState());
        packet.putLong(timeStamp);
        packet.putLong(event.getDeviceId());
        packet.putLong(eventSourceTransKeySource(event.getSource()));
    }

    /**
     * Convert the event source to key source.
     *
     * @param eventSource the event source
     * @return the key source
     */
    public static int eventSourceTransKeySource(int eventSource) {
        int keySource = KeySourceType.NONE;
        switch (eventSource) {
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
                keySource = KeySourceType.TOUCH;
                break;
        }
        return keySource;
    }

    /**
     * Convert the tool type to source tool.
     *
     * @param toolType the tool type
     * @return the source tool
     */
    public static int toolTypeTransSourceTool(int toolType) {
        int sourceTool = SourceTool.UNKNOWN;
        switch (toolType) {
            case MotionEvent.TOOL_TYPE_ERASER:
                sourceTool = SourceTool.UNKNOWN;
                break;
            case MotionEvent.TOOL_TYPE_FINGER:
                sourceTool = SourceTool.FINGER;
                break;
            case MotionEvent.TOOL_TYPE_MOUSE:
                sourceTool = SourceTool.MOUSE;
                break;
            case MotionEvent.TOOL_TYPE_STYLUS:
                sourceTool = SourceTool.PEN;
                break;
            default:
                break;
        }
        return sourceTool;
    }

    /**
     * Get the modifier keys.
     *
     * @param event the key event
     * @return the modifier keys
     */
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
                actionType = MouseActionType.MOVE;
                break;
            default:
                break;
        }
        return actionType;
    }

    private static int actionMaskToHoverActionType(int actionMasked) {
        int actionType = ActionType.UNKNOWN;
        switch (actionMasked) {
            case MotionEvent.ACTION_HOVER_MOVE:
                actionType = ActionType.HOVER_MOVE;
                break;
            case MotionEvent.ACTION_HOVER_ENTER:
                actionType = ActionType.HOVER_ENTER;
                break;
            case MotionEvent.ACTION_HOVER_EXIT:
                actionType = ActionType.HOVER_EXIT;
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
