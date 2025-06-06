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

import ohos.ace.adapter.ALog;

import java.util.HashMap;
import java.util.Map;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * TextInputPluginBase
 *
 * @since 1
 */
public abstract class TextInputPluginBase {
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

        private boolean isSelected = false;
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
                json.put("text", text);
                json.put("selectionStart", selectionStart);
                json.put("selectionEnd", selectionEnd);
                json.put("composingStart", composingStart);
                json.put("composingEnd", composingEnd);

                String lastValue = lastValueMap.get(clientId);
                if (isSelected && needUpdatedText != null) {
                    json.put("isDelete", false);
                    json.put("appendText", needUpdatedText);
                    isSelected = false;
                    needUpdatedText = null;
                } else if (lastValue != null && text.length() < lastValue.length()) {
                    json.put("isDelete", true);
                } else {
                    json.put("isDelete", false);
                    String appendText = getNewInputStr(lastValue, text, selectionEnd);
                    if (appendText != null) {
                        json.put("appendText", appendText);
                    }
                }
                TextInputPluginBase.updateEditingState(clientId, json.toString());
            } catch (JSONException ignored) {
                ALog.e(LOG_TAG, "failed parse editing config json");
            }
            lastValueMap.put(clientId, text);
        }

        private String getNewInputStr(String lastValue, String text, int selectionEnd) {
            if (lastValue == null) {
                return text;
            }
            int count = text.length() - lastValue.length();
            int start = selectionEnd - count;
            if (start < 0) {
                return null;
            }
            if (selectionEnd > text.length()) {
                return null;
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