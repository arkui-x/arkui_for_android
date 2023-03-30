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

import android.content.Context;
import java.util.ArrayList;

import ohos.ace.adapter.ALog;
import ohos.ace.adapter.capability.bridge.BridgePlugin;
import ohos.ace.adapter.capability.bridge.IMessageListener;
import ohos.ace.adapter.capability.bridge.IMethodResult;

/**
 * Test Bridge class.
 *
 * @since 10
 */
public class BridgeTest extends BridgePlugin implements IMessageListener, IMethodResult {
    /**
     * Name of Bridge.
     *
     */
    public final String name;

    /**
     * Test function without parameters.
     *
     * @return Return default string.
     */
    public String getString0Paras() {
        ALog.i("BridgeTest getString0Paras: ", "0 paras");
        return "return:getString0Paras";
    }

    /**
     * Test function with two parameters.
     *
     * @param testString string parameters.
     * @param testInt int parameters.
     *
     * @return Return default string.
     */
    public String getString2Paras(String testString, int testInt) {
        ALog.i("BridgeTest getString2Paras: ", "2 paras  " + testString + " " + testInt);
        return "return:getString2Paras";
    }

    /**
     * Test function with two parameters.
     *
     * @param testString string parameters.
     * @param testBool bool parameters.
     *
     * @return Return default string.
     */
    public String getString2ParasBool(String testString, boolean testBool) {
        ALog.i("BridgeTest getString2Paras: ", "2 paras  " + testString + " " + String.valueOf(testBool));
        return "return:getString2Paras";
    }

    /**
     * Test function with StringArray parameters.
     *
     * @param listString ArrayList parameters.
     *
     * @return Return test string.
     */
    public String getStringArrayParas(ArrayList<String> listString) {
        ALog.i("BridgeTest getStringArrayParas: ", "Array paras" + listString.get(0));
        return "return:getStringArrayParas";
    }

    /**
     * Test function with one parameters.
     *
     * @param testInt int parameters.
     *
     * @return Return number.
     */
    public int getInt(int testInt) {
        ALog.i("BridgeTest", "BridgeTest call getInt success");
        return testInt + 1;
    }

    /**
     * Bridge dependent plugin.
     *
     * @param context context of the application.
     * @param name name of bridge.
     * @param id the id of instance.
     * @return BridgePlugin object.
     */
    public BridgeTest(Context context, String name, int id) {
        super(context, name, id);
        this.name = name;
        setMethodResultListener(this);
        setMessageListener(this);
    }

    @Override
    /**
     * Listen to messages sent by other platforms.
     *
     * @param jsonObject Data to be sent.
     * @return Return another platform's data.
     *
     * @return Return empty string.
     */
    public Object onMessage(Object jsonObject) {
        ALog.i("onMessage data:", jsonObject.toString());
        return "";
    }

    @Override
    /**
     * Listen to messagesResponse sent by other platforms.
     *
     * @param jsonObject Data to be sent.
     */
    public void onMessageResponse(Object jsonObject) {
        ALog.i("onMessageResponse data:", jsonObject.toString());
    }

    @Override
    /**
     * Call other platform method successfully and return trigger.
     *
     * @param testObject the instance id of current view.
     */
    public void onSuccess(Object testObject) {
        ALog.i("onSuccess data:", testObject.toString());
    }

    @Override
    /**
     * Call other platform method failed and return trigger.
     *
     * @param methodName The method name.
     * @param errorCode The error id.
     * @param errorMessage The error message.
     */
    public void onError(String methodName, int errorCode, String errorMessage) {
        ALog.i("BridgeTest onError :", errorMessage);
    }

    @Override
    /**
     * Call other platform method successfully and return trigger.
     *
     * @param methodName The method name.
     */
    public void onMethodCancel(String methodName) {
        ALog.i("BridgeTest onMethodCancel :", methodName);
    }
}
