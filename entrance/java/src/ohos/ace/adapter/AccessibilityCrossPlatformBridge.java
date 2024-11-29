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
import android.util.ArrayMap;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityManager;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.accessibility.AccessibilityNodeProvider;

import java.lang.Class;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import java.util.Arrays;
import java.util.stream.Collectors;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONArray;

public class AccessibilityCrossPlatformBridge extends AccessibilityNodeProvider {
    private static final String TAG = "ArkUIAccessbilityProvider";
    private static FocusedNode currentFocusNode = null;
    private final AccessibilityManager accessibilityManager;
    private final ContentResolver contentResolver;
    private View arkuiRootAccessibilityView = null;
    private static final int ROOT_NODE_ID = 0;
    private static final int UNDEFINED_ACCESSIBILITY_ID = -1;
    private static final int INVALID_PARENT_ID = -2100000;
    private long jsAccessibilityStateObserverPtr = 0L;
    private int windowId = 0;
    private List<Integer> currentPageNodeIds = new ArrayList<>();
    private ArrayMap<Integer, ArkUiAccessibilityNodeInfo> arkUiframeNodes = new ArrayMap<>();
    private int inputFocusNodeId = -1;
    private int accessFocusNodeId = -1;
    private List<FocusedNode> arkUiAccFocusRouteNodes = new ArrayList<>();
	private boolean isMenuFocus = false;
    private boolean isStateChanged = true;
    private boolean disabledDelay = false;
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
    private class TextMoveUnit {
        public static final int STEP_INVALID = 0x0000;
        public static final int STEP_CHARACTER = 0x0001;
        public static final int STEP_WORD = 0x0002;
        public static final int STEP_LINE = 0x0004;
        public static final int STEP_PARAGRAPH = 0x0008;
        public static final int STEP_PAGE = 0x0010;
    }

    private class ScrollType {
        public static final int SCROLL_DEFAULT = -1;
        public static final int SCROLL_HALF = 0;
        public static final int SCROLL_FULL = 1;
    }

    private static class FocusedNode {
        public int nodeId = -1;
        public int windowId = 0;
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

        public static FocusedNode obtain(int nodeId, int windowId) {
            return new FocusedNode(nodeId, windowId);
        }

        public static FocusedNode obtain(int nodeId, int windowId, int pageId) {
            return new FocusedNode(nodeId, windowId, pageId);
        }
    }

    private static class ArkUiAccessibilityNodeInfo {
        public int nodeId = -1;
        public String componentResourceId = "";
        public String bundleName = "";
        public String componentType = "";
        public boolean isFocusable;
        public boolean isFocused;
        public boolean hasAccessibilityFocus;
        public boolean isPassword;
        public boolean isEditable;
        public int selectedBegin;
        public int selectedEnd;
        public int liveRegion;
        public int[] actionList = new int[0];
        public int textMoveStep;
        public int textLengthLimit;
        public int parentNodeId;
        public Rect boundsInScreen;
        public boolean isVisible;
        public boolean isEnabled;
        public boolean isClickable;
        public boolean isLongClickable;
        public boolean isScrollable;
        public int girdInfoRows;
        public int girdInfoColumns;
        public int itemCounts;
        public int gridItemRowIndex;
        public boolean gridItemIsSelected;
        public int gridItemRowSpan;
        public int gridItemColumnIndex;
        public int gridItemColumnSpan;
        public boolean gridItemIsHeading;
        public String content = "";
        public String hint = "";
        public String descriptionInfo = "";
        public String setTooltipText = "";
        public boolean isCheckable;
        public boolean isChecked;
        public boolean isSelected;
        public int[] childIds = new int[0];

        private ArkUiAccessibilityNodeInfo(int nodeId) {
            this.nodeId = nodeId;
        }

        public static ArkUiAccessibilityNodeInfo obtain(int nodeId) {
            return new ArkUiAccessibilityNodeInfo(nodeId);
        }

