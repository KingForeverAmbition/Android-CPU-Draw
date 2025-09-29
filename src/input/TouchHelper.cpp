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

#include "input/TouchHelper.h"
#include "core/Utils.h"
#include "core/spinlock.h"
#include <cmath>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <sys/time.h>
#include <thread>
#include <unistd.h>

#define MAX_EVENTS 5
#define MAX_FINGERS 10
#define UNGRAB 0
#define GRAB 1

namespace Input
{

// 静态变量
static struct
{
    input_event downEvent[2]{ { {}, EV_KEY, BTN_TOUCH, 1 }, { {}, EV_KEY, BTN_TOOL_FINGER, 1 } };
    input_event event[512]{ 0 };
} g_input;

static My_Vector2 g_touchScale;
static My_Vector2 g_screenSize;
static std::vector<TouchDevice> g_devices;
static int g_outputFd = -1;
static int g_orientation = 0;
static bool g_initialized = false;
static bool g_readOnly = false;
static bool g_gestureEnabled = false;

static TouchCallback g_touchCallback;
static GestureCallback g_gestureCallback;
static GestureConfig g_gestureConfig;
static spinlock g_lock;

// 手势识别状态
static struct
{
    long long lastTapTime;
    My_Vector2 lastTapPos;
    int tapCount;
    long long touchStartTime;
    My_Vector2 touchStartPos;
    bool isLongPressing;
} g_gestureState = { 0 };

// 获取当前时间
static long long GetCurrentTimeMs()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

// 检查设备是否为触摸设备
static bool CheckDeviceIsTouch(int fd)
{
    uint8_t *bits = nullptr;
    ssize_t bits_size = 0;
    int res;
    bool hasSlot = false, hasPosX = false, hasPosY = false;

    while (true)
    {
        res = ioctl(fd, EVIOCGBIT(EV_ABS, bits_size), bits);
        if (res < bits_size) break;
        bits_size = res + 16;
        bits = (uint8_t *)realloc(bits, bits_size * 2);
    }

    for (int j = 0; j < res; j++)
    {
        for (int k = 0; k < 8; k++)
        {
            if (bits[j] & (1 << k))
            {
                int code = j * 8 + k;
                if (code == ABS_MT_SLOT) hasSlot = true;
                if (code == ABS_MT_POSITION_X) hasPosX = true;
                if (code == ABS_MT_POSITION_Y) hasPosY = true;
            }
        }
    }

    free(bits);
    return hasSlot && hasPosX && hasPosY;
}

// 识别手势
static void RecognizeGesture(const TouchDevice &device)
{
    if (!g_gestureEnabled || !g_gestureCallback) return;

    GestureData gesture;
    long long currentTime = GetCurrentTimeMs();
    int activeCount = 0;
    My_Vector2 avgPos;

    // 统计活跃触摸点
    for (int i = 0; i < MAX_FINGERS; i++)
    {
        if (device.fingers[i].isDown)
        {
            activeCount++;
            avgPos += device.fingers[i].pos;
        }
    }

    if (activeCount == 0)
    {
        // 触摸结束，检查是否为Tap或Swipe
        long long duration = currentTime - g_gestureState.touchStartTime;
        float distance = (avgPos - g_gestureState.touchStartPos).length();

        if (duration < g_gestureConfig.tapMaxDuration && distance < g_gestureConfig.tapMaxDistance)
        {
            // 可能是Tap或DoubleTap
            if (currentTime - g_gestureState.lastTapTime < g_gestureConfig.doubleTapMaxInterval && (avgPos - g_gestureState.lastTapPos).length() < g_gestureConfig.tapMaxDistance)
            {
                gesture.type = GestureType::DoubleTap;
                gesture.position = avgPos;
                g_gestureCallback(gesture);
                g_gestureState.tapCount = 0;
            }
            else
            {
                gesture.type = GestureType::Tap;
                gesture.position = avgPos;
                g_gestureCallback(gesture);
                g_gestureState.lastTapTime = currentTime;
                g_gestureState.lastTapPos = avgPos;
            }
        }
        else if (distance > g_gestureConfig.swipeMinDistance)
        {
            gesture.type = GestureType::Swipe;
            gesture.position = g_gestureState.touchStartPos;
            gesture.direction = (avgPos - g_gestureState.touchStartPos) / distance;
            gesture.distance = distance;
            g_gestureCallback(gesture);
        }

        g_gestureState.isLongPressing = false;
    }
    else
    {
        avgPos = avgPos / activeCount;
        gesture.fingerCount = activeCount;

        // 检查长按
        if (!g_gestureState.isLongPressing)
        {
            long long duration = currentTime - g_gestureState.touchStartTime;
            float distance = (avgPos - g_gestureState.touchStartPos).length();

            if (duration > g_gestureConfig.longPressMinDuration && distance < g_gestureConfig.tapMaxDistance)
            {
                gesture.type = GestureType::LongPress;
                gesture.position = avgPos;
                g_gestureCallback(gesture);
                g_gestureState.isLongPressing = true;
            }
        }

        // 检查Pinch和Rotate（需要2个触摸点）
        if (activeCount == 2)
        {
            TouchPoint *t1 = nullptr, *t2 = nullptr;
            for (int i = 0; i < MAX_FINGERS; i++)
            {
                if (device.fingers[i].isDown)
                {
                    if (!t1)
                        t1 = const_cast<TouchPoint *>(&device.fingers[i]);
                    else if (!t2)
                        t2 = const_cast<TouchPoint *>(&device.fingers[i]);
                }
            }

            if (t1 && t2)
            {
                float currentDist = (t1->pos - t2->pos).length();
                float startDist = (t1->startPos - t2->startPos).length();
                float distChange = std::abs(currentDist - startDist);

                if (distChange > g_gestureConfig.pinchMinDistance)
                {
                    gesture.type = GestureType::Pinch;
                    gesture.position = (t1->pos + t2->pos) / 2;
                    gesture.scale = currentDist / startDist;
                    g_gestureCallback(gesture);
                }
            }
        }
    }
}

// 触摸事件读取线程
static void *TouchReadThread(void *arg)
{
    int deviceIndex = (int)(long)arg;
    TouchDevice &device = g_devices[deviceIndex];

    int currentSlot = 0;
    input_event events[64]{ 0 };

    static long long lastCallbackTime = 0;
    const int MIN_CALLBACK_INTERVAL = 4; // 最多60fps

    while (g_initialized)
    {
        ssize_t readSize = read(device.fd, events, sizeof(events));
        if (readSize <= 0 || (readSize % sizeof(input_event)) != 0)
        {
            continue;
        }

        size_t count = readSize / sizeof(input_event);

        g_lock.lock();

        for (size_t i = 0; i < count; i++)
        {
            input_event &ie = events[i];

            if (ie.type == EV_ABS)
            {
                if (ie.code == ABS_MT_SLOT)
                {
                    currentSlot = ie.value;
                    if (currentSlot < 0 || currentSlot >= MAX_FINGERS)
                    {
                        currentSlot = 0;
                    }
                    continue;
                }

                if (ie.code == ABS_MT_TRACKING_ID)
                {
                    TouchPoint &finger = device.fingers[currentSlot];
                    if (ie.value == -1)
                    {
                        finger.isDown = false;
                    }
                    else
                    {
                        finger.id = (deviceIndex * 2 + 1) * MAX_FINGERS + currentSlot;
                        finger.isDown = true;
                        finger.startPos = finger.pos;
                        finger.timestamp = GetCurrentTimeMs();
                    }
                    continue;
                }

                if (ie.code == ABS_MT_POSITION_X)
                {
                    device.fingers[currentSlot].pos.x = (float)ie.value * device.scaleX;
                    continue;
                }

                if (ie.code == ABS_MT_POSITION_Y)
                {
                    device.fingers[currentSlot].pos.y = (float)ie.value * device.scaleY;
                    continue;
                }

                if (ie.code == ABS_MT_PRESSURE)
                {
                    device.fingers[currentSlot].pressure = (float)ie.value / 255.0f;
                    continue;
                }
            }

            if (ie.type == EV_SYN && ie.code == SYN_REPORT)
            {
                // 计算速度
                for (int j = 0; j < MAX_FINGERS; j++)
                {
                    TouchPoint &finger = device.fingers[j];
                    if (finger.isDown)
                    {
                        long long dt = GetCurrentTimeMs() - finger.timestamp;
                        if (dt > 0)
                        {
                            // 简单的速度估算
                            finger.velocity = finger.pos - finger.startPos;
                        }
                    }
                }

                // 手势识别
                if (g_gestureEnabled)
                {
                    RecognizeGesture(device);
                }

                long long currentTime = GetCurrentTimeMs();
                if (currentTime - lastCallbackTime < MIN_CALLBACK_INTERVAL)
                {
                    continue; // 跳过，不触发回调
                }
                lastCallbackTime = currentTime;

                // 触摸回调 - 修复：无论 readOnly 是否为 true 都应该调用回调
                if (g_touchCallback)
                {
                    g_touchCallback(&g_devices);
                }
                else if (!g_readOnly)
                {
                    // 只有在非只读模式下才上传触摸事件
                    Upload();
                }
                continue;
            }
        }

        g_lock.unlock();
    }

    return nullptr;
}

bool Init(const My_Vector2 &screenSize, bool readOnly)
{
    Close();

    g_devices.clear();
    g_readOnly = readOnly;

    if (screenSize.x > screenSize.y)
    {
        g_screenSize = screenSize;
    }
    else
    {
        g_screenSize = My_Vector2(screenSize.y, screenSize.x);
    }

    DIR *dir = opendir("/dev/input/");
    if (!dir) return false;

    dirent *entry = nullptr;
    int eventCount = 0;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (strstr(entry->d_name, "event"))
        {
            eventCount++;
        }
    }
    closedir(dir);

