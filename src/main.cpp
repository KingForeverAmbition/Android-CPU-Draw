/*
 * CPU-Draw
 * Created: 2025-09-29
 * By: MaySnowL
 * 
 * Android 原生 C++ CPU 渲染
 * 
 * 特性：
 * - 纯 CPU 渲染，无需 GPU 依赖
 * - 仿 ImGui 风格悬浮窗
 * - 横竖屏自动适配
 * 
 * 仅供学习和研究使用
 */


#include "graphics/DrawList.h"
#include "input/TouchHelper.h"
#include "platform/ANativeWindowCreator.h"
#include "ui/FloatingMenu.h"
#include <chrono>
#include <cstring>
#include <thread>

// 全局变量
ANativeWindow *g_nativeWindow = nullptr;
UI::FloatingMenu *g_mainMenu = nullptr;
UI::FloatingMenu *g_miniMenu = nullptr;

// 配置变量
int g_targetFps = 120;
bool g_showDemo = true;
bool g_showMainMenu = true;
bool g_showMiniMenu = false;

// ESP配置
struct ESPConfig
{
    bool showBox = true;
    bool showLine = false;
    bool showName = true;
    bool showDistance = true;
    bool showHealth = true;
} g_espConfig;

// 颜色配置
struct ColorConfig
{
    float boxColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float lineColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    float nameColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
} g_colorConfig;

// 字体大小配置
struct FontConfig
{
    float actorSize = 24.0f;
    float itemSize = 20.0f;
} g_fontConfig;

void InitMainMenu() {
    g_mainMenu = new UI::FloatingMenu(50, 50, 500, 800);
    g_mainMenu->SetTitle("功能控制面板");
    g_mainMenu->SetDraggable(true);
    g_mainMenu->SetAnimationEnabled(false);
    
    UI::FloatingMenu::Style style;
    style.backgroundColor = Graphics::rgba(244, 247, 250, 250);
    style.titleBarColor = Graphics::rgba(217, 230, 242, 255);
    style.borderColor = Graphics::rgba(179, 198, 217, 204);
    style.textColor = Graphics::rgba(38, 51, 71, 255);
    style.titleBarHeight = 70;
    style.padding = 28;
    style.itemSpacing = 16;
    style.cornerRadius = 15;
    style.showShadow = true;
    g_mainMenu->SetStyle(style);
    
    // 标题
    UI::Label *headerLabel = g_mainMenu->AddLabel("控制面板");
    headerLabel->SetTextColor(Graphics::rgba(64, 169, 140, 255));
    headerLabel->SetFontSize(32);
    headerLabel->SetAlignment(Graphics::TextAlign::Center);
    
    g_mainMenu->AddSeparator();
    
    // ESP功能
    UI::Label *espSection = g_mainMenu->AddLabel("ESP功能");
    espSection->SetTextColor(Graphics::rgba(38, 128, 217, 255));
    espSection->SetFontSize(24);
    

    UI::Checkbox *boxCheck = g_mainMenu->AddCheckbox("方框显示", false);
    boxCheck->SetChecked(g_espConfig.showBox);
    boxCheck->SetOnValueChange([](bool v) {
        g_espConfig.showBox = v;
    });
    
    UI::Checkbox *lineCheck = g_mainMenu->AddCheckbox("射线连接", false);
    lineCheck->SetChecked(g_espConfig.showLine);
    lineCheck->SetOnValueChange([](bool v) {
        g_espConfig.showLine = v;
    });
    
    UI::Checkbox *nameCheck = g_mainMenu->AddCheckbox("名字显示", false);
    nameCheck->SetChecked(g_espConfig.showName);
    nameCheck->SetOnValueChange([](bool v) {
        g_espConfig.showName = v;
    });
    
    UI::Checkbox *distCheck = g_mainMenu->AddCheckbox("距离显示", false);
    distCheck->SetChecked(g_espConfig.showDistance);
    distCheck->SetOnValueChange([](bool v) {
        g_espConfig.showDistance = v;
    });
    
    UI::Checkbox *healthCheck = g_mainMenu->AddCheckbox("血量显示", false);
    healthCheck->SetChecked(g_espConfig.showHealth);
    healthCheck->SetOnValueChange([](bool v) {
        g_espConfig.showHealth = v;
    });
    
    g_mainMenu->AddSeparator();
    
    // 显示设置
    UI::Label *displaySection = g_mainMenu->AddLabel("显示设置");
    displaySection->SetTextColor(Graphics::rgba(138, 89, 217, 255));
    displaySection->SetFontSize(24);
    
    UI::Checkbox *demoCheck = g_mainMenu->AddCheckbox("显示演示", false);
    demoCheck->SetChecked(g_showDemo);
    demoCheck->SetOnValueChange([](bool v) {
        g_showDemo = v;
    });
    
    g_mainMenu->AddSeparator();
    
    // FPS控制
    UI::Label *fpsSection = g_mainMenu->AddLabel("帧率控制");
    fpsSection->SetTextColor(Graphics::rgba(217, 140, 89, 255));
    fpsSection->SetFontSize(24);
    
    UI::Button *fps60 = g_mainMenu->AddButton("FPS: 60");
    fps60->SetOnClick([]() { g_targetFps = 60; });
    
    UI::Button *fps120 = g_mainMenu->AddButton("FPS: 120");
    fps120->SetOnClick([]() { g_targetFps = 120; });
    
    g_mainMenu->AddSeparator();
    
    // 收缩按钮
    UI::Button *miniBtn = g_mainMenu->AddButton("收缩到迷你窗");
    miniBtn->SetColors(
        Graphics::rgba(191, 64, 89, 230),
        Graphics::rgba(217, 89, 112, 230),
        Graphics::rgba(166, 39, 64, 230)
    );
    miniBtn->SetTextColor(Graphics::rgba(255, 255, 255, 255));
    miniBtn->SetOnClick([]() {
        g_showMainMenu = false;
        g_showMiniMenu = true;
    });
    
    g_mainMenu->UpdateLayout();
}


