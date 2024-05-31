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

#define NAPI_VERSION 8

#include "core/common/ai/image_analyzer_mgr.h"
#include "core/common/ai/image_analyzer_default.h"

namespace OHOS::Ace {

ImageAnalyzerMgr& ImageAnalyzerMgr::GetInstance()
{
    static ImageAnalyzerMgr instance;
    return instance;
}

ImageAnalyzerMgr::ImageAnalyzerMgr()
{
}

bool ImageAnalyzerMgr::IsImageAnalyzerSupported()
{
    return false;
}

void ImageAnalyzerMgr::BuildNodeFunc(
    napi_value pixelMap, ImageAnalyzerConfig* config, ImageAnalyzerInnerConfig* uiConfig, void** overlayData)
{
}

void ImageAnalyzerMgr::UpdateImage(
    void** overlayData, napi_value pixelMap, ImageAnalyzerConfig* config, ImageAnalyzerInnerConfig* uiConfig)
{
}

void ImageAnalyzerMgr::UpdateConfig(void** overlayData, ImageAnalyzerConfig* config)
{
}

void ImageAnalyzerMgr::UpdateInnerConfig(void** overlayData, ImageAnalyzerInnerConfig* config)
{
}

void ImageAnalyzerMgr::Release(void** overlayData)
{
}
}