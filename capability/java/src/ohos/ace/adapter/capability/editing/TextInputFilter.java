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
import android.text.SpannableStringBuilder;
import android.text.Spanned;

/**
 * TextInputFilter is a filter for filtering text input.
 *
 * @since 1
 */
public class TextInputFilter implements InputFilter {
    private String filterRule;
    private int maxLength = -1;

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

    @Override
    public CharSequence filter(CharSequence source, int start, int end, Spanned dest, int dstart, int dend) {
        int len = dest.length() - (dend - dstart);
        if (maxLength > 0) {
            if (len == maxLength) {
                return "";
            }
        }

        SpannableStringBuilder spannableString = new SpannableStringBuilder();
        for (int i = start; i < end; i++) {
            char inputChar = source.charAt(i);
            if (isMatch(inputChar)) {
                spannableString.append(inputChar);
            }
        }

        if (maxLength > 0) {
            int newLen = len + spannableString.length();
            if (newLen > maxLength) {
                spannableString.delete(maxLength - len, spannableString.length());
            }
        }
        return spannableString;
    }
}