        public void init(String jsonStr) {
            if (!jsonStr.isEmpty()) {
                try {
                    JSONObject jsonObject = new JSONObject(jsonStr);
                    this.componentResourceId = jsonObject.getString("ComponentResourceId");
                    this.bundleName = jsonObject.getString("BundleName");
                    this.componentType = jsonObject.getString("ComponentType");
                    this.isFocusable = jsonObject.getBoolean("IsFocusable");
                    this.isFocused = jsonObject.getBoolean("IsFocused");
                    this.hasAccessibilityFocus = jsonObject.getBoolean("HasAccessibilityFocus");
                    this.isPassword = jsonObject.getBoolean("IsPassword");
                    this.isEditable = jsonObject.getBoolean("IsEditable");
                    this.selectedBegin = jsonObject.getInt("SelectedBegin");
                    this.selectedEnd = jsonObject.getInt("SelectedEnd");
                    this.liveRegion = jsonObject.getInt("LiveRegion");
                    this.textMoveStep = jsonObject.getInt("TextMoveStep");
                    JSONArray jsonArray = new JSONArray(jsonObject.getString("ActionList"));
                    if (jsonArray.length() > 0) {
                        this.actionList = new int[jsonArray.length()];
                        for (int index = 0; index < jsonArray.length(); index++) {
                            actionList[index] = jsonArray.getInt(index);
                        }
                    }
                    this.textLengthLimit = jsonObject.getInt("TextLengthLimit");
                    this.parentNodeId = jsonObject.getInt("ParentNodeId");
                    this.boundsInScreen = new Rect();
                    this.boundsInScreen.left = jsonObject.getInt("RectInScreen_leftTopX");
                    this.boundsInScreen.top = jsonObject.getInt("RectInScreen_leftTopY");
                    this.boundsInScreen.right = jsonObject.getInt("RectInScreen_rightBottomX");
                    this.boundsInScreen.bottom = jsonObject.getInt("RectInScreen_rightBottomY");
                    this.isVisible = jsonObject.getBoolean("IsVisible");
                    this.isEnabled = jsonObject.getBoolean("IsEnabled");
                    this.isClickable = jsonObject.getBoolean("IsClickable");
                    this.isLongClickable = jsonObject.getBoolean("IsLongClickable");
                    this.isScrollable = jsonObject.getBoolean("IsScrollable");
                    this.girdInfoRows = jsonObject.getInt("GirdInfoRows");
                    this.girdInfoColumns = jsonObject.getInt("GirdInfoColumns");
                    this.itemCounts = jsonObject.getInt("ItemCounts");
                    this.gridItemRowIndex = jsonObject.getInt("GridItemRowIndex");
                    this.gridItemRowSpan = jsonObject.getInt("GridItemRowSpan");
                    this.gridItemColumnIndex = jsonObject.getInt("GridItemColumnIndex");
                    this.gridItemColumnSpan = jsonObject.getInt("GridItemColumnSpan");
                    this.gridItemIsHeading = jsonObject.getBoolean("GridItemIsHeading");
                    this.gridItemIsSelected = jsonObject.getBoolean("GridItemIsSelected");
                    this.content = jsonObject.getString("Content");
                    this.hint = jsonObject.getString("Hint");
                    this.descriptionInfo = jsonObject.getString("DescriptionInfo");
                    this.isCheckable = jsonObject.getBoolean("IsCheckable");
                    this.isChecked = jsonObject.getBoolean("IsChecked");
                    this.isSelected = jsonObject.getBoolean("IsSelected");
                    JSONArray jsonChildIDArray = new JSONArray(jsonObject.getString("childIDs"));
                    if (jsonChildIDArray.length() > 0) {
                        this.childIds = new int[jsonChildIDArray.length()];
                        for (int index = 0; index < jsonChildIDArray.length(); index++) {
                            childIds[index] = jsonChildIDArray.getInt(index);
                        }
                    }
                } catch (JSONException e) {
                    Log.e(TAG, "createAccessibilityNodeInfo failed; err is ", e);
                    return;
                }
            }
        }
    }

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

    private final AccessibilityManager.AccessibilityStateChangeListener stateChangeListener = (isEnable) -> {
        onChanged(isEnable);
        isStateChanged = false;
    };

    private final AccessibilityManager.TouchExplorationStateChangeListener
        touchExplorationStateChangeListener = (isEnable) -> {
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

    public AccessibilityCrossPlatformBridge(AccessibilityManager accessibilityManager, ContentResolver contentResolver,
            View arkuiRootAccessibilityView, int windowId) {
        this.accessibilityManager = accessibilityManager;
        this.contentResolver = contentResolver;
        this.arkuiRootAccessibilityView = arkuiRootAccessibilityView;
        this.accessibilityManager.addTouchExplorationStateChangeListener(touchExplorationStateChangeListener);
        this.accessibilityManager.addAccessibilityStateChangeListener(stateChangeListener);
        this.windowId = windowId;
        nativeSetupJsAccessibilityManager(this.windowId);
    }

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
    }

