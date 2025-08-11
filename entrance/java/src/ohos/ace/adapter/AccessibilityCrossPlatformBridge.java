/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
package ohos.ace.adapter;

import android.content.ContentResolver;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityManager;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.accessibility.AccessibilityNodeProvider;
import ohos.ace.adapter.AccessibilityCrossPlatformBridge.ArkUiAccessibilityNodeInfo;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Arrays;
import java.util.Queue;
import java.util.stream.Collectors;
import java.util.function.Predicate;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONArray;

/**
 * AccessibilityCrossPlatformBridge is used to provide accessibility service for ArkUI components.
 *
 * @since 2024-11-29
 */
public class AccessibilityCrossPlatformBridge extends AccessibilityNodeProvider {
    /**
     * Invalid virtual view id.
     */
    public static final int INVALID_VIRTUAL_VIEW_ID = -1;
    private static final String TAG = "ArkUIAccessbilityProvider";
    private static FocusedNode currentFocusNode = null;
    private static final int ROOT_NODE_ID = 0;
    private static final int UNDEFINED_ACCESSIBILITY_ID = -1;
    private static final int INVALID_PARENT_ID = -2100000;
    private static final int ANDROID_API_28 = 28;
    private static final int ANDROID_API_24 = 24;
    private static final int ANDROID_API_35 = 35;
    private static final String ACE_COMPONENT_VIDEO = "Video";
    private static final String ACE_COMPONENT_TOGGLE = "Toggle";
    private static final String ACE_COMPONENT_TEXTINPUT = "TextInput";
    private static final String ACE_COMPONENT_TEXTAREA = "TextArea";
    private static final String ACE_COMPONENT_TABS = "Tabs";
    private static final String ACE_COMPONENT_SWITCH = "Switch";
    private static final String ACE_COMPONENT_SWIPERINDICATOR = "SwiperIndicator";
    private static final String ACE_COMPONENT_SWIPER = "Swiper";
    private static final String ACE_COMPONENT_SCROLL = "Scroll";
    private static final String ACE_COMPONENT_ROW = "Row";
    private static final String ACE_COMPONENT_RADIOBUTTON = "RadioButton";
    private static final String ACE_COMPONENT_RADIO = "Radio";
    private static final String ACE_COMPONENT_POPUP = "Popup";
    private static final String ACE_COMPONENT_MENU = "Menu";
    private static final String ACE_COMPONENT_DIALOG = "Dialog";
    private static final String ACE_COMPONENT_PAGE = "page";
    private static final String ACE_COMPONENT_LIST = "List";
    private static final String ACE_COMPONENT_IMAGEVIEW = "ImageView";
    private static final String ACE_COMPONENT_IMAGE = "Image";
    private static final String ACE_COMPONENT_GRIDROW = "GridRow";
    private static final String ACE_COMPONENT_GRIDCOL = "GridCol";
    private static final String ACE_COMPONENT_GRID = "Grid";
    private static final String ACE_COMPONENT_COLUMN = "Column";
    private static final String ACE_COMPONENT_CHECKBOX = "CheckBox";
    private static final String ACE_COMPONENT_BUTTON = "Button";
    private static final String ACE_COMPONENT_GRIDITEM = "GridItem";
    private static final String ACE_COMPONENT_LEFTARROW = "LeftArrow";
    private static final String ACE_COMPONENT_RIGHTARROW = "RightArrow";
    private static final String ACE_COMPONENT_NAVIGATION = "Navigation";
    private static final String ACE_COMPONENT_TEXT = "Text";
    private static final String ACE_COMPONENT_SLIDER = "Slider";
    private static final String KEY_ACCESSIBILITY_TEXT = "AccessibilityText";
    private static final String KEY_DESCRIPTION_INFO = "DescriptionInfo";
    private static final String KEY_COMPONENT_TYPE = "ComponentType";
    private static final int TYPE_PAGE_OPEN = 2049;
    private static final int TYPE_PAGE_CLOSE = 2050;
    private static final int EVENT_DELAY_TIME = 1000;

    Runnable updateNodeIds = () -> {
        if (!isTouchExplorationEnabled()) {
            return;
        }

        int[] intArray = nativeGetTreeIdArray(this.windowId);
        if (intArray == null || intArray.length <= 0) {
            return;
        }

        updateAccessibilityNodeInfod(intArray);
    };

    private final AccessibilityManager accessibilityManager;
    private View arkuiRootAccessibilityView = null;
    private long jsAccessibilityStateObserverPtr = 0L;
    private int windowId = 0;
    private List<Integer> currentPageNodeIds = new ArrayList<>();
    private Map<Integer, ArkUiAccessibilityNodeInfo> arkUiframeNodes = new LinkedHashMap<>();
    private Map<Integer, HashSet<Integer>> SwiperChldId = new LinkedHashMap<>();
    private int inputFocusNodeId = -1;
    private int accessFocusNodeId = -1;
    private List<FocusedNode> arkUiAccFocusRouteNodes = new ArrayList<>();
    private boolean isNoFocusable = false;
    private boolean isMenuFocus = false;
    private boolean isStateChanged = true;
    private boolean disabledDelay = false;

    private final AccessibilityManager.AccessibilityStateChangeListener stateChangeListener = (isEnable) -> {
        onChanged(isEnable);
        isStateChanged = false;
    };

    private final AccessibilityManager.TouchExplorationStateChangeListener touchExplorationStateChangeListener = (
            isEnable) -> {
        if (isStateChanged) {
            onChanged(isEnable);
        }

        if (isEnable && isAccessibilityEnabled()) {
            arkUiAccFocusRouteNodes.clear();
            if (arkuiRootAccessibilityView != null) {
                arkuiRootAccessibilityView.removeCallbacks(updateNodeIds);
                isMenuFocus = false;
                inputFocusNodeId = -1;
                arkuiRootAccessibilityView.post(updateNodeIds);
            }
        }

        if (!isEnable) {
            isStateChanged = true;
        }

        if (isAccessibilityEnabled()) {
            nativeTouchExplorationStateChange(true, this.windowId);
        }
    };

    /**
     * Represents an accessibility node information class, used to encapsulate
     * AccessibilityNodeInfo object and its related properties.
     */
    public static class ArkUiAccessibilityNodeInfo {
        private int nodeId = -1;
        private int[] childIds = new int[0];
        private String componentType = "";
        private boolean isDirty = true;
        private AccessibilityNodeInfo nodeInfo = null;

        private ArkUiAccessibilityNodeInfo() {
        }

        /**
         * Creates and returns an ArkUiAccessibilityNodeInfo object.
         *
         * @param nodeInfo The AccessibilityNodeInfo object used to initialize the
         *                 ArkUiAccessibilityNodeInfo
         * @param nodeId   The unique identifier for the node
         * @return The initialized ArkUiAccessibilityNodeInfo object
         */
        public static ArkUiAccessibilityNodeInfo obtain(AccessibilityNodeInfo nodeInfo, int nodeId) {
            ArkUiAccessibilityNodeInfo info = new ArkUiAccessibilityNodeInfo();
            info.nodeInfo = AccessibilityNodeInfo.obtain(nodeInfo);
            info.nodeId = nodeId;
            return info;
        }

