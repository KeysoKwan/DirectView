#include "stdafx.h"
#include "D3d11Show.h"
#include <Windows.h>
#include <stdio.h>
#include "kwan/Tex2DPixelShader.inc"
#include "kwan/Tex2DVertexShader.inc"
#pragma comment(lib, "d3d11.lib")

using namespace std;
using namespace DirectX;

D3d11Show::D3d11Show() : m_sDevice(NULL),
                         m_deviceContext(NULL),
                         m_swapChain(NULL),
                         m_renderTargetView(NULL),
                         solidColorVS_(NULL),
                         solidColorPS_(NULL),
                         inputLayout_(NULL),
                         colorMapSampler_(NULL),
                         isRendering(false),
                         m_ViewhWnd(NULL),
                         m_w(0),
                         m_h(0),
                         m_isInit(false)
{
}

D3d11Show::~D3d11Show()
{
    RealeaseD3d();
}

void D3d11Show::InitD3D(HWND hWnd, int w, int h)
{
    //创建设备和上下文
    if (m_isInit)
        return;

    D3D_FEATURE_LEVEL myFeatureLevel;
    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDevice(
        NULL, // 默认显示适配器
        D3D_DRIVER_TYPE_HARDWARE,
        0, // 不使用软件设备
        createDeviceFlags,
        NULL, 0, // 默认的特征等级数组
        D3D11_SDK_VERSION,
        &m_sDevice,
        &myFeatureLevel,
        &m_deviceContext);

    if (FAILED(hr)) {
        MessageBox(NULL, L"Create Device failed!", L"error", MB_OK);
        return;
    }

    //4X多重采样质量等级
    UINT m4xMsaaQuality(0);
    m_sDevice->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        4,
        &m4xMsaaQuality);
    //准备交换链属性
    DXGI_SWAP_CHAIN_DESC sd = {0};
    sd.BufferDesc.Width = w;
    sd.BufferDesc.Height = h;
    sd.BufferDesc.RefreshRate.Numerator = 30;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    sd.SampleDesc.Count = 4;
    sd.SampleDesc.Quality = m4xMsaaQuality - 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = hWnd;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    //创建交换链
    IDXGIDevice* dxgiDevice(NULL);
    m_sDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)(&dxgiDevice));
    IDXGIAdapter* dxgiAdapter(NULL);
    dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)(&dxgiAdapter));
    IDXGIFactory* dxgiFactory(NULL);
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)(&dxgiFactory));
    hr = dxgiFactory->CreateSwapChain(m_sDevice, &sd, &m_swapChain);
    if (FAILED(hr)) {
        MessageBox(NULL, L"Create SwapChain failed!", L"error", MB_OK);
        return;
    }
    dxgiFactory->Release();
    dxgiAdapter->Release();
    dxgiDevice->Release();

    //创建渲染目标视图
    ID3D11Texture2D* backBuffer(NULL);
    m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    hr = m_sDevice->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView);
    if (FAILED(hr)) {
        MessageBox(NULL, L"Create RenderTargetView failed!", L"error", MB_OK);
        return;
    }
    if (backBuffer)
        backBuffer->Release();

    // 将视图绑定到输出合并器阶段
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, 0);

    // 设置视口
    D3D11_VIEWPORT vp = {0};
    vp.TopLeftX = 0.f;
    vp.TopLeftY = 0.f;
    vp.Width = static_cast<float>(w);
    vp.Height = static_cast<float>(h);
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;

    m_deviceContext->RSSetViewports(1, &vp);

    // shaders
    hr = m_sDevice->CreateVertexShader(g_Tex2DVertexShader, sizeof(g_Tex2DVertexShader), nullptr, &solidColorVS_);
    if (FAILED(hr)) {
        MessageBox(NULL, L"创建顶点着色器失败!", L"error", MB_OK);
        return;
    }
    hr = m_sDevice->CreatePixelShader(g_Tex2DPixelShader, sizeof(g_Tex2DPixelShader), nullptr, &solidColorPS_);
    if (FAILED(hr)) {
        MessageBox(NULL, L"创建像素着色器失败!", L"error", MB_OK);
        return;
    }

    // input layout
    if (solidColorVS_) {
        D3D11_INPUT_ELEMENT_DESC s_DX11InputElementDesc[] =
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };
        hr = m_sDevice->CreateInputLayout(s_DX11InputElementDesc, 2, g_Tex2DVertexShader, sizeof(g_Tex2DVertexShader), &inputLayout_);
        if (FAILED(hr)) {
            MessageBox(NULL, L"创建输入布局失败!", L"error", MB_OK);
            return;
        }
    }

    if (FAILED(hr)) {
        MessageBox(NULL, L"Create Buffer failed!", L"error", MB_OK);
        return;
    }

    m_isInit = true;

    D3D11_SAMPLER_DESC colorMapDesc;
    ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
    colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = m_sDevice->CreateSamplerState(&colorMapDesc, &colorMapSampler_);

    if (FAILED(hr)) {
        MessageBox(NULL, L"Create SamplerState failed!", L"error", MB_OK);
        return;
    }
    m_drawer = unique_ptr<DrawerManagerU3D>(new DrawerManagerU3D(m_sDevice));

    m_deviceContext->IASetInputLayout(inputLayout_);         //设置顶点格式
    m_deviceContext->VSSetShader(solidColorVS_, 0, 0);       //设置顶点着色器
    m_deviceContext->PSSetShader(solidColorPS_, 0, 0);       //设置像素着色器
    m_deviceContext->PSSetSamplers(0, 1, &colorMapSampler_); //设置纹理采样器
}

