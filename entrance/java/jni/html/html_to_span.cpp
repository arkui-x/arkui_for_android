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

#include "html_to_span.h"
#include <sstream>

#include "core/text/html_utils.h"

namespace OHOS::Ace {
constexpr int ONE_PARAM = 1;
constexpr int TWO_PARAM = 2;
constexpr int THREE_PARAM = 3;
constexpr int FOUR_PARAM = 4;

constexpr int TOP_PARAM = 0;
constexpr int RIGHT_PARAM = 1;
constexpr int BOTTOM_PARAM = 2;
constexpr int LEFT_PARAM = 3;
constexpr int FIRST_PARAM = 0;
constexpr int SECOND_PARAM = 1;
constexpr int THIRD_PARAM = 2;
constexpr int FOURTH_PARAM = 3;

constexpr int MAX_STYLE_FORMAT_NUMBER = 3;

void ToLowerCase(std::string& str)
{
    for (auto& c : str) {
        c = tolower(c);
    }
}

std::vector<std::string> ParseFontFamily(const std::string& fontFamily)
{
    std::vector<std::string> fonts;
    std::stringstream ss(fontFamily);
    std::string token;
    while (std::getline(ss, token, ',')) {
        std::string font = std::string(token.begin(), token.end());
        font.erase(std::remove_if(font.begin(), font.end(), isspace), font.end());

        if (!font.empty()) {
            fonts.push_back(font);
        }
    }

    return fonts;
}

VerticalAlign StringToVerticalAlign(const std::string& align)
{
    if (align == "bottom") {
        return VerticalAlign::BOTTOM;
    }
    if (align == "middle") {
        return VerticalAlign::CENTER;
    }
    if (align == "top") {
        return VerticalAlign::TOP;
    }
    return VerticalAlign::NONE;
}

FontStyle StringToFontStyle(const std::string& fontStyle)
{
    return fontStyle == "italic" ? FontStyle::ITALIC : FontStyle::NORMAL;
}

SuperscriptStyle StringToSuperscriptStyle(const std::string& superscriptStyle)
{
    if (superscriptStyle == "superscript") {
        return SuperscriptStyle::SUPERSCRIPT;
    } else if (superscriptStyle == "subscript") {
        return SuperscriptStyle::SUBSCRIPT;
    } else if (superscriptStyle == "normal") {
        return SuperscriptStyle::NORMAL;
    }
    return SuperscriptStyle::NONE;
}

TextDecorationStyle StringToTextDecorationStyle(const std::string& textDecorationStyle)
{
    std::string value = StringUtils::TrimStr(textDecorationStyle);
    if (value == "dashed") {
        return TextDecorationStyle::DASHED;
    }
    if (value == "dotted") {
        return TextDecorationStyle::DOTTED;
    }
    if (value == "double") {
        return TextDecorationStyle::DOUBLE;
    }
    if (value == "solid") {
        return TextDecorationStyle::SOLID;
    }
    if (value == "wavy") {
        return TextDecorationStyle::WAVY;
    }

    return TextDecorationStyle::SOLID;
}

std::vector<TextDecoration> StringToTextDecoration(const std::string& textDecoration)
{
    std::istringstream ss(textDecoration);
    std::string tmp;
    std::vector<std::string> decorations;
    while (std::getline(ss, tmp, ' ')) {
        decorations.emplace_back(tmp);
    }
    std::vector<TextDecoration> result;
    for (const auto &its : decorations) {
        std::string value = StringUtils::TrimStr(its);
        TextDecoration decoration;
        if (value == "inherit") {
            decoration = TextDecoration::INHERIT;
        }
        if (value == "line-through") {
            decoration = TextDecoration::LINE_THROUGH;
        }
        if (value == "overline") {
            decoration = TextDecoration::OVERLINE;
        }
        if (value == "underline") {
            decoration = TextDecoration::UNDERLINE;
        }
        result.push_back(decoration);
    }
    return result;
}

ImageFit ConvertStrToFit(const std::string& fit)
{
    if (fit == "fill") {
        return ImageFit::FILL;
    }
    if (fit == "contain") {
        return ImageFit::CONTAIN;
    }
    if (fit == "cover") {
        return ImageFit::COVER;
    }
    if (fit == "scaledown") {
        return ImageFit::SCALE_DOWN;
    }
    if (fit == "none") {
        return ImageFit::NONE;
    }

    return ImageFit::CONTAIN;
}

HtmlToSpan::Styles HtmlToSpan::ParseStyleAttr(const std::string& style)
{
    Styles styles;
    if (style.find(':') == std::string::npos) {
        return styles;
    }
    std::regex pattern(R"(\s*([^:]+):([^;]+);?)");
    std::smatch match;
    std::string::const_iterator searchStart(style.begin());

    while (std::regex_search(searchStart, style.end(), match, pattern)) {
        if (match.size() < MAX_STYLE_FORMAT_NUMBER) {
            continue;
        }
        std::string key = std::regex_replace(match[1].str(), std::regex(R"(\s+)"), "");
        std::string value = std::regex_replace(match[2].str(), std::regex(R"(\s+)"), " ");
        ToLowerCase(key);
        styles.emplace_back(key, value);
        searchStart = match[0].second;
    }

    return styles;
}

template<class T>
T* HtmlToSpan::Get(StyleValue* styleValue) const
{
    auto v = std::get_if<T>(styleValue);
    if (v == nullptr) {
        return nullptr;
    }
    return static_cast<T*>(v);
}

// for example str = 0.00px
Dimension HtmlToSpan::FromString(const std::string& str)
{
    static const int32_t PERCENT_UNIT = 100;
    static const std::unordered_map<std::string, DimensionUnit> uMap {
        { "px", DimensionUnit::PX },
        { "vp", DimensionUnit::VP },
        { "fp", DimensionUnit::FP },
        { "%", DimensionUnit::PERCENT },
        { "lpx", DimensionUnit::LPX },
        { "auto", DimensionUnit::AUTO },
        { "rem", DimensionUnit::INVALID },
        { "em", DimensionUnit::INVALID },
    };

    double value = 0.0;
    DimensionUnit unit = DimensionUnit::VP;
    if (str.empty()) {
        LOGE("UITree |ERROR| empty string");
        return Dimension(NG::TEXT_DEFAULT_FONT_SIZE);
    }

    for (int32_t i = static_cast<int32_t>(str.length()) - 1; i >= 0; --i) {
        if (str[i] >= '0' && str[i] <= '9') {
            auto startIndex = i + 1;
            value = StringUtils::StringToDouble(str.substr(0, startIndex));
            startIndex = std::clamp(startIndex, 0, static_cast<int32_t>(str.length()));
            auto subStr = str.substr(startIndex);
            if (subStr == "pt") {
                value = static_cast<int>(value * PT_TO_PX + ROUND_TO_INT);
                break;
            }
            auto iter = uMap.find(subStr);
            if (iter != uMap.end()) {
                unit = iter->second;
            }
            value = unit == DimensionUnit::PERCENT ? value / PERCENT_UNIT : value;
            break;
        }
    }
    if (unit == DimensionUnit::PX) {
        return Dimension(value, DimensionUnit::VP);
    } else if (unit == DimensionUnit::INVALID) {
        return Dimension(NG::TEXT_DEFAULT_FONT_SIZE);
    }

    return Dimension(value, unit);
}

void HtmlToSpan::InitFont(
    const std::string& key, const std::string& value, const std::string& index, StyleValues& values)
{
    auto [ret, styleValue] = GetStyleValue<Font>(index, values);
    if (!ret) {
        return;
    }

    Font* font = Get<Font>(styleValue);
    if (font == nullptr) {
        return;
    }

    if (key == "color") {
        font->fontColor = ToSpanColor(value);
    } else if (key == "font-size") {
        font->fontSize = FromString(value);
    } else if (key == "font-weight") {
        font->fontWeight = StringUtils::StringToFontWeight(value);
    } else if (key == "font-style") {
        font->fontStyle = StringToFontStyle(value);
    } else if (key == "font-family") {
        font->fontFamilies = ParseFontFamily(value);
    } else if (key == "font-variant") { // not support
    } else if (key == "stroke-width") {
        font->strokeWidth = FromString(value);
    } else if (key == "stroke-color") {
        font->strokeColor = ToSpanColor(value);
    } else if (key == "font-superscript") {
        font->superscript = StringToSuperscriptStyle(value);
    }
}

bool HtmlToSpan::IsFontAttr(const std::string& key)
{
    if (key == "font-size" || key == "font-weight" || key == "font-style" || key == "font-family" ||
        key == "color" || key == "stroke-width" || key == "stroke-color" || key == "font-superscript") {
        return true;
    }
    return false;
}

bool HtmlToSpan::IsBackgroundColorAttr(const std::string& key) const
{
    return key == "background-color";
}

bool HtmlToSpan::IsForegroundColorAttr(const std::string& key) const
{
    return key == "foreground-color";
}

void HtmlToSpan::InitBackgroundColor(
    const std::string& key, const std::string& value, const std::string& index, StyleValues& values)
{
    auto [ret, styleValue] = GetStyleValue<TextBackgroundStyle>(index, values);
    if (!ret) {
        return;
    }

    TextBackgroundStyle* style = Get<TextBackgroundStyle>(styleValue);
    if (style == nullptr) {
        return;
    }

    if (key == "background-color") {
        style->backgroundColor = ToSpanColor(value);
    }
}

void HtmlToSpan::InitForegroundColor(
    const std::string& key, const std::string& value, const std::string& index, StyleValues& values)
{
    auto [ret, styleValue] = GetStyleValue<Font>(index, values);
    if (!ret) {
        return;
    }

    Font* font = Get<Font>(styleValue);
    if (font == nullptr) {
        return;
    }

    if (key == "foreground-color") {
        font->fontColor = ToSpanColor(value);
    }
}

void HtmlToSpan::InitParagraph(
    const std::string& key, const std::string& value, const std::string& index, StyleValues& values)
{
    auto [ret, styleValue] = GetStyleValue<SpanParagraphStyle>(index, values);
    if (!ret) {
        return;
    }

    SpanParagraphStyle* style = Get<SpanParagraphStyle>(styleValue);
    if (style == nullptr) {
        return;
    }

    if (key == "text-align") {
        style->align = StringToTextAlign(value);
    } else if (key == "vertical-align") {
        style->textVerticalAlign = StringToTextVerticalAlign(value);
    } else if (key == "word-break") {
        style->wordBreak = StringToWordBreak(value);
    } else if (key == "text-overflow") {
        style->textOverflow = StringToTextOverflow(value);
    } else if (IsTextIndentAttr(key)) {
        style->textIndent = FromString(value);
    } else {
    }
}

bool HtmlToSpan::IsParagraphAttr(const std::string& key)
{
    if (key == "text-align" || key == "word-break" || key == "text-overflow" || key == "text-indent") {
        return true;
    }
    return false;
}

bool HtmlToSpan::IsDecorationLine(const std::string& key)
{
    if (key == "none" || key == "underline" || key == "overline" || key == "line-through" || key == "blink" ||
        key == "inherit") {
        return true;
    }
    return false;
}

bool HtmlToSpan::IsDecorationStyle(const std::string& key)
{
    if (key == "solid" || key == "double" || key == "dotted" || key == "dashed" || key == "wavy" || key == "inherit") {
        return true;
    }
    return false;
}

void HtmlToSpan::InitDecoration(
    const std::string& key, const std::string& value, const std::string& index, StyleValues& values)
{
    auto [ret, styleValue] = GetStyleValue<DecorationSpanParam>(index, values);
    if (!ret) {
        return;
    }
    DecorationSpanParam* decoration = Get<DecorationSpanParam>(styleValue);
    if (decoration == nullptr) {
        return;
    }

    if (key == "text-decoration-line") {
        decoration->decorationType = StringToTextDecoration(value);
    } else if (key == "text-decoration-style") {
        decoration->decorationSytle = StringToTextDecorationStyle(value);
    } else if (key == "text-decoration-color") {
        decoration->color = ToSpanColor(value);
    } else if (key == "text-decoration-thickness") { // not supported: html has unit while lineThicknessScale is float
    } else if (key == "text-decoration") {
        std::istringstream ss1(value);
        std::string word;
        std::vector<std::string> words;
        while (ss1 >> word) {
            words.push_back(word);
            if (IsDecorationLine(word)) {
                decoration->decorationType = StringToTextDecoration(word);
            } else if (IsDecorationStyle(word)) {
                decoration->decorationSytle = StringToTextDecorationStyle(word);
            } else {
                decoration->color = ToSpanColor(word);
            }
        }
    }
}

bool HtmlToSpan::IsDecorationAttr(const std::string& key)
{
    return key.compare(0, strlen("text-decoration"), "text-decoration") == 0;
}

template<class T>
void HtmlToSpan::InitDimension(
    const std::string& key, const std::string& value, const std::string& index, StyleValues& values)
{
    if (value.compare(0, strlen("normal"), "normal") == 0) {
        return;
    }
    auto [ret, styleValue] = GetStyleValue<T>(index, values);
    if (!ret) {
        return;
    }
    T* obj = Get<T>(styleValue);
    if (obj == nullptr) {
        return;
    }
    obj->dimension = FromString(value);
}

void HtmlToSpan::InitLineHeight(const std::string& key, const std::string& value, StyleValues& values)
{
    auto [unit, size] = GetUnitAndSize(value);
    if (!unit.empty()) {
        InitDimension<LineHeightSpanSparam>(key, value, "line-height", values);
        return;
    }

    auto it = values.find("font");
    if (it == values.end()) {
        return;
    }
    Font* font = Get<Font>(&it->second);
    if (font != nullptr) {
        size = size * font->fontSize->Value();
        InitDimension<LineHeightSpanSparam>(key, std::to_string(size) + unit, "line-height", values);
    }
}

bool HtmlToSpan::IsLetterSpacingAttr(const std::string& key)
{
    return key.compare(0, strlen("letter-spacing"), "letter-spacing") == 0;
}

Color HtmlToSpan::ToSpanColor(const std::string& value)
{
    std::smatch matches;
    std::string color = value;
    std::string tmp = value;
    tmp.erase(std::remove(tmp.begin(), tmp.end(), ' '), tmp.end());
    auto regStr = "#[0-9A-Fa-f]{7,8}";
    constexpr auto tmpLeastLength = 3;
    if (std::regex_match(tmp, matches, std::regex(regStr)) && tmp.length() >= tmpLeastLength) {
        auto rgb = tmp.substr(1);
        // remove last 2 character rgba -> argb
        rgb.erase(rgb.length() - 2, 2);
        auto alpha = tmp.substr(tmp.length() - 2);
        color = "#" + alpha + rgb;
    }

    return Color::FromString(color);
}

void HtmlToSpan::InitTextShadow(
    const std::string& key, const std::string& value, const std::string& index, StyleValues& values)
{
    auto [ret, styleValue] = GetStyleValue<std::vector<Shadow>>(index, values);
    if (!ret) {
        return;
    }
    std::vector<Shadow>* shadow = Get<std::vector<Shadow>>(styleValue);
    if (shadow == nullptr) {
        return;
    }
    std::istringstream ss(value);
    std::string tmp;
    std::vector<std::vector<std::string>> shadows;
    while (std::getline(ss, tmp, ',')) {
        std::istringstream iss(tmp);
        std::string word;
        std::vector<std::string> words;
        while (iss >> word) {
            words.emplace_back(word);
        }
        if (words.size() > FOUR_PARAM || words.size() < TWO_PARAM) {
            return;
        }
        shadows.emplace_back(words);
    }
    for (const auto &its : shadows) {
        std::vector<std::string> attribute(FOUR_PARAM);
        uint8_t num = 0;
        for (const auto &it : its) {
            if (IsLength(it)) {
                attribute[num] = it;
                num++;
                continue;
            }
            attribute[FOURTH_PARAM] = it;
        }
        Shadow textShadow;
        InitShadow(textShadow, attribute);
        shadow->emplace_back(std::move(textShadow));
    }
}

void HtmlToSpan::InitShadow(Shadow &textShadow, std::vector<std::string> &attribute)
{
    if (!attribute[FIRST_PARAM].empty()) {
        textShadow.SetOffsetX(FromString(attribute[FIRST_PARAM]).Value());
    }
    if (!attribute[SECOND_PARAM].empty()) {
        textShadow.SetOffsetY(FromString(attribute[SECOND_PARAM]).Value());
    }
    if (!attribute[THIRD_PARAM].empty()) {
        textShadow.SetBlurRadius(FromString(attribute[THIRD_PARAM]).Value());
    }
    if (!attribute[FOURTH_PARAM].empty()) {
        textShadow.SetColor(ToSpanColor(attribute[FOURTH_PARAM]));
    }
}

bool HtmlToSpan::IsLength(const std::string& str)
{
    return !str.empty() &&
        (std::all_of(str.begin(), str.end(), ::isdigit) || str.find("px") != std::string::npos);
}

bool HtmlToSpan::IsTextShadowAttr(const std::string& key)
{
    return key.compare(0, strlen("text-shadow"), "text-shadow") == 0;
}

bool HtmlToSpan::IsTextIndentAttr(const std::string& key)
{
    return key.compare(0, strlen("text-indent"), "text-indent") == 0;
}

bool HtmlToSpan::IsLineHeightAttr(const std::string& key)
{
    return key.compare(0, strlen("line-height"), "line-height") == 0;
}

bool HtmlToSpan::IsPaddingAttr(const std::string& key)
{
    if (key == "padding" || key == "padding-top" || key == "padding-right" || key == "padding-bottom" ||
        key == "padding-left") {
        return true;
    }
    return false;
}

bool HtmlToSpan::IsMarginAttr(const std::string& key)
{
    if (key == "margin" || key == "margin-top" || key == "margin-right" || key == "margin-bottom" ||
        key == "margin-left") {
        return true;
    }
    return false;
}

bool HtmlToSpan::IsBorderAttr(const std::string& key)
{
    if (key == "border-radius" || key == "border-top-left-radius" || key == "border-top-right-radius" ||
        key == "border-bottom-right-radius" || key == "border-bottom-left-radius") {
        return true;
    }
    return false;
}

void HtmlToSpan::SetPaddingOption(const std::string& key, const std::string& value, ImageSpanOptions& options)
{
    if (!options.imageAttribute->paddingProp) {
        options.imageAttribute->paddingProp = std::make_optional<NG::PaddingProperty>();
    }
    auto& paddings = options.imageAttribute->paddingProp;
    if (key == "padding") {
        std::istringstream ss(value);
        std::string word;
        std::vector<std::string> words;
        while (ss >> word) {
            words.push_back(word);
        }

        size_t size = words.size();
        if (size == ONE_PARAM) {
            paddings->top = NG::CalcLength(FromString(words[TOP_PARAM]));
            paddings->right = NG::CalcLength(FromString(words[TOP_PARAM]));
            paddings->bottom = NG::CalcLength(FromString(words[TOP_PARAM]));
            paddings->left = NG::CalcLength(FromString(words[TOP_PARAM]));
        } else if (size == TWO_PARAM) {
            paddings->top = NG::CalcLength(FromString(words[TOP_PARAM]));
            paddings->right = NG::CalcLength(FromString(words[RIGHT_PARAM]));
            paddings->bottom = NG::CalcLength(FromString(words[TOP_PARAM]));
            paddings->left = NG::CalcLength(FromString(words[RIGHT_PARAM]));
        } else if (size == THREE_PARAM) {
            paddings->top = NG::CalcLength(FromString(words[TOP_PARAM]));
            paddings->right = NG::CalcLength(FromString(words[RIGHT_PARAM]));
            paddings->bottom = NG::CalcLength(FromString(words[BOTTOM_PARAM]));
            paddings->left = NG::CalcLength(FromString(words[RIGHT_PARAM]));
        } else if (size == FOUR_PARAM) {
            paddings->top = NG::CalcLength(FromString(words[TOP_PARAM]));
            paddings->right = NG::CalcLength(FromString(words[RIGHT_PARAM]));
            paddings->bottom = NG::CalcLength(FromString(words[BOTTOM_PARAM]));
            paddings->left = NG::CalcLength(FromString(words[LEFT_PARAM]));
        }
    } else if (key == "padding-top") {
        paddings->top = NG::CalcLength(FromString(value));
    } else if (key == "padding-right") {
        paddings->right = NG::CalcLength(FromString(value));
    } else if (key == "padding-bottom") {
        paddings->bottom = NG::CalcLength(FromString(value));
    } else if (key == "padding-left") {
        paddings->left = NG::CalcLength(FromString(value));
    }
}
void HtmlToSpan::SetMarginOption(const std::string& key, const std::string& value, ImageSpanOptions& options)
{
    if (!options.imageAttribute->marginProp) {
        options.imageAttribute->marginProp = std::make_optional<NG::MarginProperty>();
    }
    auto& marginProp = options.imageAttribute->marginProp;
    if (key == "margin") {
        std::istringstream ss(value);
        std::string word;
        std::vector<std::string> words;
        while (ss >> word) {
            words.push_back(word);
        }

        size_t size = words.size();
        if (size == ONE_PARAM) {
            marginProp->top = NG::CalcLength(FromString(words[TOP_PARAM]));
            marginProp->right = NG::CalcLength(FromString(words[TOP_PARAM]));
            marginProp->bottom = NG::CalcLength(FromString(words[TOP_PARAM]));
            marginProp->left = NG::CalcLength(FromString(words[TOP_PARAM]));
        } else if (size == TWO_PARAM) {
            marginProp->top = NG::CalcLength(FromString(words[TOP_PARAM]));
            marginProp->right = NG::CalcLength(FromString(words[RIGHT_PARAM]));
            marginProp->bottom = NG::CalcLength(FromString(words[TOP_PARAM]));
            marginProp->left = NG::CalcLength(FromString(words[RIGHT_PARAM]));
        } else if (size == THREE_PARAM) {
            marginProp->top = NG::CalcLength(FromString(words[TOP_PARAM]));
            marginProp->right = NG::CalcLength(FromString(words[RIGHT_PARAM]));
            marginProp->bottom = NG::CalcLength(FromString(words[BOTTOM_PARAM]));
            marginProp->left = NG::CalcLength(FromString(words[RIGHT_PARAM]));
        } else if (size == FOUR_PARAM) {
            marginProp->top = NG::CalcLength(FromString(words[TOP_PARAM]));
            marginProp->right = NG::CalcLength(FromString(words[RIGHT_PARAM]));
            marginProp->bottom = NG::CalcLength(FromString(words[BOTTOM_PARAM]));
            marginProp->left = NG::CalcLength(FromString(words[LEFT_PARAM]));
        }
    } else if (key == "margin-top") {
        marginProp->top = NG::CalcLength(FromString(value));
    } else if (key == "margin-right") {
        marginProp->right = NG::CalcLength(FromString(value));
    } else if (key == "margin-bottom") {
        marginProp->bottom = NG::CalcLength(FromString(value));
    } else if (key == "margin-left") {
        marginProp->left = NG::CalcLength(FromString(value));
    }
}
void HtmlToSpan::SetBorderOption(const std::string& key, const std::string& value, ImageSpanOptions& options)
{
    if (!options.imageAttribute->borderRadius) {
        options.imageAttribute->borderRadius = std::make_optional<NG::BorderRadiusProperty>();
        options.imageAttribute->borderRadius->multiValued = true;
    }
    auto& borderRadius = options.imageAttribute->borderRadius;
    if (key == "border-radius") {
        std::istringstream ss(value);
        std::string word;
        std::vector<std::string> words;
        while (ss >> word) {
            words.push_back(word);
        }
        size_t size = words.size();
        if (size == ONE_PARAM) {
            borderRadius->radiusTopLeft = FromString(words[TOP_PARAM]);
            borderRadius->radiusTopRight = FromString(words[TOP_PARAM]);
            borderRadius->radiusBottomRight = FromString(words[TOP_PARAM]);
            borderRadius->radiusBottomLeft = FromString(words[TOP_PARAM]);
        } else if (size == TWO_PARAM) {
            borderRadius->radiusTopLeft = FromString(words[TOP_PARAM]);
            borderRadius->radiusTopRight = FromString(words[RIGHT_PARAM]);
            borderRadius->radiusBottomRight = FromString(words[TOP_PARAM]);
            borderRadius->radiusBottomLeft = FromString(words[RIGHT_PARAM]);
        } else if (size == THREE_PARAM) {
            borderRadius->radiusTopLeft = FromString(words[TOP_PARAM]);
            borderRadius->radiusTopRight = FromString(words[RIGHT_PARAM]);
            borderRadius->radiusBottomRight = FromString(words[BOTTOM_PARAM]);
            borderRadius->radiusBottomLeft = FromString(words[RIGHT_PARAM]);
        } else if (size == FOUR_PARAM) {
            borderRadius->radiusTopLeft = FromString(words[TOP_PARAM]);
            borderRadius->radiusTopRight = FromString(words[RIGHT_PARAM]);
            borderRadius->radiusBottomRight = FromString(words[BOTTOM_PARAM]);
            borderRadius->radiusBottomLeft = FromString(words[LEFT_PARAM]);
        }
    } else if (key == "border-top-left-radius") {
        borderRadius->radiusTopLeft = FromString(value);
    } else if (key == "border-top-right-radius") {
        borderRadius->radiusTopRight = FromString(value);
    } else if (key == "border-bottom-right-radius") {
        borderRadius->radiusBottomRight = FromString(value);
    } else if (key == "border-bottom-left-radius") {
        borderRadius->radiusBottomLeft = FromString(value);
    }
}
void HtmlToSpan::HandleImgSpanOption(const Styles& styleMap, ImageSpanOptions& options)
{
    for (const auto& [key, value] : styleMap) {
        auto trimVal = StringUtils::TrimStr(value);
        if (IsPaddingAttr(key)) {
            SetPaddingOption(key, trimVal, options);
        } else if (IsMarginAttr(key)) {
            SetMarginOption(key, trimVal, options);
        } else if (IsBorderAttr(key)) {
            SetBorderOption(key, trimVal, options);
        } else if (key == "object-fit") {
            options.imageAttribute->objectFit = ConvertStrToFit(trimVal);
        } else if (key == "vertical-align") {
            options.imageAttribute->verticalAlign = StringToVerticalAlign(trimVal);
        } else if (key == "width" || key == "height") {
            HandleImageSize(key, trimVal, options);
        } else if (key == "sync-load") {
            options.imageAttribute->syncLoad = V2::ConvertStringToBool(trimVal);
        }
    }
}
void HtmlToSpan::HandleImagePixelMap(const std::string& src, ImageSpanOptions& option)
{
    if (src.empty()) {
        return;
    }
    NG::LoadNotifier loadNotifier(nullptr, nullptr, nullptr);
    RefPtr<NG::ImageLoadingContext> ctx =
        AceType::MakeRefPtr<NG::ImageLoadingContext>(ImageSourceInfo(src), std::move(loadNotifier), true);
    CHECK_NULL_VOID(ctx);
    ctx->LoadImageData();
    ctx->MakeCanvasImageIfNeed(ctx->GetImageSize(), true, ImageFit::NONE);
    auto image = ctx->MoveCanvasImage();
    if (image != nullptr) {
        auto pixelMap = image->GetPixelMap();
        if (pixelMap) {
            option.imagePixelMap = pixelMap;
        }
    }
    if (option.imagePixelMap.has_value() && option.imagePixelMap.value() != nullptr) {
        auto pixel = option.imagePixelMap.value();
        LOGI("img height: %{public}d, width: %{public}d, size:%{public}d", pixel->GetHeight(),
            pixel->GetWidth(), pixel->GetByteCount());
    } else {
        option.image = src;
    }
}

void HtmlToSpan::HandleImageSize(const std::string& key, const std::string& value, ImageSpanOptions& options)
{
    if (!options.imageAttribute->size) {
        options.imageAttribute->size = std::make_optional<ImageSpanSize>();
    }
    if (key == "width") {
        options.imageAttribute->size->width = FromString(value);
    } else {
        options.imageAttribute->size->height = FromString(value);
    }
}

void HtmlToSpan::MakeImageSpanOptions(const std::string& key, const std::string& value, ImageSpanOptions& options)
{
    if (key == "src") {
        options.image = value;
        HandleImagePixelMap(value, options);
    } else if (key == "style") {
        Styles styleMap = ParseStyleAttr(value);
        HandleImgSpanOption(styleMap, options);
    } else if (key == "width" || key == "height") {
        HandleImageSize(key, value, options);
    }
}

TextAlign HtmlToSpan::StringToTextAlign(const std::string& value)
{
    if (value == "left") {
        return TextAlign::LEFT;
    }
    if (value == "right") {
        return TextAlign::RIGHT;
    }
    if (value == "center") {
        return TextAlign::CENTER;
    }
    if (value == "justify") {
        return TextAlign::JUSTIFY;
    }
    return TextAlign::LEFT;
}

TextVerticalAlign HtmlToSpan::StringToTextVerticalAlign(const std::string& value)
{
    if (value == "baseline") {
        return TextVerticalAlign::BASELINE;
    }
    if (value == "bottom") {
        return TextVerticalAlign::BOTTOM;
    }
    if (value == "middle") {
        return TextVerticalAlign::CENTER;
    }
    if (value == "top") {
        return TextVerticalAlign::TOP;
    }
    return TextVerticalAlign::BASELINE;
}

WordBreak HtmlToSpan::StringToWordBreak(const std::string& value)
{
    if (value == "normal") {
        return WordBreak::NORMAL;
    }
    if (value == "break-all") {
        return WordBreak::BREAK_ALL;
    }
    if (value == "keep-all") {
        return WordBreak::BREAK_WORD;
    }
    return WordBreak::NORMAL;
}

TextOverflow HtmlToSpan::StringToTextOverflow(const std::string& value)
{
    if (value == "clip") {
        return TextOverflow::CLIP;
    }
    if (value == "ellipsis") {
        return TextOverflow::ELLIPSIS;
    }
    return TextOverflow::NONE;
}

template<class T>
std::pair<bool, HtmlToSpan::StyleValue*> HtmlToSpan::GetStyleValue(
    const std::string& key, std::map<std::string, StyleValue>& values)
{
    auto it = values.find(key);
    if (it == values.end()) {
        StyleValue value = T();
        it = values.emplace(key, value).first;
    }

    if (it == values.end()) {
        return std::make_pair(false, nullptr);
    }

    return std::make_pair(true, &it->second);
}

void HtmlToSpan::ToParagraphSpan(xmlNodePtr node, size_t len, size_t& pos, std::vector<SpanInfo>& spanInfos)
{
    SpanInfo info;
    info.type = HtmlType::PARAGRAPH;
    info.start = pos;
    info.end = pos + len;
    xmlAttrPtr curNode = node->properties;
    if (curNode == nullptr) {
        SpanParagraphStyle style;
        info.values.emplace_back(style);
    } else {
        for (; curNode; curNode = curNode->next) {
            auto styles = ToTextSpanStyle(curNode);
            for (auto [key, value] : styles) {
                info.values.emplace_back(value);
            }
        }
    }

    spanInfos.emplace_back(std::move(info));
}

std::pair<std::string, double> HtmlToSpan::GetUnitAndSize(const std::string& str)
{
    double value = 0.0;
    for (int32_t i = static_cast<int32_t>(str.length() - 1); i >= 0; --i) {
        if (str[i] >= '0' && str[i] <= '9') {
            value = StringUtils::StringToDouble(str.substr(0, i + 1));
            auto subStr = str.substr(i + 1);
            return { subStr, value };
        }
    }
    return { "", value };
}

std::map<std::string, HtmlToSpan::StyleValue> HtmlToSpan::ToTextSpanStyle(xmlAttrPtr curNode)
{
    auto attrContent = xmlGetProp(curNode->parent, curNode->name);
    if (attrContent == nullptr) {
        return {};
    }
    std::string strStyle(reinterpret_cast<const char*>(attrContent));
    xmlFree(attrContent);
    Styles styleMap = ParseStyleAttr(strStyle);
    std::map<std::string, StyleValue> styleValues;
    for (auto& [key, value] : styleMap) {
        auto trimVal = StringUtils::TrimStr(value);
        if (IsFontAttr(key)) {
            InitFont(key, trimVal, "font", styleValues);
        } else if (IsForegroundColorAttr(key)) {
            InitForegroundColor(key, trimVal, "font", styleValues);
        } else if (IsDecorationAttr(key)) {
            InitDecoration(key, trimVal, "decoration", styleValues);
        } else if (IsLetterSpacingAttr(key)) {
            InitDimension<LetterSpacingSpanParam>(key, trimVal, "letterSpacing", styleValues);
        } else if (IsTextShadowAttr(key)) {
            InitTextShadow(key, trimVal, "shadow", styleValues);
        } else if (IsLineHeightAttr(key)) {
            InitLineHeight(key, trimVal, styleValues);
        } else if (IsParagraphAttr(key)) {
            InitParagraph(key, trimVal, "paragrap", styleValues);
        } else if (IsBackgroundColorAttr(key)) {
            InitBackgroundColor(key, trimVal, "backgroundColor", styleValues);
        }
    }

    return styleValues;
}

void HtmlToSpan::AddStyleSpan(const std::string& element, SpanInfo& info)
{
    std::map<std::string, StyleValue> styles;
    if (element == "strong" || element == "b") {
        InitFont("font-weight", "bold", "font", styles);
    } else if (element == "sup") {
        InitFont("font-superscript", "superscript", "font", styles);
    } else if (element == "sub") {
        InitFont("font-superscript", "subscript", "font", styles);
    } else if (element == "del" || element == "s") {
        InitDecoration("text-decoration-line", "line-through", "decoration", styles);
    } else if (element == "u") {
        InitDecoration("text-decoration-line", "underline", "decoration", styles);
    } else if (element == "i" || element == "em") {
        InitFont("font-style", "italic", "font", styles);
    }

    for (auto [key, value] : styles) {
        info.values.emplace_back(value);
    }
}

void HtmlToSpan::ToTextSpan(
    const std::string& element, xmlNodePtr node, size_t len, size_t& pos, std::vector<SpanInfo>& spanInfos)
{
    SpanInfo info;
    info.type = HtmlType::TEXT;
    info.start = pos;
    info.end = pos + len;
    xmlAttrPtr curNode = node->properties;
    for (; curNode; curNode = curNode->next) {
        auto styles = ToTextSpanStyle(curNode);
        for (auto [key, value] : styles) {
            info.values.emplace_back(value);
        }
    }
    if (!element.empty()) {
        AddStyleSpan(element, info);
    }
    if (info.values.empty()) {
        return;
    }
    spanInfos.emplace_back(std::move(info));
}

void HtmlToSpan::ToAnchorSpan(xmlNodePtr node, size_t len, size_t& pos, std::vector<SpanInfo>& spanInfos)
{
    SpanInfo info;
    info.type = HtmlType::ANCHOR;
    info.start = pos;
    info.end = pos + len;
    xmlAttrPtr curNode = node->properties;
    for (; curNode; curNode = curNode->next) {
        std::string attrName = reinterpret_cast<const char*>(curNode->name);
        if (attrName == "href") {
            auto attrContent = xmlGetProp(curNode->parent, curNode->name);
            if (attrContent != nullptr) {
                std::string hrefValue = reinterpret_cast<const char*>(attrContent);
                xmlFree(attrContent);
                info.values.emplace_back(hrefValue);
            }
        }
        if (attrName == "style") {
            auto styles = ToTextSpanStyle(curNode);
            for (auto [key, value] : styles) {
                info.values.emplace_back(value);
            }
        }
    }
    spanInfos.emplace_back(std::move(info));
}

void HtmlToSpan::ToImageOptions(const std::map<std::string, std::string>& styles, ImageSpanOptions& option)
{
    option.imageAttribute = std::make_optional<ImageSpanAttribute>();
    for (auto& [key, value] : styles) {
        MakeImageSpanOptions(key, value, option);
    }
}

void HtmlToSpan::ToImage(xmlNodePtr node, size_t len, size_t& pos, std::vector<SpanInfo>& spanInfos,
    bool isProcessImageOptions)
{
    std::map<std::string, std::string> styleMap;
    xmlAttrPtr curNode = node->properties;
    for (; curNode; curNode = curNode->next) {
        auto attrContent = xmlGetProp(curNode->parent, curNode->name);
        if (attrContent != nullptr) {
            styleMap[reinterpret_cast<const char*>(curNode->name)] = reinterpret_cast<const char*>(attrContent);
            xmlFree(attrContent);
        }
    }

    ImageSpanOptions option;
    if (isProcessImageOptions) {
        ToImageOptions(styleMap, option);
    }

    SpanInfo info;
    info.type = HtmlType::IMAGE;
    info.start = pos;
    info.end = pos + len;
    info.values.emplace_back(std::move(option));
    spanInfos.emplace_back(std::move(info));
}

void HtmlToSpan::ToSpan(
    xmlNodePtr curNode, size_t& pos, std::string& allContent, std::vector<SpanInfo>& spanInfos,
    bool isNeedLoadPixelMap)
{
    size_t curNodeLen = 0;
    if (curNode->content) {
        std::string curNodeContent = reinterpret_cast<const char*>(curNode->content);
        allContent += curNodeContent;
        curNodeLen = StringUtils::ToWstring(curNodeContent).length();
    }

    std::string htmlTag = reinterpret_cast<const char*>(curNode->name);
    size_t childPos = pos + curNodeLen;
    ParseHtmlToSpanInfo(curNode->children, childPos, allContent, spanInfos);
    if (curNode->type == XML_ELEMENT_NODE) {
        if (htmlTag == "p") {
            if (curNode->parent == nullptr || curNode->parent->type != XML_ELEMENT_NODE ||
                xmlStrcmp(curNode->parent->name, (const xmlChar*)"span") != 0) {
                // The <p> contained in <span> is discarded. It is not considered as a standard writing method.
                allContent += "\n";
                childPos++;
                ToParagraphSpan(curNode, childPos - pos, pos, spanInfos);
            }
        } else if (htmlTag == "img") {
            childPos++;
            ToImage(curNode, childPos - pos, pos, spanInfos, isNeedLoadPixelMap);
        } else if (htmlTag == "a") {
            ToAnchorSpan(curNode, childPos - pos, pos, spanInfos);
        } else if (htmlTag == "br") {
            allContent += "\n";
            childPos++;
        } else {
            ToTextSpan(htmlTag, curNode, childPos - pos, pos, spanInfos);
        }
    }
    pos = childPos;
}

void HtmlToSpan::ParseHtmlToSpanInfo(
    xmlNodePtr node, size_t& pos, std::string& allContent, std::vector<SpanInfo>& spanInfos, bool isNeedLoadPixelMap)
{
    xmlNodePtr curNode = nullptr;
    for (curNode = node; curNode; curNode = curNode->next) {
        if (curNode->type == XML_ELEMENT_NODE || curNode->type == XML_TEXT_NODE) {
            ToSpan(curNode, pos, allContent, spanInfos, isNeedLoadPixelMap);
        }
    }
}

void HtmlToSpan::PrintSpanInfos(const std::vector<SpanInfo>& spanInfos)
{
    for (auto& info : spanInfos) {
        LOGI("span type %{public}d start:%{public}zu end:%{public}zu, style size:%{public}zu",
            static_cast<int>(info.type), info.start, info.end, info.values.size());
    }
}

void HtmlToSpan::AfterProcSpanInfos(std::vector<SpanInfo>& spanInfos)
{
    std::vector<std::pair<size_t, size_t>> paragraphPos;
    for (auto& info : spanInfos) {
        if (info.type == HtmlType::PARAGRAPH) {
            paragraphPos.push_back({ info.start, info.end });
        }
    }

    for (auto& pos : paragraphPos) {
        for (auto& info : spanInfos) {
            if (info.type != HtmlType::PARAGRAPH && info.type != HtmlType::IMAGE && pos.second == info.end + 1) {
                info.end += 1;
                break;
            }
        }
    }
}

RefPtr<SpanBase> HtmlToSpan::CreateSpan(size_t index, const SpanInfo& info, StyleValue& value)
{
    if (index == static_cast<uint32_t>(StyleIndex::STYLE_FONT)) {
        return MakeSpan<Font, FontSpan>(info, value);
    }

    if (index == static_cast<uint32_t>(StyleIndex::STYLE_DECORATION)) {
        return MakeDecorationSpan(info, value);
    }

    if (index == static_cast<uint32_t>(StyleIndex::STYLE_BASELINE)) {
        return MakeDimensionSpan<BaseLineSpanParam, BaselineOffsetSpan>(info, value);
    }

    if (index == static_cast<uint32_t>(StyleIndex::STYLE_LETTERSPACE)) {
        return MakeDimensionSpan<LetterSpacingSpanParam, LetterSpacingSpan>(info, value);
    }

    if (index == static_cast<uint32_t>(StyleIndex::STYLE_LINEHEIGHT)) {
        return MakeDimensionSpan<LineHeightSpanSparam, LineHeightSpan>(info, value);
    }

    if (index == static_cast<uint32_t>(StyleIndex::STYLE_SHADOWS)) {
        return MakeSpan<std::vector<Shadow>, TextShadowSpan>(info, value);
    }

    if (index == static_cast<uint32_t>(StyleIndex::STYLE_PARAGRAPH)) {
        return MakeSpan<SpanParagraphStyle, ParagraphStyleSpan>(info, value);
    }

    if (index == static_cast<uint32_t>(StyleIndex::STYLE_BACKGROUND_COLOR)) {
        return MakeSpan<TextBackgroundStyle, BackgroundColorSpan>(info, value);
    }

    if (index == static_cast<uint32_t>(StyleIndex::STYLE_URL)) {
        return MakeSpan<std::string, UrlSpan>(info, value);
    }

    return nullptr;
}

template<class T, class P>
RefPtr<SpanBase> HtmlToSpan::MakeSpan(const SpanInfo& info, StyleValue& value)
{
    auto style = Get<T>(&value);
    if (style != nullptr) {
        return AceType::MakeRefPtr<P>(*style, info.start, info.end);
    }

    return nullptr;
}

template<class T, class P>
RefPtr<SpanBase> HtmlToSpan::MakeDimensionSpan(const SpanInfo& info, StyleValue& value)
{
    auto style = Get<T>(&value);
    if (style != nullptr) {
        return AceType::MakeRefPtr<P>(style->dimension, info.start, info.end);
    }

    return nullptr;
}

RefPtr<SpanBase> HtmlToSpan::MakeDecorationSpan(const SpanInfo& info, StyleValue& value)
{
    auto style = Get<DecorationSpanParam>(&value);
    std::optional<TextDecorationOptions> options = TextDecorationOptions();
    if (style != nullptr) {
        // Enable multi-decoration line support by default
        options->enableMultiType = true;
        return AceType::MakeRefPtr<DecorationSpan>(
            std::vector<TextDecoration>({style->decorationType}), style->color,
            style->decorationSytle, 1.0f, options, info.start, info.end, nullptr);
    }

    return nullptr;
}

void HtmlToSpan::AddSpans(const SpanInfo& info, RefPtr<MutableSpanString> mutableSpan)
{
    for (auto value : info.values) {
        size_t index = value.index();
        RefPtr<SpanBase> span;
        if (index >= 0 && index < static_cast<size_t>(StyleIndex::STYLE_MAX)) {
            span = CreateSpan(index, info, value);
        }
        if (span != nullptr) {
            mutableSpan->AddSpan(span, true, true, false);
        }
    }
}

void HtmlToSpan::AddImageSpans(const SpanInfo& info, RefPtr<MutableSpanString> mutableSpan)
{
    for (auto value : info.values) {
        auto style = Get<ImageSpanOptions>(&value);
        if (style == nullptr) {
            continue;
        }
        auto span = AceType::MakeRefPtr<MutableSpanString>(*style);
        mutableSpan->InsertSpanString(info.start, span);
    }
}

RefPtr<MutableSpanString> HtmlToSpan::GenerateSpans(
    const std::string& allContent, const std::vector<SpanInfo>& spanInfos)
{
    auto mutableSpan = AceType::MakeRefPtr<MutableSpanString>(UtfUtils::Str8DebugToStr16(allContent));
    if (spanInfos.empty()) {
        return mutableSpan;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(spanInfos.size()); ++i) {
        auto info = spanInfos[i];
        if (info.type == HtmlType::IMAGE) {
            AddImageSpans(info, mutableSpan);
        }
    }
    for (int32_t i = static_cast<int32_t>(spanInfos.size()) - 1; i >= 0; --i) {
        auto info = spanInfos[i];
        if (info.type == HtmlType::PARAGRAPH) {
            AddSpans(info, mutableSpan);
        } else if (info.type != HtmlType::IMAGE) {
            AddSpans(info, mutableSpan);
        }
    }
    return mutableSpan;
}

RefPtr<MutableSpanString> HtmlToSpan::ToSpanString(const std::string& html, const bool isNeedLoadPixelMap)
{
    htmlDocPtr doc = htmlReadMemory(html.c_str(), html.length(), nullptr, "UTF-8", 0);
    if (doc == nullptr) {
        return nullptr;
    }

    auto docSharedPtr = std::shared_ptr<xmlDoc>(doc, [](htmlDocPtr doc) { xmlFreeDoc(doc); });
    if (docSharedPtr == nullptr) {
        return nullptr;
    }

    xmlNode* root = xmlDocGetRootElement(docSharedPtr.get());
    if (root == nullptr) {
        return nullptr;
    }

    size_t pos = 0;
    std::string content;
    std::vector<SpanInfo> spanInfos;
    ParseHtmlToSpanInfo(root, pos, content, spanInfos, isNeedLoadPixelMap);
    AfterProcSpanInfos(spanInfos);
    PrintSpanInfos(spanInfos);
    return GenerateSpans(content, spanInfos);
}

RefPtr<MutableSpanString> HtmlUtils::FromHtml(const std::string& html)
{
    HtmlToSpan hts;
    auto styledString = hts.ToSpanString(html);
    return styledString;
}
} // namespace OHOS::Ace