        /**
         * Creates and returns an ArkUiAccessibilityNodeInfo object.
         *
         * @param jsonStr The JSON string used to initialize the ArkUiAccessibilityNodeInfo
         */
        public void init(String jsonStr) {
            if (!jsonStr.isEmpty()) {
                try {
                    JSONObject jsonObject = new JSONObject(jsonStr);
                    componentType = jsonObject.getString(KEY_COMPONENT_TYPE);
                    JSONArray jsonChildIDArray = new JSONArray(jsonObject.getString("childIDs"));
                    childIds = new int[jsonChildIDArray.length()];
                    for (int index = 0; index < jsonChildIDArray.length(); index++) {
                        childIds[index] = jsonChildIDArray.getInt(index);
                    }
                } catch (JSONException e) {
                    Log.e(TAG, "createAccessibilityNodeInfo failed; err is " + e.getMessage());
                }
            }
        }
    }

    private static class FocusedNode {
        /**
         * The node id of the focused node
         */
        public int nodeId = -1;

        /**
         * The window id of the focused node
         */
        public int windowId = 0;

        /**
         * The page id of the focused node
         */
        public int pageId = -1;

        private FocusedNode(int nodeId, int windowId) {
            this.nodeId = nodeId;
            this.windowId = windowId;
        }

        private FocusedNode(int nodeId, int windowId, int pageId) {
            this.nodeId = nodeId;
            this.windowId = windowId;
            this.pageId = pageId;
        }

        /**
         * Creates and returns a FocusedNode object.
         *
         * @param nodeId   The unique identifier for the node
         * @param windowId The unique identifier for the window
         * @return The initialized FocusedNode object
         */
        public static FocusedNode obtain(int nodeId, int windowId) {
            return new FocusedNode(nodeId, windowId);
        }

        /**
         * Creates and returns a FocusedNode object.
         *
         * @param nodeId   The unique identifier for the node
         * @param windowId The unique identifier for the window
         * @param pageId   The unique identifier for the page
         * @return The initialized FocusedNode object
         */
        public static FocusedNode obtain(int nodeId, int windowId, int pageId) {
            return new FocusedNode(nodeId, windowId, pageId);
        }
    }

    private class TextMoveUnit {
        /**
         * The text move unit
         */
        public static final int STEP_INVALID = 0x0000;

        /**
         * Move one character at a time
         */
        public static final int STEP_CHARACTER = 0x0001;

        /**
         * Move one word at a time
         */
        public static final int STEP_WORD = 0x0002;

        /**
         * Move one line at a time
         */
        public static final int STEP_LINE = 0x0004;

        /**
         * Move one paragraph at a time
         */
        public static final int STEP_PARAGRAPH = 0x0008;

        /**
         * Move one page at a time
         */
        public static final int STEP_PAGE = 0x0010;
    }

    private class ScrollType {
        /**
         * Scroll to the default position
         */
        public static final int SCROLL_DEFAULT = -1;

        /**
         * Scroll to the half position
         */
        public static final int SCROLL_HALF = 0;

        /**
         * Scroll to the full position
         */
        public static final int SCROLL_FULL = 1;
    }

    public AccessibilityCrossPlatformBridge(AccessibilityManager accessibilityManager, ContentResolver contentResolver,
            View arkuiRootAccessibilityView, int windowId) {
        this.accessibilityManager = accessibilityManager;
        this.arkuiRootAccessibilityView = arkuiRootAccessibilityView;
        this.accessibilityManager.addTouchExplorationStateChangeListener(touchExplorationStateChangeListener);
        this.accessibilityManager.addAccessibilityStateChangeListener(stateChangeListener);
        this.windowId = windowId;
        nativeSetupJsAccessibilityManager(this.windowId);
    }

    private void onChanged(boolean isEnable) {
        disabledDelay = false;
        if (isEnable && isAccessibilityEnabled()) {
            accessibilityStateChanged(true);
            if (currentFocusNode != null) {
                currentFocusNode = null;
            }
        } else {
            disabledDelay = true;
            if (currentFocusNode != null) {
                int[] args = { currentFocusNode.nodeId, AccessibilityNodeInfo.ACTION_CLEAR_ACCESSIBILITY_FOCUS,
                        currentFocusNode.windowId };
                nativePerformAction(args, "{}");
                currentFocusNode = null;
            }
            disabledDelay = false;
            accessibilityStateChanged(false);
        }
    }

    /**
     * Release the resources.
     */
    public void release() {
        if (accessibilityManager != null) {
            accessibilityManager.removeAccessibilityStateChangeListener(stateChangeListener);
            accessibilityManager
                    .removeTouchExplorationStateChangeListener(touchExplorationStateChangeListener);
        }

        if (this.windowId > 0) {
            nativeRelease(this.windowId);
        }

        arkUiframeNodes.clear();
        arkUiAccFocusRouteNodes.clear();
        if (arkuiRootAccessibilityView != null) {
            arkuiRootAccessibilityView.removeCallbacks(updateNodeIds);
            arkuiRootAccessibilityView = null;
        }
    }

    /**
     * Gets the current focus node
     *
     * @return the current focus node
     */
    public boolean isAccessibilityEnabled() {
        if (accessibilityManager != null) {
            return accessibilityManager.isEnabled();
        }

        return false;
    }

    /**
     * Gets the current focus node
     *
     * @return the current focus node
     */
    public boolean isTouchExplorationEnabled() {
        if (accessibilityManager != null) {
            return accessibilityManager.isTouchExplorationEnabled();
        }

        return false;
    }

    ArkUiAccessibilityNodeInfo findChildAccessNodeInfo(ArkUiAccessibilityNodeInfo parent) {
        ArkUiAccessibilityNodeInfo childInfo = null;
        int parentId = parent.nodeId;
        int childCount = parent.childIds.length;

        if (childCount <= 0) {
            return null;
        }
        Iterator it = arkUiframeNodes.keySet().iterator();
        boolean isFind = false;
        while (it.hasNext()) {
            int key = (int) it.next();
            if (!isFind && key != parentId) {
                continue;
            } else {
                isFind = true;
            }
            ArkUiAccessibilityNodeInfo arkNodeInfo = arkUiframeNodes.get(key);
            if (arkNodeInfo != null && arkNodeInfo.nodeInfo != null) {
                CharSequence contentText = arkNodeInfo.nodeInfo.getText();
                CharSequence contentDescriptionText = arkNodeInfo.nodeInfo.getContentDescription();
                if ((contentText != null && !contentText.toString().isEmpty()) ||
                        (contentDescriptionText != null && !contentDescriptionText.toString().isEmpty())) {
                    childInfo = arkNodeInfo;
                    break;
                }
            }
        }
        return childInfo;
    }

    ArkUiAccessibilityNodeInfo findNodeInfo(ArkUiAccessibilityNodeInfo parent,
            Predicate<ArkUiAccessibilityNodeInfo> condition) {
        if (parent == null) {
            return null;
        }
        Queue<ArkUiAccessibilityNodeInfo> queue = new LinkedList<>();
        queue.add(parent);

        while (!queue.isEmpty()) {
            ArkUiAccessibilityNodeInfo currentNode = queue.poll();

            if (condition.test(currentNode)) {
                return currentNode;
            }

            for (int childID : currentNode.childIds) {
                ArkUiAccessibilityNodeInfo child = arkUiframeNodes.get(childID);
                if (child != null) {
                    queue.add(child);
                }
            }
        }
        return null;
    }

    ArkUiAccessibilityNodeInfo findMenuChildAccessNodeInfo(ArkUiAccessibilityNodeInfo parent) {
        return findNodeInfo(parent, node -> {
            CharSequence text = node.nodeInfo.getText();
            CharSequence contentDescription = node.nodeInfo.getContentDescription();
            return (text != null && !text.toString().isEmpty()) ||
                    (contentDescription != null && !contentDescription.toString().isEmpty());
        });
    }

