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

import java.util.ArrayList;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputFilter.LengthFilter;
import android.text.InputType;
import android.text.Selection;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

import ohos.ace.adapter.ALog;

/**
 * TextInputPluginAosp is an add-on for handling text input.
 *
 * @since 1
 */
public class TextInputPluginAosp extends TextInputPluginBase implements TextInputErrorTextHandler{
    private static final String LOG_TAG = "Ace_IME";

    private final View view;

    private final InputMethodManager imm;

    private final Handler mainHandler;

    // Using Editable util class to delegate text change.
    private Editable editable;

    private String hint;

    // Indicate whether need to call IMM restartInput.
    private boolean isRestartInputPending = false;

    private InputConnectionWrapper wrapper = null;

    /**
     * constructor of TextInputPlugin on AOSP platform
     *
     * @param view the view which request input
     * @param instanceId id of container instance
     */
    public TextInputPluginAosp(View view, int instanceId) {
        super(instanceId);
        this.view = view;
        Object temp = view.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        if (temp instanceof InputMethodManager) {
            this.imm = (InputMethodManager) temp;
        } else {
            ALog.e(LOG_TAG, "Unable to get INPUT_METHOD_SERVICE");
            this.imm = null;
        }

        mainHandler = new Handler(Looper.getMainLooper());
    }

    /**
     * Get instance of InputMethodManager
     *
     * @return the instance of InputMethodManager
     */
    public InputMethodManager getInputMethodManager() {
        return imm;
    }

    /**
     * Creates an InputConnection for IME.
     *
     * @param view The view to connect with IME.
     * @param outAttrs The configuration for IME.
     * @return The new InputConnection.
     */
    public InputConnection createInputConnection(View view, EditorInfo outAttrs) {
        ALog.d(LOG_TAG, "createInputConnection");
        final TextInputConfiguration config = getConfiguration();
        if (!hasClient() || config == null) {
            return null;
        }
        outAttrs.inputType = inputTypeFromTextInputType(config.getType(), config.isObscure());
        outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_FULLSCREEN;
        int enterAction;
        if (config.getAction() != TextInputAction.UNSPECIFIED) {
            enterAction = convertInputAction(config.getAction());
        } else {
            // Depends on whether is multiline. If multiline, nothing happens while enter.
            if ((outAttrs.inputType & InputType.TYPE_TEXT_FLAG_MULTI_LINE) != 0) {
                enterAction = EditorInfo.IME_ACTION_NONE;
            } else {
                enterAction = EditorInfo.IME_ACTION_DONE;
            }
        }

        if (config.getActionLabel() != null && config.getActionLabel().length() > 0) {
            outAttrs.actionLabel = config.getActionLabel();
            outAttrs.actionId = enterAction;
        }
        outAttrs.imeOptions |= enterAction;

        final TextInputDelegate delegate = new Delegate();
        outAttrs.initialSelStart = Selection.getSelectionStart(editable);
        outAttrs.initialSelEnd = Selection.getSelectionEnd(editable);
        wrapper = new InputConnectionWrapper(view, clientId(), delegate, editable, hint);
        return wrapper;
    }

    // Just for ensure that JNI is called from Ace UI thread.
    private void runOnUIThread(Runnable task) {
        if (Looper.myLooper() == Looper.getMainLooper()) {
            task.run();
        } else {
            mainHandler.post(task);
        }
    }

    @Override
    public void showTextInput(boolean isFocusViewChanged) {
        ALog.d(LOG_TAG, "showTextInput");
        runOnUIThread(
            new Runnable() {

                /**
                 * Show keyboard.
                 */
                public void run() {
                    view.requestFocus();
                    if (imm != null) {
                        imm.showSoftInput(view, 0);
                    }
                }
            });
    }

    /**
     * Hide keyboard.
     */
    @Override
    public void hideTextInput() {
        ALog.d(LOG_TAG, "hideTextInput");
        runOnUIThread(
            new Runnable() {

                /**
                 * Hide keyboard.
                 */
                public void run() {
                    if (imm != null) {
                        imm.hideSoftInputFromWindow(view.getApplicationWindowToken(), 0);
                    }
                }
            });
    }

    @Override
    public void onTextInputErrorTextChanged(String errorText) {
        if (wrapper != null) {
            wrapper.updateInputFilterErrorText(errorText);
        }
    }

