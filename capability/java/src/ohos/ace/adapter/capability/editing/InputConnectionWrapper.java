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

import android.content.Context;
import android.text.Editable;
import android.text.Selection;
import android.text.method.TextKeyListener;
import android.text.SpannableString;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.ExtractedText;
import android.view.inputmethod.ExtractedTextRequest;
import android.view.inputmethod.InputMethodManager;

import ohos.ace.adapter.ALog;

/**
 * Adapt to the platform implementation for receiving IME event.
 *
 * @since 1
 */
class InputConnectionWrapper extends BaseInputConnection {
    private static final String LOG_TAG = "Ace_IME";

    private final View aceView;

    private final int clientId;

    private final TextInputDelegate delegate;

    private final Editable editable;

    private final String hint;

    private int batchCount;

    private InputMethodManager imm;

    private int extractedTextRequestToken = 0;

    /**
     * constructor of InputConnectionWrapper
     *
     * @param view the view that needs input connection
     * @param client client id
     * @param delegate input delegate
     * @param editable editable text
     * @param hint hint string
     */
    InputConnectionWrapper(View view, int client, TextInputDelegate delegate, Editable editable, String hint) {
        super(view, true);
        this.aceView = view;
        this.clientId = client;
        this.delegate = delegate;
        this.editable = editable;
        this.batchCount = 0;
        this.hint = hint;
        Object manager = view.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        if (manager instanceof InputMethodManager) {
            this.imm = (InputMethodManager) manager;
        }
    }

    @Override
    public ExtractedText getExtractedText(ExtractedTextRequest request, int flags) {
        if (request == null || editable == null) {
            return null;
        }
        if (request.token != 0) {
            extractedTextRequestToken = request.token;
        }
        ExtractedText extractedText = new ExtractedText();
        extractedText.flags = 0;
        extractedText.partialStartOffset = -1;
        extractedText.partialEndOffset = -1;
        extractedText.selectionStart = Selection.getSelectionStart(editable);
        extractedText.selectionEnd = Selection.getSelectionEnd(editable);
        extractedText.startOffset = 0;
        if ((request.flags & GET_TEXT_WITH_STYLES) != 0) {
            extractedText.text = new SpannableString(editable);
        } else {
            extractedText.text = editable.toString();
        }
        return extractedText;
    }

    @Override
    public Editable getEditable() {
        return editable;
    }

    @Override
    public boolean beginBatchEdit() {
        batchCount++;
        return super.beginBatchEdit();
    }

    @Override
    public boolean endBatchEdit() {
        boolean result = super.endBatchEdit();
        batchCount--;
        onStateUpdated();
        return result;
    }

    @Override
    public boolean commitText(CharSequence text, int newCursorPosition) {
        boolean result = super.commitText(text, newCursorPosition);
        onStateUpdated();
        return result;
    }

    @Override
    public boolean deleteSurroundingText(int beforeLength, int afterLength) {
        if (Selection.getSelectionStart(editable) == -1) {
            return true;
        }

        boolean deleted = super.deleteSurroundingText(beforeLength, afterLength);
        onStateUpdated();
        return deleted;
    }

    @Override
    public boolean setComposingRegion(int composingStart, int composingEnd) {
        boolean success = super.setComposingRegion(composingStart, composingEnd);
        onStateUpdated();
        return success;
    }

    @Override
    public boolean setComposingText(CharSequence text, int newCursorPosition) {
        boolean success = false;
        if (text.length() == 0) {
            success = super.commitText(text, newCursorPosition);
        } else {
            success = super.setComposingText(text, newCursorPosition);
        }
        onStateUpdated();
        return success;
    }

    @Override
    public boolean setSelection(int start, int end) {
        boolean success = super.setSelection(start, end);
        onStateUpdated();
        return success;
    }