    ArkUiAccessibilityNodeInfo findMenuFocusAccessNodeInfo(ArkUiAccessibilityNodeInfo parent) {
        return findNodeInfo(parent, node -> node.nodeInfo.isAccessibilityFocused());
    }

    /**
     * Gets the current focus node
     *
     * @param ids the current focus node id
     */
    synchronized public void updateAccessibilityNodeInfod(int[] ids) {
        if (ids == null || ids.length <= 0 || !isTouchExplorationEnabled()) {
            return;
        }
        currentPageNodeIds.clear();
        currentPageNodeIds.addAll(Arrays.stream(ids).boxed().collect(Collectors.toList()));
        arkUiframeNodes.clear();
        for (int i = 0; i < currentPageNodeIds.size(); i++) {
            if (!arkUiframeNodes.containsKey(currentPageNodeIds.get(i))) {
                createAccessibilityNodeInfo(currentPageNodeIds.get(i), true);
            }
        }
        sendWindowContentChangeEvent(currentPageNodeIds.get(0));
        if (!isMenuFocus && performPopup()) {
            return;
        }
        if (!isMenuFocus) {
            int lastFocusRoute = getAccFocusRouteAndReset();
            if (lastFocusRoute > -1) {
                performAction(lastFocusRoute, AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, new Bundle());
                return;
            }
        }
        if (isMenuFocus) {
            ArkUiAccessibilityNodeInfo childInfo = findMenuFocusAccessNodeInfo(arkUiframeNodes.get(inputFocusNodeId));
            if (childInfo == null) {
                childInfo = findMenuChildAccessNodeInfo(arkUiframeNodes.get(inputFocusNodeId));
            }
            if (childInfo != null) {
                performAction(childInfo.nodeId, AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, new Bundle());
                return;
            }
        }
        if (inputFocusNodeId > 0 && currentPageNodeIds.contains(inputFocusNodeId)) {
            ArkUiAccessibilityNodeInfo childInfo = findChildAccessNodeInfo(arkUiframeNodes.get(inputFocusNodeId));
            if (childInfo == null) {
                return;
            }
            int nodeId = childInfo.nodeId;
            if (nodeId != -1) {
                performAction(nodeId, AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, new Bundle());
            } else {
                performAction(inputFocusNodeId, AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, new Bundle());
            }
            return;
        }
        handleDefaultFocus();
    }

    private void handleDefaultFocus() {
        if (!arkUiframeNodes.values().stream().anyMatch(info -> info.nodeInfo.isAccessibilityFocused())) {
            for (Map.Entry<Integer, ArkUiAccessibilityNodeInfo> entry : arkUiframeNodes.entrySet()) {
                AccessibilityNodeInfo nodeInfo = entry.getValue().nodeInfo;
                CharSequence text = nodeInfo.getText();
                CharSequence contentDescription = nodeInfo.getContentDescription();
                if ((text != null && !text.toString().isEmpty()) ||
                        (contentDescription != null && !contentDescription.toString().isEmpty())) {
                    performAction(entry.getKey(), AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, new Bundle());
                    break;
                }
            }
        }
    }
    
    private boolean performPopup() {
        if (arkUiframeNodes.values().stream().anyMatch(info -> ACE_COMPONENT_POPUP.equals(info.componentType))) {
            arkUiframeNodes.entrySet().stream().filter(entry -> ACE_COMPONENT_POPUP
                    .equals(entry.getValue().componentType))
                    .findFirst()
                    .ifPresent(entry -> {
                        int popId = entry.getKey();
                        int childCount = entry.getValue().childIds.length;
                        ArkUiAccessibilityNodeInfo childInfo = findChildAccessNodeInfo(entry.getValue());
                        if (childInfo == null) {
                            ALog.e(TAG, "childInfo:null");
                            return;
                        }
                        int nodeId = childInfo.nodeId;
                        if (nodeId != -1) {
                            performAction(nodeId, AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, new Bundle());
                        } else {
                            performAction(popId, AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, new Bundle());
                        }
                    });
            return true;
        }
        return false;
    }

    AccessibilityNodeInfo createAccessibilityNodeInfo(int virtualViewId, boolean isNew) {
        AccessibilityNodeInfo result = null;
        if (virtualViewId == View.NO_ID && arkuiRootAccessibilityView != null) {
            result = AccessibilityNodeInfo.obtain(arkuiRootAccessibilityView);
            arkuiRootAccessibilityView.onInitializeAccessibilityNodeInfo(result);
            result.setImportantForAccessibility(true);
            int rootId = nativeGetRootElementId(this.windowId);
            result.addChild(arkuiRootAccessibilityView, rootId);
            if (Build.VERSION.SDK_INT >= ANDROID_API_24) {
                result.setImportantForAccessibility(false);
            }
            if (isNew) {
                arkUiframeNodes.put(rootId, ArkUiAccessibilityNodeInfo.obtain(result, rootId));
            }
            return result;
        }
        boolean isDirtyNode = true;
        if (!isNew && arkUiframeNodes.containsKey(virtualViewId)) {
            isDirtyNode = arkUiframeNodes.get(virtualViewId).isDirty;
            if (!arkUiframeNodes.get(virtualViewId).isDirty) {
                return AccessibilityNodeInfo.obtain(arkUiframeNodes.get(virtualViewId).nodeInfo);
            }
        }

        String jsonStr = nativeCreateAccessibilityNodeInfo(virtualViewId, this.windowId);
        result = convertJsonToNodeInfo(jsonStr);
        if (result == null) {
            Log.e(TAG, "Failed to convert JSON to NodeInfo for virtualViewId: " + virtualViewId + ", windowId: "
                    + this.windowId + ", JSON: " + jsonStr);
            return result;
        }
        if (isNew) {
            ArkUiAccessibilityNodeInfo arkuiInfo = ArkUiAccessibilityNodeInfo.obtain(result, virtualViewId);
            arkuiInfo.init(jsonStr);
            arkuiInfo.isDirty = false;
            arkUiframeNodes.put(virtualViewId, arkuiInfo);
        } else {
            if (isDirtyNode && arkUiframeNodes.containsKey(virtualViewId)) {
                ArkUiAccessibilityNodeInfo arkuiInfo = ArkUiAccessibilityNodeInfo.obtain(result, virtualViewId);
                arkuiInfo.init(jsonStr);
                arkuiInfo.isDirty = false;
                arkUiframeNodes.replace(virtualViewId, arkuiInfo);
            }
        }
        return result;
    }

    private Rect getBoundsInScreen(Rect bounds) {
        int[] locationOnScreen = new int[2];
        arkuiRootAccessibilityView.getLocationOnScreen(locationOnScreen);
        int left = locationOnScreen[0];
        int top = locationOnScreen[1];
        int right = left + arkuiRootAccessibilityView.getWidth();
        int bottom = top + arkuiRootAccessibilityView.getHeight();
        Rect boundsInScreen = new Rect();
        boundsInScreen.left = Math.max(left, bounds.left + left);
        boundsInScreen.top = Math.max(top, bounds.top + top);
        boundsInScreen.right = Math.min(right, bounds.right + left);
        boundsInScreen.bottom = Math.min(bottom, bounds.bottom + top);
        return boundsInScreen;
    }