// 迷你窗口(没写好，将就用，建议自己改)
void InitMiniMenu()
{
    g_miniMenu = new UI::FloatingMenu(50, 50, 180, 140);
    g_miniMenu->SetTitle("迷你");
    g_miniMenu->SetDraggable(true);

    UI::FloatingMenu::Style style;
    style.backgroundColor = Graphics::rgba(230, 230, 235, 230);
    style.titleBarColor = Graphics::rgba(204, 204, 214, 255);
    style.textColor = Graphics::rgba(26, 26, 38, 255);
    style.titleBarHeight = 45;
    style.padding = 12;
    style.itemSpacing = 10;
    style.cornerRadius = 10;
    g_miniMenu->SetStyle(style);

    UI::Button *expandBtn = g_miniMenu->AddButton("展开主菜单");
    expandBtn->SetColors(Graphics::rgba(38, 128, 217, 220), Graphics::rgba(58, 148, 237, 220), Graphics::rgba(18, 108, 197, 220));
    expandBtn->SetTextColor(Graphics::rgba(255, 255, 255, 255));
    expandBtn->SetFontSize(20);
    expandBtn->SetOnClick(
        []()
        {
            g_showMainMenu = true;
            g_showMiniMenu = false;
        });

    UI::Label *fpsLabel = g_miniMenu->AddLabel("FPS: --");
    fpsLabel->SetTextColor(Graphics::rgba(100, 100, 100, 255));
    fpsLabel->SetFontSize(18);
}