void D3d11Show::RealeaseD3d(bool isClearhWnd)
{
    isRendering = false;
    //等待渲染线程返回
    WaitForSingleObject(m_hSemaphore, 100);

    SafeRelease(colorMapSampler_);
    SafeRelease(solidColorVS_);
    SafeRelease(solidColorPS_);
    SafeRelease(inputLayout_);
    SafeRelease(m_renderTargetView);
    SafeRelease(m_swapChain);
    SafeRelease(m_deviceContext);
    SafeRelease(m_sDevice);

    if (isClearhWnd) {
        if (m_ViewhWnd) m_ViewhWnd = NULL;
    }
    m_isInit = false;
}

void D3d11Show::RenderTexture()
{
    if (m_sDevice == nullptr) {
        isRendering = false;
        return;
    }
    //渲染
    float clearColor[4] = {0.0f, 0.0f, 0.25f, 1.0f};                        //背景颜色
    m_deviceContext->ClearRenderTargetView(m_renderTargetView, clearColor); //清空视口
    m_drawer->RenderAllResource(m_deviceContext);                           //渲染所有物体
    HRESULT hr = m_swapChain->Present(1, 0);                                //垂直同步输出
    if (FAILED(hr)) {
        //Present Failed
        RealeaseD3d(false);
    }
}

void D3d11Show::EndRendering()
{
    isRendering = false;
    WaitForSingleObject(m_hSemaphore, 200);
    if (m_ViewhWnd != NULL) {
        ::PostMessage(m_ViewhWnd, WM_QUIT, 0, 0);
    }
}

void D3d11Show::SetupTextureHandle(void* textureHandle, RenderingResources::ResourceViewport type)
{
    if (m_sDevice == NULL || textureHandle == 0)
        return;
    m_drawer->PushResources(U3DshowResourcesPTR(new RenderingResources(m_sDevice, (ID3D11Texture2D*)textureHandle, type)));
}

void D3d11Show::SwichProjector(DrawerManagerU3D::ProjectionType type)
{
    m_drawer->UpdateAllMatrix(type);
}

int D3d11Show::StartRenderingView(HWND hWnd, void* textureHandle, int w, int h)
{
    isRendering = false;
    if (m_hSemaphore != NULL) {
        //等渲染待线程返回
        if (WaitForSingleObject(m_hSemaphore, 100) == WAIT_OBJECT_0) {
            ReleaseSemaphore(m_hSemaphore, 1, NULL);
        }
    }

    if (hWnd == NULL)
        return -2;
    if (textureHandle == nullptr)
        return -3;
    if (hWnd != m_ViewhWnd) {
        if (m_drawer != NULL) m_drawer.reset();
        RealeaseD3d(false);
    }

    m_ViewhWnd = hWnd;
    m_w = w;
    m_h = h;
    isRendering = true;
    thread t(&D3d11Show::DoRenderingView, this, 1, textureHandle);
    t.detach();
    return 1;
}

int D3d11Show::StartRenderingView(HWND hWnd, void* leftTexturePTR, void* rightTexturePTR, int w, int h)
{
    isRendering = false;
    if (m_hSemaphore != NULL) {
        //等渲染待线程返回
        if (WaitForSingleObject(m_hSemaphore, 100) == WAIT_OBJECT_0) {
            ReleaseSemaphore(m_hSemaphore, 1, NULL);
        }
    }

    if (hWnd == NULL)
        return -2;
    if (leftTexturePTR == nullptr || rightTexturePTR == nullptr)
        return -3;
    if (hWnd != m_ViewhWnd) {
        if (m_drawer != NULL) m_drawer.reset();
        RealeaseD3d(false);
    }

    m_ViewhWnd = hWnd;
    m_w = w;
    m_h = h;
    isRendering = true;
    m_renderingThread = thread(&D3d11Show::DoRenderingView, this, 2, leftTexturePTR, rightTexturePTR);
    m_renderingThread.detach();
    return 1;
}

void D3d11Show::DoRenderingView(int count, ...)
{
    InitD3D(m_ViewhWnd, m_w, m_h);
    m_drawer->ClearResources();
    va_list arg_ptr;
    void* nArgValue;
    va_start(arg_ptr, count);
    for (int i = 0; i < count; i++) {
        nArgValue = va_arg(arg_ptr, void*);
        if (count == 1)
            SetupTextureHandle(nArgValue, RenderingResources::FULL_VIEW);
        else
            SetupTextureHandle(nArgValue, (RenderingResources::ResourceViewport)(i + 1));
    }
    va_end(arg_ptr);
    if (count == 2) {
        m_drawer->UpdateAllMatrix(DrawerManagerU3D::T_3Dleftright);
    }
    else
        m_drawer->UpdateAllMatrix(DrawerManagerU3D::T_2D);

    m_hSemaphore = CreateSemaphoreA(NULL, 1, 1, m_SemaphoreName);
    WaitForSingleObject(m_hSemaphore, 100);

    const int constFps = 60;
    while (isRendering) {
        std::this_thread::yield();
        if (!IsWindow(m_ViewhWnd)) {
            EndRendering();
        }
        float timeInOneFps = 1000.0f / constFps;
        DWORD timeBegin = GetTickCount();
        if (!IsIconic(m_ViewhWnd)) {
            RenderTexture();
        }
        DWORD timeTotal = GetTickCount() - timeBegin;
        if (timeTotal < timeInOneFps)
            Sleep(DWORD(timeInOneFps - timeTotal));
    }
    ReleaseSemaphore(m_hSemaphore, 1, NULL);
}