    public boolean isAccessibilityEnabled() {
        if (accessibilityManager != null) {
            return accessibilityManager.isEnabled();
        }

        return false;
    }

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

        if (childCount > 0) {
            Iterator it = arkUiframeNodes.keySet().iterator();
            boolean isFind = false;
            while (it.hasNext()) {
                int key = (int) it.next();
                if (!isFind && key != parentId) {
                    continue;
                } else {
                    isFind = true;
                }
                ArkUiAccessibilityNodeInfo temp = arkUiframeNodes.get(key);
                if (!temp.content.isEmpty() || !temp.descriptionInfo.isEmpty()) {
                    childInfo = temp;
                    break;
                }
            }
        }
        return childInfo;
    }

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

        if (performPopup()) {
            return;
        }

        if (!isMenuFocus) {
            int lastFocusRoute = getAccFocusRouteAndReset();
            if (lastFocusRoute > -1) {
                performAction(lastFocusRoute, AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, new Bundle());
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

        if (!arkUiframeNodes.values().stream().anyMatch(info -> info.hasAccessibilityFocus)) {
            arkUiframeNodes.entrySet().stream().filter(entry -> !entry.getValue().content.isEmpty() 
                || !entry.getValue().descriptionInfo.isEmpty())
                .findFirst()
                .ifPresent(entry -> performAction(entry.getKey(), AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS,
                 new Bundle()));
        }
    }

    private boolean performPopup() {
        if (isMenuFocus) {
            return false;
        }
        
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
        if (virtualViewId == View.NO_ID) {
            result = AccessibilityNodeInfo.obtain(arkuiRootAccessibilityView);
            arkuiRootAccessibilityView.onInitializeAccessibilityNodeInfo(result);
            result.setImportantForAccessibility(true);
            int rootId = nativeGetRootElementId(this.windowId);
            result.addChild(arkuiRootAccessibilityView, rootId);
            if (Build.VERSION.SDK_INT >= ANDROID_API_24) {
                result.setImportantForAccessibility(false);
            }
            if (isNew) {
                arkUiframeNodes.put(rootId, ArkUiAccessibilityNodeInfo.obtain(rootId));
            }
            return result;
        }

        String jsonStr = nativeCreateAccessibilityNodeInfo(virtualViewId, this.windowId);
        if (isNew) {
            ArkUiAccessibilityNodeInfo arkuiInfo = ArkUiAccessibilityNodeInfo.obtain(virtualViewId);
            arkuiInfo.init(jsonStr);
            arkUiframeNodes.put(virtualViewId, arkuiInfo);
            arkUiframeNodes.setValueAt(currentPageNodeIds.indexOf(virtualViewId), arkuiInfo);
            return result;
        }

        return ConvertJsonToNodeInfo(jsonStr);
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

    AccessibilityNodeInfo ConvertJsonToNodeInfo(String jsonStr) {
        AccessibilityNodeInfo result = null;
        if (!jsonStr.isEmpty()) {
            try {
                JSONObject jsonObject = new JSONObject(jsonStr);
                result = AccessibilityNodeInfo.obtain(arkuiRootAccessibilityView,
                        jsonObject.getInt("AccessibilityId"));
                result.setImportantForAccessibility(jsonObject.getBoolean("ImportantForAccessibility"));
                result.setViewIdResourceName(jsonObject.getString("ComponentResourceId"));
                result.setPackageName(jsonObject.getString("BundleName"));
                result.setClassName(getClassNameString(jsonObject.getString("ComponentType")));
                result.setFocusable(jsonObject.getBoolean("IsFocusable"));
                result.setFocused(jsonObject.getBoolean("IsFocused"));
                result.setAccessibilityFocused(jsonObject.getBoolean("HasAccessibilityFocus"));
                result.setPassword(jsonObject.getBoolean("IsPassword"));
                result.setEditable(jsonObject.getBoolean("IsEditable"));
                result.setTextSelection(jsonObject.getInt("SelectedBegin"), jsonObject.getInt("SelectedEnd"));
                result.setLiveRegion(jsonObject.getInt("LiveRegion"));
                setMovementGranularities(result, jsonObject);
                result.setMaxTextLength(jsonObject.getInt("TextLengthLimit"));
                setParentID(result, jsonObject);
                setBounds(result, jsonObject);
                result.setVisibleToUser(jsonObject.getBoolean("IsVisible"));
                result.setEnabled(jsonObject.getBoolean("IsEnabled"));
                result.setClickable(jsonObject.getBoolean("IsClickable"));
                result.setLongClickable(jsonObject.getBoolean("IsLongClickable"));
                result.setScrollable(jsonObject.getBoolean("IsScrollable"));
                setCollection(result, jsonObject);
                result.setText(jsonObject.getString("Content"));
                result.setHintText(jsonObject.getString("Hint"));
                result.setContentDescription(jsonObject.getString("DescriptionInfo"));
                if (!setTooltipTextAndText(result, jsonObject)) {
                    return null;
                }
                result.setCheckable(jsonObject.getBoolean("IsCheckable"));
                result.setChecked(jsonObject.getBoolean("IsChecked"));
                result.setSelected(jsonObject.getBoolean("IsSelected"));
                setChildren(result, jsonObject);
            } catch (JSONException e) {
                Log.e(TAG, "createAccessibilityNodeInfo failed; err is ", e);
                return null;
            }
        }
        return result;
    }

    private void setParentID(AccessibilityNodeInfo result, JSONObject jsonObject) throws JSONException {
        int parentID = jsonObject.getInt("ParentNodeId");
        if (parentID != INVALID_PARENT_ID) {
            result.setParent(arkuiRootAccessibilityView, parentID);
        } else {
            result.setParent(arkuiRootAccessibilityView);
        }
    }

    private void setChildren(AccessibilityNodeInfo result, JSONObject jsonObject) throws JSONException {
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
        if (ACE_COMPONENT_GRIDITEM.equals(jsonObject.getString("ComponentType"))) {
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
                    actionTemp, "ace");
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
        switch (action) {
            case AccessibilityNodeInfo.ACTION_CLEAR_ACCESSIBILITY_FOCUS:
                currentFocusNode = null;
                break;
            case AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS:
                if (null != currentFocusNode && currentFocusNode.windowId != this.windowId) {
                    int[] args = { currentFocusNode.nodeId, AccessibilityNodeInfo.ACTION_CLEAR_ACCESSIBILITY_FOCUS,
                            currentFocusNode.windowId };
                    nativePerformAction(args, jsonString);
                }
                boolean ret = performAction(virtualViewId, action, jsonString);
                currentFocusNode = FocusedNode.obtain(virtualViewId, this.windowId);
                return ret;
            default:
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
                Log.e(TAG, "performAction failed; err is ", e);
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
        return ConvertJsonToNodeInfo(jsonStr);
    }

    public void sendWindowContentChangeEvent(int virtualViewId) {
        AccessibilityEvent event = obtainAccessibilityEvent(virtualViewId,
                AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED);
        event.setContentChangeTypes(AccessibilityEvent.CONTENT_CHANGE_TYPE_SUBTREE);
        sendAccessibilityEvent(event);
    }

    public void sendAccessibilityFocusInvalidate(int virtualViewId) {
        int invalidateId = -1;
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

    public boolean onSendAccessibilityEvent(int nodeId, int accessibilityEvent, String arg) {
        AccessibilityEvent event = obtainAccessibilityEvent(nodeId, accessibilityEvent);
        String componentType = "";
        int pageID = -1;
        if (!arg.isEmpty()) {
            try {
                JSONObject jsonObject = new JSONObject(arg);
                event.setAction(jsonObject.getInt("actionType"));
                event.setContentChangeTypes(jsonObject.getInt("contentChangeType"));
                event.setMovementGranularity(jsonObject.getInt("TextMoveStep"));
                event.setAddedCount(jsonObject.getInt("InputType"));
                event.setBeforeText(jsonObject.getString("beforeText"));
                event.setChecked(jsonObject.getBoolean("IsChecked"));
                event.setClassName(getClassNameString(jsonObject.getString("ComponentType")));
                componentType = jsonObject.getString("ComponentType");
                event.setContentDescription(jsonObject.getString("DescriptionInfo"));
                event.setEnabled(jsonObject.getBoolean("IsEnabled"));
                event.setFromIndex(jsonObject.getInt("currentIndex"));
                event.setItemCount(jsonObject.getInt("itemCounts"));
                event.setCurrentItemIndex(jsonObject.getInt("currentIndex"));
                event.setPassword(jsonObject.getBoolean("IsPassword"));
                event.setScrollable(jsonObject.getBoolean("IsScrollable"));

                if (arkUiframeNodes.containsKey(nodeId)) {
                    if (arkUiframeNodes.get(nodeId).isVisible != jsonObject.getBoolean("IsVisible")) {
                        sendWindowContentChangeEvent(nodeId);
                    }
                }
                pageID = jsonObject.getInt("PageId");
            } catch (JSONException e) {
                Log.e(TAG, "onSendAccessibilityEvent failed; err is ", e);
                return false;
            }
        }

        performEvent(nodeId, accessibilityEvent, componentType, pageID);
        return sendAccessibilityEvent(event);
    }

    private void performEvent(int nodeId, int accessibilityEvent, String componentType, int pageID) {
        if (accessibilityEvent == AccessibilityEvent.TYPE_VIEW_FOCUSED) {
            if (arkuiRootAccessibilityView != null) {
                 if (ACE_COMPONENT_PAGE.equals(componentType) ||
                    ACE_COMPONENT_POPUP.equals(componentType)) {
                    arkuiRootAccessibilityView.removeCallbacks(updateNodeIds);
                    isMenuFocus = false;
                    inputFocusNodeId = nodeId;
                    arkuiRootAccessibilityView.post(updateNodeIds);
                } else if (ACE_COMPONENT_MENU.equals(componentType) ||
                    ACE_COMPONENT_DIALOG.equals(componentType)) {
					arkuiRootAccessibilityView.removeCallbacks(updateNodeIds);
                    isMenuFocus = true;
                    inputFocusNodeId = nodeId;
                    arkuiRootAccessibilityView.post(updateNodeIds);
                }
            }
        }
        if (accessibilityEvent == AccessibilityEvent.TYPE_VIEW_ACCESSIBILITY_FOCUSED) {
            accessFocusNodeId = nodeId;
            addAccFocusRoute(nodeId, pageID);
        } else if (accessibilityEvent == AccessibilityEvent.TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED) {
            accessFocusNodeId = -1;
        }
        if (accessibilityEvent == AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED
                || accessibilityEvent == AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED) {
            inputFocusNodeId = -1;
            accessFocusNodeId = -1;
            if (arkuiRootAccessibilityView != null) {
                if (ACE_COMPONENT_PAGE.equals(componentType) ||
                   ACE_COMPONENT_POPUP.equals(componentType) ||
                   ACE_COMPONENT_DIALOG.equals(componentType)) {
                   arkuiRootAccessibilityView.removeCallbacks(updateNodeIds);
                   isMenuFocus = false;
                   inputFocusNodeId = nodeId;
                   arkuiRootAccessibilityView.postDelayed(updateNodeIds,1000);
               } else if (ACE_COMPONENT_MENU.equals(componentType) ||
                   ACE_COMPONENT_DIALOG.equals(componentType)) {
                   arkuiRootAccessibilityView.removeCallbacks(updateNodeIds);
                   isMenuFocus = true;
                   inputFocusNodeId = nodeId;
                   arkuiRootAccessibilityView.postDelayed(updateNodeIds,1000);
               }
           }
        }
    }

    public boolean performAction(int virtualViewId, int accessibilityAction, String bundleString) {
        int args[] = { virtualViewId, accessibilityAction, this.windowId };
        return nativePerformAction(args, bundleString);
    }

    public boolean isEnabled() {
        return (isAccessibilityEnabled() && isTouchExplorationEnabled()) || disabledDelay;
    }

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

    public boolean unregisterJsAccessibilityStateObserver() {
        if (jsAccessibilityStateObserverPtr != 0L) {
            jsAccessibilityStateObserverPtr = 0L;
            return true;
        }
        ALog.e(TAG, "unregisterObserver called. Ptr not exists." + " windowId = " + this.windowId);
        return false;
    }

    public void accessibilityStateChanged(boolean accessibilityEnabled) {
        if (jsAccessibilityStateObserverPtr != 0L) {
            nativeAccessibilityStateChanged(accessibilityEnabled, jsAccessibilityStateObserverPtr);
        }
    }

    private void addAccFocusRoute(int nodeID, int pageID) {
        if (nodeID < 0) {
            return;
        }
        arkUiAccFocusRouteNodes.add(FocusedNode.obtain(nodeID, this.windowId, pageID));
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
                if (arkUiframeNodes.get(routeFocusedNode.nodeId).isFocusable) {
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