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

import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.util.HashSet;

/**
 * Dump operate process
 * 
 */
public final class DumpHelper {
    private static final String LOG_TAG = "DumpHelper";

    private static final HashSet<String> PARAMETERS = new HashSet<String>() {
        {
            add("-h");
            add("-element");
            add("-render");
            add("-focus");
            add("-memory");
            add("-layer");
            add("-frontend");
            add("-accessibility");
            add("-rotation");
        }
    };

    private DumpHelper() {
    }

    public static void dump(String prefix, FileDescriptor fd, PrintWriter writer, String[] args) {
        if (fd == null || args == null) {
            ALog.w(LOG_TAG, "dump failed, fd pr args is null");
            return;
        }

        if (args.length == 0) {
            printString(fd, "Param length is null" + System.lineSeparator());
        }

        String help = "";
        if ("-h".equals(args[0])) {
            help = "Usage:" + System.lineSeparator() +
                    "    -element                       dump information of Element tree." + System.lineSeparator() +
                    "    -render                        dump information of Render tree." + System.lineSeparator() +
                    "    -layer                         dump information of Layer tree." + System.lineSeparator() +
                    "    -foucs                         dump information of Focus tree." + System.lineSeparator() +
                    "    -frontend                      dump information of Frontend." + System.lineSeparator() +
                    "    -memory                        dump information of Memory usage." + System.lineSeparator() +
                    "    -accessibility                 dump information of accessibility tree."
                    + System.lineSeparator() +
                    "    -accessibility <ID>            dump the property of accessibility node with <ID>."
                    + System.lineSeparator() +
                    "    -accessibility <ID> <action>   dispatch the accessibility event to node with <ID>, <action> as below:."
                    + System.lineSeparator() +
                    "                                     2  --- custom event" + System.lineSeparator() +
                    "                                     10 --- click event" + System.lineSeparator() +
                    "                                     11 --- long click event" + System.lineSeparator() +
                    "                                     12 --- scroll forward" + System.lineSeparator() +
                    "                                     13 --- scroll backward" + System.lineSeparator() +
                    "                                     14 --- focus event" + System.lineSeparator() +
                    "    -rotation <value>              dispatch the rotation event." + System.lineSeparator();
        }

        if (!PARAMETERS.contains(args[0])) {
            help = "'" + args[0] + "' is not a vaild parameter, See '-h'." + System.lineSeparator();
        }

        if (!help.isEmpty()) {
            printString(fd, help);
            return;
        }

        nativeDump(prefix, fd, args);
    }

    private static void printString(FileDescriptor fd, String str) {
        try (FileOutputStream fos = new FileOutputStream(fd)) {
            fos.write(str.getBytes(StandardCharsets.UTF_8));
        } catch (FileNotFoundException ignored) {
            ALog.w(LOG_TAG, "Dump failed, file not found");
        } catch (SecurityException | IOException ignored) {
            ALog.w(LOG_TAG, "Dump failed, security or io exception");
        }
    }

    private static native void nativeDump(String prefix, FileDescriptor fd, String[] args);
}
