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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_SPAN_HTML_TO_SPAN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_SPAN_HTML_TO_SPAN_H

#include <list>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "libxml/HTMLparser.h"
#include "libxml/HTMLtree.h"
#include "libxml/parser.h"

#include "base/geometry/dimension.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/text/text_theme.h"
#include "core/components_ng/pattern/text/span/mutable_span_string.h"
#include "core/components_ng/pattern/text/span/span_object.h"
#include "core/components_ng/pattern/text/span/span_string.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_ng/pattern/text/text_model.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text/text_styles.h"

namespace OHOS::Ace {

class SpanStringBase;

class HtmlToSpan {
public:
    explicit HtmlToSpan() {};
    ~HtmlToSpan() {};
    RefPtr<MutableSpanString> ToSpanString(const std::string& html, const bool isNeedLoadPixelMap = true);
    using Styles = std::vector<std::pair<std::string, std::string>>;

    class DecorationSpanParam {
    public:
        std::vector<TextDecoration> decorationType;
        Color color;
        TextDecorationStyle decorationSytle;
    };
    class BaseLineSpanParam {
    public:
        Dimension dimension;
    };
    class LetterSpacingSpanParam {
    public:
        Dimension dimension;
    };
    class LineHeightSpanSparam {
    public:
        Dimension dimension;
    };
    using StyleValue = std::variant<std::monostate, Font, DecorationSpanParam, BaseLineSpanParam,
        LetterSpacingSpanParam, LineHeightSpanSparam, std::vector<Shadow>, ImageSpanOptions, SpanParagraphStyle,
        TextBackgroundStyle, std::string>;
    enum class StyleIndex {
        STYLE_NULL = 0,
        STYLE_FONT,
        STYLE_DECORATION,
        STYLE_BASELINE,
        STYLE_LETTERSPACE,
        STYLE_LINEHEIGHT,
        STYLE_SHADOWS,
        STYLE_IMAGE,
        STYLE_PARAGRAPH,
        STYLE_BACKGROUND_COLOR,
        STYLE_URL,
        STYLE_MAX
    };

private:
    enum class HtmlType {
        PARAGRAPH = 0,
        IMAGE,
        TEXT,
        ANCHOR,
        DEFAULT,
    };

    struct SpanInfo {
        HtmlType type;
        size_t start;
        size_t end;
        std::vector<StyleValue> values;
    };
    using StyleValues = std::map<std::string, StyleValue>;
    template<class T>
    T* Get(StyleValue* styleValue) const;
    Styles ParseStyleAttr(const std::string& style);
    bool IsParagraphAttr(const std::string& key);
    void InitParagraph(const std::string& key, const std::string& value, const std::string& index, StyleValues& values);
    void InitFont(const std::string& key, const std::string& value, const std::string& index, StyleValues& values);
    bool IsFontAttr(const std::string& key);
    void InitDecoration(
        const std::string& key, const std::string& value, const std::string& index, StyleValues& values);
    bool IsDecorationAttr(const std::string& key);
    template<class T>
    void InitDimension(const std::string& key, const std::string& value, const std::string& index, StyleValues& values);
    bool IsLetterSpacingAttr(const std::string& key);
    void InitTextShadow(
        const std::string& key, const std::string& value, const std::string& index, StyleValues& values);
    bool IsTextShadowAttr(const std::string& key);
    std::pair<std::string, double> GetUnitAndSize(const std::string& str);
    bool IsLength(const std::string& str);
    void InitShadow(Shadow &textShadow, std::vector<std::string> &attribute);
    void InitLineHeight(const std::string& key, const std::string& value, StyleValues& values);
    Dimension FromString(const std::string& str);
    TextAlign StringToTextAlign(const std::string& value);
    TextVerticalAlign StringToTextVerticalAlign(const std::string& value);
    WordBreak StringToWordBreak(const std::string& value);
    TextOverflow StringToTextOverflow(const std::string& value);
    bool IsTextIndentAttr(const std::string& key);
    bool IsLineHeightAttr(const std::string& key);
    bool IsPaddingAttr(const std::string& key);
    bool IsMarginAttr(const std::string& key);
    bool IsBorderAttr(const std::string& key);
    bool IsDecorationLine(const std::string& key);
    bool IsDecorationStyle(const std::string& key);
    bool IsBackgroundColorAttr(const std::string& key) const;
    void InitBackgroundColor(
        const std::string& key, const std::string& value, const std::string& index, StyleValues& values);
    bool IsForegroundColorAttr(const std::string& key) const;
    void InitForegroundColor(
        const std::string& key, const std::string& value, const std::string& index, StyleValues& values);
    void SetPaddingOption(const std::string& key, const std::string& value, ImageSpanOptions& options);
    void SetMarginOption(const std::string& key, const std::string& value, ImageSpanOptions& options);
    void SetBorderOption(const std::string& key, const std::string& value, ImageSpanOptions& options);
    template<class T>
    std::pair<bool, HtmlToSpan::StyleValue*> GetStyleValue(
        const std::string& key, std::map<std::string, StyleValue>& values);
    void HandleImgSpanOption(const Styles& styleMap, ImageSpanOptions& options);
    void HandleImagePixelMap(const std::string& src, ImageSpanOptions& option);
    void HandleImageSize(const std::string& key, const std::string& value, ImageSpanOptions& options);
    void MakeImageSpanOptions(const std::string& key, const std::string& value, ImageSpanOptions& imgOpt);