    for (int i = 0; i <= eventCount; i++)
    {
        char path[128];
        sprintf(path, "/dev/input/event%d", i);

        int fd = open(path, O_RDWR);
        if (fd < 0) continue;

        if (CheckDeviceIsTouch(fd))
        {
            TouchDevice device;

            if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &device.absX) == 0 && ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &device.absY) == 0)
            {
                device.fd = fd;

                if (!readOnly)
                {
                    ioctl(fd, EVIOCGRAB, GRAB);
                }

                g_devices.push_back(device);
            }
        }
        else
        {
            close(fd);
        }
    }

    if (g_devices.empty())
    {
        return false;
    }

    // 获取触摸范围
    int touchWidth = g_devices[0].absX.maximum;
    int touchHeight = g_devices[0].absY.maximum;

    // 创建虚拟触摸设备
    if (!readOnly)
    {
        g_outputFd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
        if (g_outputFd <= 0)
        {
            return false;
        }

        struct uinput_user_dev uiDev;
        memset(&uiDev, 0, sizeof(uiDev));

        std::string devName = getRandomString(10);
        strncpy(uiDev.name, devName.c_str(), UINPUT_MAX_NAME_SIZE);

        uiDev.id.bustype = 0;
        uiDev.id.vendor = getRandomNumber(1, 100);
        uiDev.id.product = getRandomNumber(1, 100);
        uiDev.id.version = getRandomNumber(1, 100);

        ioctl(g_outputFd, UI_SET_PROPBIT, INPUT_PROP_DIRECT);
        ioctl(g_outputFd, UI_SET_EVBIT, EV_ABS);
        ioctl(g_outputFd, UI_SET_ABSBIT, ABS_X);
        ioctl(g_outputFd, UI_SET_ABSBIT, ABS_Y);
        ioctl(g_outputFd, UI_SET_ABSBIT, ABS_MT_POSITION_X);
        ioctl(g_outputFd, UI_SET_ABSBIT, ABS_MT_POSITION_Y);
        ioctl(g_outputFd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);
        ioctl(g_outputFd, UI_SET_EVBIT, EV_SYN);
        ioctl(g_outputFd, UI_SET_EVBIT, EV_KEY);
        ioctl(g_outputFd, UI_SET_KEYBIT, BTN_TOOL_FINGER);
        ioctl(g_outputFd, UI_SET_KEYBIT, BTN_TOUCH);

        uiDev.absmin[ABS_MT_POSITION_X] = 0;
        uiDev.absmax[ABS_MT_POSITION_X] = touchWidth;
        uiDev.absmin[ABS_MT_POSITION_Y] = 0;
        uiDev.absmax[ABS_MT_POSITION_Y] = touchHeight;
        uiDev.absmin[ABS_X] = 0;
        uiDev.absmax[ABS_X] = touchWidth;
        uiDev.absmin[ABS_Y] = 0;
        uiDev.absmax[ABS_Y] = touchHeight;
        uiDev.absmin[ABS_MT_TRACKING_ID] = 0;
        uiDev.absmax[ABS_MT_TRACKING_ID] = 65535;

        write(g_outputFd, &uiDev, sizeof(uiDev));

        if (ioctl(g_outputFd, UI_DEV_CREATE))
        {
            return false;
        }
    }

    // 设置缩放比例
    for (auto &device : g_devices)
    {
        device.scaleX = (float)touchWidth / (float)device.absX.maximum;
        device.scaleY = (float)touchHeight / (float)device.absY.maximum;
    }

    My_Vector2 actualSize = screenSize;
    if (actualSize.x > actualSize.y)
    {
        std::swap(actualSize.x, actualSize.y);
    }

    g_touchScale.x = (float)touchWidth / actualSize.x;
    g_touchScale.y = (float)touchHeight / actualSize.y;

    g_initialized = true;

    for (size_t i = 0; i < g_devices.size(); i++)
    {
        pthread_t thread;
        pthread_create(&thread, nullptr, TouchReadThread, (void *)(long)i);
        pthread_detach(thread);
    }

    return true;
}