    /**
     * Called when clientId and configuration are initialized.
     */
    @Override
    protected void onInited() {
        editable = Editable.Factory.getInstance().newEditable("");
        final TextInputConfiguration config = getConfiguration();
        if (config != null) {
            ArrayList<InputFilter> filterArray = new ArrayList<InputFilter>();
            String inputFilterRule = config.getInputFilterRule();
            if (!inputFilterRule.isEmpty()) {
                TextInputFilter inputFilter = new TextInputFilter(inputFilterRule);
                inputFilter.setTextInputErrorTextHandler(this);
                filterArray.add(inputFilter);
            }
            int maxInputLength = config.getMaxInputLength();
            if (maxInputLength > 0) {
                LengthFilter lengthFilter = new InputFilter.LengthFilter(maxInputLength);
                filterArray.add(lengthFilter);
            }
            if (filterArray.size() > 0) {
                editable.setFilters(filterArray.toArray(new InputFilter[filterArray.size()]));
            }
        }

        // setTextInputClient will be followed by a call to setTextInputEditingState.
        // Do a restartInput at that time.
        isRestartInputPending = true;
    }

    /**
     * Sets the current text editing state, for example, text/selection.
     *
     * @param state The deserialized editing state.
     */
    @Override
    protected void onSetTextEditingState(TextEditState state) {
        if (state == null) {
            ALog.e(LOG_TAG, "onSetTextEditingState: state is null");
            return;
        }
        hint = state.getHint();
        if (!isRestartInputPending && state.getText().equals(editable.toString())) {
            applyStateToSelection(state);

            // Sync selection with the virtual View.
            if (imm != null) {
                imm.updateSelection(view, Math.max(Selection.getSelectionStart(editable), 0),
                    Math.max(Selection.getSelectionEnd(editable), 0),
                    BaseInputConnection.getComposingSpanStart(editable),
                    BaseInputConnection.getComposingSpanEnd(editable));
            }
        } else {
            // Text has been changed, update Editable first and update selection.
            editable.replace(0, editable.length(), state.getText());
            applyStateToSelection(state);
            // Call this method when the input text changes (for example View.setText()).
            if (imm != null) {
                imm.restartInput(view);
            }
            isRestartInputPending = false;
        }
    }

    // Update selection to Editable.
    private void applyStateToSelection(TextEditState state) {
        int selStart = state.getSelectionStart();
        int selEnd = state.getSelectionEnd();
        if (selStart >= 0 && selStart <= editable.length() && selEnd >= 0 && selEnd <= editable.length()) {
            Selection.setSelection(editable, selStart, selEnd);
        } else {
            Selection.removeSelection(editable);
        }
    }

    // Convert to android EditorInfo action.
    private static int convertInputAction(TextInputAction action) {
        switch (action) {
            case NONE: {
                return EditorInfo.IME_ACTION_NONE;
            }
            case GO: {
                return EditorInfo.IME_ACTION_GO;
            }
            case SEARCH: {
                return EditorInfo.IME_ACTION_SEARCH;
            }
            case SEND: {
                return EditorInfo.IME_ACTION_SEND;
            }
            case NEXT: {
                return EditorInfo.IME_ACTION_NEXT;
            }
            case DONE: {
                return EditorInfo.IME_ACTION_DONE;
            }
            case PREVIOUS: {
                return EditorInfo.IME_ACTION_PREVIOUS;
            }
            case UNSPECIFIED:
            default: {
                return EditorInfo.IME_ACTION_UNSPECIFIED;
            }
        }
    }

    // Convert to android.text.InputType value from config.
    private static int inputTypeFromTextInputType(TextInputType type, boolean isObscureText) {
        if (type == TextInputType.DATETIME) {
            return InputType.TYPE_CLASS_DATETIME;
        } else if (type == TextInputType.NUMBER) {
            return InputType.TYPE_CLASS_NUMBER;
        } else if (type == TextInputType.PHONE) {
            return InputType.TYPE_CLASS_PHONE;
        } else {
            ALog.d(LOG_TAG, "other text input type");
        }

        // Default to TEXT
        int textType = InputType.TYPE_CLASS_TEXT;
        if (type == TextInputType.MULTILINE) {
            textType |= InputType.TYPE_TEXT_FLAG_MULTI_LINE;
        } else if (type == TextInputType.EMAIL_ADDRESS) {
            textType |= InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS;
        } else if (type == TextInputType.URL) {
            textType |= InputType.TYPE_TEXT_VARIATION_URI;
        } else if (type == TextInputType.VISIBLE_PASSWORD) {
            textType |= InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD;
        } else {
            ALog.d(LOG_TAG, "only use class text type");
        }

        if (isObscureText) {
            // Note: both required. Some devices ignore TYPE_TEXT_FLAG_NO_SUGGESTIONS.
            textType |= InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
            textType |= InputType.TYPE_TEXT_VARIATION_PASSWORD;
        }

        return textType;
    }
}