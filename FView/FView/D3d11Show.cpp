#include "stdafx.h"
#include "D3d11Show.h"
#include <Windows.h>
#include <stdio.h>
#include <thread>

#pragma comment(lib, "d3d11.lib")

using namespace std;
using namespace DirectX;

D3d11Show::D3d11Show()
{
    m_sDevice = NULL;
    m_deviceContext = NULL;
    m_swapChain = NULL;
    m_renderTargetView = NULL;
    solidColorVS_ = NULL;
    solidColorPS_ = NULL;
    inputLayout_ = NULL;
    vertexBuffer_ = NULL;
    colorMap_ = NULL;
    colorMapSampler_ = NULL;
    isRendering = false;
    m_ViewhWnd = NULL;
    void* m_textureHandle = NULL;
    m_w = 0;
    m_h = 0;
}

D3d11Show::~D3d11Show()
{
}

struct VertexPos //结构体
{

    XMFLOAT3 pos;
    XMFLOAT2 tex0;
};

const BYTE vShaderCode[] =
    {
        68, 88, 66, 67, 35, 175,
        127, 40, 17, 17, 32, 52,
        169, 166, 100, 144, 21, 252,
        62, 125, 1, 0, 0, 0,
        164, 2, 0, 0, 6, 0,
        0, 0, 56, 0, 0, 0,
        180, 0, 0, 0, 36, 1,
        0, 0, 160, 1, 0, 0,
        248, 1, 0, 0, 76, 2,
        0, 0, 65, 111, 110, 57,
        116, 0, 0, 0, 116, 0,
        0, 0, 0, 2, 254, 255,
        76, 0, 0, 0, 40, 0,
        0, 0, 0, 0, 36, 0,
        0, 0, 36, 0, 0, 0,
        36, 0, 0, 0, 36, 0,
        1, 0, 36, 0, 0, 0,
        0, 0, 1, 2, 254, 255,
        31, 0, 0, 2, 5, 0,
        0, 128, 0, 0, 15, 144,
        31, 0, 0, 2, 5, 0,
        1, 128, 1, 0, 15, 144,
        4, 0, 0, 4, 0, 0,
        3, 192, 0, 0, 255, 144,
        0, 0, 228, 160, 0, 0,
        228, 144, 1, 0, 0, 2,
        0, 0, 12, 192, 0, 0,
        228, 144, 1, 0, 0, 2,
        0, 0, 3, 224, 1, 0,
        228, 144, 255, 255, 0, 0,
        83, 72, 68, 82, 104, 0,
        0, 0, 64, 0, 1, 0,
        26, 0, 0, 0, 95, 0,
        0, 3, 242, 16, 16, 0,
        0, 0, 0, 0, 95, 0,
        0, 3, 50, 16, 16, 0,
        1, 0, 0, 0, 103, 0,
        0, 4, 242, 32, 16, 0,
        0, 0, 0, 0, 1, 0,
        0, 0, 101, 0, 0, 3,
        50, 32, 16, 0, 1, 0,
        0, 0, 54, 0, 0, 5,
        242, 32, 16, 0, 0, 0,
        0, 0, 70, 30, 16, 0,
        0, 0, 0, 0, 54, 0,
        0, 5, 50, 32, 16, 0,
        1, 0, 0, 0, 70, 16,
        16, 0, 1, 0, 0, 0,
        62, 0, 0, 1, 83, 84,
        65, 84, 116, 0, 0, 0,
        3, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 2, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 82, 68, 69, 70,
        80, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 28, 0,
        0, 0, 0, 4, 254, 255,
        0, 1, 0, 0, 28, 0,
        0, 0, 77, 105, 99, 114,
        111, 115, 111, 102, 116, 32,
        40, 82, 41, 32, 72, 76,
        83, 76, 32, 83, 104, 97,
        100, 101, 114, 32, 67, 111,
        109, 112, 105, 108, 101, 114,
        32, 57, 46, 50, 57, 46,
        57, 53, 50, 46, 51, 49,
        49, 49, 0, 171, 171, 171,
        73, 83, 71, 78, 76, 0,
        0, 0, 2, 0, 0, 0,
        8, 0, 0, 0, 56, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 3, 0,
        0, 0, 0, 0, 0, 0,
        15, 15, 0, 0, 65, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 3, 0,
        0, 0, 1, 0, 0, 0,
        3, 3, 0, 0, 80, 79,
        83, 73, 84, 73, 79, 78,
        0, 84, 69, 88, 67, 79,
        79, 82, 68, 0, 171, 171,
        79, 83, 71, 78, 80, 0,
        0, 0, 2, 0, 0, 0,
        8, 0, 0, 0, 56, 0,
        0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 3, 0,
        0, 0, 0, 0, 0, 0,
        15, 0, 0, 0, 68, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 3, 0,
        0, 0, 1, 0, 0, 0,
        3, 12, 0, 0, 83, 86,
        95, 80, 79, 83, 73, 84,
        73, 79, 78, 0, 84, 69,
        88, 67, 79, 79, 82, 68,
        0, 171, 171, 171};
