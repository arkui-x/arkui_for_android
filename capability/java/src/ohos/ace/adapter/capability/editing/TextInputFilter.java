/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import android.text.InputFilter;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextUtils;

/**
 * TextInputFilter is a filter for filtering text input.
 *
 * @since 1
 */
public class TextInputFilter implements InputFilter {
    private String filterRule;
    private int maxLength = -1;
    private TextInputErrorTextHandler handler = null;

    public TextInputFilter(String filterRule) {
        this.filterRule = filterRule;
    }

    public void setMaxLength(int maxLength) {
        this.maxLength = maxLength;
    }

    private boolean isMatch(char inputChar) {
        String charStr = Character.toString(inputChar);
        return charStr.matches(filterRule);
    }

    /**
     * Set textinput error text handler.
     * @param handler the handler
     */
    public void setTextInputErrorTextHandler(TextInputErrorTextHandler handler) {
        this.handler = handler;
    }

    @Override
    public CharSequence filter(CharSequence source, int start, int end, Spanned dest, int dstart, int dend) {
        if (TextUtils.isEmpty(source.toString())) {
            return "";
        }

        StringBuilder resultStr = new StringBuilder();
        StringBuilder errorTextStr = new StringBuilder();
        for (int i = start; i < end; i++) {
            char inputChar = source.charAt(i);
            if (isMatch(inputChar)) {
                resultStr.append(inputChar);
            } else {
                errorTextStr.append(inputChar);
            }
        }
        if (handler != null && errorTextStr.length() > 0) {
            handler.onTextInputErrorTextChanged(errorTextStr.toString());
        }

        if (source instanceof Spanned) {
            SpannableString resultSpStr = new SpannableString(resultStr);
            TextUtils.copySpansFrom((Spanned) source, start, resultStr.length(), null, resultSpStr, 0);
            return resultSpStr;
        } else {
            return resultStr;
        }
    }
}
