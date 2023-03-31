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
import ohos.ace.adapter.capability.bridge.BridgeManager;
import ohos.ace.adapter.capability.bridge.MethodData;
import ohos.ace.adapter.capability.bridge.BridgeErrorCode;

/**
 * EnvironmentTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class BridgeManagerTest {
    private AceEnv aceEnv;
    private Context context;
    private BridgeManager aceBridgeManager;
    private BridgeTest bridgeTest;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        this.context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        this.aceEnv = AceEnv.getInstance();
        aceBridgeManager = aceBridgeManager.getInstance();
        bridgeTest = new BridgeTest(this.context, "Bridge", 100);
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        this.context = null;
        this.aceBridgeManager = null;
    }

    /**
     * getInstance
     */
    @Test
    public void getInstance_0100() {
        BridgeManager obj = aceBridgeManager.getInstance();
        Assert.assertNotEquals(null, obj);
    }

    /**
     * registerAceBridgePlugin
     */
    @Test
    public void registerAceBridgePlugin_0200() {
        aceBridgeManager.registerBridgePlugin("testBridgePlugin", this.bridgeTest);
        Assert.assertNotEquals(null, bridgeTest);
    }

    /**
     * unRegisterAceBridgePlugin
     */
    @Test
    public void unRegisterAceBridgePlugin_0300() {
        boolean retB = aceBridgeManager.unRegisterBridgePlugin("testBridgePlugin");
        Assert.assertTrue(retB);
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0400() {
        try {
            String jsonString = "{\"\"}";
            aceBridgeManager.jsCallMethod("Bridge", "getString0Paras", jsonString);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertFalse(false);
        }
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0401() {
        try {
            aceBridgeManager.jsCallMethod("Bridge", "getString0Paras", null);
            Assert.assertTrue(false);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(true);
        }
    }

    /**
     * jsCallMethod
     */
    @Test
    public void jsCallMethod_0402() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "aaa");
            jObj.put("1", true);
            aceBridgeManager.jsCallMethod("Bridge", "getString2ParasBool", jObj.toString());
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
    public void jsCallMethod_0403() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "aaa");
            jObj.put("1", "ddddddddddddddd");
            aceBridgeManager.jsCallMethod("Bridge", "getString2ParasBool", jObj.toString());
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
    public void jsCallMethod_0404() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "aaa");
            jObj.put("1", 123);
            aceBridgeManager.jsCallMethod("Bridge", "getString2Paras", jObj.toString());
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
    public void jsCallMethod_0405() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "aaa");
            jObj.put("1", "123");
            aceBridgeManager.jsCallMethod("Bridge", "getString2Paras", jObj.toString());
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
    public void jsCallMethod_0406() {
        try {
            String jsonString = "{\"0:\"zhangshan\",\"1:\"1,\"2:78}";
            aceBridgeManager.jsCallMethod("Bridge", "getString2Paras", jsonString);
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
    public void jsCallMethod_0407() {
        try {
            ArrayList<String> list = new ArrayList<>();
            list.add("A");
            list.add("BBBB");
            JSONArray jsonArray = new JSONArray();
            jsonArray.put(list);
            JSONObject jObj = new JSONObject();
            jObj.put("0", jsonArray);

            aceBridgeManager.jsCallMethod("Bridge", "getStringArrayParas", jObj.toString());
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
    public void jsCallMethod_0408() {
        try {
            ArrayList<String> list = new ArrayList<>();
            list.add("A");
            list.add("BBBB");
            JSONArray jsonArray = new JSONArray();
            jsonArray.put(list);
            JSONObject jObj = new JSONObject();
            jObj.put("0", jsonArray);

            aceBridgeManager.jsCallMethod("Bridge", "getString2Paras", jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * platformSendMethodResult
     */
    @Test
    public void platformSendMethodResult_0500() {

        try {
            String result = "";
            aceBridgeManager.platformSendMethodResult("Bridge", "getString2Paras", result);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }

    }

    /**
     * platformSendMethodResult
     */
    @Test
    public void platformSendMethodResult_0501() {

        try {
            String result = "";
            aceBridgeManager.platformSendMethodResult("Bridge", "getString2ParasNNNNNNNN", result);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }

    }

    /**
     * platformCallMethod
     */
    @Test
    public void platformCallMethod_0600() {
        String result = "";
        Object[] testObject = {"aaa", 123};
        MethodData methodData = new MethodData("getString2Paras", testObject);
        BridgeErrorCode errorCode = aceBridgeManager.platformCallMethod("Bridge", methodData);
        Assert.assertEquals(BridgeErrorCode.BRIDGE_ERROR_NO, errorCode);
    }

    /**
     * platformCallMethod
     */
    @Test
    public void platformCallMethod_0601() {
        String result = "";
        Object[] testObject = {"aaa", 123};
        MethodData methodData = new MethodData("getString2ParasDDEOEOOEE", testObject);
        BridgeErrorCode errorCode = aceBridgeManager.platformCallMethod("Bridge", methodData);
        Assert.assertNotEquals(BridgeErrorCode.BRIDGE_METHOD_NAME_ERROR, errorCode);
    }

    /**
     * platformCallMethod
     */
    @Test
    public void platformCallMethod_0602() {
        String result = "";
        Object[] testObject = {"aaa", "edooifwfew"};
        MethodData methodData = new MethodData("getString2Paras", testObject);
        BridgeErrorCode errorCode = aceBridgeManager.platformCallMethod("Bridge", methodData);
        Assert.assertNotEquals(BridgeErrorCode.BRIDGE_METHOD_PARAM_ERROR, errorCode);
    }

    /**
     * jsSendMethodResult
     */
    @Test
    public void jsSendMethodResult_0700() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "aaa");
            jObj.put("1", 123);
            aceBridgeManager.jsSendMethodResult("Bridge", "getString2Paras", jObj.toString());
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
    public void jsSendMessage_0800() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "aaa");
            jObj.put("1", 123);
            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
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
    public void jsSendMessage_0801() {
        try {
            aceBridgeManager.jsSendMessage("Bridge", null);
            Assert.assertTrue(false);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(true);
        }
    }

    /**
     * jsSendMessage
     */
    @Test
    public void jsSendMessage_0802() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890");
            jObj.put("1", 123);

            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
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
    public void jsSendMessage_0803() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "qwfergrthytjk[opkgpgkropwgjmkpjoivjaerovjrovnbrbnoininbonobrno"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "qwfergrthytjk[opkgpgkropwgjmkpjoivjaerovjrovnbrbnoininbonobrnofewgvrtwgbrthfwefer");
            jObj.put("1", 123);

            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
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
    public void jsSendMessage_0804() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试"
                    + "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用"
                    + "中文测试使用中文测试");

            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
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
    public void jsSendMessage_0805() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试"
                    + "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使"
                    + "用中文测试使用中文测试");
            jObj.put("1", 123454);
            jObj.put("2", true);
            jObj.put("3", 033444);

            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
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
    public void jsSendMessage_0806() {
        try {
            ArrayList<String> list = new ArrayList<>();
            list.add("A");
            list.add("BBBB");
            JSONArray jsonArray = new JSONArray();
            jsonArray.put(list);
            JSONObject jObj = new JSONObject();
            jObj.put("0", jsonArray);

            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
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
    public void jsSendMessage_0807() {
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

            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * platformSendMessageResponse
     */
    @Test
    public void platformSendMessageResponse_0900() {
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

            aceBridgeManager.platformSendMessageResponse("Bridge", jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * platformSendMessageResponse
     */
    @Test
    public void platformSendMessageResponse_0901() {
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

            aceBridgeManager.platformSendMessageResponse("Bridgeddfeaferwgwr4grghw5th5weh", jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * platformSendMessageResponse
     */
    @Test
    public void platformSendMessageResponse_0902() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "ddddddddddddddwefd");

            aceBridgeManager.platformSendMessageResponse("Bridgeddfeaferwgwr4grghw5th5weh", jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * platformSendMessage
     */
    @Test
    public void platformSendMessage_1000() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "aaa");
            jObj.put("1", 123);

            aceBridgeManager.platformSendMessage("Bridge", jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * platformSendMessage
     */
    @Test
    public void platformSendMessage_1001() {
        try {
            aceBridgeManager.platformSendMessage("Bridge", null);
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * platformSendMessage
     */
    @Test
    public void platformSendMessage_1002() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890");
            jObj.put("1", 123);

            aceBridgeManager.platformSendMessage("Bridge", jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * platformSendMessage
     */
    @Test
    public void platformSendMessage_1003() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "qwfergrthytjk[opkgpgkropwgjmkpjoivjaerovjrovnbrbnoininbonobrno"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    + "qwfergrthytjk[opkgpgkropwgjmkpjoivjaerovjrovnbrbnoininbonobrnofewgvrtwgbrthfwefer");
            jObj.put("1", 123);

            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
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
    public void platformSendMessage_1004() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试"
                    + "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试"
                    + "使用中文测试使用中文测试");

            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * platformSendMessage
     */
    @Test
    public void platformSendMessage_1005() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试"
                    + "使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使用中文测试使"
                    + "用中文测试使用中文测试");
            jObj.put("1", 123454);
            jObj.put("2", true);
            jObj.put("3", 033444);

            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * platformSendMessage
     */
    @Test
    public void platformSendMessage_1006() {
        try {
            ArrayList<String> list = new ArrayList<>();
            list.add("A");
            list.add("BBBB");
            JSONArray jsonArray = new JSONArray();
            jsonArray.put(list);
            JSONObject jObj = new JSONObject();
            jObj.put("0", jsonArray);

            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }

    /**
     * platformSendMessage
     */
    @Test
    public void platformSendMessage_1007() {
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

            aceBridgeManager.jsSendMessage("Bridge", jObj.toString());
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
    public void jsSendMessageResponse_1100() {
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

            aceBridgeManager.jsSendMessageResponse("Bridge", jObj.toString());
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
    public void jsSendMessageResponse_1101() {
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

            aceBridgeManager.jsSendMessageResponse("Bridgeddfeaferwgwr4grghw5th5weh", jObj.toString());
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
    public void jsSendMessageResponse_1102() {
        try {
            JSONObject jObj = new JSONObject();
            jObj.put("0", "ddddddddddddddwefd");

            aceBridgeManager.jsSendMessageResponse("Bridgeddfeaferwgwr4grghw5th5weh", jObj.toString());
            Assert.assertTrue(true);
        } catch (Exception e) {
            e.printStackTrace();
            Assert.assertTrue(false);
        }
    }
}