const BYTE pShaderCode[] =
    {
        68, 88, 66, 67, 73, 254,
        165, 242, 166, 124, 202, 235,
        69, 167, 49, 49, 191, 88,
        172, 64, 1, 0, 0, 0,
        200, 2, 0, 0, 6, 0,
        0, 0, 56, 0, 0, 0,
        164, 0, 0, 0, 16, 1,
        0, 0, 140, 1, 0, 0,
        60, 2, 0, 0, 148, 2,
        0, 0, 65, 111, 110, 57,
        100, 0, 0, 0, 100, 0,
        0, 0, 0, 2, 255, 255,
        60, 0, 0, 0, 40, 0,
        0, 0, 0, 0, 40, 0,
        0, 0, 40, 0, 0, 0,
        40, 0, 1, 0, 36, 0,
        0, 0, 40, 0, 0, 0,
        0, 0, 1, 2, 255, 255,
        31, 0, 0, 2, 0, 0,
        0, 128, 0, 0, 3, 176,
        31, 0, 0, 2, 0, 0,
        0, 144, 0, 8, 15, 160,
        66, 0, 0, 3, 0, 0,
        15, 128, 0, 0, 228, 176,
        0, 8, 228, 160, 1, 0,
        0, 2, 0, 8, 15, 128,
        0, 0, 228, 128, 255, 255,
        0, 0, 83, 72, 68, 82,
        100, 0, 0, 0, 64, 0,
        0, 0, 25, 0, 0, 0,
        90, 0, 0, 3, 0, 96,
        16, 0, 0, 0, 0, 0,
        88, 24, 0, 4, 0, 112,
        16, 0, 0, 0, 0, 0,
        85, 85, 0, 0, 98, 16,
        0, 3, 50, 16, 16, 0,
        1, 0, 0, 0, 101, 0,
        0, 3, 242, 32, 16, 0,
        0, 0, 0, 0, 69, 0,
        0, 9, 242, 32, 16, 0,
        0, 0, 0, 0, 70, 16,
        16, 0, 1, 0, 0, 0,
        70, 126, 16, 0, 0, 0,
        0, 0, 0, 96, 16, 0,
        0, 0, 0, 0, 62, 0,
        0, 1, 83, 84, 65, 84,
        116, 0, 0, 0, 2, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 2, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        82, 68, 69, 70, 168, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 2, 0,
        0, 0, 28, 0, 0, 0,
        0, 4, 255, 255, 0, 1,
        0, 0, 116, 0, 0, 0,
        92, 0, 0, 0, 3, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 1, 0,
        0, 0, 106, 0, 0, 0,
        2, 0, 0, 0, 5, 0,
        0, 0, 4, 0, 0, 0,
        255, 255, 255, 255, 0, 0,
        0, 0, 1, 0, 0, 0,
        13, 0, 0, 0, 99, 111,
        108, 111, 114, 83, 97, 109,
        112, 108, 101, 114, 95, 0,
        99, 111, 108, 111, 114, 77,
        97, 112, 95, 0, 77, 105,
        99, 114, 111, 115, 111, 102,
        116, 32, 40, 82, 41, 32,
        72, 76, 83, 76, 32, 83,
        104, 97, 100, 101, 114, 32,
        67, 111, 109, 112, 105, 108,
        101, 114, 32, 57, 46, 50,
        57, 46, 57, 53, 50, 46,
        51, 49, 49, 49, 0, 171,
        171, 171, 73, 83, 71, 78,
        80, 0, 0, 0, 2, 0,
        0, 0, 8, 0, 0, 0,
        56, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0,
        3, 0, 0, 0, 0, 0,
        0, 0, 15, 0, 0, 0,
        68, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 1, 0,
        0, 0, 3, 3, 0, 0,
        83, 86, 95, 80, 79, 83,
        73, 84, 73, 79, 78, 0,
        84, 69, 88, 67, 79, 79,
        82, 68, 0, 171, 171, 171,
        79, 83, 71, 78, 44, 0,
        0, 0, 1, 0, 0, 0,
        8, 0, 0, 0, 32, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 3, 0,
        0, 0, 0, 0, 0, 0,
        15, 0, 0, 0, 83, 86,
        95, 84, 65, 82, 71, 69,
        84, 0, 171, 171};

