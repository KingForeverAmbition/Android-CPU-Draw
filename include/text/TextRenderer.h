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

#ifndef TEXT_TEXTRENDERER_H
#define TEXT_TEXTRENDERER_H

#include "core/VectorStruct.h"
#include <cstdint>
#include <string>

namespace Text
{

// 文本对齐方式
enum class Alignment
{
    Left,
    Center,
    Right
};

// 文本样式
struct TextStyle
{
    int fontSize;
    uint32_t color;
    bool bold;
    bool italic;
    float lineSpacing;
    float letterSpacing;

    TextStyle() : fontSize(24), color(0xFFFFFFFF), bold(false), italic(false), lineSpacing(1.2f), letterSpacing(0.0f)
    {
    }
};

// 字体信息
struct FontMetrics
{
    int ascent;
    int descent;
    int lineGap;
    float scale;
};

// 初始化
bool InitFont();

// 释放
void ShutdownFont();

// 检查是否已初始化
bool IsFontInitialized();

// 普通文本
void RenderText(uint32_t *pixels, int stride, int width, int height, int x, int y, const std::string &text, int font_size, uint32_t color);

void RenderTextF(uint32_t *pixels, int stride, int width, int height, float x, float y, const std::string &text, int font_size, uint32_t color);

// 格式文本
void RenderTextStyled(uint32_t *pixels, int stride, int width, int height, int x, int y, const std::string &text, const TextStyle &style);

// 多行文本
void RenderTextMultiline(uint32_t *pixels, int stride, int width, int height, int x, int y, const std::string &text, int font_size, uint32_t color, int maxWidth = -1);

// 对齐文本
void RenderTextAligned(uint32_t *pixels, int stride, int width, int height, int x, int y, int box_width, const std::string &text, int font_size, uint32_t color, Alignment align);

// 字符渲染
void RenderChar(uint32_t *pixels, int stride, int width, int height, int codepoint, int x, int y, int font_size, uint32_t color); // 改为 int

// 尺寸计算
My_Vector2 CalcTextSize(const std::string &text, int font_size);
My_Vector2 CalcTextSizeStyled(const std::string &text, const TextStyle &style);
My_Vector2 CalcTextSizeMultiline(const std::string &text, int font_size, int maxWidth = -1);

// 字符尺寸
My_Vector2 CalcCharSize(unsigned char c, int font_size);

// 字符宽度
float GetCharAdvance(unsigned char c, int font_size);

// 文本换行
std::vector<std::string> WrapText(const std::string &text, int font_size, int maxWidth);

// 获取字体信息
FontMetrics GetFontMetrics(int font_size);

// 文本截断
std::string TruncateText(const std::string &text, int font_size, int maxWidth);

// 获取字符索引
int GetCharIndexFromPos(const std::string &text, int font_size, int pixel_x);

// 获取像素位置
int GetPosFromCharIndex(const std::string &text, int font_size, int char_index);

// UTF-8
struct UTF8Char
{
    char bytes[5];
    int length;
};

bool IsUTF8(const std::string &text);
UTF8Char GetUTF8Char(const std::string &text, size_t &index);
size_t UTF8Length(const std::string &text);

} // namespace Text

#endif // TEXT_TEXTRENDERER_H