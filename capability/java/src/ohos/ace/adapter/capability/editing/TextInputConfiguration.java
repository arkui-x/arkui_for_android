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

package ohos.ace.adapter.capability.editing;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * TextInputConfiguration for IME.
 *
 */
public class TextInputConfiguration {
    private final TextInputType type;
    private final boolean obscure;
    private final TextInputAction action;
    private final String actionLabel;
    private final boolean autoCorrect;
    private final String capitalization;
    private final String keyboardAppearance;

    private TextInputConfiguration(
        TextInputType type,
        boolean obscure,
        TextInputAction action,
        String actionLabel,
        boolean autoCorrect,
        String capitalization,
        String keyboardAppearance) {
        this.type = type;
        this.obscure = obscure;
        this.action = action;
        this.actionLabel = actionLabel;
        this.autoCorrect = autoCorrect;
        this.capitalization = capitalization;
        this.keyboardAppearance = keyboardAppearance;
    }

    /**
     * Deserialize from JSON.
     *
     * @param config The json format configuration.
     * @return config for ime.
     */
    public static TextInputConfiguration fromJson(JSONObject config) throws JSONException {
        if (config == null) {
            return null;
        }

        return new TextInputConfiguration(
            // Currently support text only.
            TextInputType.of(config.getInt("type")),
            config.getBoolean("obscureText"),
            TextInputAction.of(config.getInt("action")),
            config.getString("actionLabel"),
            config.getBoolean("autoCorrect"),
            config.getString("capitalization"),
            config.getString("keyboardAppearance")
        );
    }

    /**
     * Gets the type.
     *
     * @return The type.
     */
    public TextInputType getType() {
        return type;
    }

    /**
     * Gets if need obscure.
     *
     * @return true if can obscure.
     */
    public boolean isObscure() {
        return obscure;
    }

    /**
     * Gets the action.
     *
     * @return The action.
     */
    public TextInputAction getAction() {
        return action;
    }

    /**
     * Gets the action label.
     *
     * @return The action label.
     */
    public String getActionLabel() {
        return actionLabel;
    }

    /**
     * Gets if can auto correct.
     *
     * @return true if can auto correct
     */
    public boolean canAutoCorrect() {
        return autoCorrect;
    }

    /**
     * Gets the capitalization setting.
     *
     * @return The auto capitalization config.
     */
    public String getCapitalization() {
        return capitalization;
    }

    /**
     * Gets the keyboard appearance.
     *
     * @return The keyboard appearance config.
     */
    public String getKeyboardAppearance() {
        return keyboardAppearance;
    }
}