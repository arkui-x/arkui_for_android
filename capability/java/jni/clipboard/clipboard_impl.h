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

#ifndef FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_CLIPBOARD_PROXY_IMPL_H
#define FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_CLIPBOARD_PROXY_IMPL_H

#include <functional>
#include <string>

#include "base/memory/referenced.h"
#include "base/thread/task_executor.h"
#include "base/utils/macros.h"
#include "core/common/clipboard/clipboard.h"
#include "core/common/clipboard/clipboard_interface.h"

namespace OHOS::Ace::Platform {

using CallbackSetClipboardPixmapData = std::function<void(const RefPtr<PixelMap>&)>;
using CallbackGetClipboardPixmapData = std::function<const RefPtr<PixelMap>(void)>;

class ClipboardImpl final : public Clipboard {
public:
    explicit ClipboardImpl(const RefPtr<TaskExecutor>& taskExecutor) : Clipboard(taskExecutor) {}
    ~ClipboardImpl() override = default;

    void SetData(
        const std::string& data, CopyOptions copyOption = CopyOptions::InApp, bool isDragData = false) override;
    void GetData(const std::function<void(const std::string&)>& callback, bool syncMode = false) override;
    void SetPixelMapData(const RefPtr<PixelMap>& pixmap, CopyOptions copyOption = CopyOptions::InApp) override;
    void GetPixelMapData(const std::function<void(const RefPtr<PixelMap>&)>& callback, bool syncMode = false) override;
    void Clear() override;
    void HasData(const std::function<void(bool hasData)>& callback) override;
    void RegisterCallbackSetClipboardPixmapData(CallbackSetClipboardPixmapData callback);
    void RegisterCallbackGetClipboardPixmapData(CallbackGetClipboardPixmapData callback);
    void AddPixelMapRecord(const RefPtr<PasteDataMix>& pasteData, const RefPtr<PixelMap>& pixmap) override;
    void AddImageRecord(const RefPtr<PasteDataMix>& pasteData, const std::string& uri) override;
    void AddTextRecord(const RefPtr<PasteDataMix>& pasteData, const std::string& selectedStr) override;
    void SetData(const RefPtr<PasteDataMix>& pasteData, CopyOptions copyOption) override;
    void GetData(const std::function<void(const std::string&, bool isLastRecord)>& textCallback,
        const std::function<void(const RefPtr<PixelMap>&, bool isLastRecord)>& pixelMapCallback,
        const std::function<void(const std::string&, bool isLastRecord)>& urlCallback, bool syncMode = false) override;
    RefPtr<PasteDataMix> CreatePasteDataMix() override;

private:
    CallbackSetClipboardPixmapData callbackSetClipboardPixmapData_;
    CallbackGetClipboardPixmapData callbackGetClipboardPixmapData_;
    ACE_DISALLOW_COPY_AND_MOVE(ClipboardImpl);
};
} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_ANDROID_CAPABILITY_JAVA_JNI_CLIPBOARD_CLIPBOARD_PROXY_IMPL_H