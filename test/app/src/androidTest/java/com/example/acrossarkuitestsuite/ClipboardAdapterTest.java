package com.example.acrossarkuitestsuite;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.test.platform.app.InstrumentationRegistry;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.*;

import ohos.ace.adapter.AceEnv;
import ohos.ace.adapter.capability.clipboard.ClipboardPluginAosp;
import ohos.ace.adapter.capability.clipboard.ClipboardPluginBase;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class ClipboardAdapterTest {
    private AceEnv aceEnv;
    private Context context;
    private ClipboardPluginBase clipboardPluginBase;
    @Before
    public void doBefore() {
        this.context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        this.aceEnv = AceEnv.getInstance();
        this.clipboardPluginBase = new ClipboardPluginAosp(this.context);
        Intent intent = new Intent();
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setClass(context, MainActivity.class);
        context.startActivity(intent);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    @After
    public void doAfter() {
        this.clipboardPluginBase.clear();
        this.clipboardPluginBase = null;
        this.aceEnv = null;
        this.context = null;
    }

    @Test
    public void test_clipboard_setData() {
        Log.i("TEST", "test_clipboard_setData");
        this.clipboardPluginBase.setData("test clipboard");
        String getData = this.clipboardPluginBase.getData();
        assertEquals("test clipboard", getData);
    }

    @Test
    public void test_clipboard_clearData() {
        this.clipboardPluginBase.setData("test clipboard");
        this.clipboardPluginBase.clear();
        assertEquals("", this.clipboardPluginBase.getData());
    }
}