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
     * The selected range.
     *
     * @since 20
     */
    protected static class SelectedRange {
        /**
         * The start position of the selected text range.
         */
        public final int selectionStart;

        /**
         * The end position of the selected text range.
         */
        public final int selectionEnd;

        /**
         * Constructor to initialize the selected range.
         *
         * @param selectionStart the start position of the selection
         * @param selectionEnd the end position of the selection
         */
        public SelectedRange(int selectionStart, int selectionEnd) {
            this.selectionStart = selectionStart;
            this.selectionEnd = selectionEnd;
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
        private static final Map<Integer, SelectedRange> selectedRange = new HashMap<>();

        private boolean isSelected = false;
        private boolean isComposing = false;
        private boolean isNewCommitText = false;
        private int finishComposingTextStart = -1;
        private int lastComposingEnd = -1;
        private int lastComposingStart = -1;
        private int lastSelectionEnd = -1;
        private int lastSelectionStart = -1;
        private String needUpdatedText = null;
        private String finishComposingText = null;
        private String composingText = null;

        public Delegate(int clientID, String initData) {
            Delegate.lastValueMap.put(clientID, initData);
        }

        private static class DiffResult {
            /**
             * The start of the selection in the previous text.
             */
            public final int preSelStart;

            /**
             * The end of the selection in the previous text.
             */
            public final int preSelEnd;

            /**
             * The start of the selection in the current text.
             */
            public final int curSelStart;

            /**
             * The end of the selection in the current text.
             */
            public final int curSelEnd;

            /**
             * The text that is appended to the previous text.
             */
            private String newAppendText;

            /**
             * Constructor to initialize the DiffResult.
             *
             * @param preSelStart the start of the selection in the previous text
             * @param preSelEnd the end of the selection in the previous text
             * @param curSelStart the start of the selection in the current text
             * @param curSelEnd the end of the selection in the current text
             * @param newAppendText the text that is appended to the previous text
             */
            public DiffResult(int preSelStart, int preSelEnd, int curSelStart, int curSelEnd, String newAppendText) {
                this.preSelStart = preSelStart;
                this.preSelEnd = preSelEnd;
                this.curSelStart = curSelStart;
                this.curSelEnd = curSelEnd;
                this.newAppendText = newAppendText;
            }

            /**
             * Set new append text.
             *
             * @param newAppendText new append text
             */
            public void setNewAppendText(String newAppendText) {
                this.newAppendText = newAppendText;
            }

            /**
             * Get new append text.
             *
             * @return new append text
             */
            public String getNewAppendText() {
                return newAppendText;
            }
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
                    String newInputStr = "";
                    if (lastCommittedTexts.containsKey(clientId)) {
                        newInputStr = lastCommittedTexts.remove(clientId);
                    }
                    json.put("appendText", "".equals(needUpdatedText) &&
                        !"".equals(newInputStr) ? newInputStr : needUpdatedText);
                    isSelected = false;
                    needUpdatedText = null;
                    setComposedState(false, -1, -1);
                } else if (lastSelectionEnd > lastSelectionStart) {
                    String appendText = getAppendText(text, selectionEnd);
                    json.put("appendText", appendText);
                    lastSelectionEnd = -1;
                    lastSelectionStart = -1;
                    setComposedState(false, -1, -1);
                } else if (lastValue != null && text.length() < lastValue.length()) {
                    json.put("isDelete", true);
                    setComposedState(false, -1, -1);
                } else if (composingEnd > composingStart &&
                    isSingleLetterInput(lastValue, text, composingStart, composingEnd)) {
                    json.put("appendText", text.substring(composingEnd - 1, composingEnd));
                    setComposedState(false, -1, -1);
                } else {
                    handleOtherInput(json, text, clientId, composingStart, composingEnd);
                }
                tryModifyText(json, text, lastValue, selectionEnd, clientId);
                TextInputPluginBase.updateEditingState(clientId, json.toString());
            } catch (JSONException ignored) {
                ALog.e(LOG_TAG, "failed parse editing config json");
            }
            lastValueMap.put(clientId, text);
            reset();
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
            return text.substring(Math.max(start, 0), Math.min(selectionEnd, text.length()));
        }

        @Override
        public void setFinishComposingText(String finishComposingText, int finishComposingTextStart) {
            this.finishComposingText = finishComposingText;
            this.finishComposingTextStart = finishComposingTextStart;
        }

        /**
         * Set composing text and update the composing state.
         *
         * @param composingText The text to set as composing
         */
        @Override
        public void setComposingText(String composingText) {
            this.composingText = composingText;
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
            if (isNewCommitText) {
                isNewCommitText = false;
                setComposedState(false, -1, -1);
                return;
            }
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
        public void setSelectedState(int selectionStart, int selectionEnd) {
            this.lastSelectionStart = selectionStart;
            this.lastSelectionEnd = selectionEnd;
        }

        private String getAppendText(String text, int selectionEnd) {
            if (text == null || text.isEmpty()) {
                return "";
            }
            int count = lastSelectionEnd - lastSelectionStart;
            int start = selectionEnd - count;
            return text.substring(start > 0 ? start : 0, Math.min(text.length(), selectionEnd));
        }

        private boolean isSingleLetterInput(String lastValue, String text, int composingStart, int composingEnd) {
            if (composingEnd > text.length() || composingStart > text.length()) {
                return false;
            }
            if (composingEnd - 1 > lastValue.length() || composingStart > lastValue.length()) {
                return false;
            }
            if (text.length() - lastValue.length() != 1) {
                return false;
            }
            char letterCh = text.charAt(composingEnd - 1);
            if (!Character.isLetter(letterCh)) {
                return false;
            }
            if (composingStart > composingEnd) {
                return false;
            }
            if (!lastValue.substring(composingStart, composingEnd - 1).equals(text.substring(composingStart,
                composingEnd - 1))) {
                return false;
            }
            if (lastValue.substring(composingStart, composingEnd - 1).matches("[a-zA-Z]+")) {
                return true;
            }
            return false;
        }

        private void reset() {
            lastCommittedTexts.clear();
            isSelected = false;
            lastSelectionEnd = -1;
            lastSelectionStart = -1;
            needUpdatedText = null;
            finishComposingText = null;
            composingText = null;
        }

        private boolean isAppendText(String lastValue, String text, String appendText, int selectionEnd) {
            if (lastValue == null || text == null || appendText == null || selectionEnd < 0) {
                return false;
            }

            if (selectionEnd > text.length()) {
                return false;
            }

            int appendTextLength = text.length() - lastValue.length();
            if (appendTextLength <= 0) {
                return false;
            }

            int appendStart = selectionEnd - appendTextLength;
            if (appendStart < 0 || appendTextLength != appendText.length()) {
                return false;
            }

            String textWithoutAppend = text.substring(0, appendStart) + text.substring(selectionEnd);
            return lastValue.equals(textWithoutAppend);
        }

        private String getCommitText(String lastValue, String text,
            int selectionEnd, JSONObject json, int clientId) throws JSONException {
            String appendText = lastCommittedTexts.remove(clientId);
            if (composingText != null && composingText.length() > 0) {
                String newText = appendText + composingText;
                if (newText.equals(text.substring(Math.max(0, selectionEnd - newText.length()),
                    Math.min(selectionEnd, text.length())))) {
                    appendText = newText;
                }
                composingText = null;
            }
            int appendStart = selectionEnd - appendText.length();
            appendStart = Math.max(appendStart, 0);
            if (lastValue.length() <= appendStart) {
                return appendText;
            }
            String newCommitText = lastValue.substring(0, appendStart) + appendText + lastValue.substring(appendStart);
            if (text.length() == (lastValue.length() + appendText.length()) &&
                (newCommitText.equals(text) || isComposing)) {
                return appendText;
            }
            json.put("composingStart", appendStart);
            json.put("composingEnd", appendStart + (appendText.length() - (text.length() - lastValue.length())));
            tryModifyComposeText(json, lastValue, text, appendText);
            isNewCommitText = true;
            return appendText;
        }

        private void handleOtherInput(JSONObject json, String text, int clientId,
            int composingStart, int composingEnd) throws JSONException {
            String appendText = null;
            boolean isAppendText = false;
            String lastValue = lastValueMap.get(clientId);
            int selectionEnd = json.getInt("selectionEnd");
            if (lastCommittedTexts.containsKey(clientId)) {
                appendText = getCommitText(lastValue, text, selectionEnd, json, clientId);
                isAppendText = isAppendText(lastValue, text, appendText, selectionEnd);
            } else {
                appendText = getNewInputStr(lastValue, text, selectionEnd, composingStart, composingEnd);
            }
            if (appendText != null && !appendText.isEmpty()) {
                json.put("appendText", appendText);
            }

            if (isAppendText && composingStart == -1) {
                setComposedState(false, -1, -1);
            } else {
                handleComposingText(json, composingStart, composingEnd);
            }
        }

        private void tryModifyComposeText(JSONObject json, String lastValue, String text,
            String appendText) throws JSONException {
            int start = Math.max(0, json.getInt("composingStart"));
            int end = Math.min(lastValue.length(), json.getInt("composingEnd"));
            if (start == end) {
                return;
            }
            StringBuilder lastValueStr = new StringBuilder(lastValue);
            lastValueStr.replace(start, end, appendText);
            if (!lastValueStr.toString().equals(text)) {
                start = Math.max(0, json.getInt("composingStart") - 1);
                end = Math.min(lastValue.length(), json.getInt("composingEnd") - 1);
                if (end <= 0) {
                    return;
                }
                if (new StringBuilder(lastValue).replace(start, end, appendText).toString().equals(text)) {
                    json.put("composingStart", json.getInt("composingStart") - 1);
                    json.put("composingEnd", json.getInt("composingEnd") - 1);
                }
            }
        }

        private void tryModifyText(JSONObject json, String text, String lastValue, int selectionEnd,
            int clientId) throws JSONException {
            if (text == null || lastValue == null || selectionEnd < 0 || selectionEnd > text.length()) {
                return;
            }
            int start = json.getInt("composingStart");
            int end = json.getInt("composingEnd");
            if (start < 0 || end <= 0 || start == end) {
                if (selectedRange.containsKey(clientId)) {
                    start = selectedRange.get(clientId).selectionStart;
                    end = selectedRange.get(clientId).selectionEnd;
                }
            }
            if (start > end || !json.has("appendText")) {
                return;
            }
            String curAppendText = json.getString("appendText");
            if (isCorrectData(lastValue, text, start, end, json.getString("appendText"))) {
                return;
            }
            DiffResult diffResult = getDiffResult(lastValue, text, start, selectionEnd);
            if (diffResult.preSelStart == diffResult.preSelEnd) {
                if (start == end && end != diffResult.preSelEnd) {
                    String newAppendText = text.substring(Math.max(start, 0),
                                                            Math.min(selectionEnd - 1, text.length()));
                    if (isCorrectData(lastValue, text, start, end, newAppendText)) {
                        diffResult.setNewAppendText(newAppendText);
                    }
                }
                json.put("composingStart", -1);
                json.put("composingEnd", -1);
            } else {
                json.put("composingStart", diffResult.preSelStart);
                json.put("composingEnd", diffResult.preSelEnd);
            }
            json.put("appendText", diffResult.getNewAppendText());
        }

        private boolean isCorrectData(String preText, String curText, int selectionStart,
            int selectionEnd, String appendText) {
            if (preText == null || curText == null) {
                return false;
            }
            if (selectionStart < 0 || selectionEnd < 0 || selectionStart > preText.length() ||
                selectionEnd > preText.length() || selectionStart > selectionEnd) {
                return false;
            }
            StringBuilder newText = new StringBuilder(preText);
            newText.replace(selectionStart, selectionEnd, appendText);
            return newText.toString().equals(curText);
        }

        private DiffResult getDiffResult(String preText, String curText, int start, int selectionEnd) {
            int curTextLength = curText.length();
            int preTextLength = preText.length();
            int left = 0;
            while (left < curTextLength && left < preTextLength && curText.charAt(left) == preText.charAt(left)) {
                left++;
            }
            int rightText = curTextLength - 1;
            int rightLast = preTextLength - 1;
            while (rightText >= left && rightLast >= left
                    && curText.charAt(rightText) == preText.charAt(rightLast)) {
                rightText--;
                rightLast--;
            }

            int lastDelEnd = rightLast + 1;
            int textInsertStart = left;
            int textInsertEnd = rightText + 1;
            int lastDelStart = left;
            String insertedText = curText.substring(textInsertStart, Math.max(textInsertEnd, selectionEnd));
            if (selectionEnd > textInsertEnd) {
                lastDelEnd = lastDelEnd + (selectionEnd - textInsertEnd);
            }
            if (selectionEnd < textInsertEnd && textInsertEnd - selectionEnd == 1 &&
                " ".equals(curText.substring(selectionEnd, selectionEnd + 1))) {
                int substringStart = Math.max(start, 0);
                int substringEnd = Math.max(selectionEnd, substringStart);
                insertedText = curText.substring(substringStart, substringEnd);
            }

            return new DiffResult(lastDelStart, lastDelEnd, textInsertStart, textInsertEnd, insertedText);
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
     * Finish composing.
     */
    protected abstract void finishComposing();

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
            if (Delegate.lastCommittedTexts.containsKey(clientId())) {
                String existing = Delegate.lastCommittedTexts.get(clientId());
                if (" ".equals(existing)) {
                    Delegate.lastCommittedTexts.put(clientId(), existing + text);
                } else {
                    Delegate.lastCommittedTexts.put(clientId(), text);
                }
            } else {
                Delegate.lastCommittedTexts.put(clientId(), text);
            }
        }
    }

    /**
     * Set the selected range.
     *
     * @param range The selected range.
     */
    protected void setSelectedRange(SelectedRange range) {
        if (hasClient()) {
            Delegate.selectedRange.put(clientId(), range);
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