// Demo演示 (从别的地方扣过来的，不知道叫啥)
void DrawDemoContent(Graphics::DrawList &dl, int width, int height)
{
    if (!g_showDemo) return;

    dl.AddRect(50, 50, 250, 250, Graphics::rgba(0, 255, 0, 255));
    dl.AddLine(0, 0, width - 1, height - 1, Graphics::rgba(255, 255, 0, 255));
    dl.AddRectFilled(300, 50, 400, 150, Graphics::rgba(0, 0, 255, 128));
    dl.AddCircle(600, 200, 50, Graphics::rgba(255, 0, 255, 255));
    dl.AddLineF(100.5f, 300.5f, 500.5f, 350.5f, Graphics::rgba(0, 255, 255, 255));

    My_Vector2 textSize = dl.CalcTextSize("Hello CPU Render!", 32);
    dl.AddText((int)(width / 2 - textSize.x / 2), height - 100, "Hello CPU Render!", 32, Graphics::rgba(255, 255, 255, 255));
    dl.AddRectRoundedFilled(650, 50, 800, 150, 10, Graphics::rgba(255, 128, 0, 200));
    dl.AddGradientLinear(50, 300, 250, 400, Graphics::rgba(255, 0, 0, 200), Graphics::rgba(0, 0, 255, 200));
}

// 绘制ESP演示
void DrawESPDemo(Graphics::DrawList &dl, int width, int height)
{
    int centerX = width / 2;
    int centerY = height / 2;
    int boxW = 100;
    int boxH = 180;

    uint32_t boxColor = Graphics::rgba(g_colorConfig.boxColor[0] * 255, g_colorConfig.boxColor[1] * 255, g_colorConfig.boxColor[2] * 255, g_colorConfig.boxColor[3] * 255);

    uint32_t lineColor = Graphics::rgba(g_colorConfig.lineColor[0] * 255, g_colorConfig.lineColor[1] * 255, g_colorConfig.lineColor[2] * 255, g_colorConfig.lineColor[3] * 255);

    uint32_t nameColor = Graphics::rgba(g_colorConfig.nameColor[0] * 255, g_colorConfig.nameColor[1] * 255, g_colorConfig.nameColor[2] * 255, g_colorConfig.nameColor[3] * 255);

    // 方框
    if (g_espConfig.showBox)
    {
        dl.AddRect(centerX - boxW / 2, centerY - boxH / 2, centerX + boxW / 2, centerY + boxH / 2, boxColor);
    }

    // 射线
    if (g_espConfig.showLine)
    {
        dl.AddLine(width / 2, height, centerX, centerY + boxH / 2, lineColor);
    }

    // 名字
    if (g_espConfig.showName)
    {
        dl.AddText(centerX - 30, centerY - boxH / 2 - 25, "蔡徐坤", (int)g_fontConfig.actorSize, nameColor);
    }

    // 距离
    if (g_espConfig.showDistance)
    {
        dl.AddText(centerX - 20, centerY + boxH / 2 + 5, "120m", (int)g_fontConfig.itemSize, Graphics::rgba(255, 255, 0, 255));
    }

    // 血量条
    if (g_espConfig.showHealth)
    {
        int healthBarW = boxW;
        int healthBarH = 6;
        int healthX = centerX - boxW / 2;
        int healthY = centerY + boxH / 2 + 25;

        // 背景
        dl.AddRectFilled(healthX, healthY, healthX + healthBarW, healthY + healthBarH, Graphics::rgba(60, 60, 60, 200));
        // 当前血量
        dl.AddRectFilled(healthX, healthY, healthX + healthBarW * 0.75f, healthY + healthBarH, Graphics::rgba(0, 255, 0, 220));
    }
}

// 主绘制函数
void DrawFrame(uint32_t *pixels, int stride, int width, int height) {
    memset(pixels, 0x00, stride * height * 4);
    
    Graphics::DrawList dl(pixels, stride, width, height);
    
    // 演示
    if (g_showDemo) {
        DrawDemoContent(dl, width, height);
    }
    
    // ESP
    DrawESPDemo(dl, width, height);
    
    // 绘制主菜单
    if (g_showMainMenu && g_mainMenu) {
        g_mainMenu->Draw(dl);
    }
    
    // 绘制迷你菜单
    if (g_showMiniMenu && g_miniMenu) {
        g_miniMenu->SetVisible(true);
        g_miniMenu->Draw(dl);
    }
}

