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

#ifndef GRAPHICS_DRAWLIST_H
#define GRAPHICS_DRAWLIST_H

#include "core/VectorStruct.h"
#include "graphics/Primitives.h"
#include <string>
#include <vector>

namespace Graphics
{

// 文本对齐方式
enum class TextAlign
{
    Left,
    Center,
    Right
};

// 绘制命令列表
class DrawList
{
  public:
    DrawList(uint32_t *buffer, int stride, int width, int height);
    ~DrawList();

    // 获取缓冲区信息
    int GetWidth() const
    {
        return width;
    }
    int GetHeight() const
    {
        return height;
    }
    int GetStride() const
    {
        return stride;
    }
    uint32_t *GetPixels() const
    {
        return pixels;
    }

    // 基础绘制
    void AddPixel(int x, int y, uint32_t color);
    void AddPixelF(float x, float y, uint32_t color);

    // 线条
    void AddLine(int x0, int y0, int x1, int y1, uint32_t color);
    void AddLineF(float x0, float y0, float x1, float y1, uint32_t color);
    void AddLineThick(int x0, int y0, int x1, int y1, uint32_t color, int thickness);

    // 矩形
    void AddRect(int x0, int y0, int x1, int y1, uint32_t color);
    void AddRectF(float x0, float y0, float x1, float y1, uint32_t color);
    void AddRectFilled(int x0, int y0, int x1, int y1, uint32_t color);
    void AddRectRounded(int x0, int y0, int x1, int y1, int radius, uint32_t color);
    void AddRectRoundedFilled(int x0, int y0, int x1, int y1, int radius, uint32_t color);

    // 圆形
    void AddCircle(int cx, int cy, int radius, uint32_t color);
    void AddCircleF(float cx, float cy, float radius, uint32_t color);
    void AddCircleFilled(int cx, int cy, int radius, uint32_t color);

    // 三角形
    void AddTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
    void AddTriangleFilled(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

    // 多边形
    void AddPolygon(const int *points, int point_count, uint32_t color);
    void AddPolygonFilled(const int *points, int point_count, uint32_t color);

    // 贝塞尔曲线
    void AddBezierCubic(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t color, int segments = 20);
    void AddBezierQuadratic(float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color, int segments = 15);

    // 渐变
    void AddGradientLinear(int x0, int y0, int x1, int y1, uint32_t color_start, uint32_t color_end);
    void AddGradientRadial(int cx, int cy, int radius, uint32_t color_center, uint32_t color_edge);

    // 文本
    void AddText(int x, int y, const std::string &text, int font_size, uint32_t color);
    void AddText(float x, float y, const std::string &text, int font_size, uint32_t color);
    void AddTextAligned(int x, int y, const std::string &text, int font_size, uint32_t color, TextAlign align);

    // 文本工具
    My_Vector2 CalcTextSize(const std::string &text, int font_size);

    // 清屏
    void Clear(uint32_t color = rgba(0, 0, 0, 255));

    // 裁剪区域
    void PushClipRect(int x0, int y0, int x1, int y1);
    void PopClipRect();
    void ClearClipRect();

    // 变换
    void PushTransform(float tx, float ty, float scale = 1.0f, float rotation = 0.0f);
    void PopTransform();

  private:
    uint32_t *pixels;
    int stride;
    int width;
    int height;

    // 裁剪区域栈
    struct ClipRect
    {
        int x0, y0, x1, y1;
    };
    std::vector<ClipRect> clipRectStack;

    // 变换栈
    struct Transform
    {
        float tx, ty, scale, rotation;
    };
    std::vector<Transform> transformStack;

    bool IsPointInClipRect(int x, int y) const;
    void ApplyTransform(float &x, float &y) const;
};

} // namespace Graphics

#endif // GRAPHICS_DRAWLIST_H