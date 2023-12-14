/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "base/log/frame_report.h"

namespace OHOS::Ace {

FrameReport& FrameReport::GetInstance()
{
    static FrameReport instance;
    return instance;
}

FrameReport::FrameReport() {}

FrameReport::~FrameReport() {}

bool FrameReport::LoadLibrary()
{
    frameSchedHandle_ = nullptr;
    frameSchedSoLoaded_ = false;
    return false;
}

void FrameReport::CloseLibrary() {}

void* FrameReport::LoadSymbol(const char* symName)
{
    return nullptr;
}

void FrameReport::Init()
{
    frameInitFunc_ = nullptr;
}

int FrameReport::GetEnable()
{
    frameGetEnableFunc_ = nullptr;
    return false;
}

void FrameReport::BeginFlushAnimation()
{
    beginFlushAnimationFunc_ = nullptr;
}

void FrameReport::EndFlushAnimation()
{
    endFlushAnimationFunc_ = nullptr;
}

void FrameReport::BeginFlushBuild()
{
    beginFlushBuildFunc_ = nullptr;
}

void FrameReport::EndFlushBuild()
{
    endFlushBuildFunc_ = nullptr;
}

void FrameReport::BeginFlushLayout()
{
    beginFlushLayoutFunc_ = nullptr;
}

void FrameReport::EndFlushLayout()
{
    endFlushLayoutFunc_ = nullptr;
}

void FrameReport::BeginFlushRender()
{
    beginFlushRenderFunc_ = nullptr;
}

void FrameReport::EndFlushRender()
{
    endFlushRenderFunc_ = nullptr;
}

void FrameReport::BeginFlushRenderFinish()
{
    beginFlushRenderFinishFunc_ = nullptr;
}

void FrameReport::EndFlushRenderFinish()
{
    endFlushRenderFinishFunc_ = nullptr;
}

void FrameReport::BeginProcessPostFlush()
{
    beginProcessPostFunc_ = nullptr;
}

void FrameReport::BeginListFling()
{
    beginListFlingFunc_ = nullptr;
}

void FrameReport::EndListFling()
{
    endListFlingFunc_ = nullptr;
}

void FrameReport::FlushBegin()
{
    flushBeginFunc_ = nullptr;
}

void FrameReport::FlushEnd()
{
    flushEndFunc_ = nullptr;
}

} // namespace OHOS::Ace