void Close()
{
    if (g_initialized)
    {
        g_initialized = false;

        for (auto &device : g_devices)
        {
            if (!g_readOnly)
            {
                ioctl(device.fd, EVIOCGRAB, UNGRAB);
            }
            close(device.fd);
        }

        if (g_outputFd > 0)
        {
            ioctl(g_outputFd, UI_DEV_DESTROY);
            close(g_outputFd);
            g_outputFd = -1;
        }

        g_devices.clear();
        memset(g_input.event, 0, sizeof(g_input.event));
    }
}

bool IsInitialized()
{
    return g_initialized;
}

void Upload()
{
    static bool isFirstDown = true;
    int eventCount = 0;
    int fingerCount = 0;

    for (auto &device : g_devices)
    {
        for (auto &finger : device.fingers)
        {
            if (finger.isDown)
            {
                if (fingerCount++ > 20) goto finish;

                g_input.event[eventCount++] = { .type = EV_ABS, .code = ABS_X, .value = (int)finger.pos.x };
                g_input.event[eventCount++] = { .type = EV_ABS, .code = ABS_Y, .value = (int)finger.pos.y };
                g_input.event[eventCount++] = { .type = EV_ABS, .code = ABS_MT_POSITION_X, .value = (int)finger.pos.x };
                g_input.event[eventCount++] = { .type = EV_ABS, .code = ABS_MT_POSITION_Y, .value = (int)finger.pos.y };
                g_input.event[eventCount++] = { .type = EV_ABS, .code = ABS_MT_TRACKING_ID, .value = finger.id };
                g_input.event[eventCount++] = { .type = EV_SYN, .code = SYN_MT_REPORT, .value = 0 };
            }
        }
    }

finish:
    bool hasTouch = false;
    if (eventCount == 0)
    {
        g_input.event[eventCount++] = { .type = EV_SYN, .code = SYN_MT_REPORT, .value = 0 };
        if (!isFirstDown)
        {
            isFirstDown = true;
            g_input.event[eventCount++] = { .type = EV_KEY, .code = BTN_TOUCH, .value = 0 };
            g_input.event[eventCount++] = { .type = EV_KEY, .code = BTN_TOOL_FINGER, .value = 0 };
        }
    }
    else
    {
        hasTouch = true;
    }

    g_input.event[eventCount++] = { .type = EV_SYN, .code = SYN_REPORT, .value = 0 };

    if (hasTouch && isFirstDown)
    {
        isFirstDown = false;
        write(g_outputFd, &g_input, sizeof(input_event) * (eventCount + 2));
    }
    else
    {
        write(g_outputFd, g_input.event, sizeof(input_event) * eventCount);
    }
}

