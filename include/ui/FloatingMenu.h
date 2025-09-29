/*
 * CPU-Draw - Floating Menu Module
 * Created: 2025-09-29
 * By: MaySnowL
 * 
 * 悬浮窗菜单模块
 * 可拖拽的 UI 面板
 * 
 * 特性：
 * - 拖拽支持
 * - 自动布局
 * - 样式定制
 * 
 * 仅供学习和研究使用
 */

#ifndef UI_FLOATINGMENU_H
#define UI_FLOATINGMENU_H

#include "graphics/DrawList.h"
#include "input/TouchHelper.h"
#include "ui/UIWidget.h"
#include <memory>
#include <vector>

namespace UI
{

// 悬浮窗菜单
class FloatingMenu
{
  public:
    FloatingMenu(float x, float y, float width, float height);
    ~FloatingMenu();

    // 绘制菜单
    void Draw(Graphics::DrawList &dl);

    // 处理触摸事件
    void HandleTouch(const std::vector<Input::TouchDevice> &devices);

    // 更新
    void Update(float deltaTime);

    // 添加组件
    Button *AddButton(const std::string &label);
    Slider *AddSlider(const std::string &label, float minValue, float maxValue, float initialValue = 0.0f);
    Checkbox *AddCheckbox(const std::string &label, bool initialValue = false);
    Label *AddLabel(const std::string &text);
    TextInput *AddTextInput(const std::string &placeholder = "");
    Separator *AddSeparator();

    // 添加自定义组件
    void AddWidget(Widget *widget);

    // 移除组件
    void RemoveWidget(int id);
    void ClearWidgets();

    // 菜单控制
    void SetVisible(bool visible)
    {
        isVisible = visible;
    }
    bool IsVisible() const
    {
        return isVisible;
    }

    void SetMinimized(bool minimized);
    bool IsMinimized() const
    {
        return isMinimized;
    }

    void SetPosition(const My_Vector2 &pos)
    {
        menuPos = pos;
        UpdateLayout();
    }
    My_Vector2 GetPosition() const
    {
        return menuPos;
    }

    void SetSize(const My_Vector2 &s)
    {
        menuSize = s;
        UpdateLayout();
    }
    My_Vector2 GetSize() const
    {
        return menuSize;
    }

    // 标题栏
    void SetTitle(const std::string &title)
    {
        this->title = title;
    }
    std::string GetTitle() const
    {
        return title;
    }

    // 样式配置
    struct Style
    {
        uint32_t backgroundColor;
        uint32_t titleBarColor;
        uint32_t borderColor;
        uint32_t textColor;
        int titleBarHeight;
        int borderWidth;
        int padding;
        int itemSpacing;
        int cornerRadius;
        bool showShadow;

        Style() : backgroundColor(Graphics::rgba(40, 40, 45, 240)), titleBarColor(Graphics::rgba(50, 50, 55, 255)), borderColor(Graphics::rgba(80, 80, 85, 255)), textColor(Graphics::rgba(220, 220, 220, 255)), titleBarHeight(50), borderWidth(2), padding(10), itemSpacing(8), cornerRadius(8), showShadow(true)
        {
        }
    };

    void SetStyle(const Style &style)
    {
        this->style = style;
    }
    const Style &GetStyle() const
    {
        return style;
    }

    // 可拖拽
    void SetDraggable(bool draggable)
    {
        isDraggable = draggable;
    }
    bool IsDraggable() const
    {
        return isDraggable;
    }

    // 可调整大小
    void SetResizable(bool resizable)
    {
        isResizable = resizable;
    }
    bool IsResizable() const
    {
        return isResizable;
    }

    // 自动布局
    void UpdateLayout();
    void SetAutoLayout(bool autoLayout)
    {
        this->autoLayout = autoLayout;
    }

    // 折叠/展开动画
    void SetAnimationEnabled(bool enabled)
    {
        animationEnabled = enabled;
    }
    bool IsAnimationEnabled() const
    {
        return animationEnabled;
    }

  private:
    // 菜单属性
    My_Vector2 menuPos;
    My_Vector2 menuSize;
    My_Vector2 minimizedSize;
    std::string title;
    bool isVisible;
    bool isMinimized;
    bool isDraggable;
    bool isResizable;
    bool autoLayout;
    bool animationEnabled;

    // 样式
    Style style;

    // 组件
    std::vector<Widget *> widgets;
    int nextWidgetId;

    // 交互
    bool isDragging;
    bool isResizing;
    My_Vector2 dragOffset;
    int activeTouchId;

    // 动画
    float animationProgress;
    float targetHeight;
    float currentHeight;

    // 方法
    void DrawTitleBar(Graphics::DrawList &dl);
    void DrawContent(Graphics::DrawList &dl);
    void DrawShadow(Graphics::DrawList &dl);
    void DrawResizeHandle(Graphics::DrawList &dl);

    bool IsPointInTitleBar(const My_Vector2 &point) const;
    bool IsPointInResizeHandle(const My_Vector2 &point) const;
    bool IsPointInMinimizeButton(const My_Vector2 &point) const;
    bool IsPointInCloseButton(const My_Vector2 &point) const;

    My_Vector2 GetContentOffset() const;
    float GetContentHeight() const;
};

// 菜单管理器
class MenuManager
{
  public:
    static MenuManager &Instance()
    {
        static MenuManager instance;
        return instance;
    }

    void AddMenu(FloatingMenu *menu);
    void RemoveMenu(FloatingMenu *menu);
    void ClearMenus();

    void DrawAll(Graphics::DrawList &dl);
    void HandleTouchAll(const std::vector<Input::TouchDevice> &devices);
    void UpdateAll(float deltaTime);

    void SetActiveMenu(FloatingMenu *menu);
    FloatingMenu *GetActiveMenu() const
    {
        return activeMenu;
    }

  private:
    MenuManager() : activeMenu(nullptr)
    {
    }
    ~MenuManager()
    {
    }
    MenuManager(const MenuManager &) = delete;
    MenuManager &operator=(const MenuManager &) = delete;

    std::vector<FloatingMenu *> menus;
    FloatingMenu *activeMenu;
};

} // namespace UI

#endif // UI_FLOATINGMENU_H