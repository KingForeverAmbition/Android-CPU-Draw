/*
 * CPU-Draw - Text Rendering Module
 * Created: 2025-09-29
 * By: MaySnowL
 * 
 * 文本渲染模块
 * 基于 STB 的字体渲染
 * 
 * 特性：
 * - UTF-8 中文支持
 * - 多行文本
 * - 文本对齐
 * 
 * 仅供学习和研究使用
 */

 
#include "text/TextRenderer.h"
#include "graphics/Primitives.h"
#include "text/Font.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

namespace Text
{

static stbtt_fontinfo g_font;
static bool g_font_initialized = false;

bool InitFont()
{
    if (g_font_initialized) return true;

    const unsigned char *ttf_buffer = reinterpret_cast<const unsigned char *>(OPPOSans_H);
    if (!stbtt_InitFont(&g_font, ttf_buffer, 0))
    {
        return false;
    }

    g_font_initialized = true;
    return true;
}

void ShutdownFont()
{
    g_font_initialized = false;
}

bool IsFontInitialized()
{
    return g_font_initialized;
}

void RenderChar(uint32_t *pixels, int stride, int width, int height,
                int codepoint, int x, int y, int font_size, uint32_t color) {  // 改为 int codepoint
    if (!InitFont()) return;
    
    float scale = stbtt_ScaleForPixelHeight(&g_font, font_size);
    
    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBox(&g_font, codepoint, scale, scale, &x0, &y0, &x1, &y1);
    
    int w = x1 - x0;
    int h = y1 - y0;
    
    unsigned char *bitmap = stbtt_GetCodepointBitmap(&g_font, 0, scale, codepoint, &w, &h, 0, 0);
    if (!bitmap) return;
    
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            uint8_t alpha = bitmap[j * w + i];
            if (alpha > 0) {
                uint8_t r = color & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t b = (color >> 16) & 0xFF;
                uint32_t blended = Graphics::rgba(r, g, b, alpha);
                Graphics::put_pixel(pixels, stride, width, height, 
                                   x + i + x0, y + j + y0, blended);
            }
        }
    }
    
    stbtt_FreeBitmap(bitmap, nullptr);
}

void RenderText(uint32_t *pixels, int stride, int width, int height, int x, int y, const std::string &text, int font_size, uint32_t color)
{
    if (!InitFont()) return;

    float scale = stbtt_ScaleForPixelHeight(&g_font, font_size);
    int cursor_x = x;
    int cursor_y = y;

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&g_font, &ascent, &descent, &lineGap);
    float line_height = (ascent - descent + lineGap) * scale;

    size_t i = 0;
    while (i < text.size())
    {
        unsigned char c = text[i];

        // 处理换行
        if (c == '\n')
        {
            cursor_x = x;
            cursor_y += line_height;
            i++;
            continue;
        }

        // 解码 UTF-8
        int codepoint = 0;
        int bytes = 0;

        if (c < 0x80)
        {
            // ASCII (1 byte)
            codepoint = c;
            bytes = 1;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            // 2 bytes
            codepoint = ((c & 0x1F) << 6) | (text[i + 1] & 0x3F);
            bytes = 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            // 3 bytes (中文通常在这里)
            codepoint = ((c & 0x0F) << 12) | ((text[i + 1] & 0x3F) << 6) | (text[i + 2] & 0x3F);
            bytes = 3;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            // 4 bytes
            codepoint = ((c & 0x07) << 18) | ((text[i + 1] & 0x3F) << 12) | ((text[i + 2] & 0x3F) << 6) | (text[i + 3] & 0x3F);
            bytes = 4;
        }
        else
        {
            // 无效字符，跳过
            i++;
            continue;
        }

        // 渲染字符（使用 codepoint 而不是 c）
        RenderChar(pixels, stride, width, height, codepoint, cursor_x, cursor_y, font_size, color);

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&g_font, codepoint, &advance, &lsb);
        cursor_x += static_cast<int>(advance * scale);

        i += bytes;
    }
}

void RenderTextF(uint32_t *pixels, int stride, int width, int height, float x, float y, const std::string &text, int font_size, uint32_t color)
{
    RenderText(pixels, stride, width, height, (int)x, (int)y, text, font_size, color);
}

