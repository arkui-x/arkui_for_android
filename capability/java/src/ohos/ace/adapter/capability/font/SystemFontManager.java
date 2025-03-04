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
package ohos.ace.adapter.capability.font;

import android.os.Build;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import ohos.ace.adapter.ALog;

/**
 * SystemFontManager to handle system default fonts information.
 *
 * @since 12
 */
public class SystemFontManager {
    private static final String LOG_TAG = "SystemFontManager";

    private static final String CLASS_SYSTEM_FONTS = "android.graphics.fonts.SystemFonts";
    private static final String METHOD_SYSTEM_FONTS_GET_AVAILABLE_FONTS = "getAvailableFonts";
    private static final String CLASS_FONT = "android.graphics.fonts.Font";
    private static final String METHOD_FONT_GET_FILE = "getFile";
    private static final String METHOD_FONT_GET_STYLE = "getStyle";
    private static final String CLASS_FONT_STYLE = "android.graphics.fonts.FontStyle";
    private static final String METHOD_FONT_STYLE_GET_WEIGHT = "getWeight";
    private static final String METHOD_FONT_STYLE_GET_SLANT = "getSlant";

    private Class<?> systemFontsClazz;
    private Method getAvailableFontsMethod;
    private Class<?> fontClazz;
    private Method getFileMethod;
    private Method getStyleMethod;
    private Class<?> fontStyleClazz;
    private Method getWeightMethod;
    private Method getSlantMethod;

    public SystemFontManager() {
        nativeInit();
        prepareReflection();
    }

    void prepareReflection() {
        try {
            systemFontsClazz = Class.forName(CLASS_SYSTEM_FONTS);
            getAvailableFontsMethod = systemFontsClazz.getMethod(METHOD_SYSTEM_FONTS_GET_AVAILABLE_FONTS);
        } catch (ClassNotFoundException | NoSuchMethodException ignored) {
            ALog.w(LOG_TAG, "SystemFonts class or method not found");
        }
        try {
            fontClazz = Class.forName(CLASS_FONT);
            getFileMethod = fontClazz.getMethod(METHOD_FONT_GET_FILE);
            getStyleMethod = fontClazz.getMethod(METHOD_FONT_GET_STYLE);
        } catch (ClassNotFoundException | NoSuchMethodException ignored) {
            ALog.w(LOG_TAG, "Font class or method not found");
        }
        try {
            fontStyleClazz = Class.forName(CLASS_FONT_STYLE);
            getWeightMethod = fontStyleClazz.getMethod(METHOD_FONT_STYLE_GET_WEIGHT);
            getSlantMethod = fontStyleClazz.getMethod(METHOD_FONT_STYLE_GET_SLANT);
        } catch (ClassNotFoundException | NoSuchMethodException ignored) {
            ALog.w(LOG_TAG, "FontStyle class or method not found");
        }
    }

    /**
     * Get system available font object set.
     *
     * @return system available font object set.
     */
    private Set<Object> getSystemFonts() {
        Set<Object> availableFonts = null;
        try {
            availableFonts = (Set<Object>) getAvailableFontsMethod.invoke(systemFontsClazz);
        } catch (IllegalAccessException | InvocationTargetException | NullPointerException ignored) {
            ALog.e(LOG_TAG, "get system available font object set failed");
        }
        return availableFonts;
    }

    /**
     * Get font information from font file object.
     *
     * @param fontFile font file object.
     * @param fontInfo font information object.
     */
    private void getFontInfoFromFile(File fontFile, SystemFontInfo fontInfo) {
        if (fontFile == null || fontInfo == null) {
            return;
        }
        try {
            String fontPath = fontFile.getCanonicalPath();
            fontInfo.setPath(fontPath);
        } catch (IOException ignore) {
            ALog.e(LOG_TAG, "get system font info getCanonicalPath failed");
        }
        String fontName = fontFile.getName();
        int lastIndex = fontName.lastIndexOf(".");
        String prefix = fontName.substring(0, lastIndex);
        String name = prefix.replaceAll("[-_]", " ");
        fontInfo.setName(name);
    }

    /**
     * Get font information from font style object.
     *
     * @param fontStyle font style object.
     * @param fontInfo font information object.
     */
    private void getFontInfoFromStyle(Object fontStyle, SystemFontInfo fontInfo) {
        if (fontStyle == null || fontInfo == null) {
            return;
        }
        try {
            int weight = (int) getWeightMethod.invoke(fontStyle);
            fontInfo.setWeight(weight);
            int slant = (int) getSlantMethod.invoke(fontStyle);
            fontInfo.setItalic(slant > 0);
        } catch (IllegalAccessException | InvocationTargetException | NullPointerException ignored) {
            ALog.e(LOG_TAG, "get system font info from font style failed");
        }
    }

    /**
     * Get fonts information from font object set.
     *
     * @param fonts font object set.
     * @return font information list.
     */
    private List<SystemFontInfo> getFontInfoList(Set<Object> fonts) {
        if (fonts == null || fonts.isEmpty()) {
            return null;
        }
        List<SystemFontInfo> result = new ArrayList<>();
        try {
            for (Object fontObject : fonts) {
                SystemFontInfo fontInfo = new SystemFontInfo();
                File fontFile = (File) getFileMethod.invoke(fontObject);
                getFontInfoFromFile(fontFile, fontInfo);
                Object fontStyle = getStyleMethod.invoke(fontObject);
                getFontInfoFromStyle(fontStyle, fontInfo);
                result.add(fontInfo);
            }
        } catch (IllegalAccessException | InvocationTargetException | NullPointerException ignored) {
            ALog.e(LOG_TAG, "get system font info list failed");
        }
        return result;
    }

    /**
     * Get system default fonts information.
     *
     * @return system default fonts information.
     */
    public SystemFontInfo[] getSystemFontInfoList() {
        if (Build.VERSION.SDK_INT < 29) {
            ALog.e(LOG_TAG, "sdk build version not support get system font info list");
            return null;
        }
        Set<Object> availableFonts = getSystemFonts();
        List<SystemFontInfo> fontInfoList = getFontInfoList(availableFonts);
        if (fontInfoList != null) {
            SystemFontInfo[] fontInfos = new SystemFontInfo[fontInfoList.size()];
            fontInfoList.toArray(fontInfos);
            return fontInfos;
        }
        return null;
    }

    /**
     * native func for Init.
     */
    public native void nativeInit();
}