    AccessibilityNodeInfo convertJsonToNodeInfo(String jsonStr) {
        AccessibilityNodeInfo result = null;
        if (!jsonStr.isEmpty()) {
            try {
                JSONObject jsonObject = new JSONObject(jsonStr);
                int accessibilityId = jsonObject.getInt("AccessibilityId");
                String componentType = jsonObject.getString(KEY_COMPONENT_TYPE);
                result = AccessibilityNodeInfo.obtain(arkuiRootAccessibilityView,
                        accessibilityId);
                populateNodeInfoFromJson(result, jsonObject);
                boolean isArrayButton = ACE_COMPONENT_LEFTARROW.equals(componentType)
                        || ACE_COMPONENT_RIGHTARROW.equals(componentType);
                boolean isTextInputChild = isChildOfComponentType(accessibilityId, componentType, "Stack",
                        ACE_COMPONENT_TEXTINPUT);
                boolean isSideBarChild = isChildOfComponentType(accessibilityId, componentType, ACE_COMPONENT_BUTTON,
                        "SideBarContainer");
                result.setClassName(getClassNameString(isTextInputChild ? ACE_COMPONENT_BUTTON : componentType));
                String accessibilityText = jsonObject.getString(KEY_ACCESSIBILITY_TEXT).trim();
                String accessibilityDescription = jsonObject.getString(KEY_DESCRIPTION_INFO).trim();
                String text = counterCovertContent(jsonObject.getString("Content"),
                        jsonObject.getInt("AccessibilityId"),
                        componentType);
                boolean isFocusable = jsonObject.getBoolean("IsFocusable")
                        || jsonObject.getBoolean("ImportantForAccessibility")
                        || !text.isEmpty()
                        || !accessibilityText.isEmpty()
                        || !accessibilityDescription.isEmpty();
                result.setFocusable(isFocusable);
                setMovementGranularities(result, jsonObject);
                setParentID(result, jsonObject);
                setBounds(result, jsonObject);
                setCollection(result, jsonObject);
                result.setText(isTextInputChild ? "显示或隐藏密码" : text);
                result.setHintText(jsonObject.getString("Hint"));
                String contentDescription = accessibilityText.isEmpty() ? text + "\n" + accessibilityDescription
                        : accessibilityText + "\n" + accessibilityDescription;
                result.setContentDescription(contentDescription.trim());
                setChildren(result, jsonObject, isTextInputChild || isSideBarChild || isArrayButton
                        || ACE_COMPONENT_SLIDER.equals(componentType));
                if (!setTooltipTextAndText(result, jsonObject)) {
                    Log.e(TAG, "!setTooltipTextAndText, jsonStr: " + jsonStr);
                    return null;
                }
            } catch (JSONException e) {
                Log.e(TAG, "createAccessibilityNodeInfo failed; err is " + e.getMessage());
                return null;
            }
        }
        return result;
    }

    private void populateNodeInfoFromJson(AccessibilityNodeInfo nodeInfo, JSONObject jsonObject) {
        try {
            nodeInfo.setImportantForAccessibility(jsonObject.getBoolean("ImportantForAccessibility"));
            nodeInfo.setViewIdResourceName(jsonObject.getString("ComponentResourceId"));
            nodeInfo.setPackageName(jsonObject.getString("BundleName"));
            nodeInfo.setFocused(jsonObject.getBoolean("IsFocused"));
            nodeInfo.setAccessibilityFocused(jsonObject.getBoolean("HasAccessibilityFocus"));
            nodeInfo.setPassword(jsonObject.getBoolean("IsPassword"));
            nodeInfo.setEditable(jsonObject.getBoolean("IsEditable"));
            nodeInfo.setTextSelection(jsonObject.getInt("SelectedBegin"), jsonObject.getInt("SelectedEnd"));
            nodeInfo.setLiveRegion(jsonObject.getInt("LiveRegion"));
            nodeInfo.setMaxTextLength(jsonObject.getInt("TextLengthLimit"));
            nodeInfo.setVisibleToUser(jsonObject.getBoolean("IsVisible"));
            nodeInfo.setEnabled(jsonObject.getBoolean("IsEnabled"));
            nodeInfo.setClickable(jsonObject.getBoolean("IsClickable"));
            nodeInfo.setLongClickable(jsonObject.getBoolean("IsLongClickable"));
            nodeInfo.setScrollable(jsonObject.getBoolean("IsScrollable"));
            nodeInfo.setCheckable(jsonObject.getBoolean("IsCheckable"));
            nodeInfo.setChecked(jsonObject.getBoolean("IsChecked"));
            nodeInfo.setSelected(jsonObject.getBoolean("IsSelected"));
        } catch (JSONException e) {
            Log.e(TAG, "populateNodeInfoFromJson failed; err is " + e.getMessage());
        }
    }

    private boolean isChildOfComponentType(int nodeID, String componentType, String childType, String parentType) {
        if (!componentType.equals(childType)) {
            return false;
        }
        for (ArkUiAccessibilityNodeInfo value : arkUiframeNodes.values()) {
            if (!value.componentType.equals(parentType)) {
                continue;
            }
            for (int id : value.childIds) {
                if (id == nodeID) {
                    return true;
                }
            }
        }
        return false;
    }

    private String counterCovertContent(String content, int nodeID, String componentType) {
        if (!ACE_COMPONENT_TEXT.equals(componentType) || !"-".equals(content)) {
            return content;
        }

        for (ArkUiAccessibilityNodeInfo value : arkUiframeNodes.values()) {
            for (int id : value.childIds) {
                if (id == nodeID) {
                    return ACE_COMPONENT_BUTTON.equals(value.componentType) ? "减" : content;
                }
            }
        }

        return content;
    }

    private void setParentID(AccessibilityNodeInfo result, JSONObject jsonObject) throws JSONException {
        int parentID = jsonObject.getInt("ParentNodeId");
        if (parentID != INVALID_PARENT_ID) {
            result.setParent(arkuiRootAccessibilityView, parentID);
        } else {
            result.setParent(arkuiRootAccessibilityView);
        }
    }

    private void setChildren(AccessibilityNodeInfo result, JSONObject jsonObject, boolean noSupport)
            throws JSONException {
        String componentType = jsonObject.getString(KEY_COMPONENT_TYPE);
        if (ACE_COMPONENT_TEXTINPUT.equals(componentType)
                || ACE_COMPONENT_BUTTON.equals(componentType) || noSupport) {
            return;
        }
        JSONArray jsonChildIDArray = new JSONArray(jsonObject.getString("childIDs"));
        for (int index = 0; index < jsonChildIDArray.length(); index++) {
            result.addChild(arkuiRootAccessibilityView, jsonChildIDArray.getInt(index));
        }
    }

    private void setBounds(AccessibilityNodeInfo result, JSONObject jsonObject) throws JSONException {
        final Rect boundsInScreen = new Rect();
        boundsInScreen.left = jsonObject.getInt("RectInScreen_leftTopX");
        boundsInScreen.top = jsonObject.getInt("RectInScreen_leftTopY");
        boundsInScreen.right = jsonObject.getInt("RectInScreen_rightBottomX");
        boundsInScreen.bottom = jsonObject.getInt("RectInScreen_rightBottomY");
        final Rect bounds = getBoundsInScreen(boundsInScreen);
        result.setBoundsInScreen(bounds);
    }