void RenderTextStyled(uint32_t *pixels, int stride, int width, int height, int x, int y, const std::string &text, const TextStyle &style)
{
    if (!InitFont()) return;

    float scale = stbtt_ScaleForPixelHeight(&g_font, style.fontSize);
    int cursor_x = x;
    int cursor_y = y;

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&g_font, &ascent, &descent, &lineGap);
    float line_height = (ascent - descent + lineGap) * scale * style.lineSpacing;

    for (size_t i = 0; i < text.size(); ++i)
    {
        unsigned char c = text[i];

        if (c == '\n')
        {
            cursor_x = x;
            cursor_y += line_height;
            continue;
        }

        RenderChar(pixels, stride, width, height, c, cursor_x, cursor_y, style.fontSize, style.color);

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&g_font, c, &advance, &lsb);
        cursor_x += static_cast<int>((advance * scale) + style.letterSpacing);
    }
}

void RenderTextMultiline(uint32_t *pixels, int stride, int width, int height, int x, int y, const std::string &text, int font_size, uint32_t color, int maxWidth)
{
    if (maxWidth > 0)
    {
        std::vector<std::string> lines = WrapText(text, font_size, maxWidth);
        int cursor_y = y;

        FontMetrics metrics = GetFontMetrics(font_size);
        float line_height = (metrics.ascent - metrics.descent + metrics.lineGap) * metrics.scale;

        for (const auto &line : lines)
        {
            RenderText(pixels, stride, width, height, x, cursor_y, line, font_size, color);
            cursor_y += line_height;
        }
    }
    else
    {
        RenderText(pixels, stride, width, height, x, y, text, font_size, color);
    }
}

void RenderTextAligned(uint32_t *pixels, int stride, int width, int height, int x, int y, int box_width, const std::string &text, int font_size, uint32_t color, Alignment align)
{
    My_Vector2 text_size = CalcTextSize(text, font_size);

    int offset_x = 0;
    switch (align)
    {
    case Alignment::Center: offset_x = (box_width - text_size.x) / 2; break;
    case Alignment::Right: offset_x = box_width - text_size.x; break;
    case Alignment::Left:
    default: offset_x = 0; break;
    }

    RenderText(pixels, stride, width, height, x + offset_x, y, text, font_size, color);
}

My_Vector2 CalcTextSize(const std::string &text, int font_size)
{
    if (!InitFont()) return My_Vector2(0, 0);

    float scale = stbtt_ScaleForPixelHeight(&g_font, font_size);

    float width = 0.0f;
    float max_width = 0.0f;
    int line_count = 1;

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&g_font, &ascent, &descent, &lineGap);
    float line_height = (ascent - descent + lineGap) * scale;

    for (size_t i = 0; i < text.size(); ++i)
    {
        unsigned char c = text[i];

        if (c == '\n')
        {
            max_width = std::max(max_width, width);
            width = 0.0f;
            line_count++;
            continue;
        }

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&g_font, c, &advance, &lsb);
        width += advance * scale;
    }

    max_width = std::max(max_width, width);
    return My_Vector2(max_width, line_height * line_count);
}

My_Vector2 CalcTextSizeStyled(const std::string &text, const TextStyle &style)
{
    if (!InitFont()) return My_Vector2(0, 0);

    float scale = stbtt_ScaleForPixelHeight(&g_font, style.fontSize);

    float width = 0.0f;
    float max_width = 0.0f;
    int line_count = 1;

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&g_font, &ascent, &descent, &lineGap);
    float line_height = (ascent - descent + lineGap) * scale * style.lineSpacing;

    for (size_t i = 0; i < text.size(); ++i)
    {
        unsigned char c = text[i];

        if (c == '\n')
        {
            max_width = std::max(max_width, width);
            width = 0.0f;
            line_count++;
            continue;
        }

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&g_font, c, &advance, &lsb);
        width += (advance * scale) + style.letterSpacing;
    }

    max_width = std::max(max_width, width);
    return My_Vector2(max_width, line_height * line_count);
}

My_Vector2 CalcTextSizeMultiline(const std::string &text, int font_size, int maxWidth)
{
    if (maxWidth <= 0)
    {
        return CalcTextSize(text, font_size);
    }

    std::vector<std::string> lines = WrapText(text, font_size, maxWidth);

    float max_width = 0.0f;
    for (const auto &line : lines)
    {
        My_Vector2 line_size = CalcTextSize(line, font_size);
        max_width = std::max(max_width, line_size.x);
    }

    FontMetrics metrics = GetFontMetrics(font_size);
    float line_height = (metrics.ascent - metrics.descent + metrics.lineGap) * metrics.scale;

    return My_Vector2(max_width, line_height * lines.size());
}

