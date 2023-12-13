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

import org.json.JSONException;
import org.json.JSONObject;

/**
 * State of an on-going text editing session.
 *
 * @since 1
 */
public class TextEditState {
    private final String text;
    private final String hint;
    private final int selectionStart;
    private final int selectionEnd;

    /**
     * Deserialize textEditState from JSON.
     *
     * @param textEditState Text editing state in JSON format.
     * @return TextEditState object.
     */
    public static TextEditState fromJson(JSONObject textEditState) throws JSONException {
        return new TextEditState(
            textEditState.getString("text"),
            textEditState.getString("hint"),
            textEditState.getInt("selectionStart"),
            textEditState.getInt("selectionEnd")
        );
    }

    private TextEditState(String text, String hint, int selectionStart, int selectionEnd) {
        this.text = text;
        this.hint = hint;
        this.selectionStart = selectionStart;
        this.selectionEnd = selectionEnd;
    }

    /**
     * Get text of TextEditState
     *
     * @return text of TextEditState
     */
    public String getText() {
        return text;
    }

    /**
     * Get hint of TextEditState
     *
     * @return hint of TextEditState
     */
    public String getHint() {
        return hint;
    }

    /**
     * Get selectionStart of TextEditState
     *
     * @return selectionStart of TextEditState
     */
    public int getSelectionStart() {
        return selectionStart;
    }

    /**
     * Get selectionEnd of TextEditState
     *
     * @return selectionEnd of TextEditState
     */
    public int getSelectionEnd() {
        return selectionEnd;
    }
}