    private void setCollection(AccessibilityNodeInfo result, JSONObject jsonObject) throws JSONException {
        if (jsonObject.getBoolean("IsScrollable")) {
            if (jsonObject.getInt("GirdInfoRows") > 0 || jsonObject.getInt("GirdInfoColumns") > 0) {
                result.setCollectionInfo(AccessibilityNodeInfo.CollectionInfo.obtain(
                        jsonObject.getInt("GirdInfoRows"),
                        jsonObject.getInt("GirdInfoColumns"),
                        false));
            } else {
                result.setCollectionInfo(
                        AccessibilityNodeInfo.CollectionInfo.obtain(
                                jsonObject.getInt("ItemCounts"),
                                0,
                                false));
            }
        }
        if (ACE_COMPONENT_GRIDITEM.equals(jsonObject.getString(KEY_COMPONENT_TYPE))) {
            result.setCollectionItemInfo(AccessibilityNodeInfo.CollectionItemInfo.obtain(
                    jsonObject.getInt("GridItemRowIndex"),
                    jsonObject.getInt("GridItemRowSpan"),
                    jsonObject.getInt("GridItemColumnIndex"),
                    jsonObject.getInt("GridItemColumnSpan"),
                    jsonObject.getBoolean("GridItemIsHeading"),
                    jsonObject.getBoolean("GridItemIsSelected")));
        }
    }

    private void setMovementGranularities(AccessibilityNodeInfo result, JSONObject jsonObject) throws JSONException {
        int granularities = 0;
        JSONArray jsonArray = new JSONArray(jsonObject.getString("ActionList"));
        int textStep = jsonObject.getInt("TextMoveStep");
        for (int index = 0; index < jsonArray.length(); index++) {
            int actionTemp = jsonArray.getInt(index);
            AccessibilityNodeInfo.AccessibilityAction customAction = new AccessibilityNodeInfo.AccessibilityAction(
                    actionTemp, "");
            result.addAction(customAction);
            switch (actionTemp) {
                case AccessibilityNodeInfo.ACTION_NEXT_AT_MOVEMENT_GRANULARITY:
                case AccessibilityNodeInfo.ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY:
                    if (textStep == TextMoveUnit.STEP_CHARACTER) {
                        granularities |= AccessibilityNodeInfo.MOVEMENT_GRANULARITY_CHARACTER;
                    } else if (textStep == TextMoveUnit.STEP_WORD) {
                        granularities |= AccessibilityNodeInfo.MOVEMENT_GRANULARITY_WORD;
                    } else if (textStep == TextMoveUnit.STEP_LINE) {
                        granularities |= AccessibilityNodeInfo.MOVEMENT_GRANULARITY_LINE;
                    } else if (textStep == TextMoveUnit.STEP_PARAGRAPH) {
                        granularities |= AccessibilityNodeInfo.MOVEMENT_GRANULARITY_PARAGRAPH;
                    } else if (textStep == TextMoveUnit.STEP_PAGE) {
                        granularities |= AccessibilityNodeInfo.MOVEMENT_GRANULARITY_PAGE;
                    }
                    break;
                default:
                    break;
            }
        }
        result.setMovementGranularities(granularities);
    }

    private boolean setTooltipTextAndText(AccessibilityNodeInfo result, JSONObject jsonObject) throws JSONException {
        if (Build.VERSION.SDK_INT > ANDROID_API_28) {
            try {
                Class<?> a11yNodeInfoClazz = result.getClass();
                Method setTooltipText = a11yNodeInfoClazz.getMethod("setTooltipText", CharSequence.class);
                setTooltipText.invoke(result, "");
                Method setHeading = a11yNodeInfoClazz.getMethod("setHeading", boolean.class);
                setHeading.invoke(result, jsonObject.getBoolean("GridItemIsHeading"));
            } catch (NoSuchMethodException e) {
                Log.e(TAG, "createAccessibilityNodeInfo failed, NoSuchMethodException.");
                return false;
            } catch (IllegalAccessException e) {
                Log.e(TAG, "createAccessibilityNodeInfo failed, IllegalAccessException.");
                return false;
            } catch (InvocationTargetException e) {
                Log.e(TAG, "createAccessibilityNodeInfo failed, InvocationTargetException.");
                return false;
            }
        }
        return true;
    }

    @Override
    public AccessibilityNodeInfo createAccessibilityNodeInfo(int virtualViewId) {
        return createAccessibilityNodeInfo(virtualViewId, false);
    }

    private String getClassNameString(String aceComponentType) {
        String androidClassName = "android.view.View";
        switch (aceComponentType) {
            case ACE_COMPONENT_TEXT:
                androidClassName = "android.widget.TextView";
                break;
            case ACE_COMPONENT_TEXTINPUT:
            case ACE_COMPONENT_TEXTAREA:
                androidClassName = "android.widget.EditText";
                break;
            case ACE_COMPONENT_ROW:
            case ACE_COMPONENT_COLUMN:
                androidClassName = "android.widget.ViewGroup";
                break;
            case ACE_COMPONENT_POPUP:
                androidClassName = "android.widget.PopupWindow";
                break;
            case ACE_COMPONENT_BUTTON:
            case ACE_COMPONENT_LEFTARROW:
            case ACE_COMPONENT_RIGHTARROW:
                androidClassName = "android.widget.Button";
                break;
            case ACE_COMPONENT_IMAGE:
            case ACE_COMPONENT_IMAGEVIEW:
                androidClassName = "android.widget.ImageView";
                break;
            case ACE_COMPONENT_VIDEO:
                androidClassName = "android.widget.VideoView";
                break;
            case ACE_COMPONENT_GRID:
                androidClassName = "android.widget.GridView";
                break;
            case ACE_COMPONENT_LIST:
                androidClassName = "android.widget.ListView";
                break;
            case ACE_COMPONENT_GRIDROW:
            case ACE_COMPONENT_GRIDCOL:
            case ACE_COMPONENT_SCROLL:
                androidClassName = "android.widget.ScrollView";
                break;
            case ACE_COMPONENT_TABS:
                androidClassName = "android.widget.TableLayout";
                break;
            case ACE_COMPONENT_SWIPER:
                androidClassName = "androidx.viewpager.widget.ViewPager";
                break;
            case ACE_COMPONENT_SWIPERINDICATOR:
                androidClassName = "com.android.internal.widget.PagerAdapter";
                break;
            case ACE_COMPONENT_RADIO:
            case ACE_COMPONENT_RADIOBUTTON:
                androidClassName = "android.widget.RadioButton";
                break;
            case ACE_COMPONENT_CHECKBOX:
                androidClassName = "android.widget.CheckBox";
                break;
            case ACE_COMPONENT_TOGGLE:
            case ACE_COMPONENT_SWITCH:
                androidClassName = "android.widget.Switch";
                break;
            default:
                break;
        }
        return androidClassName;
    }

    @Override
    public void addExtraDataToAccessibilityNodeInfo(int virtualViewId, AccessibilityNodeInfo info, String extraDataKey,
            Bundle arguments) {
        super.addExtraDataToAccessibilityNodeInfo(virtualViewId, info, extraDataKey, arguments);
    }

