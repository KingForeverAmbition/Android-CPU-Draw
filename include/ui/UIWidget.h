/*
 * CPU-Draw - UI Widget Module
 * Created: 2025-09-29
 * By: MaySnowL
 * 
 * UI 组件模块
 * 提供按钮, 复选框等常用控件
 * 
 * 特性：
 * - 触摸 ID 追踪
 * - 状态管理
 * - 回调机制
 * 
 * 仅供学习和研究使用
 */

#ifndef UI_UIWIDGET_H
#define UI_UIWIDGET_H

#include "core/VectorStruct.h"
#include "graphics/DrawList.h"
#include "input/TouchHelper.h"
#include <functional>
#include <string>

namespace UI
{

// UI组件基类
class Widget
{
  public:
    Widget() : pos(0, 0), size(100, 40), visible(true), enabled(true), id(0)
    {
    }
    virtual ~Widget()
    {
    }

    // 核心接口
    virtual void Draw(Graphics::DrawList &dl) = 0;
    virtual bool HandleTouch(const Input::TouchPoint &touch) = 0;
    virtual void Update(float deltaTime)
    {
    }

    // 位置和尺寸
    void SetPosition(const My_Vector2 &p)
    {
        pos = p;
    }
    void SetSize(const My_Vector2 &s)
    {
        size = s;
    }
    My_Vector2 GetPosition() const
    {
        return pos;
    }
    My_Vector2 GetSize() const
    {
        return size;
    }

    // 边界检测
    bool Contains(const My_Vector2 &point) const
    {
        return point.x >= pos.x && point.x <= pos.x + size.x && point.y >= pos.y && point.y <= pos.y + size.y;
    }

    // 可见性和启用状态
    void SetVisible(bool v)
    {
        visible = v;
    }
    void SetEnabled(bool e)
    {
        enabled = e;
    }
    bool IsVisible() const
    {
        return visible;
    }
    bool IsEnabled() const
    {
        return enabled;
    }

    // ID
    void SetId(int i)
    {
        id = i;
    }
    int GetId() const
    {
        return id;
    }

  protected:
    My_Vector2 pos;
    My_Vector2 size;
    bool visible;
    bool enabled;
    int id;
    int activeTouchId;
};

// 按钮组件
class Button : public Widget
{
  public:
    using ClickCallback = std::function<void()>;

    Button(const std::string &text = "Button");

    void Draw(Graphics::DrawList &dl) override;
    bool HandleTouch(const Input::TouchPoint &touch) override;

    void SetText(const std::string &text)
    {
        this->text = text;
    }
    std::string GetText() const
    {
        return text;
    }

    void SetOnClick(const ClickCallback &callback)
    {
        onClick = callback;
    }

    void SetColors(uint32_t normal, uint32_t hover, uint32_t press)
    {
        normalColor = normal;
        hoverColor = hover;
        pressColor = press;
    }

    void SetTextColor(uint32_t color)
    {
        textColor = color;
    }
    void SetFontSize(int size)
    {
        fontSize = size;
    }
    void SetRounded(bool rounded)
    {
        isRounded = rounded;
    }

  private:
    std::string text;
    ClickCallback onClick;
    bool isHovered;
    bool isPressed;
    uint32_t normalColor;
    uint32_t hoverColor;
    uint32_t pressColor;
    uint32_t textColor;
    int fontSize;
    bool isRounded;
};

// 滑块组件
class Slider : public Widget
{
  public:
    using ValueChangeCallback = std::function<void(float)>;

    Slider(float minValue = 0.0f, float maxValue = 100.0f);

    void Draw(Graphics::DrawList &dl) override;
    bool HandleTouch(const Input::TouchPoint &touch) override;

    void SetValue(float v);
    float GetValue() const
    {
        return value;
    }

    void SetRange(float min, float max)
    {
        minValue = min;
        maxValue = max;
        SetValue(value);
    }

    void SetLabel(const std::string &label)
    {
        this->label = label;
    }
    void SetShowValue(bool show)
    {
        showValue = show;
    }
    void SetOnValueChange(const ValueChangeCallback &callback)
    {
        onValueChange = callback;
    }

  private:
    std::string label;
    float value;
    float minValue;
    float maxValue;
    bool isDragging;
    bool showValue;
    ValueChangeCallback onValueChange;

    float GetPercentage() const
    {
        return (value - minValue) / (maxValue - minValue);
    }

    void SetFromX(float x);
};

// 复选框
class Checkbox : public Widget
{
  public:
    using ValueChangeCallback = std::function<void(bool)>;

    Checkbox(const std::string &label = "Checkbox");

    void Draw(Graphics::DrawList &dl) override;
    bool HandleTouch(const Input::TouchPoint &touch) override;

    void SetChecked(bool checked);
    bool IsChecked() const
    {
        return isChecked;
    }

    void SetLabel(const std::string &label)
    {
        this->label = label;
    }
    void SetOnValueChange(const ValueChangeCallback &callback)
    {
        onValueChange = callback;
    }

  private:
    std::string label;
    bool isChecked;
    bool isHovered;
    ValueChangeCallback onValueChange;
};

// 文本标签
class Label : public Widget
{
  public:
    Label(const std::string &text = "Label");

    void Draw(Graphics::DrawList &dl) override;
    bool HandleTouch(const Input::TouchPoint &touch) override
    {
        return false;
    }

    void SetText(const std::string &text)
    {
        this->text = text;
    }
    std::string GetText() const
    {
        return text;
    }

    void SetTextColor(uint32_t color)
    {
        textColor = color;
    }
    void SetFontSize(int size)
    {
        fontSize = size;
    }
    void SetAlignment(Graphics::TextAlign align)
    {
        alignment = align;
    }

  private:
    std::string text;
    uint32_t textColor;
    int fontSize;
    Graphics::TextAlign alignment;
};

// 输入框
class TextInput : public Widget
{
  public:
    using TextChangeCallback = std::function<void(const std::string &)>;

    TextInput(const std::string &placeholder = "");

    void Draw(Graphics::DrawList &dl) override;
    bool HandleTouch(const Input::TouchPoint &touch) override;

    void SetText(const std::string &text)
    {
        this->text = text;
    }
    std::string GetText() const
    {
        return text;
    }

    void SetPlaceholder(const std::string &ph)
    {
        placeholder = ph;
    }
    void SetOnTextChange(const TextChangeCallback &callback)
    {
        onTextChange = callback;
    }

    void SetFocused(bool focused)
    {
        isFocused = focused;
    }
    bool IsFocused() const
    {
        return isFocused;
    }

  private:
    std::string text;
    std::string placeholder;
    bool isFocused;
    TextChangeCallback onTextChange;
    float cursorBlinkTime;
};

// 分隔线组件
class Separator : public Widget
{
  public:
    Separator();

    void Draw(Graphics::DrawList &dl) override;
    bool HandleTouch(const Input::TouchPoint &touch) override
    {
        return false;
    }

    void SetColor(uint32_t color)
    {
        this->color = color;
    }
    void SetThickness(int thickness)
    {
        this->thickness = thickness;
    }

  private:
    uint32_t color;
    int thickness;
};

} // namespace UI

#endif // UI_UIWIDGET_H