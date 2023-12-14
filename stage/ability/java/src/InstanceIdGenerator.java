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

package ohos.stage.ability.adapter;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * A class for being used to generate ability instanceId
 *
 * @since 1
 */
public final class InstanceIdGenerator {

    private static final AtomicInteger ID_GENERATOR = new AtomicInteger(1);

    /**
     * get the instance id, and increment self
     * @return the instance id
     */
    public static int getAndIncrement() {
        return ID_GENERATOR.getAndIncrement();
    }

    /**
     * get the instance id
     * @return the instance id
     */
    public static int get() {
        return ID_GENERATOR.get();
    }
}