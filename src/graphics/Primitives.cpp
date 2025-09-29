/*
 * CPU-Draw - Graphics Primitives Module
 * Created: 2025-09-29
 * By: MaySnowL
 * 
 * 图形绘制模块
 * 提供绘制功能
 * 
 * 特性：
 * - Alpha 混合支持
 * - 多种图形绘制算法
 * - 渐变填充
 * 
 * 仅供学习和研究使用
 */

#include "graphics/Primitives.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace Graphics
{

// Alpha混合
static inline uint32_t blend_alpha(uint32_t fg, uint32_t bg)
{
    uint8_t r = fg & 0xFF;
    uint8_t g = (fg >> 8) & 0xFF;
    uint8_t b = (fg >> 16) & 0xFF;
    uint8_t a = (fg >> 24) & 0xFF;

    if (a == 255) return fg;
    if (a == 0) return bg;

    uint8_t br = bg & 0xFF;
    uint8_t bg_ = (bg >> 8) & 0xFF;
    uint8_t bb = (bg >> 16) & 0xFF;

    uint8_t nr = (r * a + br * (255 - a)) / 255;
    uint8_t ng = (g * a + bg_ * (255 - a)) / 255;
    uint8_t nb = (b * a + bb * (255 - a)) / 255;

    return nr | (ng << 8) | (nb << 16) | (255 << 24);
}

void put_pixel(uint32_t *pixels, int stride, int width, int height, int x, int y, uint32_t color)
{
    if (x < 0 || x >= width || y < 0 || y >= height) return;

    uint32_t *dst = pixels + y * stride + x;
    uint8_t a = (color >> 24) & 0xFF;

    if (a == 255)
    {
        *dst = color;
    }
    else if (a > 0)
    {
        *dst = blend_alpha(color, *dst);
    }
}

void put_pixel_fast(uint32_t *pixels, int stride, int width, int height, int x, int y, uint32_t color)
{
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    pixels[y * stride + x] = color;
}

void put_pixelF(uint32_t *pixels, int stride, int width, int height, float x, float y, uint32_t color)
{
    put_pixel(pixels, stride, width, height, (int)(x + 0.5f), (int)(y + 0.5f), color);
}

void draw_line(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, uint32_t color)
{
    // Bresenham's line algorithm
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true)
    {
        put_pixel(pixels, stride, width, height, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_lineF(uint32_t *pixels, int stride, int width, int height, float x0, float y0, float x1, float y1, uint32_t color)
{
    float dx = x1 - x0;
    float dy = y1 - y0;
    int steps = std::max(std::abs(dx), std::abs(dy));

    if (steps == 0)
    {
        put_pixelF(pixels, stride, width, height, x0, y0, color);
        return;
    }

    float sx = dx / steps;
    float sy = dy / steps;
    float x = x0;
    float y = y0;

    for (int i = 0; i <= steps; i++)
    {
        put_pixelF(pixels, stride, width, height, x, y, color);
        x += sx;
        y += sy;
    }
}

void draw_line_thick(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, uint32_t color, int thickness)
{
    if (thickness <= 1)
    {
        draw_line(pixels, stride, width, height, x0, y0, x1, y1, color);
        return;
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len == 0) return;

    float nx = -dy / len;
    float ny = dx / len;

    for (int t = -thickness / 2; t <= thickness / 2; t++)
    {
        int sx0 = x0 + nx * t;
        int sy0 = y0 + ny * t;
        int sx1 = x1 + nx * t;
        int sy1 = y1 + ny * t;
        draw_line(pixels, stride, width, height, sx0, sy0, sx1, sy1, color);
    }
}

void draw_rect(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, uint32_t color)
{
    draw_line(pixels, stride, width, height, x0, y0, x1, y0, color);
    draw_line(pixels, stride, width, height, x0, y1, x1, y1, color);
    draw_line(pixels, stride, width, height, x0, y0, x0, y1, color);
    draw_line(pixels, stride, width, height, x1, y0, x1, y1, color);
}

void draw_rectF(uint32_t *pixels, int stride, int width, int height, float x0, float y0, float x1, float y1, uint32_t color)
{
    draw_lineF(pixels, stride, width, height, x0, y0, x1, y0, color);
    draw_lineF(pixels, stride, width, height, x0, y1, x1, y1, color);
    draw_lineF(pixels, stride, width, height, x0, y0, x0, y1, color);
    draw_lineF(pixels, stride, width, height, x1, y0, x1, y1, color);
}

void draw_rect_filled(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, uint32_t color)
{
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);

    x0 = std::max(0, x0);
    y0 = std::max(0, y0);
    x1 = std::min(width - 1, x1);
    y1 = std::min(height - 1, y1);

    for (int y = y0; y <= y1; y++)
    {
        for (int x = x0; x <= x1; x++)
        {
            put_pixel(pixels, stride, width, height, x, y, color);
        }
    }
}

void draw_rect_rounded(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, int radius, uint32_t color)
{
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);

    radius = std::min(radius, std::min((x1 - x0) / 2, (y1 - y0) / 2));

    // 直线部分
    draw_line(pixels, stride, width, height, x0 + radius, y0, x1 - radius, y0, color);
    draw_line(pixels, stride, width, height, x0 + radius, y1, x1 - radius, y1, color);
    draw_line(pixels, stride, width, height, x0, y0 + radius, x0, y1 - radius, color);
    draw_line(pixels, stride, width, height, x1, y0 + radius, x1, y1 - radius, color);

    // 圆角
    int x = radius, y = 0, err = 0;
    while (x >= y)
    {
        put_pixel(pixels, stride, width, height, x1 - radius + x, y0 + radius - y, color);
        put_pixel(pixels, stride, width, height, x1 - radius + y, y0 + radius - x, color);
        put_pixel(pixels, stride, width, height, x0 + radius - y, y0 + radius - x, color);
        put_pixel(pixels, stride, width, height, x0 + radius - x, y0 + radius - y, color);
        put_pixel(pixels, stride, width, height, x0 + radius - x, y1 - radius + y, color);
        put_pixel(pixels, stride, width, height, x0 + radius - y, y1 - radius + x, color);
        put_pixel(pixels, stride, width, height, x1 - radius + y, y1 - radius + x, color);
        put_pixel(pixels, stride, width, height, x1 - radius + x, y1 - radius + y, color);

        y++;
        if (err <= 0)
        {
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void draw_rect_rounded_filled(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, int radius, uint32_t color)
{
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);

    radius = std::min(radius, std::min((x1 - x0) / 2, (y1 - y0) / 2));

    // 中间矩形部分
    draw_rect_filled(pixels, stride, width, height, x0, y0 + radius, x1, y1 - radius, color);
    draw_rect_filled(pixels, stride, width, height, x0 + radius, y0, x1 - radius, y0 + radius - 1, color);
    draw_rect_filled(pixels, stride, width, height, x0 + radius, y1 - radius + 1, x1 - radius, y1, color);

    // 四个圆角
    int x = radius, y = 0, err = 0;
    while (x >= y)
    {
        draw_line(pixels, stride, width, height, x0 + radius - x, y0 + radius - y, x0 + radius - x, y0 + radius - y, color);
        draw_line(pixels, stride, width, height, x1 - radius + x, y0 + radius - y, x1 - radius + x, y0 + radius - y, color);
        draw_line(pixels, stride, width, height, x0 + radius - x, y1 - radius + y, x0 + radius - x, y1 - radius + y, color);
        draw_line(pixels, stride, width, height, x1 - radius + x, y1 - radius + y, x1 - radius + x, y1 - radius + y, color);

        y++;
        if (err <= 0)
        {
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void draw_circle(uint32_t *pixels, int stride, int width, int height, int cx, int cy, int radius, uint32_t color)
{
    int x = radius, y = 0, err = 0;

    while (x >= y)
    {
        put_pixel(pixels, stride, width, height, cx + x, cy + y, color);
        put_pixel(pixels, stride, width, height, cx + y, cy + x, color);
        put_pixel(pixels, stride, width, height, cx - y, cy + x, color);
        put_pixel(pixels, stride, width, height, cx - x, cy + y, color);
        put_pixel(pixels, stride, width, height, cx - x, cy - y, color);
        put_pixel(pixels, stride, width, height, cx - y, cy - x, color);
        put_pixel(pixels, stride, width, height, cx + y, cy - x, color);
        put_pixel(pixels, stride, width, height, cx + x, cy - y, color);

        y++;
        if (err <= 0)
        {
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void draw_circle_filled(uint32_t *pixels, int stride, int width, int height, int cx, int cy, int radius, uint32_t color)
{
    int x = radius, y = 0, err = 0;

    while (x >= y)
    {
        draw_line(pixels, stride, width, height, cx - x, cy + y, cx + x, cy + y, color);
        draw_line(pixels, stride, width, height, cx - y, cy + x, cx + y, cy + x, color);
        draw_line(pixels, stride, width, height, cx - x, cy - y, cx + x, cy - y, color);
        draw_line(pixels, stride, width, height, cx - y, cy - x, cx + y, cy - x, color);

        y++;
        if (err <= 0)
        {
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void draw_circleF(uint32_t *pixels, int stride, int width, int height, float cx, float cy, float radius, uint32_t color)
{
    draw_circle(pixels, stride, width, height, (int)cx, (int)cy, (int)radius, color);
}

void draw_triangle(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    draw_line(pixels, stride, width, height, x0, y0, x1, y1, color);
    draw_line(pixels, stride, width, height, x1, y1, x2, y2, color);
    draw_line(pixels, stride, width, height, x2, y2, x0, y0, color);
}

void draw_triangle_filled(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // 排序顶点 y0 <= y1 <= y2
    if (y0 > y1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    if (y0 > y2)
    {
        std::swap(x0, x2);
        std::swap(y0, y2);
    }
    if (y1 > y2)
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    if (y1 == y2)
    {
        // 平底三角形
        int sx = (x1 < x2) ? x1 : x2;
        int ex = (x1 > x2) ? x1 : x2;
        for (int y = y0; y <= y1; y++)
        {
            float t = (y - y0) / (float)(y1 - y0);
            int xl = x0 + t * (sx - x0);
            int xr = x0 + t * (ex - x0);
            draw_line(pixels, stride, width, height, xl, y, xr, y, color);
        }
    }
    else if (y0 == y1)
    {
        // 平顶三角形
        int sx = (x0 < x1) ? x0 : x1;
        int ex = (x0 > x1) ? x0 : x1;
        for (int y = y0; y <= y2; y++)
        {
            float t = (y - y0) / (float)(y2 - y0);
            int xl = sx + t * (x2 - sx);
            int xr = ex + t * (x2 - ex);
            draw_line(pixels, stride, width, height, xl, y, xr, y, color);
        }
    }
    else
    {
        // 分割成两个三角形
        int x3 = x0 + ((y1 - y0) * (x2 - x0)) / (y2 - y0);
        draw_triangle_filled(pixels, stride, width, height, x0, y0, x1, y1, x3, y1, color);
        draw_triangle_filled(pixels, stride, width, height, x1, y1, x3, y1, x2, y2, color);
    }
}

void draw_bezier_cubic(uint32_t *pixels, int stride, int width, int height, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t color, int segments)
{
    float px = x0, py = y0;

    for (int i = 1; i <= segments; i++)
    {
        float t = i / (float)segments;
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        float x = uuu * x0 + 3 * uu * t * x1 + 3 * u * tt * x2 + ttt * x3;
        float y = uuu * y0 + 3 * uu * t * y1 + 3 * u * tt * y2 + ttt * y3;

        draw_lineF(pixels, stride, width, height, px, py, x, y, color);
        px = x;
        py = y;
    }
}

void draw_bezier_quadratic(uint32_t *pixels, int stride, int width, int height, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color, int segments)
{
    float px = x0, py = y0;

    for (int i = 1; i <= segments; i++)
    {
        float t = i / (float)segments;
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;

        float x = uu * x0 + 2 * u * t * x1 + tt * x2;
        float y = uu * y0 + 2 * u * t * y1 + tt * y2;

        draw_lineF(pixels, stride, width, height, px, py, x, y, color);
        px = x;
        py = y;
    }
}

void fill_gradient_linear(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, uint32_t color_start, uint32_t color_end)
{
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);

    uint8_t r0 = get_red(color_start), g0 = get_green(color_start), b0 = get_blue(color_start);
    uint8_t r1 = get_red(color_end), g1 = get_green(color_end), b1 = get_blue(color_end);

    for (int y = y0; y <= y1; y++)
    {
        float t = (y - y0) / (float)(y1 - y0 + 1);
        uint8_t r = r0 + t * (r1 - r0);
        uint8_t g = g0 + t * (g1 - g0);
        uint8_t b = b0 + t * (b1 - b0);
        uint32_t color = rgba(r, g, b);

        for (int x = x0; x <= x1; x++)
        {
            put_pixel_fast(pixels, stride, width, height, x, y, color);
        }
    }
}

void fill_gradient_radial(uint32_t *pixels, int stride, int width, int height, int cx, int cy, int radius, uint32_t color_center, uint32_t color_edge)
{
    uint8_t r0 = get_red(color_center), g0 = get_green(color_center), b0 = get_blue(color_center);
    uint8_t r1 = get_red(color_edge), g1 = get_green(color_edge), b1 = get_blue(color_edge);

    for (int y = cy - radius; y <= cy + radius; y++)
    {
        for (int x = cx - radius; x <= cx + radius; x++)
        {
            int dx = x - cx;
            int dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist <= radius)
            {
                float t = dist / radius;
                uint8_t r = r0 + t * (r1 - r0);
                uint8_t g = g0 + t * (g1 - g0);
                uint8_t b = b0 + t * (b1 - b0);
                uint32_t color = rgba(r, g, b);

                put_pixel(pixels, stride, width, height, x, y, color);
            }
        }
    }
}

void clear_screen(uint32_t *pixels, int stride, int width, int height, uint32_t color)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            pixels[y * stride + x] = color;
        }
    }
}

} // namespace Graphics