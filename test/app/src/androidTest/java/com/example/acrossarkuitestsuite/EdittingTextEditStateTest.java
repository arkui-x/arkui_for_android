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

package com.example.acrossarkuitestsuite;

import android.util.Log;

import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Test;
import org.junit.runner.RunWith;
import static org.junit.Assert.*;

import ohos.ace.adapter.capability.editing.TextEditState;

/**
 * EdittingTextEditStateTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class EdittingTextEditStateTest {
    @Test
    public void test_testedit() {
        JSONObject textInfo = new JSONObject();
        TextEditState state = null;
        String jsonString = "{\"text\":\"AcrossArkui\",\"hint\":\"ArkUI\",\"selectionStart\":5,\"selectionEnd\":8}";
        try {
            textInfo = new JSONObject(jsonString);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        try {
            state = TextEditState.fromJson(textInfo);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        assertNotNull(state);
        assertEquals("AcrossArkui", state.getText());
        assertEquals("ArkUI", state.getHint());
        assertEquals(5,state.getSelectionStart());
        assertEquals(8, state.getSelectionEnd());
    }
}
