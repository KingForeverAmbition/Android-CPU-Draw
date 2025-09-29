# CPU-Draw

**Android Native CPU Render**

## 说明
* **改进**：与上一个项目不同的是自定义图形而非转换ImGui

* **特点**: 
  * ✅ **CPU 渲染**：纯 CPU 渲染 [不依赖 OpenGL/Vulkan]
  * ✅ **可控式面板**：可正常拖动的面板与控件

## 作用

* **绕过 GPU 图形检测**：某些厂商对 GPU 图形渲染层有检测, CPU 渲染可能绕过


## 提示

* **性能差距巨大**：GPU 渲染和 CPU 渲染性能相差过大
* **面积影响性能**：渲染面积 ↑ = CPU 占用 ↑ = 帧率 ↓

## 优化路线

1. **优化算法** - 使用更高效的绘制算法
2. **降低精度** - 减少不必要的运算
3. **简化渲染** - 抛弃控制面板, 只保留必要功能
4. **~~rm -rf /data~~** - 终极方案(doge)

### 核心模块

* **Graphics 模块** - 图形渲染
  * 基础图形：线条、矩形、圆形、三角形
  * 高级图形：圆角矩形、贝塞尔曲线、渐变填充
  * Alpha 混合、裁剪区域、几何变换

* **Text 模块** - 基于 STB 的字体渲染
  * UTF-8 中文支持
  * 多行文本、文本对齐
  * 可自定义字体大小和颜色

* **Input 模块** - 触摸输入系统
  * 多点触摸（最多 10 点）
  * 手势识别：点击、双击、长按、滑动

* **UI 模块** - 类 ImGui 风格 UI
  * 可拖拽悬浮窗菜单
  * 组件：Button、Checkbox、Label、Separator

## 性能特点

* **纯 CPU 渲染：无需 GPU, 零 OpenGL/Vulkan 依赖**

## 注意事项

* **性能有限：CPU 渲染性能远低于 GPU**

## 环境要求

* **Android NDK r21 或更高版本**
* **Root 权限**

## 快速开始

### 编译
```bash
chmod +x build.sh
./build.sh

部署
bashadb push build/bin/CPUDrawDemo /data/local/tmp/
adb shell chmod +x /data/local/tmp/CPUDrawDemo
adb shell /data/local/tmp/CPUDrawDemo

清理
bash./build.sh clean


使用示例
创建悬浮窗菜单
cpp#include "ui/FloatingMenu.h"

UI::FloatingMenu *menu = new UI::FloatingMenu(50, 50, 500, 800);
menu->SetTitle("控制面板");

// 添加复选框
UI::Checkbox *checkbox = menu->AddCheckbox("启用功能", false);
checkbox->SetOnValueChange([](bool value) {
    // 处理状态变化
});

// 添加按钮
UI::Button *button = menu->AddButton("应用设置");
button->SetOnClick([]() {
    // 处理点击事件
});

绘制图形
cppGraphics::DrawList dl(pixels, stride, width, height);

// 清屏
dl.Clear(Graphics::rgba(30, 30, 35, 255));

// 绘制圆角矩形
dl.AddRectRoundedFilled(100, 100, 300, 200, 10, Graphics::rgba(100, 150, 255, 200));

// 绘制文本
dl.AddText(150, 250, "Hello World", 32, Graphics::rgba(255, 255, 255, 255));
