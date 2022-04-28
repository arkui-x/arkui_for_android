package com.example.acrossarkuitestsuite;

import android.util.Log;

import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;
import static org.junit.Assert.*;
import ohos.ace.adapter.capability.editing.TextInputAction;

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
