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

import android.webkit.ConsoleMessage;

public class AceWebConsoleMessageObject {
    private static final String LOG_TAG = "AceWebConsoleMessageObject";

    private static final int ONELEVEL = 1;

    private ConsoleMessage consoleMessage;

    public AceWebConsoleMessageObject(ConsoleMessage consoleMessage) {
        this.consoleMessage = consoleMessage;
    }

    public int getLineNumber() {
        return this.consoleMessage.lineNumber();
    }

    public String getMessage() {
        return this.consoleMessage.message().toString();
    }

    public int getMessageLevel() {
        return this.consoleMessage.messageLevel().ordinal() + ONELEVEL;
    }

    public String getSourceId() {
        return this.consoleMessage.sourceId().toString();
    }
}