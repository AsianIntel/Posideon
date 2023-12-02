#pragma once

#include "defines.h"
#include <dxgi1_3.h>
#include <d3d11.h>
#include <wrl.h>
#include <cstdint>
#include <utility>

namespace Posideon {
    template<typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    class Renderer {
        uint32_t m_width;
        uint32_t m_height;

        ComPtr<IDXGIFactory2> m_dxgi_factory;
        ComPtr<ID3D11Device> m_device;
        ComPtr<ID3D11DeviceContext> m_device_context;
        ComPtr<IDXGISwapChain1> m_swapchain;
        ComPtr<ID3D11RenderTargetView> m_render_target {};

    public:
        Renderer(uint32_t width, uint32_t height, ComPtr<IDXGIFactory2> factory, ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> device_context, ComPtr<IDXGISwapChain1> swapchain)
            :m_width(width), m_height(height),
            m_dxgi_factory(std::move(factory)), m_device(std::move(device)),
            m_device_context(std::move(device_context)), m_swapchain(std::move(swapchain)) {}

        void create_swapchain_resources();
        void destroy_swapchain_resources();
        void render();
        void on_resize(uint32_t width, uint32_t height);
    };

    Renderer initialize_renderer(uint32_t width, uint32_t height, HWND hwnd);
}