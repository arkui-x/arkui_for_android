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

package ohos.ace.adapter.capability.web;

import android.webkit.WebBackForwardList;
import ohos.ace.adapter.AceResourcePlugin;

import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.atomic.AtomicLong;

/**
 * The class is the base class for creating web on Android and ohos platform.
 *
 * @since 1
 */
public abstract class AceWebPluginBase extends AceResourcePlugin {
    private static final String LOG_TAG = "AceWebPluginBase";

    private static Map<Long, AceWebBase> objectMap;

    private final AtomicLong nextWebId = new AtomicLong(0L);

    /**
     * Initialize the webview.
     */
    protected native void nativeInit();

    /**
     * Native init webview database methods.
     */
    protected native void nativeInitWebDataBase();

    /**
     * Native init webview download item methods.
     *
     * @param value The value of the webview.
     * @param asyncCallbackInfoId The ID of the asynchronous callback information.
     */
    protected native static void onReceiveValue(String value, long asyncCallbackInfoId);

    /**
     * Native method to handle the received JavaScript execution result.
     *
     * @param webId The webview id.
     * @param portHandle The port handle for the message.
     * @param result The result of the message.
     */
    protected native static void onMessage(long webId, String portHandle, String result);

    /**
     * Initialize the webview.
     */
    private static boolean hasInit = false;

    /**
     * Initialize the webview.
     */
    protected static boolean richTextInit = false;
    
    private static final int CAN_NOT_POST_MESSAGE = 17100010;

    private static final int CAN_NOT_REGISTER_MESSAGE_EVENT = 17100006;

    public AceWebPluginBase() {
        // plugin name is web, version is 1.0.
        super("web", 1.0f);
        objectMap = new HashMap<Long, AceWebBase>();
    }

    /**
     * This is called to get a atomic id.
     *
     * @return id
     */
    public long getAtomicId() {
        return nextWebId.getAndIncrement();
    }

    /**
     * This is called to add a resource object to map.
     *
     * @param id web id
     * @param web web object
     */
    public void addResource(long id, AceWebBase web) {
        objectMap.put(id, web);
        registerCallMethod(web.getCallMethod());
        if (!hasInit && !richTextInit) {
            nativeInit();
            nativeInitWebDataBase();
            hasInit = true;
        }
        richTextInit = false;
    }

    /**
     * This is called to create AceWebBase.
     *
     * @param param calling param
     * @return resource id
     */
    @Override
    public abstract long create(Map<String, String> param);

    /**
     * This is called to get AceWebBase object by id.
     *
     * @param id id of object
     * @return object or null if id not found
     */
    @Override
    public Object getObject(long id) {
        if (objectMap.containsKey(id)) {
            return objectMap.get(id);
        }
        return null;
    }

    public static Map<Long, AceWebBase> getObjectMap() {
        return objectMap;
    }

    /**
     * This is called for activity resume.
     *
     */
    public void onActivityResume() {
        for (Map.Entry<Long, AceWebBase> entry : objectMap.entrySet()) {
            entry.getValue().onActivityResume();
        }
    }

    /**
     * This is called for activity pause.
     *
     */
    public void onActivityPause() {
        for (Map.Entry<Long, AceWebBase> entry : objectMap.entrySet()) {
            entry.getValue().onActivityPause();
        }
    }

    /**
     * This is called to release AceWebBase by id.
     *
     * @param id id of object
     * @return result of release
     */
    @Override
    public boolean release(long id) {
        if (objectMap.containsKey(id)) {
            AceWebBase web = objectMap.get(id);
            unregisterCallMethod(web.getCallMethod());
            web.release();
            objectMap.remove(id);
            return true;
        }
        return false;
    }

    /**
     * This is called to release all AceWebBase.
     *
     */
    @Override
    public void release() {
        for (Map.Entry<Long, AceWebBase> entry : objectMap.entrySet()) {
            entry.getValue().release();
        }
    }

