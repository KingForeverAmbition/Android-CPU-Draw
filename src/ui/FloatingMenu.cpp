#include "ui/FloatingMenu.h"
#include "graphics/Primitives.h"
#include <algorithm>
#include <cstring>

namespace UI
{

// ==================== FloatingMenu ====================

FloatingMenu::FloatingMenu(float x, float y, float width, float height) : menuPos(x, y), menuSize(width, height), minimizedSize(width, 50), title("Menu"), isVisible(true), isMinimized(false), isDraggable(true), isResizable(false), autoLayout(true), animationEnabled(true), nextWidgetId(1), isDragging(false), isResizing(false), activeTouchId(-1), animationProgress(0), targetHeight(height), currentHeight(height)
{
}

FloatingMenu::~FloatingMenu()
{
    for (auto widget : widgets)
    {
        delete widget;
    }
    widgets.clear();
}

void FloatingMenu::Draw(Graphics::DrawList &dl)
{
    if (!isVisible) return;

    // 绘制阴影
    if (style.showShadow && !isMinimized)
    {
        DrawShadow(dl);
    }

    // 绘制背景
    float drawHeight = isMinimized ? style.titleBarHeight : currentHeight;
    dl.AddRectRoundedFilled(menuPos.x, menuPos.y, menuPos.x + menuSize.x, menuPos.y + drawHeight, style.cornerRadius, style.backgroundColor);

    // 绘制边框
    dl.AddRectRounded(menuPos.x, menuPos.y, menuPos.x + menuSize.x, menuPos.y + drawHeight, style.cornerRadius, style.borderColor);

    // 绘制标题栏
    DrawTitleBar(dl);

    // 绘制内容区域
    if (!isMinimized)
    {
        DrawContent(dl);
    }

    // 绘制调整大小手柄
    if (isResizable && !isMinimized)
    {
        DrawResizeHandle(dl);
    }
}

void FloatingMenu::DrawTitleBar(Graphics::DrawList &dl)
{
    // 标题栏背景
    dl.AddRectRoundedFilled(menuPos.x, menuPos.y, menuPos.x + menuSize.x, menuPos.y + style.titleBarHeight, style.cornerRadius, style.titleBarColor);

    // 标题文本
    My_Vector2 titleSize = dl.CalcTextSize(title, 24);
    int titleX = menuPos.x + (menuSize.x - titleSize.x) / 2;
    int titleY = menuPos.y + (style.titleBarHeight - titleSize.y) / 2;
    dl.AddText(titleX, titleY, title, 24, style.textColor);

    // 最小化按钮
    int btnSize = 20;
    int btnY = menuPos.y + (style.titleBarHeight - btnSize) / 2;
    int btnX = menuPos.x + menuSize.x - btnSize - 10;

    dl.AddRectRoundedFilled(btnX, btnY, btnX + btnSize, btnY + btnSize, 3, Graphics::rgba(80, 80, 85, 200));

    // 绘制最小化图标（横线或加号）
    if (isMinimized)
    {
        // 加号表示展开
        dl.AddLine(btnX + btnSize / 2, btnY + 5, btnX + btnSize / 2, btnY + btnSize - 5, Graphics::rgba(200, 200, 200, 255));
        dl.AddLine(btnX + 5, btnY + btnSize / 2, btnX + btnSize - 5, btnY + btnSize / 2, Graphics::rgba(200, 200, 200, 255));
    }
    else
    {
        // 横线表示最小化
        dl.AddLine(btnX + 5, btnY + btnSize / 2, btnX + btnSize - 5, btnY + btnSize / 2, Graphics::rgba(200, 200, 200, 255));
    }
}

void FloatingMenu::DrawContent(Graphics::DrawList &dl)
{
    My_Vector2 contentOffset = GetContentOffset();

    // 设置裁剪区域
    //dl.PushClipRect(menuPos.x, menuPos.y + style.titleBarHeight, menuPos.x + menuSize.x, menuPos.y + currentHeight);

    // 绘制所有组件
    for (auto widget : widgets)
    {
        if (widget->IsVisible())
        {
            widget->Draw(dl);
        }
    }

    //dl.PopClipRect();
}

void FloatingMenu::DrawShadow(Graphics::DrawList &dl)
{
    // 简单的阴影效果
    int shadowOffset = 4;
    uint32_t shadowColor = Graphics::rgba(0, 0, 0, 60);

    dl.AddRectRoundedFilled(menuPos.x + shadowOffset, menuPos.y + shadowOffset, menuPos.x + menuSize.x + shadowOffset, menuPos.y + currentHeight + shadowOffset, style.cornerRadius, shadowColor);
}

void FloatingMenu::DrawResizeHandle(Graphics::DrawList &dl)
{
    int handleSize = 15;
    int handleX = menuPos.x + menuSize.x - handleSize;
    int handleY = menuPos.y + currentHeight - handleSize;

    // 绘制三条斜线表示可调整大小
    uint32_t handleColor = Graphics::rgba(120, 120, 120, 200);
    for (int i = 0; i < 3; i++)
    {
        int offset = i * 4;
        dl.AddLine(handleX + offset, handleY + handleSize, handleX + handleSize, handleY + offset, handleColor);
    }
}

void FloatingMenu::HandleTouch(const std::vector<Input::TouchDevice> &devices) {
    if (!isVisible) return;
    
    for (const auto &device : devices) {
        for (const auto &finger : device.fingers) {
            My_Vector2 screenPos = Input::TouchToScreen(finger.pos);
            
            // 触摸抬起处理
            if (!finger.isDown) {
                if (finger.id == activeTouchId) {
                    isDragging = false;
                    isResizing = false;
                    activeTouchId = -1;
                }
                
                // 让所有控件都处理抬起事件
                if (!isMinimized) {
                    for (auto widget : widgets) {
                        if (widget->IsVisible() && widget->IsEnabled()) {
                            Input::TouchPoint touch;
                            touch.pos = screenPos;
                            touch.startPos = Input::TouchToScreen(finger.startPos);
                            touch.isDown = false;
                            touch.id = finger.id;
                            widget->HandleTouch(touch);
                        }
                    }
                }
                continue;
            }
            
            // 检查最小化按钮
            if (IsPointInMinimizeButton(screenPos) && !finger.isDown) {
                SetMinimized(!isMinimized);
                return;
            }
            
            // 拖拽处理
            if (isDraggable && IsPointInTitleBar(screenPos) && activeTouchId == -1) {
                if (!isDragging) {
                    isDragging = true;
                    activeTouchId = finger.id;
                    dragOffset = screenPos - menuPos;
                }
            }
            
            if (isDragging && finger.id == activeTouchId) {
                My_Vector2 newPos = screenPos - dragOffset;
                // 只有位置真的变化时才更新
                if (newPos.x != menuPos.x || newPos.y != menuPos.y) {
                    menuPos = newPos;
                    UpdateLayout();
                }
                continue;  // 添加continue，避免继续处理控件
            }
            
            // 调整大小处理
            if (isResizable && !isMinimized && IsPointInResizeHandle(screenPos) && activeTouchId == -1) {
                if (!isResizing) {
                    isResizing = true;
                    activeTouchId = finger.id;
                    dragOffset = screenPos - menuPos - My_Vector2(menuSize.x, currentHeight);
                }
            }
            
            if (isResizing && finger.id == activeTouchId) {
                My_Vector2 delta = screenPos - menuPos - dragOffset;
                menuSize.x = std::max(200.0f, delta.x);
                targetHeight = std::max(100.0f, delta.y);
                currentHeight = targetHeight;
                UpdateLayout();
                return;
            }
            
            // 组件触摸处理
            if (!isMinimized && !isDragging && !isResizing) {
                bool anyWidgetHandled = false;
                
                for (auto widget : widgets) {
                    if (!widget->IsVisible() || !widget->IsEnabled()) continue;
                    
                    Input::TouchPoint touch;
                    touch.pos = screenPos;
                    touch.startPos = Input::TouchToScreen(finger.startPos);
                    touch.isDown = finger.isDown;
                    touch.id = finger.id;
                    
                    // 如果已经有控件在处理这个触摸ID，只让那个控件继续处理
                    if (widget->HandleTouch(touch)) {
                        anyWidgetHandled = true;
                        // 不要break，让其他控件也能处理抬起事件
                    }
                }
                
                if (anyWidgetHandled) {
                    continue;  // 已处理，跳过后续逻辑
                }
            }
        }
    }
}

void FloatingMenu::Update(float deltaTime)
{
    if (!isVisible) return;

    // 更新动画
    if (animationEnabled && std::abs(currentHeight - targetHeight) > 1.0f)
    {
        float speed = 800.0f; // 像素/秒
        float delta = targetHeight - currentHeight;
        float step = std::min(std::abs(delta), speed * deltaTime);
        currentHeight += (delta > 0 ? step : -step);
    }

    // 更新组件
    for (auto widget : widgets)
    {
        widget->Update(deltaTime);
    }
}

Button *FloatingMenu::AddButton(const std::string &label)
{
    Button *btn = new Button(label);
    btn->SetId(nextWidgetId++);
    widgets.push_back(btn);
    if (autoLayout) UpdateLayout();
    return btn;
}

Slider *FloatingMenu::AddSlider(const std::string &label, float minValue, float maxValue, float initialValue)
{
    Slider *slider = new Slider(minValue, maxValue);
    slider->SetLabel(label);
    slider->SetValue(initialValue);
    slider->SetId(nextWidgetId++);
    widgets.push_back(slider);
    if (autoLayout) UpdateLayout();
    return slider;
}

Checkbox *FloatingMenu::AddCheckbox(const std::string &label, bool initialValue)
{
    Checkbox *checkbox = new Checkbox(label);
    checkbox->SetChecked(initialValue);
    checkbox->SetId(nextWidgetId++);
    widgets.push_back(checkbox);
    if (autoLayout) UpdateLayout();
    return checkbox;
}

Label *FloatingMenu::AddLabel(const std::string &text)
{
    Label *label = new Label(text);
    label->SetId(nextWidgetId++);
    widgets.push_back(label);
    if (autoLayout) UpdateLayout();
    return label;
}

TextInput *FloatingMenu::AddTextInput(const std::string &placeholder)
{
    TextInput *input = new TextInput(placeholder);
    input->SetId(nextWidgetId++);
    widgets.push_back(input);
    if (autoLayout) UpdateLayout();
    return input;
}

Separator *FloatingMenu::AddSeparator()
{
    Separator *sep = new Separator();
    sep->SetId(nextWidgetId++);
    widgets.push_back(sep);
    if (autoLayout) UpdateLayout();
    return sep;
}

void FloatingMenu::AddWidget(Widget *widget)
{
    if (widget)
    {
        widget->SetId(nextWidgetId++);
        widgets.push_back(widget);
        if (autoLayout) UpdateLayout();
    }
}

void FloatingMenu::RemoveWidget(int id)
{
    for (auto it = widgets.begin(); it != widgets.end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            delete *it;
            widgets.erase(it);
            if (autoLayout) UpdateLayout();
            return;
        }
    }
}

