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

import org.junit.Test;
import org.junit.runner.RunWith;
import static org.junit.Assert.*;
import ohos.ace.adapter.capability.editing.TextInputAction;



/**
 * EdittingTextInputActionTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class EdittingTextInputActionTest {
    @Test
    public void test_textinput_action_normal() {
        TextInputAction action = TextInputAction.of(3);
        assertEquals(3, action.getValue());
        action = TextInputAction.of(5);
        assertEquals(5, action.getValue());
        action = TextInputAction.of(1);
        assertEquals(1, action.getValue());
        action = TextInputAction.of(7);
        assertEquals(7, action.getValue());

    }

    @Test
    public void test_textinput_action_illegal() {
        TextInputAction action = TextInputAction.of(8);
        assertEquals(0, action.getValue());

    }
}
