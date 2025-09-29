/*
 * CPU-Draw - DrawList Module
 * Created: 2025-09-29
 * By: MaySnowL
 * 
 * 绘制接口模块
 * 封装图形渲染函数，方便使用
 * 
 * 特性：
 * - 裁剪区域
 * - 几何变换
 * - 文本渲染
 * 
 * 仅供学习和研究使用
 */

#include "graphics/DrawList.h"
#include "text/TextRenderer.h"
#include <algorithm>
#include <cmath>

namespace Graphics
{

DrawList::DrawList(uint32_t *buffer, int stride, int width, int height) : pixels(buffer), stride(stride), width(width), height(height)
{
}

DrawList::~DrawList()
{
}

void DrawList::AddPixel(int x, int y, uint32_t color)
{
    if (!IsPointInClipRect(x, y)) return;
    put_pixel(pixels, stride, width, height, x, y, color);
}

void DrawList::AddPixelF(float x, float y, uint32_t color)
{
    ApplyTransform(x, y);
    AddPixel((int)x, (int)y, color);
}

void DrawList::AddLine(int x0, int y0, int x1, int y1, uint32_t color)
{
    draw_line(pixels, stride, width, height, x0, y0, x1, y1, color);
}

void DrawList::AddLineF(float x0, float y0, float x1, float y1, uint32_t color)
{
    ApplyTransform(x0, y0);
    ApplyTransform(x1, y1);
    draw_lineF(pixels, stride, width, height, x0, y0, x1, y1, color);
}

void DrawList::AddLineThick(int x0, int y0, int x1, int y1, uint32_t color, int thickness)
{
    draw_line_thick(pixels, stride, width, height, x0, y0, x1, y1, color, thickness);
}

void DrawList::AddRect(int x0, int y0, int x1, int y1, uint32_t color)
{
    draw_rect(pixels, stride, width, height, x0, y0, x1, y1, color);
}

void DrawList::AddRectF(float x0, float y0, float x1, float y1, uint32_t color)
{
    ApplyTransform(x0, y0);
    ApplyTransform(x1, y1);
    draw_rectF(pixels, stride, width, height, x0, y0, x1, y1, color);
}

void DrawList::AddRectFilled(int x0, int y0, int x1, int y1, uint32_t color)
{
    draw_rect_filled(pixels, stride, width, height, x0, y0, x1, y1, color);
}

void DrawList::AddRectRounded(int x0, int y0, int x1, int y1, int radius, uint32_t color)
{
    draw_rect_rounded(pixels, stride, width, height, x0, y0, x1, y1, radius, color);
}

void DrawList::AddRectRoundedFilled(int x0, int y0, int x1, int y1, int radius, uint32_t color)
{
    draw_rect_rounded_filled(pixels, stride, width, height, x0, y0, x1, y1, radius, color);
}

void DrawList::AddCircle(int cx, int cy, int radius, uint32_t color)
{
    draw_circle(pixels, stride, width, height, cx, cy, radius, color);
}

void DrawList::AddCircleF(float cx, float cy, float radius, uint32_t color)
{
    ApplyTransform(cx, cy);
    draw_circleF(pixels, stride, width, height, cx, cy, radius, color);
}

void DrawList::AddCircleFilled(int cx, int cy, int radius, uint32_t color)
{
    draw_circle_filled(pixels, stride, width, height, cx, cy, radius, color);
}

void DrawList::AddTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    draw_triangle(pixels, stride, width, height, x0, y0, x1, y1, x2, y2, color);
}

void DrawList::AddTriangleFilled(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    draw_triangle_filled(pixels, stride, width, height, x0, y0, x1, y1, x2, y2, color);
}

void DrawList::AddPolygon(const int *points, int point_count, uint32_t color)
{
    draw_polygon(pixels, stride, width, height, points, point_count, color);
}

void DrawList::AddPolygonFilled(const int *points, int point_count, uint32_t color)
{
    draw_polygon_filled(pixels, stride, width, height, points, point_count, color);
}

