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

#ifndef GRAPHICS_PRIMITIVES_H
#define GRAPHICS_PRIMITIVES_H

#include <cstdint>

namespace Graphics
{

// 颜色
inline uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
    return r | (g << 8) | (b << 16) | (a << 24);
}

// 提取分量
inline uint8_t get_red(uint32_t color)
{
    return color & 0xFF;
}
inline uint8_t get_green(uint32_t color)
{
    return (color >> 8) & 0xFF;
}
inline uint8_t get_blue(uint32_t color)
{
    return (color >> 16) & 0xFF;
}
inline uint8_t get_alpha(uint32_t color)
{
    return (color >> 24) & 0xFF;
}

// 像素操作
void put_pixel(uint32_t *pixels, int stride, int width, int height, int x, int y, uint32_t color);
void put_pixel_fast(uint32_t *pixels, int stride, int width, int height, int x, int y, uint32_t color);
void put_pixelF(uint32_t *pixels, int stride, int width, int height, float x, float y, uint32_t color);

// 线条
void draw_line(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, uint32_t color);
void draw_lineF(uint32_t *pixels, int stride, int width, int height, float x0, float y0, float x1, float y1, uint32_t color);
void draw_line_thick(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, uint32_t color, int thickness);

// 矩形
void draw_rect(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, uint32_t color);
void draw_rectF(uint32_t *pixels, int stride, int width, int height, float x0, float y0, float x1, float y1, uint32_t color);
void draw_rect_filled(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, uint32_t color);
void draw_rect_rounded(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, int radius, uint32_t color);
void draw_rect_rounded_filled(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, int radius, uint32_t color);

// 圆形
void draw_circle(uint32_t *pixels, int stride, int width, int height, int cx, int cy, int radius, uint32_t color);
void draw_circle_filled(uint32_t *pixels, int stride, int width, int height, int cx, int cy, int radius, uint32_t color);
void draw_circleF(uint32_t *pixels, int stride, int width, int height, float cx, float cy, float radius, uint32_t color);

// 三角形
void draw_triangle(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void draw_triangle_filled(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

// 多边形
void draw_polygon(uint32_t *pixels, int stride, int width, int height, const int *points, int point_count, uint32_t color);
void draw_polygon_filled(uint32_t *pixels, int stride, int width, int height, const int *points, int point_count, uint32_t color);

// 贝塞尔曲线
void draw_bezier_cubic(uint32_t *pixels, int stride, int width, int height, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t color, int segments = 20);
void draw_bezier_quadratic(uint32_t *pixels, int stride, int width, int height, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color, int segments = 15);

// 渐变填充
void fill_gradient_linear(uint32_t *pixels, int stride, int width, int height, int x0, int y0, int x1, int y1, uint32_t color_start, uint32_t color_end);
void fill_gradient_radial(uint32_t *pixels, int stride, int width, int height, int cx, int cy, int radius, uint32_t color_center, uint32_t color_edge);

// 清屏
void clear_screen(uint32_t *pixels, int stride, int width, int height, uint32_t color);

} // namespace Graphics

#endif // GRAPHICS_PRIMITIVES_H