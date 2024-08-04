#include "Window.h"
#include "Core.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if(ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return 1;

    switch(msg)
    {
        case WM_CLOSE:
        {
            window->Close();
            break;
        }

        case WM_SIZE:
        {
            int width = LOWORD(lparam);
            int height = HIWORD(lparam);
            window->OnResize(width, height);
            break;
        }

        case WM_DESTROY:
        {
            window->Close();
            break;
        }

        default:
            return ::DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return 0;
}

Window::Window(int width, int height, LPCWSTR name)
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpszClassName = L"CorvusEngineWindowClass";
    wc.lpfnWndProc = &WndProc;

    if(!::RegisterClassExW(&wc))
        LOG(Error, "Window Class not created !");

    m_hwnd = ::CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, wc.lpszClassName, name, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, nullptr, this);

    if(!m_hwnd)
        LOG(Error, "Window not created !");

    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    ::ShowWindow(m_hwnd, SW_SHOW);
    ::UpdateWindow(m_hwnd);

    m_isRunning = true;
}

Window::~Window()
{
    ::DestroyWindow(m_hwnd);
}

void Window::BroadCast()
{
    MSG msg;
    while(PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) > 0)
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
}

void Window::Close()
{
    m_isRunning = false;
}

void Window::OnResize(int width, int height)
{
    if(m_resize)
        m_resize(width, height);
}

void Window::Maximize()
{
    // Get the monitor work area dimensions
    RECT workArea;
    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST), &monitorInfo);
    workArea = monitorInfo.rcWork;

    // Set the window position and size
    SetWindowPos(m_hwnd, HWND_TOP, workArea.left, workArea.top, workArea.right - workArea.left, workArea.bottom - workArea.top, SWP_SHOWWINDOW);
}

void Window::GetSize(uint32_t &width, uint32_t &height)
{
    RECT ClientRect;
    GetClientRect(m_hwnd, &ClientRect);
    width = ClientRect.right - ClientRect.left;
    height = ClientRect.bottom - ClientRect.top;
}