package com.example.acrossarkuitestsuite;

import android.util.Log;

import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Test;
import org.junit.runner.RunWith;
import static org.junit.Assert.*;

import ohos.ace.adapter.capability.editing.TextEditState;

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