void FloatingMenu::ClearWidgets()
{
    for (auto widget : widgets)
    {
        delete widget;
    }
    widgets.clear();
    if (autoLayout) UpdateLayout();
}

void FloatingMenu::SetMinimized(bool minimized)
{
    if (isMinimized != minimized)
    {
        isMinimized = minimized;
        if (animationEnabled)
        {
            targetHeight = isMinimized ? style.titleBarHeight : GetContentHeight();
        }
        else
        {
            currentHeight = isMinimized ? style.titleBarHeight : GetContentHeight();
        }
    }
}

void FloatingMenu::UpdateLayout() {
    // 添加脏标记，避免重复计算
    static bool layoutDirty = true;
    if (!layoutDirty && !autoLayout) return;
    
    My_Vector2 contentOffset = GetContentOffset();
    float currentY = contentOffset.y;
    
    for (auto widget : widgets) {
        My_Vector2 widgetSize = widget->GetSize();
        widgetSize.x = menuSize.x - style.padding * 2;
        widget->SetSize(widgetSize);
        widget->SetPosition(My_Vector2(menuPos.x + style.padding, currentY));
        currentY += widgetSize.y + style.itemSpacing;
    }
    
    float contentHeight = currentY - contentOffset.y + style.padding;
    targetHeight = style.titleBarHeight + contentHeight;
    
    if (!animationEnabled) {
        currentHeight = targetHeight;
    }
    
    layoutDirty = false;
}