// 触摸回调
void HandleTouchCallback(std::vector<Input::TouchDevice> *devices)
{
    if (g_showMainMenu && g_mainMenu)
    {
        g_mainMenu->HandleTouch(*devices);
    }

    if (g_showMiniMenu && g_miniMenu)
    {
        g_miniMenu->HandleTouch(*devices);
    }
}

// 渲染线程
void RenderThread()
{
    android::ANativeWindowCreator::DisplayInfo displayInfo = android::ANativeWindowCreator::GetDisplayInfo();

    int windowWidth = displayInfo.width;
    int windowHeight = displayInfo.height;

    // 创建窗口
    g_nativeWindow = android::ANativeWindowCreator::Create("CPU Draw RGBA Demo", windowWidth, windowHeight, false);

    if (!g_nativeWindow)
    {
        return;
    }

    int actualWidth = ANativeWindow_getWidth(g_nativeWindow);
    int actualHeight = ANativeWindow_getHeight(g_nativeWindow);

    My_Vector2 screenSize(actualWidth, actualHeight);

    if (!Input::Init(screenSize, true))
    {
        return;
    }

    // 设置屏幕方向
    if (actualWidth > actualHeight)
    {
        Input::SetOrientation(1); // 横屏
    }
    else
    {
        Input::SetOrientation(0); // 竖屏
    }

    Input::SetTouchCallback(HandleTouchCallback);

    // 初始化UI
    InitMainMenu();
    InitMiniMenu();


    int lastOrientation = displayInfo.width > displayInfo.height ? 1 : 0;
    // 主循环
    while (true)
    {
        auto frameStart = std::chrono::steady_clock::now();

        android::ANativeWindowCreator::DisplayInfo currentInfo = android::ANativeWindowCreator::GetDisplayInfo();

        int currentOrientation = currentInfo.width > currentInfo.height ? 1 : 0;

        if (currentOrientation != lastOrientation)
        {
            // 重新初始化触摸
            Input::Close();
            My_Vector2 newSize(currentInfo.width, currentInfo.height);
            Input::Init(newSize, true);
            Input::SetOrientation(currentOrientation);
            Input::SetTouchCallback(HandleTouchCallback);

            lastOrientation = currentOrientation;
        }

        int width = ANativeWindow_getWidth(g_nativeWindow);
        int height = ANativeWindow_getHeight(g_nativeWindow);
        ANativeWindow_setBuffersGeometry(g_nativeWindow, width, height, WINDOW_FORMAT_RGBA_8888);

        // 锁定缓冲区
        ANativeWindow_Buffer buffer;
        if (ANativeWindow_lock(g_nativeWindow, &buffer, nullptr) != 0)
        {
            break;
        }

        uint32_t *pixels = static_cast<uint32_t *>(buffer.bits);

        // 清空缓冲区
        memset(pixels, 0x00, buffer.stride * buffer.height * 4);

        DrawFrame(pixels, buffer.stride, width, height);

        ANativeWindow_unlockAndPost(g_nativeWindow);

        // 更新菜单动画
        float deltaTime = 0.008f;
        if (g_showMainMenu && g_mainMenu) {
            g_mainMenu->Update(deltaTime);
        }
        if (g_showMiniMenu && g_miniMenu) {
            g_miniMenu->Update(deltaTime);
        }

        // 帧率控制
        auto frameEnd = std::chrono::steady_clock::now();
        auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart).count();

        int targetFrameTime = 1000 / g_targetFps;
        if (frameDuration < targetFrameTime)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(targetFrameTime - frameDuration));
        }
    }

    Input::Close();
    delete g_mainMenu;
    delete g_miniMenu;
}

int main()
{
    std::thread renderThread(RenderThread);
    renderThread.join();
    return 0;
}