void DrawList::AddBezierCubic(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t color, int segments)
{
    ApplyTransform(x0, y0);
    ApplyTransform(x1, y1);
    ApplyTransform(x2, y2);
    ApplyTransform(x3, y3);
    draw_bezier_cubic(pixels, stride, width, height, x0, y0, x1, y1, x2, y2, x3, y3, color, segments);
}

void DrawList::AddBezierQuadratic(float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color, int segments)
{
    ApplyTransform(x0, y0);
    ApplyTransform(x1, y1);
    ApplyTransform(x2, y2);
    draw_bezier_quadratic(pixels, stride, width, height, x0, y0, x1, y1, x2, y2, color, segments);
}

void DrawList::AddGradientLinear(int x0, int y0, int x1, int y1, uint32_t color_start, uint32_t color_end)
{
    fill_gradient_linear(pixels, stride, width, height, x0, y0, x1, y1, color_start, color_end);
}

void DrawList::AddGradientRadial(int cx, int cy, int radius, uint32_t color_center, uint32_t color_edge)
{
    fill_gradient_radial(pixels, stride, width, height, cx, cy, radius, color_center, color_edge);
}

void DrawList::AddText(int x, int y, const std::string &text, int font_size, uint32_t color)
{
    Text::RenderText(pixels, stride, width, height, x, y, text, font_size, color);
}

void DrawList::AddText(float x, float y, const std::string &text, int font_size, uint32_t color)
{
    ApplyTransform(x, y);
    AddText((int)x, (int)y, text, font_size, color);
}

void DrawList::AddTextAligned(int x, int y, const std::string &text, int font_size, uint32_t color, TextAlign align)
{
    My_Vector2 size = CalcTextSize(text, font_size);

    int offset_x = 0;
    switch (align)
    {
    case TextAlign::Center: offset_x = -size.x / 2; break;
    case TextAlign::Right: offset_x = -size.x; break;
    case TextAlign::Left:
    default: offset_x = 0; break;
    }

    AddText(x + offset_x, y, text, font_size, color);
}

My_Vector2 DrawList::CalcTextSize(const std::string &text, int font_size)
{
    return Text::CalcTextSize(text, font_size);
}

void DrawList::Clear(uint32_t color)
{
    clear_screen(pixels, stride, width, height, color);
}

void DrawList::PushClipRect(int x0, int y0, int x1, int y1)
{
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);

    x0 = std::max(0, x0);
    y0 = std::max(0, y0);
    x1 = std::min(width - 1, x1);
    y1 = std::min(height - 1, y1);

    clipRectStack.push_back({ x0, y0, x1, y1 });
}

void DrawList::PopClipRect()
{
    if (!clipRectStack.empty())
    {
        clipRectStack.pop_back();
    }
}

void DrawList::ClearClipRect()
{
    clipRectStack.clear();
}

void DrawList::PushTransform(float tx, float ty, float scale, float rotation)
{
    transformStack.push_back({ tx, ty, scale, rotation });
}

void DrawList::PopTransform()
{
    if (!transformStack.empty())
    {
        transformStack.pop_back();
    }
}

bool DrawList::IsPointInClipRect(int x, int y) const
{
    if (clipRectStack.empty()) return true;

    const ClipRect &rect = clipRectStack.back();
    return x >= rect.x0 && x <= rect.x1 && y >= rect.y0 && y <= rect.y1;
}

void DrawList::ApplyTransform(float &x, float &y) const
{
    if (transformStack.empty()) return;

    for (const auto &t : transformStack)
    {
        // 缩放
        x *= t.scale;
        y *= t.scale;

        // 旋转
        if (t.rotation != 0.0f)
        {
            float cos_r = std::cos(t.rotation);
            float sin_r = std::sin(t.rotation);
            float nx = x * cos_r - y * sin_r;
            float ny = x * sin_r + y * cos_r;
            x = nx;
            y = ny;
        }

        // 平移
        x += t.tx;
        y += t.ty;
    }
}

} // namespace Graphics