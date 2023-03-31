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

package com.example.acrossarkuitestsuite;

import org.json.JSONArray;
import org.json.JSONObject;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import android.content.Context;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import java.util.ArrayList;

import ohos.ace.adapter.AceEnv;
import ohos.ace.adapter.capability.bridge.MethodData;

/**
 * EnvironmentTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class BridgePluginTest {
    private AceEnv aceEnv;
    private Context context;
    private BridgeTest bridgeTest;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        this.context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        this.aceEnv = AceEnv.getInstance();
        bridgeTest = new BridgeTest(this.context, "Bridge", 200);
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        this.context = null;
        this.bridgeTest = null;
    }

    /**
     * getBridgeName
     */
    @Test
    public void getBridgeName_0100() {

        String retStr = "";
        retStr = bridgeTest.getBridgeName();
        Assert.assertEquals("Bridge", retStr);
    }

    /**
     * getActivityId
     */
    @Test
    public void getActivityId_0200() {
        int ret = -1;
        ret = bridgeTest.getActivityId();
        Assert.assertEquals(200, ret);
    }

    /**
     * unRegister
     */
    @Test
    public void unRegister_0300() {
        try {
            bridgeTest.unRegister("bridge");
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * unRegister
     */
    @Test
    public void unRegister_0301() {
        try {
            bridgeTest.unRegister("bridgefererwrgfwrgrtg");
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * setMessageListener
     */
    @Test
    public void setMessageListener_0400() {
        try {
            bridgeTest.setMessageListener(bridgeTest);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * setMethodResultListener
     */
    @Test
    public void setMethodResultListener_0500() {
        try {
            bridgeTest.setMethodResultListener(bridgeTest);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0600() {
        try {
            String result = "";
            Object[] testObject = {"aaa", 123};
            MethodData methodData = new MethodData("getString2Paras", testObject);
            bridgeTest.jsCallMethod("bridge", methodData);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0601() {
        try {
            String result = "";
            Object[] testObject = {"aaa", 123};
            MethodData methodData = new MethodData("getString2ParasDDEOEOOEE", testObject);
            bridgeTest.jsCallMethod("bridge", methodData);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0602() {
        try {
            Object[] testObject = {"aaa", "edooifwfew"};
            MethodData methodData = new MethodData("getString2Paras", testObject);
            bridgeTest.jsCallMethod("bridge", methodData);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0603() {
        try {
            Object[] testObject = {"aaa", 1};
            MethodData methodData = new MethodData("getString2ParasBool", testObject);
            bridgeTest.jsCallMethod("Bridge", methodData);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0604() {
        try {
            Object[] testObject = {"aaa", 123};
            MethodData methodData = new MethodData("getString2Paras", testObject);
            bridgeTest.jsCallMethod("Bridge", methodData);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0605() {
        try {
            Object[] testObject = {"aaa", "123"};
            MethodData methodData = new MethodData("getString2Paras", testObject);
            bridgeTest.jsCallMethod("Bridge", methodData);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0606() {
        try {
            Object[] testObject = {"aaa", 123, "ddfwe"};
            MethodData methodData = new MethodData("getString2Paras", testObject);
            bridgeTest.jsCallMethod("Bridge", methodData);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0607() {
        try {
            ArrayList<String> list = new ArrayList<>();
            list.add("A");
            list.add("BBBB");
            Object[] testObject = {list};
            MethodData methodData = new MethodData("getStringArrayParas", testObject);
            bridgeTest.jsCallMethod("Bridge", methodData);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0608() {
        try {
            ArrayList<String> list = new ArrayList<>();
            list.add("A");
            list.add("BBBB");
            Object[] testObject = {list, "dddd"};
            MethodData methodData = new MethodData("getStringArrayParas", testObject);
            bridgeTest.jsCallMethod("Bridge", methodData);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * callMethod
     */
    @Test
    public void callMethod_0700() {
        try {
            String result = "";
            Object[] testObject = {"aaa", 123};
            MethodData methodData = new MethodData("getString2Paras", testObject);
            bridgeTest.callMethod(methodData);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * sendMessage
     */
    @Test
    public void sendMessage_0800() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "aaa");
            jObj.put("1", 123);

            bridgeTest.sendMessage(jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * sendMessage
     */
    @Test
    public void sendMessage_0801() {
        try {
            bridgeTest.sendMessage(null);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * sendMessage
     */
    @Test
    public void sendMessage_0802() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890");
            jObj.put("1", 123);

            bridgeTest.sendMessage(jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * sendMessage
     */
    @Test
    public void sendMessage_0803() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "qwfergrthytjk[opkgpgkropwgjmkpjoivjaerovjrovnbrbnoininbonobrno"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "qwfergrthytjk[opkgpgkropwgjmkpjoivjaerovjrovnbrbnoininbonobrnofewgvrtwgbrthfwefer");
            jObj.put("1", 123);

            bridgeTest.sendMessage(jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * sendMessage
     */
    @Test
    public void sendMessage_0804() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试"
                    + "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用"
                    + "中文测试使用中文测试");

            bridgeTest.sendMessage(jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * sendMessage
     */
    @Test
    public void sendMessage_0805() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试"
                    + "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用"
                    + "中文测试使用中文测试");
            jObj.put("1", 123454);
            jObj.put("2", true);
            jObj.put("3", 033444);

            bridgeTest.sendMessage(jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * sendMessage
     */
    @Test
    public void sendMessage_0806() {
        try {
            ArrayList<String> list = new ArrayList<>();
            list.add("A");
            list.add("BBBB");
            JSONArray jsonArray = new JSONArray();
            jsonArray.put(list);
            JSONObject jObj = new JSONObject();
            jObj.put("0", jsonArray);

            bridgeTest.sendMessage(jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * sendMessage
     */
    @Test
    public void sendMessage_0807() {
        try {
            ArrayList<String> list = new ArrayList<>();
            list.add("A");
            list.add("BBBB");
            JSONArray jsonArray = new JSONArray();
            jsonArray.put(list);
            JSONObject jObj = new JSONObject();
            jObj.put("0", jsonArray);
            jObj.put("1", "111111111111");
            jObj.put("2", "222222222222");

            bridgeTest.sendMessage(jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsSendMessage
     */
    @Test
    public void jsSendMessage_0900() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "aaa");
            jObj.put("1", 123);

            bridgeTest.jsSendMessage(jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsSendMethodResult
     */
    @Test
    public void jsSendMethodResult_0900() {
        try {
            Object result = new Object();
            bridgeTest.jsSendMethodResult(result, "", 0, "");
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * jsSendMessageResponse
     */
    @Test
    public void jsSendMessageResponse_1000() {
        try {
            bridgeTest.jsSendMessageResponse(this);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }
}