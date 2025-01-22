/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

/**
 * The WebDataBaseGeolocationPermissionsDao class is used to handle data objects
 * in the database of Web component geolocation permission management objects.
 *
 * @since 1
 */
public class WebDataBaseGeolocationPermissions {
    private Long _id;
    private String origin;
    private Integer result;

    public WebDataBaseGeolocationPermissions(String origin, Integer result) {
        this.origin = origin;
        this.result = result;
    }

    public WebDataBaseGeolocationPermissions(Long _id, String origin, Integer result) {
        this._id = _id;
        this.origin = origin;
        this.result = result;
    }

    public Long getId() {
        return _id;
    }

    public void setId(Long _id) {
        this._id = _id;
    }

    public String getOrigin() {
        return origin;
    }

    public void setOrigin(String origin) {
        this.origin = origin;
    }

    public Integer getResult() {
        return result;
    }

    public void setResult(Integer result) {
        this.result = result;
    }

    @Override
    public String toString() {
        return "WebDataBaseGeolocationPermissions{" + "_id=" + _id + ", origin='" + origin + '\'' + ", result=" + result
                + '}';
    }
}