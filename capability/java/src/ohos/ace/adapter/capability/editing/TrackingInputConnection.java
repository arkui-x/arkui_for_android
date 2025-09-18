/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
import android.view.inputmethod.InputConnectionWrapper;

/**
 * A wrapper for InputConnection that tracks commit text and selection changes.
 *
 * @since 20
 */
public class TrackingInputConnection extends InputConnectionWrapper {
    private static final String TAG = "Ace_IME";
    private String lastCommittedText;
    private int commitStart;
    private int commitEnd;
    private InputCommitListener listener;

    /**
     * Creates a new TrackingInputConnection that wraps the given target InputConnection
     * and reports committed text to the provided listener.
     *
     * @param target the target InputConnection to wrap
     * @param listener the listener to notify when text is committed
     */
    public TrackingInputConnection(InputConnection target, InputCommitListener listener) {
        super(target, true);
        this.listener = listener;
    }

    /**
     * Commits text to the target InputConnection and records the committed text.
     *
     * This method will notify the configured InputCommitListener with the committed
     * text before delegating the actual commit to the wrapped InputConnection.
     *
     * @param text the text to commit
     * @param newCursorPosition the new cursor position relative to the length of the text
     * @return true if the underlying InputConnection handled the commit, false otherwise
     */
    @Override
    public boolean commitText(CharSequence text, int newCursorPosition) {
        if (text != null) {
            lastCommittedText = text.toString();
            if (listener != null) {
                listener.onCommit(lastCommittedText);
            }
        }
        return super.commitText(text, newCursorPosition);
    }

    /**
     * Listener interface for receiving committed text callbacks.
     */
    public interface InputCommitListener {
        /**
         * Called when text is committed via commitText.
         *
         * @param text the committed text
         */
        void onCommit(String text);
    }
}