    @Override
    public boolean performAction(int virtualViewId, int action, Bundle arguments) {
        String jsonString = "{}";
        if (!isEnableFocus(virtualViewId, "Calendar")) {
            return true;
        }
        switch (action) {
            case AccessibilityNodeInfo.ACTION_CLEAR_ACCESSIBILITY_FOCUS:
                currentFocusNode = null;
                break;
            case AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS:
                if (isNoFocusable) {
                    isNoFocusable = false;
                    return false;
                }
                if (currentFocusNode != null && currentFocusNode.windowId != this.windowId) {
                    int[] args = {currentFocusNode.nodeId, AccessibilityNodeInfo.ACTION_CLEAR_ACCESSIBILITY_FOCUS,
                                    currentFocusNode.windowId};
                    nativePerformAction(args, jsonString);
                }
                boolean ret = performAction(virtualViewId, action, jsonString);
                currentFocusNode = FocusedNode.obtain(virtualViewId, this.windowId);
                return ret;
            case AccessibilityNodeInfo.ACTION_SCROLL_FORWARD:
                isNoFocusable = !isEnableFocus(virtualViewId, ACE_COMPONENT_GRID);
                break;
        }
        if (arguments != null) {
            try {
                JSONObject jsonObject = new JSONObject();
                if (arguments.containsKey(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_START_INT)) {
                    int selectTextBegin = arguments.getInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_START_INT);
                    jsonObject.put("selectTextBegin", selectTextBegin);
                }
                if (arguments.containsKey(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_END_INT)) {
                    int selectTextEnd = arguments.getInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_END_INT);
                    jsonObject.put("selectTextEnd", selectTextEnd);
                }
                if (arguments.containsKey(AccessibilityNodeInfo.ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE)) {
                    String setText = arguments.getString(AccessibilityNodeInfo.ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE);
                    jsonObject.put("setText", setText);
                }
                setMovementGranularityArg(arguments, jsonObject);
                setScrollAmountArg(arguments, jsonObject);
                jsonString = jsonObject.toString();
            } catch (JSONException e) {
                Log.e(TAG, "performAction failed; err is " + e.getMessage());
                return false;
            }
        }
        return performAction(virtualViewId, action, jsonString);
    }

    private void setMovementGranularityArg(Bundle arguments, JSONObject jsonObject) throws JSONException {
        if (arguments.containsKey(AccessibilityNodeInfo.ACTION_ARGUMENT_MOVEMENT_GRANULARITY_INT)) {
            int textMoveUnit = TextMoveUnit.STEP_INVALID;
            int movementGranularity = arguments
                    .getInt(AccessibilityNodeInfo.ACTION_ARGUMENT_MOVEMENT_GRANULARITY_INT);
            switch (movementGranularity) {
                case AccessibilityNodeInfo.MOVEMENT_GRANULARITY_CHARACTER:
                    textMoveUnit = TextMoveUnit.STEP_CHARACTER;
                    break;
                case AccessibilityNodeInfo.MOVEMENT_GRANULARITY_WORD:
                    textMoveUnit = TextMoveUnit.STEP_WORD;
                    break;
                case AccessibilityNodeInfo.MOVEMENT_GRANULARITY_LINE:
                    textMoveUnit = TextMoveUnit.STEP_LINE;
                    break;
                case AccessibilityNodeInfo.MOVEMENT_GRANULARITY_PARAGRAPH:
                    textMoveUnit = TextMoveUnit.STEP_PARAGRAPH;
                    break;
                case AccessibilityNodeInfo.MOVEMENT_GRANULARITY_PAGE:
                    textMoveUnit = TextMoveUnit.STEP_PAGE;
                    break;
                default:
                    break;
            }
            jsonObject.put("textMoveUnit", textMoveUnit);
        }
    }

    private void setScrollAmountArg(Bundle arguments, JSONObject jsonObject) throws JSONException {
        if (Build.VERSION.SDK_INT >= ANDROID_API_35) {
            final String ACTION_ARGUMENT_SCROLL_AMOUNT_FLOAT =
                "android.view.accessibility.action.ARGUMENT_SCROLL_AMOUNT_FLOAT";
            if (arguments.containsKey(ACTION_ARGUMENT_SCROLL_AMOUNT_FLOAT)) {
                float scrollFloat = arguments.getFloat(ACTION_ARGUMENT_SCROLL_AMOUNT_FLOAT);
                int scrolltype = ScrollType.SCROLL_DEFAULT;
                if (scrollFloat > 0.25f && scrollFloat < 0.75f) {
                    scrolltype = ScrollType.SCROLL_HALF;
                } else if (scrollFloat >= 0.75f) {
                    scrolltype = ScrollType.SCROLL_FULL;
                }
                jsonObject.put("scrolltype", scrolltype);
            }
        }
    }

    @Override
    public List<AccessibilityNodeInfo> findAccessibilityNodeInfosByText(String text, int virtualViewId) {
        return super.findAccessibilityNodeInfosByText(text, virtualViewId);
    }

    /**
     * Sends accessibility event.
     *
     * @param viewId the view id
     * @param eventType the event type
     */
    public void sendAccessibilityEvent(int viewId, int eventType) {
        if (!isEnabled()) {
            return;
        }
        if (viewId == ROOT_NODE_ID) {
            arkuiRootAccessibilityView.sendAccessibilityEvent(eventType);
        } else {
            sendAccessibilityEvent(obtainAccessibilityEvent(viewId, eventType));
        }
    }

    private AccessibilityEvent obtainAccessibilityEvent(int virtualViewId, int eventType) {
        AccessibilityEvent event = obtainAccessibilityEvent(eventType);
        event.setPackageName(arkuiRootAccessibilityView.getContext().getPackageName());
        event.setSource(arkuiRootAccessibilityView, virtualViewId);
        return event;
    }

    /**
     * Convert json to node info.
     *
     * @param eventType the event type
     * @return the accessibility node info
     */
    public AccessibilityEvent obtainAccessibilityEvent(int eventType) {
        return AccessibilityEvent.obtain(eventType);
    }

    private boolean sendAccessibilityEvent(AccessibilityEvent event) {
        if (!isEnabled()) {
            return false;
        }
        if (arkuiRootAccessibilityView.getParent() == null) {
            Log.e(TAG, "sendAccessibilityEvent getParent is null");
            accessibilityManager.sendAccessibilityEvent(event);
            return false;
        }

        return arkuiRootAccessibilityView.getParent()
                .requestSendAccessibilityEvent(arkuiRootAccessibilityView, event);
    }

    @Override
    public AccessibilityNodeInfo findFocus(int focus) {
        if (focus != AccessibilityNodeInfo.FOCUS_INPUT
                && focus != AccessibilityNodeInfo.FOCUS_ACCESSIBILITY) {
            return null;
        }

        String jsonStr = nativeFindFocusedElementInfo(focus, this.windowId);
        return convertJsonToNodeInfo(jsonStr);
    }

    /**
     * Send window content change event.
     *
     * @param virtualViewId the virtual view id
     */
    public void sendWindowContentChangeEvent(int virtualViewId) {
        AccessibilityEvent event = obtainAccessibilityEvent(virtualViewId,
                AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED);
        event.setContentChangeTypes(AccessibilityEvent.CONTENT_CHANGE_TYPE_SUBTREE);
        sendAccessibilityEvent(event);
    }

    /**
     * Send accessibility focus invalidate.
     *
     * @param virtualViewId the virtual view id
     */
    public void sendAccessibilityFocusInvalidate(int virtualViewId) {
        int invalidateId = INVALID_VIRTUAL_VIEW_ID;
        if (virtualViewId >= 0) {
            invalidateId = virtualViewId;
        } else {
            invalidateId = accessFocusNodeId;
        }
        if (invalidateId >= 0) {
            performAction(invalidateId, AccessibilityNodeInfo.ACTION_CLEAR_ACCESSIBILITY_FOCUS, new Bundle());
            performAction(invalidateId, AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, new Bundle());
        }
    }

    private int getChildItemCount(int parentId) {
        ArkUiAccessibilityNodeInfo parentNodeInfo = arkUiframeNodes.get(parentId);
        if (parentNodeInfo == null) {
            return 0;
        }
        int childCount = parentNodeInfo.childIds.length;
        for (int i = 0; i < parentNodeInfo.childIds.length; i++) {
            int childRescuCount = getChildItemCount(parentNodeInfo.childIds[i]);
            childCount += childRescuCount;
        }
        return childCount;
    }

    /**
     * Update all node info.
     *
     * @param nodeId the node id
     * @param accessibilityEvent the accessibility event
     * @param arg the arg
     * @return the boolean
     */
    public boolean onSendAccessibilityEvent(int nodeId, int accessibilityEvent, String arg) {
        AccessibilityEvent event = obtainAccessibilityEvent(nodeId, accessibilityEvent);
        String componentType = "";
        int pageID = -1;
        int parentId = -1;
        isNoFocusable = false;
        if (arg.isEmpty()) {
            return false;
        }
        try {
            JSONObject jsonObject = new JSONObject(arg);
            parentId = jsonObject.getInt("ParentNodeId");
            event.setSource(arkuiRootAccessibilityView, sliderPerformAction(nodeId, parentId));
            convertJsonToEvent(nodeId, event, jsonObject);
            componentType = jsonObject.getString(KEY_COMPONENT_TYPE);
            if (arkUiframeNodes.containsKey(nodeId)) {
                int chilIdCount = arkUiframeNodes.get(nodeId).childIds.length;
                if (arkUiframeNodes.get(nodeId).nodeInfo.isVisibleToUser() != jsonObject.getBoolean("IsVisible")) {
                    sendWindowContentChangeEvent(nodeId);
                }
                arkUiframeNodes.get(nodeId).isDirty = true;
                if (accessibilityEvent == AccessibilityEvent.TYPE_VIEW_SCROLLED && chilIdCount > 0) {
                    updateAllNodeInfo();
                    int childCount = getChildItemCount(nodeId) + 1;
                    int index = currentPageNodeIds.indexOf(nodeId);
                    int indexEnd = childCount + index;
                    if (indexEnd > currentPageNodeIds.size()) {
                        indexEnd = currentPageNodeIds.size();
                    }
                    for (int i = index; i < indexEnd; i++) {
                        arkUiframeNodes.get(currentPageNodeIds.get(i)).isDirty = true;
                    }
                    sendWindowContentChangeEvent(nodeId);
                }
            }
            pageID = jsonObject.getInt("PageId");
        } catch (JSONException e) {
            Log.e(TAG, "onSendAccessibilityEvent failed; err is " + e.getMessage());
            return false;
        }
        boolean[] isDrop = {false};
        performEvent(nodeId, accessibilityEvent, componentType, pageID, isDrop);
        if (!isDrop[0]) {
            isDrop[0] = performFocusAction(nodeId, parentId, accessibilityEvent);
        }
        if (accessibilityEvent == TYPE_PAGE_OPEN || accessibilityEvent == TYPE_PAGE_CLOSE) {
            event.setEventType(AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED);
        }
        return isDrop[0] ? true : sendAccessibilityEvent(event);
    }

    private void convertJsonToEvent(int nodeId, AccessibilityEvent event, JSONObject jsonObject) {
        try {
            event.setAction(jsonObject.getInt("actionType"));
            event.setContentChangeTypes(jsonObject.getInt("contentChangeType"));
            event.setMovementGranularity(jsonObject.getInt("TextMoveStep"));
            event.setAddedCount(jsonObject.getInt("InputType"));
            event.setBeforeText(jsonObject.getString("beforeText"));
            event.setChecked(jsonObject.getBoolean("IsChecked"));
            event.setClassName(getClassNameString(jsonObject.getString(KEY_COMPONENT_TYPE)));
            int parentId = jsonObject.getInt("ParentNodeId");
            event.setSource(arkuiRootAccessibilityView, sliderPerformAction(nodeId, parentId));
            event.setContentDescription(jsonObject.getString(KEY_DESCRIPTION_INFO).trim());
            event.setEnabled(jsonObject.getBoolean("IsEnabled"));
            event.setFromIndex(jsonObject.getInt("currentIndex"));
            event.setItemCount(jsonObject.getInt("itemCounts"));
            event.setCurrentItemIndex(jsonObject.getInt("currentIndex"));
            event.setPassword(jsonObject.getBoolean("IsPassword"));
            event.setScrollable(jsonObject.getBoolean("IsScrollable"));
        } catch (JSONException e) {
            Log.e(TAG, "ConvertJsonToEvent failed; err is " + e.getMessage());
        }
    }

    private void performEvent(int nodeId, int accessibilityEvent, String componentType, int pageID, boolean[] isDrop) {
        if (accessibilityEvent == AccessibilityEvent.TYPE_VIEW_FOCUSED) {
            performUpdateNodeId(nodeId, accessibilityEvent, componentType, false, isDrop);
        }
        if (accessibilityEvent == AccessibilityEvent.TYPE_VIEW_ACCESSIBILITY_FOCUSED) {
            accessFocusNodeId = nodeId;
            addAccFocusRoute(nodeId, pageID);
        }
        if (accessibilityEvent == AccessibilityEvent.TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED) {
            accessFocusNodeId = -1;
        }
        if (accessibilityEvent == AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED
                || accessibilityEvent == AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED) {
            inputFocusNodeId = -1;
            accessFocusNodeId = -1;
            SwiperChldId.clear();
            performUpdateNodeId(nodeId, accessibilityEvent, componentType, true, isDrop);
        }

        if (accessibilityEvent == TYPE_PAGE_OPEN) {
            inputFocusNodeId = -1;
            accessFocusNodeId = -1;
            SwiperChldId.clear();
            performUpdateNodeId(nodeId, accessibilityEvent, componentType, false, isDrop);
        }

        if (accessibilityEvent == TYPE_PAGE_CLOSE) {
            if (accessFocusNodeId >= 0) {
                performAction(accessFocusNodeId,
                        AccessibilityNodeInfo.ACTION_CLEAR_ACCESSIBILITY_FOCUS, new Bundle());
            }
            inputFocusNodeId = -1;
            accessFocusNodeId = -1;
            SwiperChldId.clear();
            performUpdateNodeId(nodeId, accessibilityEvent, componentType, false, isDrop);
        }
        if (accessibilityEvent == AccessibilityEvent.TYPE_VIEW_SCROLLED) {
            performUpdateNodeId(nodeId, accessibilityEvent, componentType, false, isDrop);
        }
        if (accessibilityEvent == AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED) {
            isDrop[0] = true;
        }
    }

    private void performUpdateNodeId(int nodeId, int eventType, String componentType, boolean isDelay,
            boolean[] isDrop) {
        if (arkuiRootAccessibilityView == null) {
            return;
        }
        if (ACE_COMPONENT_PAGE.equals(componentType) ||
                ACE_COMPONENT_POPUP.equals(componentType) ||
                ACE_COMPONENT_DIALOG.equals(componentType) ||
                ACE_COMPONENT_SCROLL.equals(componentType) ||
                ACE_COMPONENT_NAVIGATION.equals(componentType)) {
            arkuiRootAccessibilityView.removeCallbacks(updateNodeIds);
            isMenuFocus = false;
            inputFocusNodeId = nodeId;
            if (isDelay) {
                arkuiRootAccessibilityView.postDelayed(updateNodeIds, EVENT_DELAY_TIME);
            } else {
                arkuiRootAccessibilityView.post(updateNodeIds);
            }
        } else if (ACE_COMPONENT_MENU.equals(componentType)) {
            arkuiRootAccessibilityView.removeCallbacks(updateNodeIds);
            if (eventType == TYPE_PAGE_CLOSE) {
                isMenuFocus = false;
            } else {
                isMenuFocus = true;
                isDrop[0] = true;
            }
            inputFocusNodeId = nodeId;
            if (isDelay) {
                arkuiRootAccessibilityView.postDelayed(updateNodeIds, EVENT_DELAY_TIME);
            } else {
                arkuiRootAccessibilityView.post(updateNodeIds);
            }
        }
    }

    /**
     * Perform action.
     *
     * @param virtualViewId the virtual view id
     * @param accessibilityAction the accessibility action
     * @param bundleString the bundle string
     * @return the boolean
     */
    public boolean performAction(int virtualViewId, int accessibilityAction, String bundleString) {
        int[] args = {virtualViewId, accessibilityAction, this.windowId};
        return nativePerformAction(args, bundleString);
    }

    /**
     * Perform action.
     *
     * @return the boolean
     */
    public boolean isEnabled() {
        return (isAccessibilityEnabled() && isTouchExplorationEnabled()) || disabledDelay;
    }

    /**
     * Register js accessibility state observer.
     *
     * @param objectPtr the object ptr
     * @return the boolean
     */
    public boolean registerJsAccessibilityStateObserver(long objectPtr) {
        if (jsAccessibilityStateObserverPtr == 0L) {
            jsAccessibilityStateObserverPtr = objectPtr;
            return true;
        } else if (jsAccessibilityStateObserverPtr == objectPtr) {
            ALog.i(TAG, "registerObserver successed. Ptr already existed. Ptr = "
                    + objectPtr + " windowId = " + this.windowId);
            return true;
        }
        ALog.e(TAG, "registerObserver failed. Ptr=" + objectPtr + " windowId = " + this.windowId);
        return false;
    }

    /**
     * Unregister js accessibility state observer.
     *
     * @return the boolean
     */
    public boolean unregisterJsAccessibilityStateObserver() {
        if (jsAccessibilityStateObserverPtr != 0L) {
            jsAccessibilityStateObserverPtr = 0L;
            return true;
        }
        ALog.e(TAG, "unregisterObserver called. Ptr not exists." + " windowId = " + this.windowId);
        return false;
    }

    /**
     * Accessibility state changed.
     *
     * @param accessibilityEnabled the accessibility enabled
     */
    public void accessibilityStateChanged(boolean accessibilityEnabled) {
        if (jsAccessibilityStateObserverPtr != 0L) {
            nativeAccessibilityStateChanged(accessibilityEnabled, jsAccessibilityStateObserverPtr);
        }
    }

    private void addAccFocusRoute(int nodeID, int pageID) {
        if (nodeID < 0) {
            return;
        }
        updateSwiperChildID();
        boolean isFind = false;
        for (FocusedNode focusedNode : arkUiAccFocusRouteNodes) {
            if (focusedNode.nodeId == nodeID) {
                isFind = true;
                break;
            }
        }

        if (!isFind) {
            arkUiAccFocusRouteNodes.add(FocusedNode.obtain(nodeID, this.windowId, pageID));
        }
    }

    private boolean isSwiperChildFocused(int nodeId) {
        if (SwiperChldId.containsKey(nodeId)) {
            return SwiperChldId.get(nodeId).contains(accessFocusNodeId);
        }
        return false;
    }

    private boolean performFocusAction(int nodeId, int parentId, int event) {
        boolean isPerformFocusAction = false;
        if (event == AccessibilityEvent.TYPE_VIEW_SCROLLED) {
            updateSwiperChildID();
            boolean isChildFocused = isSwiperChildFocused(nodeId);
            if (isChildFocused) {
                ArkUiAccessibilityNodeInfo nodeInfo = arkUiframeNodes.get(nodeId);
                if (nodeInfo != null && nodeInfo.childIds.length > 0) {
                    performAction(nodeInfo.childIds[0], AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS,
                            new Bundle());
                    isPerformFocusAction = true;
                }
            }
        }
        return isPerformFocusAction;
    }

    private int sliderPerformAction(int nodeId, int parentId) {
        ArkUiAccessibilityNodeInfo nodeInfo = arkUiframeNodes.get(nodeId);
        if (nodeInfo != null && ACE_COMPONENT_SLIDER.equals(nodeInfo.componentType)) {
            return nodeId;
        }
        for (ArkUiAccessibilityNodeInfo value : arkUiframeNodes.values()) {
            if (!ACE_COMPONENT_SLIDER.equals(value.componentType)) {
                continue;
            }
            for (int id : value.childIds) {
                if (id == parentId) {
                    return value.nodeId;
                }
            }
        }
        return nodeId;
    }

    private void updateAllNodeInfo() {
        int[] idsArray = nativeGetTreeIdArray(this.windowId);
        currentPageNodeIds.clear();
        currentPageNodeIds.addAll(Arrays.stream(idsArray).boxed().collect(Collectors.toList()));
        arkUiframeNodes.clear();
        for (int i = 0; i < currentPageNodeIds.size(); i++) {
            if (!arkUiframeNodes.containsKey(currentPageNodeIds.get(i))) {
                createAccessibilityNodeInfo(currentPageNodeIds.get(i), true);
            }
        }
    }

    private void updateSwiperChildID() {
        for (ArkUiAccessibilityNodeInfo value : arkUiframeNodes.values()) {
            if (!ACE_COMPONENT_SWIPER.equals(value.componentType)) {
                continue;
            }
            if (value.childIds.length != 0 && value.childIds[0] == accessFocusNodeId) {
                if (!SwiperChldId.containsKey(value.nodeId)) {
                    SwiperChldId.put(value.nodeId, new HashSet<>());
                }
                SwiperChldId.get(value.nodeId).add(accessFocusNodeId);
                break;
            }
        }
    }

    private boolean isEnableFocus(int nodeID, String componentType) {
        ArkUiAccessibilityNodeInfo arkNodeInfo = arkUiframeNodes.get(nodeID);
        if (arkNodeInfo == null) {
            return true;
        }
        return !arkNodeInfo.componentType.equals(componentType);
    }

    private int getAccFocusRouteAndReset() {
        int ret = -1;
        FocusedNode routeFocusedNode = null;
        int routeFocusedIndex = -1;
        for (int index = arkUiAccFocusRouteNodes.size() - 1; index >= 0; index--) {
            FocusedNode currentNode = arkUiAccFocusRouteNodes.get(index);
            if (arkUiframeNodes.containsKey(currentNode.nodeId)) {
                routeFocusedNode = currentNode;
                routeFocusedIndex = index;
                if (arkUiframeNodes.get(routeFocusedNode.nodeId).nodeInfo.isFocusable()) {
                    ret = routeFocusedNode.nodeId;
                }
                break;
            }
        }

        if (routeFocusedIndex > -1) {
            int routeStartIndex = routeFocusedIndex - 1;
            while (routeStartIndex >= 0) {
                if (arkUiAccFocusRouteNodes.get(routeStartIndex).pageId != routeFocusedNode.pageId) {
                    break;
                }
                routeStartIndex--;
            }
            arkUiAccFocusRouteNodes.subList(routeStartIndex + 1, arkUiAccFocusRouteNodes.size()).clear();
            arkUiAccFocusRouteNodes.add(routeFocusedNode);
        }

        return ret;
    }

    private native boolean nativePerformAction(int[] args, String bundleString);
    private native void nativeSetupJsAccessibilityManager(int windowId);
    private native String nativeCreateAccessibilityNodeInfo(int nodeId, int windowId);
    private native void nativeAccessibilityStateChanged(
            boolean accessibilityEnabled, long jsAccessibilityStateObserver);
    private native String nativeFindFocusedElementInfo(int focusType, int windowId);
    private native int[] nativeGetTreeIdArray(int windowId);
    private native void nativeTouchExplorationStateChange(boolean touchExplorationStateChange, int windowId);
    private native int nativeGetRootElementId(int windowId);
    private native boolean nativeRelease(int windowId);
}