#include "Renderer.hpp"

namespace x {
    bool Renderer::Initialize(HWND hwnd, int width, int height) {
        // Create device and swap chain
        DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
        swapChainDesc.BufferCount                        = 1;
        swapChainDesc.BufferDesc.Width                   = width;
        swapChainDesc.BufferDesc.Height                  = height;
        swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow                       = hwnd;
        swapChainDesc.SampleDesc.Count                   = 1;
        swapChainDesc.SampleDesc.Quality                 = 0;
        swapChainDesc.Windowed                           = TRUE;

        D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};

        UINT numFeatureLevels = ARRAYSIZE(featureLevels);
        D3D_FEATURE_LEVEL featureLevel;

        UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        #ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif

        // Create device and swap chain
        HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr,
                                                   // Default adapter
                                                   D3D_DRIVER_TYPE_HARDWARE,
                                                   // Hardware acceleration
                                                   nullptr,
                                                   // No software device
                                                   createDeviceFlags,
                                                   // Debug mode if needed
                                                   featureLevels,
                                                   numFeatureLevels,
                                                   D3D11_SDK_VERSION,
                                                   &swapChainDesc,
                                                   &_swapChain,
                                                   &_device,
                                                   &featureLevel,
                                                   &_context);

        if (FAILED(hr)) { return false; }

        // Create render target view
        hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &_backBuffer);
        if (FAILED(hr)) { return false; }

        hr = _device->CreateRenderTargetView(_backBuffer.Get(), nullptr, &_renderTargetView);
        if (FAILED(hr)) { return false; }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC depthStencilDesc = {};
        depthStencilDesc.Width                = width;
        depthStencilDesc.Height               = height;
        depthStencilDesc.MipLevels            = 1;
        depthStencilDesc.ArraySize            = 1;
        depthStencilDesc.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.SampleDesc.Count     = 1;
        depthStencilDesc.SampleDesc.Quality   = 0;
        depthStencilDesc.Usage                = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL;

        ComPtr<ID3D11Texture2D> depthStencilTexture;
        hr = _device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilTexture);
        if (FAILED(hr)) { return false; }

        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format                        = depthStencilDesc.Format;
        depthStencilViewDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2D;

        hr = _device->CreateDepthStencilView(depthStencilTexture.Get(), &depthStencilViewDesc, &_depthStencilView);
        if (FAILED(hr)) { return false; }

        // Create depth stencil state
        D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
        depthStencilStateDesc.DepthEnable              = TRUE;
        depthStencilStateDesc.DepthWriteMask           = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilStateDesc.DepthFunc                = D3D11_COMPARISON_LESS;
        depthStencilStateDesc.StencilEnable            = FALSE;

        hr = _device->CreateDepthStencilState(&depthStencilStateDesc, &_depthStencilState);
        if (FAILED(hr)) { return false; }

        // Set render targets and viewport
        _context->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());
        _context->OMSetDepthStencilState(_depthStencilState.Get(), 0);

        D3D11_VIEWPORT viewport = {};
        viewport.Width          = CAST<f32>(width);
        viewport.Height         = CAST<f32>(height);
        viewport.MinDepth       = 0.0f;
        viewport.MaxDepth       = 1.0f;
        viewport.TopLeftX       = 0.0f;
        viewport.TopLeftY       = 0.0f;

        _context->RSSetViewports(1, &viewport);

        return true;
    }

    void Renderer::ResizeSwapchainBuffers(u32 width, u32 height) {
        _renderTargetView.Reset();
        _depthStencilView.Reset();
        _backBuffer.Reset();

        DX_THROW_IF_FAILED(_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0))

        DX_THROW_IF_FAILED(_swapChain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer)))
        DX_THROW_IF_FAILED(_device->CreateRenderTargetView(_backBuffer.Get(), None, &_renderTargetView))

        D3D11_TEXTURE2D_DESC depthStencilDesc = {};
        depthStencilDesc.Width                = width;
        depthStencilDesc.Height               = height;
        depthStencilDesc.MipLevels            = 1;
        depthStencilDesc.ArraySize            = 1;
        depthStencilDesc.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.SampleDesc.Count     = 1;
        depthStencilDesc.SampleDesc.Quality   = 0;
        depthStencilDesc.Usage                = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL;

        ComPtr<ID3D11Texture2D> depthStencilBuffer;
        DX_THROW_IF_FAILED(_device->CreateTexture2D(&depthStencilDesc, None, &depthStencilBuffer))
        DX_THROW_IF_FAILED(_device->CreateDepthStencilView(depthStencilBuffer.Get(), None, &_depthStencilView))

        _context->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());
    }

    void Renderer::BeginFrame() {
        constexpr float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        BeginFrame(clearColor);
    }

    void Renderer::BeginFrame(const f32 clearColor[4]) {
        _context->ClearRenderTargetView(_renderTargetView.Get(), clearColor);
        _context->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    void Renderer::EndFrame() {
        DX_THROW_IF_FAILED(_swapChain->Present(0, 0));
    }

    void Renderer::OnResize(u32 width, u32 height) {
        ResizeSwapchainBuffers(width, height);

        D3D11_VIEWPORT viewport;
        viewport.Width    = CAST<f32>(width);
        viewport.Height   = CAST<f32>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;

        _context->RSSetViewports(1, &viewport);
    }
}