    /**
     * This is called to load url by id.
     *
     * @param id id of object
     * @param url url to load
     * @param header header of url
     */
    public void loadUrl(long id, String url, HashMap<String, String> header) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            webBase.loadUrl(url, header);
        }
    }

    /**
     * This is called to load data by id.
     *
     * @param id id of object
     * @param params params of data
     */
    public void loadData(long id, HashMap<String, String> params) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            webBase.loadData(params);
        }
    }

    /**
     * This is called to get url by id.
     *
     * @param id id of object
     * @return url
     */
    public String getUrl(long id) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            return webBase.getUrl();
        }
        return "";
    }

    /**
     * This is called to forward by id.
     *
     * @param id id of object
     * @return result of forward
     */
    public String accessForward(long id) {
        if (objectMap.containsKey(id)) {
            Map<String, String> defaultParam = new HashMap<String, String>();
            AceWebBase webBase = objectMap.get(id);
            return webBase.accessForward(defaultParam);
        }
        return "";
    }

    /**
     * This is called to forward by id.
     *
     * @param id id of object
     */
    public void forward(long id) {
        if (objectMap.containsKey(id)) {
            Map<String, String> defaultParam = new HashMap<String, String>();
            AceWebBase webBase = objectMap.get(id);
            webBase.forward(defaultParam);
        }
    }

    /**
     * This is called to go back by id.
     *
     * @param id id of object
     * @return result of go back
     */
    public String accessBackward(long id) {
        if (objectMap.containsKey(id)) {
            Map<String, String> defaultParam = new HashMap<String, String>();
            AceWebBase webBase = objectMap.get(id);
            return webBase.accessBackward(defaultParam);
        }
        return "";
    }

    /**
     * This is called to go back by id.
     *
     * @param id id of object
     */
    public void backward(long id) {
        if (objectMap.containsKey(id)) {
            Map<String, String> defaultParam = new HashMap<String, String>();
            AceWebBase webBase = objectMap.get(id);
            webBase.backward(defaultParam);
        }
    }

    /**
     * This is called to refresh by id.
     *
     * @param id id of object
     */
    public void refresh(long id) {
        if (objectMap.containsKey(id)) {
            Map<String, String> defaultParam = new HashMap<String, String>();
            AceWebBase webBase = objectMap.get(id);
            webBase.reload(defaultParam);
        }
    }

    /**
     * Evaluates the given JavaScript code in the context of the web view identified by the specified ID.
     *
     * @param id webId.
     * @param script The JavaScript code to be evaluated.
     * @param asyncCallbackInfoId The identifier for the asynchronous callback information.
     */
    public void evaluateJavascript(long id, String script, long asyncCallbackInfoId) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            webBase.evaluateJavascript(script, asyncCallbackInfoId);
        }
    }

    /**
     * Get the back forward entries of the webview by id.
     *
     * @param id id of object
     * @return WebBackForwardList of the webview
     */
    public WebBackForwardList getBackForwardEntries(long id) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            return webBase.getBackForwardEntries();
        }
        return null;
    }

    /**
     * Clear the cache of the webview.
     *
     * @param id id of object
     * @param includeDiskFiles Whether to clear disk files.
     */
    public void removeCache(long id, boolean includeDiskFiles) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            webBase.clearCache(includeDiskFiles);
        }
    }

    /**
     * Go back or forward in the history stack.
     *
     * @param id id of object
     * @param steps The number of steps to go back or forward.
     */
    public void backOrForward(long id, int steps) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            webBase.goBackOrForward(steps);
        }
    }

    /**
     * Get the title of the web page.
     *
     * @param id id of object
     * @return The title of the web page.
     */
    public String getTitle(long id) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            return webBase.getTitle();
        }
        return "";
    }

    /**
     * Get the height of the content of the web page.
     *
     * @param id id of object
     * @return The height of the content of the web page.
     */
    public int getPageHeight(long id) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            return webBase.getContentHeight();
        }
        return -1;
    }

    /**
     * Get the width of the content of the web page.
     *
     * @param id id of object
     * @param step The number of steps to scroll.
     * @return The width of the content of the web page.
     */
    public String accessStep(long id, int step) {
        if (objectMap.containsKey(id)) {
            Map<String, String> defaultParam = new HashMap<String, String>();
            defaultParam.put("accessStep", Integer.toString(step));
            AceWebBase webBase = objectMap.get(id);
            return webBase.accessStep(defaultParam);
        }
        return "";
    }

    /**
     * Scroll to the specified position.
     *
     * @param id id of object
     * @param x The x-coordinate of the scroll position.
     * @param y The y-coordinate of the scroll position.
     */
    public void scrollTo(long id, int x, int y) {
        if (objectMap.containsKey(id)) {
            Map<String, String> defaultParam = new HashMap<String, String>();
            defaultParam.put("scroll_to_x", Integer.toString(x));
            defaultParam.put("scroll_to_y", Integer.toString(y));
            AceWebBase webBase = objectMap.get(id);
            webBase.scrollTo(defaultParam);
        }
    }

    /**
     * Scroll the content of the webview by the specified amount.
     *
     * @param id id of object
     * @param x The x-coordinate of the scroll position.
     * @param y The y-coordinate of the scroll position.
     */
    public void scrollBy(long id, int x, int y) {
        if (objectMap.containsKey(id)) {
            Map<String, String> defaultParam = new HashMap<String, String>();
            defaultParam.put("scroll_by_deltax", Integer.toString(x));
            defaultParam.put("scroll_by_deltay", Integer.toString(y));
            AceWebBase webBase = objectMap.get(id);
            webBase.scrollBy(defaultParam);
        }
    }

    /**
     * zoom the webview with the specified ID.
     *
     * @param id id of object
     * @param step The number of steps to zoom.
     */
    public void zoom(long id, float step) {
        if (objectMap.containsKey(id)) {
            Map<String, String> defaultParam = new HashMap<String, String>();
            defaultParam.put("zoom_factor", Float.toString(step));
            AceWebBase webBase = objectMap.get(id);
            webBase.zoom(defaultParam);
        }
    }

    /**
     * stop the webview with the specified ID.
     *
     * @param id id of object
     */
    public void stop(long id) {
        if (objectMap.containsKey(id)) {
            Map<String, String> defaultParam = new HashMap<String, String>();
            AceWebBase webBase = objectMap.get(id);
            webBase.stopLoading(defaultParam);
        }
    }

    /**
     * Scroll through WebView content.
     *
     * @param id id of object.
     */
    public void clearHistory(long id) {
        if (objectMap.containsKey(id)) {
            Map<String, String> defaultParam = new HashMap<String, String>();
            AceWebBase webBase = objectMap.get(id);
            webBase.clearHistory(defaultParam);
        }
    }

    /**
     * Set the custom user agent of the specified ID.
     *
     * @param id id of object
     * @param userAgent custom user agent
     */
    public void setCustomUserAgent(long id, String userAgent) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            webBase.setUserAgentString(userAgent);
        }
    }

    /**
     * Get the custom user agent of the specified ID.
     *
     * @param id id of object
     * @return custom user agent
     */
    public String getCustomUserAgent(long id) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            return webBase.getUserAgentString();
        }
        return "";
    }

    /**
     * notify activity lifecycle changed to plugin.
     *
     * @param isBackground to background state
     */
    @Override
    public void notifyLifecycleChanged(Boolean isBackground) {
        if (isBackground) {
            onActivityPause();
        } else {
            onActivityResume();
        }
    }

    /**
     * Create a pair of message ports for communication between Webview and HTML5.
     *
     * @param id Wevbiew id.
     * @return Message ports.
     */
    public String[] createWebMessagePorts(long id) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            return webBase.createWebMessagePorts();
        }
        String[] ports = {"", ""};
        return ports;
    }

    /**
     * Post message to HTML5.
     *
     * @param id Wevbiew id.
     * @param message Message to be posted.
     * @param ports Message port.
     * @param uri Message uri.
     */
    public void postWebMessage(long id, String message, String[] ports, String uri) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            webBase.postWebMessage(message, ports, uri);
        }
    }

    /**
     * Close the message port with the specified ID.
     *
     * @param id Wevbiew id.
     * @param portHandle Message port handle.
     */
    public void closeWebMessagePort(long id, String portHandle) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            webBase.closeWebMessagePort(portHandle);
        }
    }

    /**
     * Post message to HTML5.
     *
     * @param id Wevbiew id.
     * @param portHandle Message port handle.
     * @param webMessage The Message is a message sent to H5.
     * @return The result of the message event.
     */
    public int postMessageEvent(long id, String portHandle, String webMessage) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            return webBase.postMessageEvent(portHandle, webMessage);
        }
        return CAN_NOT_POST_MESSAGE;
    }

    /**
     * Register message event.
     *
     * @param id Wevbiew id.
     * @param portHandle Message port handle.
     * @return The result of the message event.
     */
    public int onWebMessagePortEvent(long id, String portHandle) {
        if (objectMap.containsKey(id)) {
            AceWebBase webBase = objectMap.get(id);
            return webBase.onWebMessagePortEvent(id, portHandle);
        }
        return CAN_NOT_REGISTER_MESSAGE_EVENT;
    }
}
