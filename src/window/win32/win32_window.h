#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>

#include "defines.h"
#include "window/window.h"

namespace Posideon {
    struct Win32Window : public Window {
        HWND m_hwnd;
        uint32_t m_width;
        uint32_t m_height;

        Win32Window(uint32_t width, uint32_t height);

        virtual void run() override;
    };
}