#include "win32_window.h"

namespace Posideon {
    LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    Win32Window::Win32Window(uint32_t width, uint32_t height) {
        m_width = width;
        m_height = height;

        WNDCLASSEX wc{
                .cbSize = sizeof(WNDCLASSEX),
                .style = CS_HREDRAW | CS_VREDRAW,
                .lpfnWndProc = wnd_proc,
                .hInstance = GetModuleHandle(nullptr),
                .hCursor = LoadCursor(nullptr, IDC_ARROW),
                .lpszClassName = "Posideon Engine",
        };
        RegisterClassEx(&wc);

        RECT windowRect = {.left = 0, .top = 0, .right = 1080, .bottom = 960};
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        m_hwnd = CreateWindow(
                wc.lpszClassName,
                "Posideon Engine",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                windowRect.right - windowRect.left,
                windowRect.bottom - windowRect.top,
                nullptr,
                nullptr,
                GetModuleHandle(nullptr),
                this
        );
        POSIDEON_ASSERT(m_hwnd != nullptr)

        ShowWindow(m_hwnd, SW_SHOW);
    }

    void Win32Window::run() {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        auto* window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch (msg) {
            case WM_CREATE: {
                auto create_struct = reinterpret_cast<LPCREATESTRUCT>(lParam);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
                return 0;
            }
            case WM_DESTROY: {
                PostQuitMessage(0);
                return 0;
            }
            default: {
                return DefWindowProc(hwnd, msg, wParam, lParam);
            }
        }
    }
}