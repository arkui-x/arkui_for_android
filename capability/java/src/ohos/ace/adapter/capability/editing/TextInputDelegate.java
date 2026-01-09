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

package ohos.ace.adapter.capability.editing;

/**
 * TextInputDelegate
 *
 * @since 1
 */
public interface TextInputDelegate {
    /**
     * Called to pass the whole text content and the editing state when the editing state changes.
     *
     * @param clientId The client identifier
     * @param text The whole text content
     * @param selectionStart The selection start
     * @param selectionEnd The selection end
     * @param composingStart The composing start
     * @param composingEnd The composing end
     */
    void updateEditingState(int clientId, String text, int selectionStart, int selectionEnd, int composingStart,
        int composingEnd);

    /**
     * Called when the action button clicked, for example, GO / SEARCH, is clicked.
     *
     * @param clientId The client identifier
     * @param action The action identifier
     */
    void performAction(int clientId, TextInputAction action);

    /**
     * Called to pass the error text when the error text changes.
     *
     * @param clientId The client identifier
     * @param errorText The error text content
     */
    void updateInputFilterErrorText(int client, String errorText);

    /**
     * Called when the keyboard is closed.
     *
     * @param clientId The client identifier
     */
    void notifyKeyboardClosedByUser(int clientId);

    /**
     * Called when the selected text is submitted.
     *
     * @param isSelected The selected state
     * @param text The selected text content
     */
    void setSelectedState(boolean isSelected, CharSequence text);

    /**
     * Set the selection range in the text.
     *
     * @param selectionStart The start position of the selection
     * @param selectionEnd The end position of the selection
     */
    void setSelectedState(int selectionStart, int selectionEnd);

    /**
     * Finish composing text and update the composing state.
     *
     * @param finishComposingText The text to complete the composing
     * @param composingStart The start position of the composing text
     */
    void setFinishComposingText(String finishComposingText, int composingStart);

    /**
     * Set composing text and update the composing state.
     *
     * @param composingText The text to set as composing
     */
    void setComposingText(String composingText);
}