void D3d11Show::RenderTexture(HWND hWnd, void* textureHandle, int w, int h)
{
    if (m_sDevice == 0)
        InitD3D(hWnd, textureHandle, w, h);
    else {
        ID3D11DeviceContext* ctx = NULL;
        m_sDevice->GetImmediateContext(&ctx);

        //渲染
        float clearColor[4] = {0.0f, 0.0f, 0.25f, 1.0f};
        ctx->ClearRenderTargetView(m_renderTargetView, clearColor);

        unsigned int stride = sizeof(VertexPos);
        unsigned int offset = 0;

        ctx->IASetInputLayout(inputLayout_);
        ctx->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ctx->VSSetShader(solidColorVS_, 0, 0);
        ctx->PSSetShader(solidColorPS_, 0, 0);
        ctx->PSSetShaderResources(0, 1, &colorMap_);
        ctx->PSSetSamplers(0, 1, &colorMapSampler_);
        ctx->Draw(6, 0);
        ctx->Release();
        HRESULT hr = m_swapChain->Present(0, 0);

        if (FAILED(hr)) {
            //Present Failed
            RealeaseD3d(false);

        }
    }
}

void D3d11Show::InitD3D(HWND hWnd, void* textureHandle, int w, int h)
{
    //创建设备和上下文
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
    sd.BufferDesc.RefreshRate.Numerator = 60;
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
    hr = m_sDevice->CreateVertexShader(vShaderCode, sizeof(vShaderCode), nullptr, &solidColorVS_);
    if (FAILED(hr)) {
        MessageBox(NULL, L"创建顶点着色器失败!", L"error", MB_OK);
        return;
    }
    hr = m_sDevice->CreatePixelShader(pShaderCode, sizeof(pShaderCode), nullptr, &solidColorPS_);
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
        hr = m_sDevice->CreateInputLayout(s_DX11InputElementDesc, 2, vShaderCode, sizeof(vShaderCode), &inputLayout_);
        if (FAILED(hr)) {
            MessageBox(NULL, L"创建输入布局失败!", L"error", MB_OK);
            return;
        }
    }

    VertexPos vertices[] =
        {
            {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},
            {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)},
            {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},

            {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
            {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)},
            {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},
        };

    D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory(&vertexDesc, sizeof(vertexDesc));
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.ByteWidth = sizeof(VertexPos) * 6;

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(resourceData));
    resourceData.pSysMem = vertices;

    hr = m_sDevice->CreateBuffer(&vertexDesc, &resourceData, &vertexBuffer_);

    if (FAILED(hr)) {
        MessageBox(NULL, L"Create Buffer failed!", L"error", MB_OK);
        return;
    }

    ID3D11Texture2D* d3dtex = (ID3D11Texture2D*)textureHandle;
    D3D11_SHADER_RESOURCE_VIEW_DESC vdesc;
    vdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    vdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    vdesc.Texture2D.MostDetailedMip = 0;
    vdesc.Texture2D.MipLevels = 1;

    hr = m_sDevice->CreateShaderResourceView(d3dtex, &vdesc, &colorMap_);

    //fopen_s(&fp, "VoxelLog1.txt", "a");
    //char log4[100] = "CreateShaderResourceView\r\n";
    //fprintf(fp, log4);
    //fflush(fp);
    //fclose(fp);

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
}

void D3d11Show::RealeaseD3d(bool isClearhWnd)
{
    if (colorMapSampler_) colorMapSampler_->Release();
    if (colorMap_) colorMap_->Release();
    if (solidColorVS_) solidColorVS_->Release();
    if (solidColorPS_) solidColorPS_->Release();
    if (inputLayout_) inputLayout_->Release();
    if (vertexBuffer_) vertexBuffer_->Release();

    colorMapSampler_ = 0;
    colorMap_ = 0;
    solidColorVS_ = 0;
    solidColorPS_ = 0;
    inputLayout_ = 0;
    vertexBuffer_ = 0;

    if (m_renderTargetView) m_renderTargetView->Release();
    if (m_swapChain) m_swapChain->Release();
    if (m_deviceContext) m_deviceContext->Release();
    if (m_sDevice) m_sDevice->Release();

    m_renderTargetView = 0;
    m_swapChain = 0;
    m_deviceContext = 0;
    m_sDevice = 0;

    if (isClearhWnd) {
        if (m_textureHandle) m_textureHandle = NULL;
        if (m_ViewhWnd) m_ViewhWnd = NULL;
    }
}

void D3d11Show::EndRendering()
{
    if (isRendering) {
        isRendering = false;
        if (m_ViewhWnd) {
            RealeaseD3d();
        }
    }
}

void D3d11Show::StartRenderingView(HWND hWnd, void* textureHandle, int w, int h)
{
    m_ViewhWnd = hWnd;
    m_textureHandle = textureHandle;
    m_w = w;
    m_h = h;
    isRendering = true;
    thread t(&D3d11Show::DoRenderingView, this);
    t.detach();
}

void D3d11Show::DoRenderingView()
{
    while (true) {
        std::this_thread::yield();
        if (!IsWindow(m_ViewhWnd)) {
            EndRendering();
        }

        if (isRendering) {
            RenderTexture(m_ViewhWnd, m_textureHandle, m_w, m_h);
        }
        else
            return;
    }
}
