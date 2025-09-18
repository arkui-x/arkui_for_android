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

package ohos.ace.adapter.capability.editing;

import android.view.inputmethod.InputConnection;

import ohos.ace.adapter.ALog;
import ohos.ace.adapter.capability.editing.TrackingInputConnection;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;

/**
 * TextInputPluginBase
 *
 * @since 1
 */
public abstract class TextInputPluginBase implements TrackingInputConnection.InputCommitListener {
    private static final String LOG_TAG = "Ace_IME";
    private static final int CLIENT_ID_NONE = -1;

    private int clientId = CLIENT_ID_NONE;
    private TextInputConfiguration config;

    public TextInputPluginBase(int instanceId) {
        try {
            nativeInit(instanceId);
        } catch (UnsatisfiedLinkError ignored) {
            // May occurred in light-weighted devices.
            ALog.e(LOG_TAG, "nativeInit method is not found.");
        }
    }

    /**
     * Delegate implementation.
     *
     * @since 1
     */
    public static class Delegate implements TextInputDelegate {
        private static Map<Integer, String> lastValueMap = new HashMap<>();
        private static final Map<Integer, String> lastCommittedTexts = new HashMap<>();

        private boolean isSelected = false;
        private boolean isComposing = false;
        private int lastComposingEnd = -1;
        private int lastComposingStart = -1;
        private String needUpdatedText = null;

        public Delegate(int clientID, String initData) {
            Delegate.lastValueMap.put(clientID, initData);
        }

        @Override
        public void updateEditingState(int clientId, String text, int selectionStart, int selectionEnd,
            int composingStart, int composingEnd) {
            if (text == null) {
                return;
            }
            try {
                JSONObject json = new JSONObject();
                setJsonDefault(json, text, selectionStart, selectionEnd);
                String lastValue = lastValueMap.get(clientId);
                if (isSelected && needUpdatedText != null) {
                    json.put("appendText", needUpdatedText);
                    if (lastCommittedTexts.containsKey(clientId)) {
                        lastCommittedTexts.remove(clientId);
                    }
                    isSelected = false;
                    needUpdatedText = null;
                } else if (lastValue != null && text.length() < lastValue.length()) {
                    json.put("isDelete", true);
                } else {
                    String appendText = null;
                    if (lastCommittedTexts.containsKey(clientId)) {
                        appendText = lastCommittedTexts.remove(clientId);
                    } else {
                        appendText = getNewInputStr(lastValue, text, selectionEnd, composingStart, composingEnd);
                    }
                    if (appendText != null && !appendText.isEmpty()) {
                        json.put("appendText", appendText);
                    }
                    handleComposingText(json, composingStart, composingEnd);
                }
                TextInputPluginBase.updateEditingState(clientId, json.toString());
            } catch (JSONException ignored) {
                ALog.e(LOG_TAG, "failed parse editing config json");
            }
            lastValueMap.put(clientId, text);
        }

        private String getNewInputStr(String lastValue,
                                        String text,
                                        int selectionEnd,
                                        int composingStart,
                                        int composingEnd) {
            if (lastValue == null) {
                return text;
            }
            int count = text.length() - lastValue.length();
            if (composingStart != -1 && (composingEnd > (composingStart + 1))) {
                if (count == 0) {
                    return "";
                }
                return text.substring(composingStart, composingEnd);
            }
            int start = selectionEnd - count;
            if (start < 0) {
                return "";
            }
            if (selectionEnd > text.length()) {
                return "";
            }
            return text.substring(start, selectionEnd);
        }

        /**
         * Release the last value map.
         */
        public static void release() {
            lastValueMap.clear();
        }

        @Override
        public void performAction(int clientId, TextInputAction action) {
            if (action == null) {
                return;
            }
            TextInputPluginBase.performAction(clientId, action.getValue());
        }

        @Override
        public void updateInputFilterErrorText(int clientId, String errorText) {
            if (errorText.length() > 0) {
                TextInputPluginBase.updateInputFilterErrorText(clientId, errorText);
            }
        }

        @Override
        public void notifyKeyboardClosedByUser(int clientId) {
            TextInputPluginBase.notifyKeyboardClosedByUser(clientId);
        }

        @Override
        public void setSelectedState(boolean isSelected, CharSequence text) {
            this.isSelected = isSelected;
            this.needUpdatedText = text.toString();
        }

        private void setComposedState(boolean isComposing, int commitStart, int composingEnd) {
            this.isComposing = isComposing;
            this.lastComposingStart = commitStart;
            this.lastComposingEnd = composingEnd;
        }