void Down(float x, float y, int touchId)
{
    if (g_devices.empty()) return;

    g_lock.lock();
    TouchPoint &touch = g_devices[0].fingers[9];
    touch.id = touchId >= 0 ? touchId : 19;
    touch.pos = My_Vector2(x, y) * g_touchScale;
    touch.startPos = touch.pos;
    touch.isDown = true;
    touch.timestamp = GetCurrentTimeMs();
    Upload();
    g_lock.unlock();
}

void Move(float x, float y, int touchId)
{
    if (g_devices.empty()) return;

    g_lock.lock();
    TouchPoint &touch = g_devices[0].fingers[9];
    touch.pos = My_Vector2(x, y) * g_touchScale;
    Upload();
    g_lock.unlock();
}

void Up(int touchId)
{
    if (g_devices.empty()) return;

    g_lock.lock();
    TouchPoint &touch = g_devices[0].fingers[9];
    touch.isDown = false;
    Upload();
    g_lock.unlock();
}

void TouchAt(const My_Vector2 &pos, int durationMs)
{
    Down(pos.x, pos.y);
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    Up();
}

void Swipe(const My_Vector2 &start, const My_Vector2 &end, int durationMs)
{
    Down(start.x, start.y);

    int steps = durationMs / 16; // ~60fps
    My_Vector2 delta = (end - start) / steps;

    for (int i = 1; i < steps; i++)
    {
        My_Vector2 pos = start + delta * i;
        Move(pos.x, pos.y);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    Move(end.x, end.y);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    Up();
}

void MultiTouch(const std::vector<My_Vector2> &positions)
{
    // 简单实现：同时按下多个点
    // TODO: 更复杂的多点触摸实现
}

void SetTouchCallback(const TouchCallback &callback)
{
    g_touchCallback = callback;
}

void SetGestureCallback(const GestureCallback &callback)
{
    g_gestureCallback = callback;
}

My_Vector2 TouchToScreen(const My_Vector2 &touchCoord)
{
    float x = touchCoord.x, y = touchCoord.y;
    float xt = x / g_touchScale.x;
    float yt = y / g_touchScale.y;

    switch (g_orientation)
    {
    case 1:
        x = yt;
        y = g_screenSize.y - xt;
        break;
    case 2:
        x = g_screenSize.x - xt;
        y = g_screenSize.y - yt;
        break;
    case 3:
        y = xt;
        x = g_screenSize.x - yt;
        break;
    default:
        x = xt;
        y = yt;
        break;
    }

    return My_Vector2(x, y);
}

My_Vector2 ScreenToTouch(const My_Vector2 &screenCoord)
{
    return screenCoord * g_touchScale;
}

My_Vector2 GetScale()
{
    return g_touchScale;
}

void SetOrientation(int orientation)
{
    g_orientation = orientation % 4;
}

const std::vector<TouchDevice> &GetDevices()
{
    return g_devices;
}

std::vector<TouchPoint> GetActiveTouches()
{
    std::vector<TouchPoint> activeTouches;

    for (const auto &device : g_devices)
    {
        for (const auto &finger : device.fingers)
        {
            if (finger.isDown)
            {
                activeTouches.push_back(finger);
            }
        }
    }

    return activeTouches;
}

int GetTouchCount()
{
    int count = 0;

    for (const auto &device : g_devices)
    {
        for (const auto &finger : device.fingers)
        {
            if (finger.isDown)
            {
                count++;
            }
        }
    }

    return count;
}

bool IsTouching()
{
    return GetTouchCount() > 0;
}

bool IsTouchingAt(const My_Vector2 &pos, float radius)
{
    My_Vector2 touchPos = ScreenToTouch(pos);
    float radiusSq = radius * radius;

    for (const auto &device : g_devices)
    {
        for (const auto &finger : device.fingers)
        {
            if (finger.isDown)
            {
                My_Vector2 screenFinger = TouchToScreen(finger.pos);
                float distSq = (screenFinger - pos).x * (screenFinger - pos).x + (screenFinger - pos).y * (screenFinger - pos).y;
                if (distSq <= radiusSq)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

TouchPoint *GetTouchById(int id)
{
    for (auto &device : g_devices)
    {
        for (auto &finger : device.fingers)
        {
            if (finger.isDown && finger.id == id)
            {
                return &finger;
            }
        }
    }

    return nullptr;
}

TouchPoint *GetNearestTouch(const My_Vector2 &pos)
{
    TouchPoint *nearest = nullptr;
    float minDist = std::numeric_limits<float>::max();
    My_Vector2 touchPos = ScreenToTouch(pos);

    for (auto &device : g_devices)
    {
        for (auto &finger : device.fingers)
        {
            if (finger.isDown)
            {
                float dist = (finger.pos - touchPos).length();
                if (dist < minDist)
                {
                    minDist = dist;
                    nearest = &finger;
                }
            }
        }
    }

    return nearest;
}

void SetGestureConfig(const GestureConfig &config)
{
    g_gestureConfig = config;
}

const GestureConfig &GetGestureConfig()
{
    return g_gestureConfig;
}

void EnableGestureRecognition(bool enable)
{
    g_gestureEnabled = enable;
}

bool IsGestureRecognitionEnabled()
{
    return g_gestureEnabled;
}

} // namespace Input