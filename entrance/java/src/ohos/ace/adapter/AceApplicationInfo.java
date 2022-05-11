/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

import java.util.Locale;
import java.util.Set;
import java.util.StringJoiner;
import java.util.concurrent.CompletableFuture;

/**
 * The info of the running application
 *
 * @since 1
 */
public final class AceApplicationInfo {
    private static final String LOG_TAG = "AceApplicationInfo";

    private static final AceApplicationInfo INSTANCE = new AceApplicationInfo();

    /**
     * Common interface for locale changed.
     *
     */
    public interface IAceLocaleChanged {
        /**
         * Called when locale changed.
         *
         */
        void onLocaleChanged();
    }

    /**
     * Common interface for locale fallback.
     *
     */
    public interface IAceLocaleFallback {
        /**
         * Called when locale fallback.
         *
         * @param locale     current locale
         * @param localeList support locale list
         * @return locale priority string, such as "zh-CN,en-US".
         */
        String onLocaleFallback(String locale, String[] localeList);
    }

    private static String applicationLanguage;

    private String packageName;

    private int pid;

    private IAceLocaleChanged localeChangedCallback;

    private IAceLocaleFallback localeFallbackCallback;

    private String languageTag;

    /**
     * Get AceApplicationInfo instance.
     *
     * @return the singleton instance of AceApplication
     */
    public static AceApplicationInfo getInstance() {
        return INSTANCE;
    }

    private AceApplicationInfo() {
        nativeInitialize(this);
    }

    /**
     * set up external icu data
     *
     * @param icuData the path of the icu data file
     */
    public void setupIcuRes(String icuData) {
        nativeSetupIcuRes(icuData);
    }

    /**
     * set package info to native
     *
     * @param packageName         name of this application package
     * @param uid                 uid of the application
     * @param isDebug             ture if the application is debug version
     * @param needDebugBreakPoint trur if need debug break point
     */
    public void setPackageInfo(String packageName, int uid, boolean isDebug, boolean needDebugBreakPoint) {
        this.packageName = packageName;
        nativeSetPackageInfo(packageName, uid, isDebug, needDebugBreakPoint);
    }

    /**
     * Set process id
     *
     * @param pid the process id
     */
    public void setPid(int pid) {
        this.pid = pid;
    }

    /**
     * Get package name of this application.
     *
     * @return the package name
     */
    public String getPackageName() {
        return packageName;
    }

    /**
     * Get pid of this application
     *
     * @return process id
     */
    public int getPid() {
        return pid;
    }


    /**
     * Get the default country or region
     *
     * @return the country code
     */
    public String getCountryOrRegion() {
        return Locale.getDefault().getCountry();
    }

    /**
     * Get the default language.
     *
     * @return the language code
     */
    public String getLanguage() {
        return Locale.getDefault().getLanguage();
    }


    /**
     * Get the default script, empty string or an ISO 15924 4-letter script code.
     *
     * @return the script code
     */
    public String getScript() {
        return Locale.getDefault().getScript();
    }

    /**
     * Get a string consisting of keyword/values pairs, such as "collation=phonebook;currency=euro"
     *
     * @return a string consisting of keyword/values pairs.
     */
    public String getKeywordsAndValues() {
        Set<String> keywords = Locale.getDefault().getUnicodeLocaleKeys();
        if (keywords.isEmpty()) {
            return "";
        }
        StringBuffer keywordsAndValues = new StringBuffer();
        for (String key : keywords) {
            keywordsAndValues.append(key).append("=").append(Locale.getDefault().getUnicodeLocaleType(key)).append(";");
        }
        keywordsAndValues.deleteCharAt(keywordsAndValues.length() - 1);
        return keywordsAndValues.toString();
    }

    /**
     * Get the language tag.
     *
     * @return the language tag
     */
    public String getLanguageTag() {
        return languageTag;
    }