    @Override
    public boolean sendKeyEvent(KeyEvent event) {
        ALog.d(LOG_TAG, "action & keycode: " + event.getAction() + " , " + event.getKeyCode());

        // Only care about down event.
        // Note: If the key is held down, ACTION_DOWN will be triggered multiple times by IME.
        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            if (event.getKeyCode() == KeyEvent.KEYCODE_DEL) {
                return deleteSelection(event);
            } else if (event.getKeyCode() == KeyEvent.KEYCODE_DPAD_LEFT) {
                moveToLeft();
            } else if (event.getKeyCode() == KeyEvent.KEYCODE_DPAD_RIGHT) {
                moveToRight();
            } else {
                return enterCharacter(event);
            }
            return true;
        }
        return false;
    }

    @Override
    public boolean performEditorAction(int actionCode) {
        ALog.d(LOG_TAG, "performEditorAction: " + actionCode);
        TextInputAction action;
        switch (actionCode) {
            case EditorInfo.IME_ACTION_NONE: {
                action = TextInputAction.NONE;
                break;
            }
            case EditorInfo.IME_ACTION_UNSPECIFIED: {
                action = TextInputAction.UNSPECIFIED;
                break;
            }
            case EditorInfo.IME_ACTION_GO: {
                action = TextInputAction.GO;
                break;
            }
            case EditorInfo.IME_ACTION_SEARCH: {
                action = TextInputAction.SEARCH;
                break;
            }
            case EditorInfo.IME_ACTION_SEND: {
                action = TextInputAction.SEND;
                break;
            }
            case EditorInfo.IME_ACTION_NEXT: {
                action = TextInputAction.NEXT;
                break;
            }
            case EditorInfo.IME_ACTION_PREVIOUS: {
                action = TextInputAction.PREVIOUS;
                break;
            }
            case EditorInfo.IME_ACTION_DONE:
            default: {
                action = TextInputAction.DONE;
                break;
            }
        }
        delegate.performAction(clientId, action);
        return true;
    }

    private void onStateUpdated() {
        // If the IME is in the middle of a batch edit, then wait until it completes.
        if (batchCount > 0 || imm == null) {
            return;
        }

        int selectionStart = Selection.getSelectionStart(editable);
        int selectionEnd = Selection.getSelectionEnd(editable);
        int composingStart = BaseInputConnection.getComposingSpanStart(editable);
        int composingEnd = BaseInputConnection.getComposingSpanEnd(editable);

        imm.updateSelection(this.aceView, selectionStart, selectionEnd, composingStart, composingEnd);

        // Update extract text to imm, enable show input text in fullscreen mode.
        ExtractedText extractedText = new ExtractedText();
        final CharSequence content = editable;
        final int length = content.length();
        extractedText.partialStartOffset = -1;
        extractedText.partialEndOffset = -1;
        extractedText.startOffset = 0;
        extractedText.selectionStart = selectionStart;
        extractedText.selectionEnd = selectionEnd;
        extractedText.flags = 0;
        extractedText.text = content.subSequence(0, length);
        imm.updateExtractedText(aceView, extractedTextRequestToken, extractedText);

        delegate.updateEditingState(clientId, editable.toString(), selectionStart, selectionEnd, composingStart,
            composingEnd);
    }

    public void updateInputFilterErrorText(String errorText) {
        delegate.updateInputFilterErrorText(clientId, errorText);
    }

    // Sanitizes the index to ensure the index is within the range of the contents of editable.
    private static int sanitizeIndex(int index, Editable editable) {
        return Math.max(0, Math.min(editable.length(), index));
    }

    private boolean deleteSelection(KeyEvent event) {
        int selStart = sanitizeIndex(Selection.getSelectionStart(editable), editable);
        int selEnd = sanitizeIndex(Selection.getSelectionEnd(editable), editable);
        if (selEnd > selStart) {
            Selection.setSelection(editable, selStart);
            editable.delete(selStart, selEnd);
            onStateUpdated();
            return true;
        } else if (selStart > 0) {
            if (TextKeyListener.getInstance().onKeyDown(null, editable, event.getKeyCode(), event)) {
                onStateUpdated();
                return true;
            }
        } else {
            ALog.w(LOG_TAG, "illegal selection.");
        }
        return false;
    }

    private void moveToRight() {
        int selStart = Selection.getSelectionStart(editable);
        int newSel = Math.min(selStart + 1, editable.length());
        setSelection(newSel, newSel);
    }

    private void moveToLeft() {
        int selStart = Selection.getSelectionStart(editable);
        int newSel = Math.max(selStart - 1, 0);
        setSelection(newSel, newSel);
    }

    private boolean enterCharacter(KeyEvent event) {
        int character = event.getUnicodeChar();
        if (character != 0) {
            int selStart = Math.max(0, Selection.getSelectionStart(editable));
            int selEnd = Math.max(0, Selection.getSelectionEnd(editable));
            if (selEnd != selStart) {
                editable.delete(selStart, selEnd);
            }
            editable.insert(selStart, String.valueOf((char) character));
            setSelection(selStart + 1, selStart + 1);
            onStateUpdated();
            return true;
        }
        return false;
    }
}
