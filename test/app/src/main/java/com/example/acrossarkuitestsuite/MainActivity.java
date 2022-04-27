package com.example.acrossarkuitestsuite;

import androidx.appcompat.app.AppCompatActivity;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import ohos.ace.adapter.AceActivity;

public class MainActivity extends AceActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.e("HiHelloWorld", "HiHelloWorldActivity");
        setVersion(2);   // add serVersion calling before super onStart
        setInstanceName("MainAbility");
        super.onCreate(savedInstanceState);
    }
}