    /**
     * Change the locale
     *
     * @param language the language code
     * @param country the country code
     */
    public void changeLocale(String language, String country) {
        if (language == null || country == null || language.isEmpty() || country.isEmpty()) {
            ALog.e(LOG_TAG, "the language or country is null");
            return;
        }

        String languageLower = language.toLowerCase(Locale.ENGLISH);
        String countryUpper = country.toUpperCase(Locale.ENGLISH);
        Locale.setDefault(new Locale(languageLower, countryUpper));
        setLocale();

        if (localeChangedCallback != null) {
            localeChangedCallback.onLocaleChanged();
        } else {
            ALog.w(LOG_TAG, "localeChangeCallback is null");
        }
    }

    /**
     * Set the locale changed callback.
     *
     * @param localeChangedCallback the locale changed callback
     */
    public void setLocaleChanged(IAceLocaleChanged localeChangedCallback) {
        this.localeChangedCallback = localeChangedCallback;
    }

    /**
     * Set the locale fallback callback.
     *
     * @param localeFallbackCallback the locale fallback callback
     */
    public void setLocaleFallback(IAceLocaleFallback localeFallbackCallback) {
        this.localeFallbackCallback = localeFallbackCallback;
    }

    private String getLocaleFallback(String locale, String[] localeList) {
        if (localeFallbackCallback != null) {
            return localeFallbackCallback.onLocaleFallback(locale, localeList);
        }
        return "";
    }

    /**
     * Set the locale asynchronous
     *
     */
    public void setLocale() {
        StringBuilder languageBuilder = new StringBuilder();
        languageBuilder.append(getLanguage()).append(getScript()).append(getCountryOrRegion())
                .append(getKeywordsAndValues());
        String tempLanguage = languageBuilder.toString();
        if (tempLanguage.equals(applicationLanguage)) {
            return;
        }
        applicationLanguage = tempLanguage;
        CompletableFuture.runAsync(() -> {
            StringJoiner stringJoiner = new StringJoiner("-");
            stringJoiner.add(getLanguage());
            stringJoiner.add(getScript());
            stringJoiner.add(getCountryOrRegion());
            stringJoiner.add(getKeywordsAndValues());
            languageTag = stringJoiner.toString();

            nativeLocaleChanged(getLanguage(), getCountryOrRegion(), getScript(), getKeywordsAndValues());
        });
    }

    /**
     * Set the locale synchronous
     *
     */
    public void setSyncLocale() {
        StringBuilder languageBuilder = new StringBuilder();
        languageBuilder.append(getLanguage()).append(getScript()).append(getCountryOrRegion())
                .append(getKeywordsAndValues());
        String tempLanguage = languageBuilder.toString();
        if (tempLanguage.equals(applicationLanguage)) {
            return;
        }
        applicationLanguage = tempLanguage;

        StringJoiner stringJoiner = new StringJoiner("-");
        stringJoiner.add(getLanguage());
        stringJoiner.add(getScript());
        stringJoiner.add(getCountryOrRegion());
        stringJoiner.add(getKeywordsAndValues());
        languageTag = stringJoiner.toString();

        nativeLocaleChanged(getLanguage(), getCountryOrRegion(), getScript(), getKeywordsAndValues());
    }

    /**
     * Set user id.
     *
     * @param userId user ID
     */
    public void setUserId(int userId) {
        nativeSetUserId(userId);
    }

    /**
     * Set process name.
     *
     * @param processName name of process
     */
    public void setProcessName(String processName) {
        nativeSetProcessName(processName);
    }

    /**
     * Set accessibility if enabled.
     *
     * @param enabled true if accessibility enabled
     */
    public void setAccessibilityEnabled(boolean enabled) {
        nativeSetAccessibilityEnabled(enabled);
    }

    private native void nativeSetPackageInfo(String packageName, int uid, boolean isDebug, boolean needDebugBreakPoint);

    private native void nativeInitialize(AceApplicationInfo info);

    private native void nativeLocaleChanged(String language, String country, String script, String keywoardsAndValues);

    private native void nativeSetUserId(int userId);

    private native void nativeSetProcessName(String processName);

    private native void nativeSetupIcuRes(String icuData);

    private native void nativeSetAccessibilityEnabled(boolean enabled);
}