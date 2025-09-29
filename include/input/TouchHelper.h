/*
 * CPU-Draw - Touch Input Module
 * Created: 2025-09-29
 * By: MaySnowL
 * 
 * 触摸输入处理模块
 * 多点触摸和手势识别
 * 
 * 特性：
 * - 最多 10 点触摸
 * - 手势识别
 * - 屏幕旋转适配
 * 
 * 仅供学习和研究使用
 */

#ifndef INPUT_TOUCHHELPER_H
#define INPUT_TOUCHHELPER_H

#include "core/VectorStruct.h"
#include <functional>
#include <linux/input.h>
#include <vector>

namespace Input
{

// 触摸对象
struct TouchPoint
{
    My_Vector2 pos;      // 当前位置
    My_Vector2 startPos; // 起始位置
    My_Vector2 velocity; // 速度
    int id;              // 触摸ID
    bool isDown;         // 是否按下
    float pressure;      // 压力
    long long timestamp; // 时间戳

    TouchPoint() : id(0), isDown(false), pressure(1.0f), timestamp(0)
    {
    }

    // 计算移动距离
    float GetDistance() const
    {
        return (pos - startPos).length();
    }

    // 计算移动方向向量
    My_Vector2 GetDirection() const
    {
        My_Vector2 dir = pos - startPos;
        float len = dir.length();
        return len > 0 ? dir / len : My_Vector2(0, 0);
    }
};

// 触摸设备
struct TouchDevice
{
    int fd;                   // 文件描述符
    float scaleX, scaleY;     // 缩放
    input_absinfo absX, absY; // 范围
    TouchPoint fingers[10];   // 最多支持10个触摸点

    TouchDevice() : fd(-1), scaleX(1.0f), scaleY(1.0f)
    {
        memset(&absX, 0, sizeof(absX));
        memset(&absY, 0, sizeof(absY));
    }
};

// 手势类型
enum class GestureType
{
    None,
    Tap,       // 单击
    DoubleTap, // 双击
    LongPress, // 长按
    Swipe,     // 滑动
    Pinch,     // 捏合
    Rotate     // 旋转
};

// 手势数据
struct GestureData
{
    GestureType type;
    My_Vector2 position;
    My_Vector2 direction;
    float distance;
    float scale;    // 用于Pinch
    float rotation; // 用于Rotate
    int fingerCount;

    GestureData() : type(GestureType::None), distance(0), scale(1.0f), rotation(0), fingerCount(0)
    {
    }
};

// 触摸回调函数类型
using TouchCallback = std::function<void(std::vector<TouchDevice> *)>;
using GestureCallback = std::function<void(const GestureData &)>;

// 初始化触摸系统
bool Init(const My_Vector2 &screenSize, bool readOnly = false);

// 关闭触摸系统
void Close();

// 检查是否已初始化
bool IsInitialized();

// 基础触摸操作
void Down(float x, float y, int touchId = 0);
void Move(float x, float y, int touchId = 0);
void Up(int touchId = 0);

// 更高级的触摸操作
void TouchAt(const My_Vector2 &pos, int durationMs = 50);
void Swipe(const My_Vector2 &start, const My_Vector2 &end, int durationMs = 300);
void MultiTouch(const std::vector<My_Vector2> &positions);

// 上传触摸事件到系统
void Upload();

// 设置回调
void SetTouchCallback(const TouchCallback &callback);
void SetGestureCallback(const GestureCallback &callback);

// 坐标转换
My_Vector2 TouchToScreen(const My_Vector2 &touchCoord);
My_Vector2 ScreenToTouch(const My_Vector2 &screenCoord);

// 获取缩放比例
My_Vector2 GetScale();

// 设置屏幕方向
void SetOrientation(int orientation);

// 获取当前触摸点
const std::vector<TouchDevice> &GetDevices();
std::vector<TouchPoint> GetActiveTouches();
int GetTouchCount();

// 触摸点查询
bool IsTouching();
bool IsTouchingAt(const My_Vector2 &pos, float radius = 10.0f);
TouchPoint *GetTouchById(int id);
TouchPoint *GetNearestTouch(const My_Vector2 &pos);

// 手势识别配置
struct GestureConfig
{
    float tapMaxDistance;     // 点击最大移动距离
    int tapMaxDuration;       // 点击最大持续时间（毫秒）
    int doubleTapMaxInterval; // 双击最大间隔（毫秒）
    int longPressMinDuration; // 长按最小持续时间（毫秒）
    float swipeMinDistance;   // 滑动最小距离
    float pinchMinDistance;   // 捏合最小距离
    float rotateMinAngle;     // 旋转最小角度（弧度）

    GestureConfig() : tapMaxDistance(20.0f), tapMaxDuration(300), doubleTapMaxInterval(400), longPressMinDuration(500), swipeMinDistance(50.0f), pinchMinDistance(20.0f), rotateMinAngle(0.1f)
    {
    }
};

void SetGestureConfig(const GestureConfig &config);
const GestureConfig &GetGestureConfig();

// 启用/禁用手势识别
void EnableGestureRecognition(bool enable);
bool IsGestureRecognitionEnabled();

} // namespace Input

#endif // INPUT_TOUCHHELPER_H