My_Vector2 FloatingMenu::GetContentOffset() const
{
    return My_Vector2(menuPos.x + style.padding, menuPos.y + style.titleBarHeight + style.padding);
}

float FloatingMenu::GetContentHeight() const
{
    if (widgets.empty())
    {
        return style.titleBarHeight + style.padding * 2;
    }

    float totalHeight = style.titleBarHeight + style.padding;
    for (const auto widget : widgets)
    {
        totalHeight += widget->GetSize().y + style.itemSpacing;
    }
    totalHeight += style.padding;

    return totalHeight;
}

bool FloatingMenu::IsPointInTitleBar(const My_Vector2 &point) const
{
    return point.x >= menuPos.x && point.x <= menuPos.x + menuSize.x && point.y >= menuPos.y && point.y <= menuPos.y + style.titleBarHeight;
}

bool FloatingMenu::IsPointInResizeHandle(const My_Vector2 &point) const
{
    int handleSize = 15;
    return point.x >= menuPos.x + menuSize.x - handleSize && point.x <= menuPos.x + menuSize.x && point.y >= menuPos.y + currentHeight - handleSize && point.y <= menuPos.y + currentHeight;
}

bool FloatingMenu::IsPointInMinimizeButton(const My_Vector2 &point) const
{
    int btnSize = 20;
    int btnY = menuPos.y + (style.titleBarHeight - btnSize) / 2;
    int btnX = menuPos.x + menuSize.x - btnSize - 10;

    return point.x >= btnX && point.x <= btnX + btnSize && point.y >= btnY && point.y <= btnY + btnSize;
}

bool FloatingMenu::IsPointInCloseButton(const My_Vector2 &point) const
{
    // TODO: 实现关闭按钮检测
    return false;
}

// ==================== MenuManager ====================

void MenuManager::AddMenu(FloatingMenu *menu)
{
    if (menu)
    {
        menus.push_back(menu);
    }
}

void MenuManager::RemoveMenu(FloatingMenu *menu)
{
    auto it = std::find(menus.begin(), menus.end(), menu);
    if (it != menus.end())
    {
        menus.erase(it);
    }
}

void MenuManager::ClearMenus()
{
    menus.clear();
    activeMenu = nullptr;
}

void MenuManager::DrawAll(Graphics::DrawList &dl)
{
    for (auto menu : menus)
    {
        menu->Draw(dl);
    }
}

void MenuManager::HandleTouchAll(const std::vector<Input::TouchDevice> &devices)
{
    for (auto menu : menus)
    {
        menu->HandleTouch(devices);
    }
}

void MenuManager::UpdateAll(float deltaTime)
{
    for (auto menu : menus)
    {
        menu->Update(deltaTime);
    }
}

void MenuManager::SetActiveMenu(FloatingMenu *menu)
{
    activeMenu = menu;
}

} // namespace UI