    Color ToSpanColor(const std::string& color);

    std::map<std::string, HtmlToSpan::StyleValue> ToTextSpanStyle(xmlAttrPtr curNode);
    void AddStyleSpan(const std::string& element, SpanInfo& info);
    void ToTextSpan(const std::string& element, xmlNodePtr node, size_t len,
        size_t& pos, std::vector<SpanInfo>& spanInfos);

    void ToImageOptions(const std::map<std::string, std::string>& styles, ImageSpanOptions& option);
    void ToImage(xmlNodePtr node, size_t len, size_t& pos, std::vector<SpanInfo>& spanInfos,
        bool isProcessImageOptions = true);

    void ToParagraphStyle(const Styles& styleMap, SpanParagraphStyle& style);
    void ToParagraphSpan(xmlNodePtr node, size_t len, size_t& pos, std::vector<SpanInfo>& spanInfos);

    void ParseHtmlToSpanInfo(
        xmlNodePtr node, size_t& pos, std::string& allContent, std::vector<SpanInfo>& spanInfos,
        bool isNeedLoadPixelMap = true);
    void ToSpan(xmlNodePtr curNode, size_t& pos, std::string& allContent, std::vector<SpanInfo>& spanInfos,
        bool isNeedLoadPixelMap = true);
    void PrintSpanInfos(const std::vector<SpanInfo>& spanInfos);
    void AfterProcSpanInfos(std::vector<SpanInfo>& spanInfos);
    bool IsValidNode(const std::string& name);

    RefPtr<SpanBase> CreateSpan(size_t index, const SpanInfo& info, StyleValue& value);
    template<class T, class P>
    RefPtr<SpanBase> MakeSpan(const SpanInfo& info, StyleValue& value);
    template<class T, class P>
    RefPtr<SpanBase> MakeDimensionSpan(const SpanInfo& info, StyleValue& value);
    RefPtr<SpanBase> MakeDecorationSpan(const SpanInfo& info, StyleValue& value);
    void AddImageSpans(const SpanInfo& info, RefPtr<MutableSpanString> mutableSpan);
    void AddSpans(const SpanInfo& info, RefPtr<MutableSpanString> span);
    void ToAnchorSpan(xmlNodePtr node, size_t len, size_t& pos, std::vector<SpanInfo>& spanInfos);
    std::string CleanTextSpaces(const std::string& text);

    std::string GetHtmlContent(xmlNodePtr node);
    RefPtr<MutableSpanString> GenerateSpans(const std::string& allContent, const std::vector<SpanInfo>& spanInfos);
    std::vector<SpanInfo> spanInfos_;
    static constexpr double PT_TO_PX = 1.3;
    static constexpr double ROUND_TO_INT = 0.5;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_SPAN_HTML_TO_SPAN_H