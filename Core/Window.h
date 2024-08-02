#pragma once
#include <Windows.h>
#include <functional>

class Window
{

public:
    Window(int width, int height, LPCWSTR name);
    ~Window();

    void BroadCast();
    bool IsRunning() { return m_isRunning; }
    void Close();
    void OnResize(int width, int height);
    void DefineOnResize(std::function<void(int width, int height)> resizeFunc) { m_resize = resizeFunc; }
    void Maximize();
    HWND GetHandle() { return m_hwnd; }
    void GetSize(uint32_t& width, uint32_t& height);

private:
    bool m_isRunning;
    HWND m_hwnd;

    std::function<void(int width, int height)> m_resize;
};