My_Vector2 CalcCharSize(unsigned char c, int font_size)
{
    if (!InitFont()) return My_Vector2(0, 0);

    float scale = stbtt_ScaleForPixelHeight(&g_font, font_size);

    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBox(&g_font, c, scale, scale, &x0, &y0, &x1, &y1);

    return My_Vector2(x1 - x0, y1 - y0);
}

float GetCharAdvance(unsigned char c, int font_size)
{
    if (!InitFont()) return 0.0f;

    float scale = stbtt_ScaleForPixelHeight(&g_font, font_size);

    int advance, lsb;
    stbtt_GetCodepointHMetrics(&g_font, c, &advance, &lsb);

    return advance * scale;
}

std::vector<std::string> WrapText(const std::string &text, int font_size, int maxWidth)
{
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string word, line;

    while (stream >> word)
    {
        std::string test_line = line.empty() ? word : line + " " + word;
        My_Vector2 size = CalcTextSize(test_line, font_size);

        if (size.x > maxWidth && !line.empty())
        {
            lines.push_back(line);
            line = word;
        }
        else
        {
            line = test_line;
        }
    }

    if (!line.empty())
    {
        lines.push_back(line);
    }

    return lines;
}

FontMetrics GetFontMetrics(int font_size)
{
    FontMetrics metrics = { 0 };

    if (!InitFont()) return metrics;

    metrics.scale = stbtt_ScaleForPixelHeight(&g_font, font_size);
    stbtt_GetFontVMetrics(&g_font, &metrics.ascent, &metrics.descent, &metrics.lineGap);

    return metrics;
}

std::string TruncateText(const std::string &text, int font_size, int maxWidth)
{
    const std::string ellipsis = "...";
    My_Vector2 full_size = CalcTextSize(text, font_size);

    if (full_size.x <= maxWidth)
    {
        return text;
    }

    My_Vector2 ellipsis_size = CalcTextSize(ellipsis, font_size);
    int available_width = maxWidth - ellipsis_size.x;

    std::string result;
    float current_width = 0.0f;

    for (size_t i = 0; i < text.size(); ++i)
    {
        float char_width = GetCharAdvance(text[i], font_size);

        if (current_width + char_width > available_width)
        {
            break;
        }

        result += text[i];
        current_width += char_width;
    }

    return result + ellipsis;
}

int GetCharIndexFromPos(const std::string &text, int font_size, int pixel_x)
{
    float current_x = 0.0f;

    for (size_t i = 0; i < text.size(); ++i)
    {
        float char_width = GetCharAdvance(text[i], font_size);

        if (current_x + char_width / 2 > pixel_x)
        {
            return i;
        }

        current_x += char_width;
    }

    return text.size();
}

int GetPosFromCharIndex(const std::string &text, int font_size, int char_index)
{
    if (char_index < 0) return 0;
    if (char_index > (int)text.size()) char_index = text.size();

    float pos = 0.0f;

    for (int i = 0; i < char_index && i < (int)text.size(); ++i)
    {
        pos += GetCharAdvance(text[i], font_size);
    }

    return (int)pos;
}

bool IsUTF8(const std::string &text)
{
    for (size_t i = 0; i < text.size(); ++i)
    {
        unsigned char c = text[i];
        if (c > 127) return true;
    }
    return false;
}

UTF8Char GetUTF8Char(const std::string &text, size_t &index)
{
    UTF8Char result;
    result.length = 0;

    if (index >= text.size()) return result;

    unsigned char c = text[index];

    if ((c & 0x80) == 0)
    {
        result.bytes[0] = c;
        result.length = 1;
        index++;
    }
    else if ((c & 0xE0) == 0xC0)
    {
        result.bytes[0] = text[index++];
        result.bytes[1] = text[index++];
        result.length = 2;
    }
    else if ((c & 0xF0) == 0xE0)
    {
        result.bytes[0] = text[index++];
        result.bytes[1] = text[index++];
        result.bytes[2] = text[index++];
        result.length = 3;
    }
    else if ((c & 0xF8) == 0xF0)
    {
        result.bytes[0] = text[index++];
        result.bytes[1] = text[index++];
        result.bytes[2] = text[index++];
        result.bytes[3] = text[index++];
        result.length = 4;
    }

    result.bytes[result.length] = '\0';
    return result;
}

size_t UTF8Length(const std::string &text)
{
    size_t count = 0;
    size_t i = 0;

    while (i < text.size())
    {
        UTF8Char ch = GetUTF8Char(text, i);
        if (ch.length > 0) count++;
    }

    return count;
}

} // namespace Text