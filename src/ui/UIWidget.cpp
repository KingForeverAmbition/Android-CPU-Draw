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

#include "ui/UIWidget.h"
#include "graphics/Primitives.h"
#include <algorithm>
#include <cmath>

namespace UI
{

// ==================== Button ====================

Button::Button(const std::string &text)
    : text(text), isHovered(false), isPressed(false), normalColor(Graphics::rgba(209, 224, 237, 255)), // 常规颜色
      hoverColor(Graphics::rgba(184, 206, 230, 255)),                                                  // 悬停颜色
      pressColor(Graphics::rgba(140, 179, 217, 255)),                                                  // 按下颜色
      textColor(Graphics::rgba(38, 51, 71, 255)),                                                      // 文本颜色
      fontSize(26), isRounded(true)                                                                    // 字体大小
{
    size = My_Vector2(200, 60);
}

void Button::Draw(Graphics::DrawList &dl)
{
    if (!visible) return;

    uint32_t currentColor = normalColor;
    if (!enabled)
    {
        currentColor = Graphics::rgba(200, 200, 200, 150);
    }
    else if (isPressed)
    {
        currentColor = pressColor;
    }
    else if (isHovered)
    {
        currentColor = hoverColor;
    }

    if (isRounded)
    {
        dl.AddRectRoundedFilled(pos.x, pos.y, pos.x + size.x, pos.y + size.y, 8, currentColor);
        dl.AddRectRounded(pos.x, pos.y, pos.x + size.x, pos.y + size.y, 8, Graphics::rgba(179, 198, 217, 200));
    }
    else
    {
        dl.AddRectFilled(pos.x, pos.y, pos.x + size.x, pos.y + size.y, currentColor);
        dl.AddRect(pos.x, pos.y, pos.x + size.x, pos.y + size.y, Graphics::rgba(179, 198, 217, 200));
    }

    My_Vector2 textSize = dl.CalcTextSize(text, fontSize);
    int textX = pos.x + (size.x - textSize.x) / 2;
    int textY = pos.y + (size.y - textSize.y) / 2;

    uint32_t finalTextColor = enabled ? textColor : Graphics::rgba(120, 120, 120, 255);
    dl.AddText(textX, textY, text, fontSize, finalTextColor);
}

bool Button::HandleTouch(const Input::TouchPoint &touch)
{
    if (!visible || !enabled) return false;

    bool contains = Contains(touch.pos);

    if (!touch.isDown)
    {
        if (activeTouchId == touch.id)
        {
            if (contains && isPressed && onClick)
            {
                onClick();
            }
            isPressed = false;
            isHovered = false;
            activeTouchId = -1;
        }
        return false;
    }

    if (contains)
    {
        if (activeTouchId == -1)
        {
            activeTouchId = touch.id;
            isHovered = true;
            isPressed = true;
            return true;
        }
        else if (activeTouchId == touch.id)
        {
            isHovered = true;
            isPressed = true;
            return true;
        }
    }
    else if (activeTouchId == touch.id)
    {
        isHovered = false;
        isPressed = false;
    }

    return activeTouchId == touch.id;
}

// ==================== Checkbox ====================

Checkbox::Checkbox(const std::string &label) : label(label), isChecked(false), isHovered(false)
{
    size = My_Vector2(300, 50);
}

void Checkbox::SetChecked(bool checked)
{
    if (isChecked != checked)
    {
        isChecked = checked;
        if (onValueChange)
        {
            onValueChange(isChecked);
        }
    }
}

void Checkbox::Draw(Graphics::DrawList &dl)
{
    if (!visible) return;

    int boxSize = 36;
    int boxX = pos.x;
    int boxY = pos.y + (size.y - boxSize) / 2;

    uint32_t bgColor = isHovered ? Graphics::rgba(224, 234, 242, 240) : Graphics::rgba(224, 234, 242, 220);

    dl.AddRectRoundedFilled(boxX, boxY, boxX + boxSize, boxY + boxSize, 6, bgColor);
    dl.AddRectRounded(boxX, boxY, boxX + boxSize, boxY + boxSize, 6, Graphics::rgba(179, 198, 217, 255));

    if (isChecked)
    {
        uint32_t checkColor = Graphics::rgba(64, 191, 115, 255);
        dl.AddLineThick(boxX + 9, boxY + 18, boxX + 15, boxY + 27, checkColor, 4);
        dl.AddLineThick(boxX + 15, boxY + 27, boxX + 27, boxY + 9, checkColor, 4);
    }

    if (!label.empty())
    {
        dl.AddText((int)(boxX + boxSize + 15), (int)(pos.y + (size.y - 26) / 2), label, 26, Graphics::rgba(38, 51, 71, 255));
    }
}

bool Checkbox::HandleTouch(const Input::TouchPoint &touch)
{
    if (!visible || !enabled) return false;

    bool contains = Contains(touch.pos);

    if (!touch.isDown)
    {
        if (activeTouchId == touch.id)
        {
            if (contains)
            {
                SetChecked(!isChecked);
            }
            isHovered = false;
            activeTouchId = -1;
        }
        return false;
    }

    if (contains)
    {
        if (activeTouchId == -1)
        {
            activeTouchId = touch.id;
            isHovered = true;
            return true;
        }
        else if (activeTouchId == touch.id)
        {
            isHovered = true;
            return true;
        }
    }
    else if (activeTouchId == touch.id)
    {
        isHovered = false;
    }

    return activeTouchId == touch.id;
}

// ==================== Label ====================

Label::Label(const std::string &text) : text(text), textColor(Graphics::rgba(38, 51, 71, 255)), fontSize(26), alignment(Graphics::TextAlign::Left)
{
    size = My_Vector2(300, 40);
}

void Label::Draw(Graphics::DrawList &dl)
{
    if (!visible) return;

    int textX = pos.x;
    My_Vector2 textSize = dl.CalcTextSize(text, fontSize);

    switch (alignment)
    {
    case Graphics::TextAlign::Center: textX = pos.x + (size.x - textSize.x) / 2; break;
    case Graphics::TextAlign::Right: textX = pos.x + size.x - textSize.x; break;
    default: break;
    }

    dl.AddText(textX, (int)pos.y, text, fontSize, textColor);
}

// ==================== Separator ====================

Separator::Separator() : color(Graphics::rgba(179, 198, 217, 200)), thickness(2)
{
    size = My_Vector2(300, 18);
}

void Separator::Draw(Graphics::DrawList &dl)
{
    if (!visible) return;

    int lineY = pos.y + size.y / 2;
    for (int i = 0; i < thickness; i++)
    {
        dl.AddLine(pos.x, lineY + i, pos.x + size.x, lineY + i, color);
    }
}

// ==================== TextInput ====================

TextInput::TextInput(const std::string &placeholder) : placeholder(placeholder), isFocused(false), cursorBlinkTime(0)
{
    size = My_Vector2(300, 50);
}

void TextInput::Draw(Graphics::DrawList &dl)
{
    if (!visible) return;

    uint32_t bgColor = isFocused ? Graphics::rgba(224, 234, 242, 220) : Graphics::rgba(234, 240, 245, 220);

    dl.AddRectRoundedFilled(pos.x, pos.y, pos.x + size.x, pos.y + size.y, 6, bgColor);
    dl.AddRectRounded(pos.x, pos.y, pos.x + size.x, pos.y + size.y, 6, isFocused ? Graphics::rgba(100, 150, 255, 255) : Graphics::rgba(179, 198, 217, 255));

    int textX = pos.x + 12;
    int textY = pos.y + (size.y - 24) / 2;

    if (!text.empty())
    {
        dl.AddText(textX, textY, text, 24, Graphics::rgba(38, 51, 71, 255));

        if (isFocused && ((int)(cursorBlinkTime * 2) % 2 == 0))
        {
            My_Vector2 textSize = dl.CalcTextSize(text, 24);
            dl.AddLine(textX + textSize.x + 3, textY, textX + textSize.x + 3, textY + 24, Graphics::rgba(38, 51, 71, 255));
        }
    }
    else if (!placeholder.empty())
    {
        dl.AddText(textX, textY, placeholder, 24, Graphics::rgba(150, 150, 150, 180));
    }
}

bool TextInput::HandleTouch(const Input::TouchPoint &touch)
{
    if (!visible || !enabled) return false;

    bool contains = Contains(touch.pos);

    if (!touch.isDown)
    {
        if (contains)
        {
            isFocused = true;
            return true;
        }
        else
        {
            isFocused = false;
        }
    }

    return isFocused;
}

} // namespace UI