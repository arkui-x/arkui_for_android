/**
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

package ohos.ace.adapter.capability.bridge;

/**
 * Bridge error code.
 *
 * @since 10
 */
public enum BridgeErrorCode {
    BRIDGE_ERROR_NO(0, "Correct!!"),
    BRIDGE_NAME_ERROR(1, "Bridge name error!"),
    BRIDGE_CREATE_ERROR(2, "Bridge creation failure!"),
    BRIDGE_INVALID(3, "Bridge unavailable!"),
    BRIDGE_METHOD_NAME_ERROR(4, "Method name error!"),
    BRIDGE_METHOD_RUNNING(5, "Method is running..."),
    BRIDGE_METHOD_UNIMPL(6, "Method not implemented!"),
    BRIDGE_METHOD_PARAM_ERROR(7, "Method parameter error!"),
    BRIDGE_METHOD_EXISTS(8, "Method already exists!"),
    BRIDGE_DATA_ERROR(9, "Data error"),
    BRIDGE_EXCEEDS_SAFE_INTEGER(10, "Data exceeds safe integer"),
    BRIDGE_CODEC_TYPE_MISMATCH(11, "Bridge codec type mismatch"),
    BRIDGE_CODEC_INVALID(12, "Bridge codec is invalid");

    private int id;
    private String errorMessage;

    /**
     * Constructor of BridgeErrorCode.
     *
     * @param id BridgeErrorCodeId.
     * @param errorMessage BridgeErrorMessage.
     */
    BridgeErrorCode(int id, String errorMessage) {
        this.id = id;
        this.errorMessage = errorMessage;
    }

    /**
     * Get id of BridgeErrorCode.
     *
     * @return The BridgeErrorCodeId.
     */
    public int getId() {
        return id;
    }

    /**
     * Get errorMessage of BridgeErrorCode.
     *
     * @return The BridgeErrorMessage.
     */
    public String getErrorMessage() {
        return this.errorMessage;
    }
}
