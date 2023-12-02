#include "render.h"

namespace Posideon {
    Renderer initialize_renderer(uint32_t width, uint32_t height, HWND hwnd) {
        ComPtr<IDXGIFactory2> dxgi_factory;
        HRESULT res = CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory));
        POSIDEON_ASSERT(res >= 0)

        constexpr D3D_FEATURE_LEVEL device_feature_level = D3D_FEATURE_LEVEL_11_0;
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> device_context;
        res = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &device_feature_level, 1, D3D11_SDK_VERSION, &device, nullptr, &device_context);
        POSIDEON_ASSERT(res >= 0)

        DXGI_SWAP_CHAIN_DESC1 swapchain_descriptor {
            .Width = width,
            .Height = height,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            .SampleDesc = { .Count = 1, .Quality = 0 },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        };

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreen_descriptor {
            .Windowed = true
        };

        ComPtr<IDXGISwapChain1> swapchain;
        res = dxgi_factory->CreateSwapChainForHwnd(device.Get(), hwnd, &swapchain_descriptor, &fullscreen_descriptor, nullptr, &swapchain);
        POSIDEON_ASSERT(res >= 0)

        Renderer renderer { width, height, dxgi_factory, device, device_context, swapchain };
        renderer.create_swapchain_resources();
        return renderer;
    }

    void Renderer::create_swapchain_resources() {
        ID3D11Texture2D* backbuffer;
        HRESULT res = m_swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));
        POSIDEON_ASSERT(res >= 0);

        res = m_device->CreateRenderTargetView(backbuffer, nullptr, &m_render_target);
        POSIDEON_ASSERT(res >= 0);
    }

    void Renderer::destroy_swapchain_resources() {
        m_render_target.Reset();
    }

    void Renderer::on_resize(uint32_t width, uint32_t height) {
        m_device_context->Flush();

        destroy_swapchain_resources();

        HRESULT res = m_swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        POSIDEON_ASSERT(res >= 0)

        create_swapchain_resources();
    }

    void Renderer::render() {
        D3D11_VIEWPORT viewport {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width = static_cast<float>(m_width),
            .Height = static_cast<float>(m_height),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };

        constexpr float clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        m_device_context->ClearRenderTargetView(m_render_target.Get(), clear_color);
        m_device_context->RSSetViewports(1, &viewport);
        m_device_context->OMSetRenderTargets(1, m_render_target.GetAddressOf(), nullptr);

        m_swapchain->Present(1, 0);
    }
}