        private void setJsonDefault(JSONObject json, String text,
                                    int selectionStart, int selectionEnd) throws JSONException {
            json.put("text", text);
            json.put("selectionStart", selectionStart);
            json.put("selectionEnd", selectionEnd);
            json.put("composingStart", -1);
            json.put("composingEnd", -1);
            json.put("isDelete", false);
        }

        private void handleComposingText(JSONObject json, int composingStart, int composingEnd)
            throws JSONException {
            if (composingStart != -1 && (composingEnd > composingStart)) {
                json.put("composingEnd", composingStart);
                if (isComposing && (lastComposingEnd < composingEnd)) {
                    json.put("composingEnd", lastComposingEnd);
                    json.put("composingStart", composingStart);
                }
                setComposedState(true, composingStart, composingEnd);
            } else {
                if (isComposing) {
                    json.put("composingStart", lastComposingStart);
                    json.put("composingEnd", lastComposingEnd);
                }
                setComposedState(false, -1, -1);
            }
        }
    }

    /**
     * Check if the client is available.
     *
     * @return whether has client.
     */
    protected boolean hasClient() {
        return clientId != CLIENT_ID_NONE;
    }

    /**
     * Get the client ID.
     *
     * @return clientId.
     */
    protected int clientId() {
        return clientId;
    }

    /**
     * Get the configuration.
     *
     * @return configuration of TextInput.
     */
    protected TextInputConfiguration getConfiguration() {
        return config;
    }

    /**
     * Register a client with an ID to identify a client in the case of a client change.
     *
     * @param client           The client
     * @param serializedConfig The configuration
     */
    private void setTextInputClient(int client, String serializedConfig) {
        this.clientId = client;
        try {
            JSONObject object = new JSONObject(serializedConfig);
            if (object != null) {
                this.config = TextInputConfiguration.fromJson(object);
            }
        } catch (JSONException ignored) {
            ALog.e(LOG_TAG, "failed parse editing config json");
            return;
        }
        onInited();
    }

    /**
     * Will called after being clientId & configuration initialized.
     */
    protected abstract void onInited();

    /**
     * Show keyboard.
     *
     * @param isFocusViewChanged Indicates the focus view is changed or not.
     */
    protected abstract void showTextInput(boolean isFocusViewChanged);

    /**
     * Hide keyboard.
     */
    protected abstract void hideTextInput();

    /**
     * release function.
     */
    public abstract void release();

    /**
     * Set the current input text editing state, for example, text/selection.
     *
     * @param serialized The serialized editing state.
     */
    private void setTextInputEditingState(String serialized) {
        TextEditState state = null;
        try {
            JSONObject object = new JSONObject(serialized);
            if (object == null) {
                ALog.e(LOG_TAG, "setTextInputEditingState invalid state");
                return;
            }
            state = TextEditState.fromJson(object);
        } catch (JSONException ignored) {
            ALog.e(LOG_TAG, "failed parse editing state json");
            return;
        }

        onSetTextEditingState(state);
    }

    /**
     * Set the current input text editing state, for example, text/selection. Subclass SHOULD override this method.
     *
     * @param state The deserialized editing state.
     */
    protected void onSetTextEditingState(TextEditState state) {
    }

    /**
     * Unbind the connection with the current client.
     */
    private void clearTextInputClient() {
        clientId = CLIENT_ID_NONE;
        onClosed();
    }

    /**
     * Called after the connection is closed. Subclass can override this method.
     */
    protected void onClosed() {}

    /**
     * Callback invoked when text is committed from the input connection.
     *
     * This method stores the committed text for the current client so that
     * it can be consumed when calculating editing deltas.
     *
     * @param text the text that was committed
     */
    @Override
    public void onCommit(String text) {
        if (hasClient()) {
            Delegate.lastCommittedTexts.put(clientId(), text);
        }
    }

    /**
     * Wrap the original InputConnection with a TrackingInputConnection that
     * intercepts commit events and forwards them to this listener.
     *
     * Subclasses should call this when creating their InputConnection (for example,
     * inside onCreateInputConnection) to enable commit tracking.
     *
     * @param original the original InputConnection to wrap
     * @return a TrackingInputConnection delegating to the original connection, or null if original is null
     */
    protected InputConnection wrapInputConnection(InputConnection original) {
        if (original == null) {
            return original;
        }
        if (original instanceof TrackingInputConnection) {
            return original;
        }
        return new TrackingInputConnection(original, this);
    }

    private static native void updateEditingState(int client, String state);

    private static native void performAction(int client, int action);

    private static native void updateInputFilterErrorText(int client, String errorText);

    private static native void notifyKeyboardClosedByUser(int client);

    /**
     * native func for Init.
     *
     * @param instanceId The instanceId
     */
    protected native void nativeInit(int instanceId);
}