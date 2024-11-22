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

 package ohos.ace.adapter.capability.platformview;

 import ohos.ace.adapter.capability.platformview.IPlatformView;
 
 /**
  * The class for interface PlatformViewFactory on Android platform.
  *
  * @since 1
  */
 public abstract class PlatformViewFactory {
     /**
      * Using xComponentId to Obtain Platform View.
      *
      * @return IPlatformView
      */
     public abstract IPlatformView getPlatformView(String xComponentId);
 }