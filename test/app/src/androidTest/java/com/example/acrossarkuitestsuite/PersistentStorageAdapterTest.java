package com.example.acrossarkuitestsuite;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import static org.junit.Assert.*;
import ohos.ace.adapter.AceEnv;
import ohos.ace.adapter.capability.clipboard.ClipboardPluginBase;
import ohos.ace.adapter.capability.storage.PersistentStorageAosp;
import ohos.ace.adapter.capability.storage.PersistentStorageBase;

@RunWith(AndroidJUnit4.class)
public class PersistentStorageAdapterTest {
    private AceEnv aceEnv;
    private Context context;
    private PersistentStorageBase persistentStorageBase;
    @Before
    public void doBefore() {
        this.aceEnv = AceEnv.getInstance();
        this.context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        Log.i("TEST", "init persistent storage");
        this.persistentStorageBase = new PersistentStorageAosp(this.context);
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
        this.persistentStorageBase.clear();
        this.persistentStorageBase = null;
        this.context = null;
        this.aceEnv = null;
    }

    @Test
    @Ignore
    public void test_persistentstroge_set_value() {
        Log.i("TEST", "test_persistentstroge_set_value");
        this.persistentStorageBase.set("name", "arkui");
        assertEquals("arkui", this.persistentStorageBase.get("name"));
        this.persistentStorageBase.set("name","arkui2.0");
        assertEquals("arkui2.0", this.persistentStorageBase.get("name"));
    }

    @Test
    public void test_persistentstroge_get_null() {
        String ret = this.persistentStorageBase.get("name");
        assertEquals("", ret);
    }
}
