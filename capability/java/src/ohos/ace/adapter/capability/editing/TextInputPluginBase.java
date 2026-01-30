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
        private static final String TEXT = "text";
        private static final String SELECTION_START = "selectionStart";
        private static final String SELECTION_END = "selectionEnd";
        private static final String COMPOSING_START = "composingStart";
        private static final String COMPOSING_END = "composingEnd";
        private static final String APPEND_TEXT = "appendText";
        private static final String IS_DELETE = "isDelete";

        private boolean isSelected = false;
        private boolean isComposing = false;
        private boolean isNewCommitText = false;
        private boolean isDeleted = false;
        private int finishComposingTextStart = -1;
        private int lastComposingEnd = -1;
        private int lastComposingStart = -1;
        private int lastSelectionEnd = -1;
        private int lastSelectionStart = -1;
        private int lastSelectionEndFromCommit = -1;
        private int lastSelectionStartFromCommit = -1;
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
            if (!validateInput(text, selectionStart, selectionEnd, composingStart, composingEnd)) {
                return;
            }
            try {
                JSONObject json = new JSONObject();
                setJsonDefault(json, text, selectionStart, selectionEnd);
                String lastValue = lastValueMap.get(clientId);
                if (isSelected && needUpdatedText != null) {
                    processSelectedFromCommitText(clientId, json, text, lastValue);
                } else if (lastSelectionEnd > lastSelectionStart) {
                    String appendText = getAppendText(text, selectionEnd);
                    json.put(APPEND_TEXT, appendText);
                    lastSelectionEnd = -1;
                    lastSelectionStart = -1;
                    setComposedState(false, -1, -1);
                } else if (lastValue != null && isDelete(clientId, json, text, lastValue, selectionEnd)) {
                    json.put(IS_DELETE, true);
                    setComposedState(false, -1, -1);
                } else if (composingEnd > composingStart &&
                    isSingleLetterInput(lastValue, text, composingStart, composingEnd)) {
                    json.put(APPEND_TEXT, substringSafe(text, composingEnd - 1, composingEnd));
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

        private boolean processDeletedFromCommitText(int clientId, String text, String lastValue,
            JSONObject json) throws JSONException {
            if (text == null || lastValue == null) {
                return false;
            }
            int diffLength = lastValue.length() - text.length();
            int selectionStart = json.getInt(SELECTION_START);
            int selectionEnd = json.getInt(SELECTION_END);
            if (diffLength <= 0 || selectionStart != selectionEnd) {
                return false;
            }

            String expectedText = substringSafe(lastValue, 0, selectionStart) +
                                    substringSafe(lastValue, selectionEnd + diffLength, lastValue.length());
            if (text.equals(expectedText)) {
                return true;
            }
            return false;
        }

        private void processSelectedFromCommitText(int clientId, JSONObject json,
            String text, String lastValue) throws JSONException {
            String newInputStr = "";
            if (lastCommittedTexts.containsKey(clientId)) {
                newInputStr = lastCommittedTexts.remove(clientId);
            }
            String needAppendText = "".equals(needUpdatedText) &&
                                    !"".equals(newInputStr) ? newInputStr : needUpdatedText;
            json.put(APPEND_TEXT, needAppendText);
            if (needAppendText != null && needAppendText.isEmpty() && lastSelectionStartFromCommit >= 0 &&
                lastSelectionEndFromCommit >= 0 && lastSelectionStartFromCommit < lastSelectionEndFromCommit) {
                if (isDeleted) {
                    json.put(IS_DELETE, true);
                } else if (composingText != null && !composingText.isEmpty()) {
                    json.put(APPEND_TEXT, composingText);
                } else {
                    if (processDeletedFromCommitText(clientId, text, lastValue, json)) {
                        json.put(IS_DELETE, true);
                    }
                }
            }
            isSelected = false;
            needUpdatedText = null;
            setComposedState(false, -1, -1);
        }

        private boolean validateInput(String text, int selectionStart, int selectionEnd,
            int composingStart, int composingEnd) {
            if (text == null) {
                return false;
            }
            int length = text.length();
            if (selectionStart < 0 || selectionStart > length ||
                selectionEnd < 0 || selectionEnd > length ||
                selectionStart > selectionEnd) {
                return false;
            }
            if (composingStart != -1 || composingEnd != -1) {
                if (composingStart < 0 || composingStart > length ||
                    composingEnd < 0 || composingEnd > length ||
                    composingStart > composingEnd) {
                    return false;
                }
            }
            return true;
        }

        private boolean isDelete(int clientId, JSONObject json, String text,
            String lastValue, int selectionEnd) throws JSONException {
            if (text == null || lastValue == null) {
                return false;
            }

            int lengthDiff = lastValue.length() - text.length();
            if (lengthDiff <= 0) {
                return false;
            }

            if (lengthDiff == 1) {
                String expectedText = substringSafe(lastValue, 0, selectionEnd) +
                                    substringSafe(lastValue, selectionEnd + 1, lastValue.length());
                if (text.equals(expectedText)) {
                    return true;
                }
            }

            if (selectedRange != null && selectedRange.containsKey(clientId)) {
                SelectedRange range = selectedRange.get(clientId);
                int start = range.selectionStart;
                int end = range.selectionEnd;

                if (start >= 0 && end > start && end - start == lengthDiff) {
                    String expectedText = substringSafe(lastValue, 0, start) +
                                        substringSafe(lastValue, end, lastValue.length());
                    if (text.equals(expectedText)) {
                        return true;
                    }
                }
            }

            String newText = substringSafe(lastValue, 0, selectionEnd) +
                                    substringSafe(lastValue, selectionEnd + lengthDiff, lastValue.length());
            int selectionStart = json.optInt(SELECTION_START);
            if (selectionStart >= 0 && selectionStart == selectionEnd && text.equals(newText)) {
                json.put(COMPOSING_START, selectionEnd);
                json.put(COMPOSING_END, selectionEnd + lengthDiff);
                return true;
            }

            return isDeleted;
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
                return substringSafe(text, composingStart, composingEnd);
            }
            int start = selectionEnd - count;
            if (start < 0) {
                return "";
            }
            if (selectionEnd > text.length()) {
                return "";
            }
            return substringSafe(text, Math.max(start, 0), Math.min(selectionEnd, text.length()));
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
         * Set whether to delete
         *
         * @param isDeleted Set whether to delete
         */
        @Override
        public void setDeletedFlag(boolean isDeleted) {
            this.isDeleted = isDeleted;
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
        public void setSelectedState(boolean isSelected, CharSequence text, int selectionStart, int selectionEnd) {
            this.isSelected = isSelected;
            this.needUpdatedText = text.toString();
            this.lastSelectionStartFromCommit = selectionStart;
            this.lastSelectionEndFromCommit = selectionEnd;
        }

        private void setComposedState(boolean isComposing, int commitStart, int composingEnd) {
            this.isComposing = isComposing;
            this.lastComposingStart = commitStart;
            this.lastComposingEnd = composingEnd;
        }

        private void setJsonDefault(JSONObject json, String text,
                                    int selectionStart, int selectionEnd) throws JSONException {
            json.put(TEXT, text);
            json.put(SELECTION_START, selectionStart);
            json.put(SELECTION_END, selectionEnd);
            json.put(COMPOSING_START, -1);
            json.put(COMPOSING_END, -1);
            json.put(IS_DELETE, false);
        }

        private void processSelectedInputEnglish(JSONObject json, String lastValue, int clientId,
            int composingStart, int composingEnd) throws JSONException {
            if (selectedRange != null && selectedRange.containsKey(clientId)) {
                SelectedRange range = selectedRange.get(clientId);
                int start = range.selectionStart;
                int end = range.selectionEnd;
                String text = json.optString(TEXT);
                int lengthDiff = lastValue.length() - text.length() + (composingEnd - composingStart);
                if (start >= 0 && end > start && end - start == lengthDiff) {
                    String expectedText = substringSafe(lastValue, 0, start) +
                                        substringSafe(lastValue, end, lastValue.length());
                    String newText = substringSafe(text, 0, composingStart) +
                                    substringSafe(text, composingEnd, text.length());
                    if (newText.equals(expectedText)) {
                        json.put(APPEND_TEXT, substringSafe(text, composingStart, composingEnd));
                    }
                }
            }
        }

        private void handleComposingText(JSONObject json, String lastValue, int clientId,
            int composingStart, int composingEnd) throws JSONException {
            if (isNewCommitText) {
                isNewCommitText = false;
                setComposedState(false, -1, -1);
                return;
            }
            if (composingStart != -1 && (composingEnd > composingStart)) {
                json.put(COMPOSING_END, composingStart);
                if (isComposing && (lastComposingEnd < composingEnd)) {
                    json.put(COMPOSING_END, lastComposingEnd);
                    json.put(COMPOSING_START, composingStart);
                } else {
                    processSelectedInputEnglish(json, lastValue, clientId, composingStart, composingEnd);
                }
                setComposedState(true, composingStart, composingEnd);
            } else {
                if (isComposing) {
                    json.put(COMPOSING_START, lastComposingStart);
                    json.put(COMPOSING_END, lastComposingEnd);
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
            return substringSafe(text, start > 0 ? start : 0, Math.min(text.length(), selectionEnd));
        }

        private boolean isSingleLetterInput(String lastValue, String text, int composingStart, int composingEnd) {
            if (lastValue == null) {
                return false;
            }
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
            if (!substringSafe(lastValue, composingStart, composingEnd - 1).equals(substringSafe(text,
                composingStart, composingEnd - 1))) {
                return false;
            }
            if (substringSafe(lastValue, composingStart, composingEnd - 1).matches("[a-zA-Z]+")) {
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
            isDeleted = false;
            lastSelectionEndFromCommit = -1;
            lastSelectionStartFromCommit = -1;
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

            String textWithoutAppend = substringSafe(text, 0, appendStart) + text.substring(selectionEnd);
            return lastValue.equals(textWithoutAppend);
        }

        private String getCommitText(String lastValue, String text,
            int selectionEnd, JSONObject json, int clientId) throws JSONException {
            String appendText = lastCommittedTexts.remove(clientId);
            if (composingText != null && composingText.length() > 0) {
                String newText = appendText + composingText;
                if (newText.equals(substringSafe(text, Math.max(0, selectionEnd - newText.length()),
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
            String newCommitText = substringSafe(lastValue, 0, appendStart) +
                appendText + lastValue.substring(appendStart);
            if (text.length() == (lastValue.length() + appendText.length()) &&
                (newCommitText.equals(text) || isComposing)) {
                return appendText;
            }
            json.put(COMPOSING_START, appendStart);
            json.put(COMPOSING_END, appendStart + (appendText.length() - (text.length() - lastValue.length())));
            tryModifyComposeText(json, lastValue, text, appendText);
            isNewCommitText = true;
            return appendText;
        }

        private void handleOtherInput(JSONObject json, String text, int clientId,
            int composingStart, int composingEnd) throws JSONException {
            String appendText = null;
            boolean isAppendText = false;
            String lastValue = lastValueMap.get(clientId);
            int selectionEnd = json.getInt(SELECTION_END);
            if (lastCommittedTexts.containsKey(clientId)) {
                if (lastValue != null) {
                    appendText = getCommitText(lastValue, text, selectionEnd, json, clientId);
                    isAppendText = isAppendText(lastValue, text, appendText, selectionEnd);
                }
            } else {
                appendText = getNewInputStr(lastValue, text, selectionEnd, composingStart, composingEnd);
            }
            if (appendText != null && !appendText.isEmpty()) {
                json.put(APPEND_TEXT, appendText);
            }

            if (isAppendText && composingStart == -1) {
                setComposedState(false, -1, -1);
            } else {
                handleComposingText(json, lastValue, clientId, composingStart, composingEnd);
            }
        }

        private void tryModifyComposeText(JSONObject json, String lastValue, String text,
            String appendText) throws JSONException {
            int start = Math.max(0, json.getInt(COMPOSING_START));
            int end = Math.min(lastValue.length(), json.getInt(COMPOSING_END));
            if (start == end) {
                return;
            }
            StringBuilder lastValueStr = new StringBuilder(lastValue);
            lastValueStr.replace(start, end, appendText);
            if (!lastValueStr.toString().equals(text)) {
                start = Math.max(0, json.getInt(COMPOSING_START) - 1);
                end = Math.min(lastValue.length(), json.getInt(COMPOSING_END) - 1);
                if (end <= 0) {
                    return;
                }
                if (new StringBuilder(lastValue).replace(start, end, appendText).toString().equals(text)) {
                    json.put(COMPOSING_START, json.getInt(COMPOSING_START) - 1);
                    json.put(COMPOSING_END, json.getInt(COMPOSING_END) - 1);
                }
            }
        }

        private void handleSelectionError(String text, String lastValue, JSONObject json,
            int start, int end) throws JSONException {
            if (text == null || lastValue == null) {
                return;
            }
            if (start == end && text.length() == lastValue.length() &&
                    !text.equals(lastValue) && !json.has(APPEND_TEXT)) {
                json.put(APPEND_TEXT, "");
            }
        }

        private void tryModifyText(JSONObject json, String text, String lastValue, int selectionEnd,
            int clientId) throws JSONException {
            if (text == null || lastValue == null || selectionEnd < 0 || selectionEnd > text.length()) {
                return;
            }
            int start = json.getInt(COMPOSING_START);
            int end = json.getInt(COMPOSING_END);
            if (start < 0 || end <= 0 || start == end) {
                if (selectedRange.containsKey(clientId)) {
                    start = selectedRange.get(clientId).selectionStart;
                    end = selectedRange.get(clientId).selectionEnd;
                }
            }
            handleSelectionError(text, lastValue, json, start, end);
            if (start > end || !json.has(APPEND_TEXT)) {
                return;
            }
            String curAppendText = json.getString(APPEND_TEXT);
            if (isCorrectData(lastValue, text, start, end, json.getString(APPEND_TEXT))) {
                return;
            }
            DiffResult diffResult = getDiffResult(lastValue, text, start, selectionEnd);
            if (diffResult.preSelStart == diffResult.preSelEnd) {
                if (start == end && end != diffResult.preSelEnd) {
                    String newAppendText = substringSafe(text, Math.max(start, 0),
                                                            Math.min(selectionEnd - 1, text.length()));
                    if (isCorrectData(lastValue, text, start, end, newAppendText)) {
                        diffResult.setNewAppendText(newAppendText);
                    }
                }
                json.put(COMPOSING_START, -1);
                json.put(COMPOSING_END, -1);
            } else {
                json.put(COMPOSING_START, diffResult.preSelStart);
                json.put(COMPOSING_END, diffResult.preSelEnd);
            }
            json.put(APPEND_TEXT, diffResult.getNewAppendText());
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
            String insertedText = substringSafe(curText, textInsertStart, Math.max(textInsertEnd, selectionEnd));
            if (selectionEnd > textInsertEnd) {
                lastDelEnd = lastDelEnd + (selectionEnd - textInsertEnd);
            }
            if (selectionEnd < textInsertEnd && textInsertEnd - selectionEnd == 1 &&
                " ".equals(substringSafe(curText, selectionEnd, selectionEnd + 1))) {
                int substringStart = Math.max(start, 0);
                int substringEnd = Math.max(selectionEnd, substringStart);
                insertedText = substringSafe(curText, substringStart, substringEnd);
            }

            return new DiffResult(lastDelStart, lastDelEnd, textInsertStart, textInsertEnd, insertedText);
        }

        private String substringSafe(String text, int start, int end) {
            if (text == null || text.isEmpty()) {
                return "";
            }

            int startSafe = Math.max(0, start);
            int endSafe = Math.min(end, text.length());
            if (startSafe >= endSafe) {
                return "";
            }
            return text.substring(startSafe, endSafe);
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