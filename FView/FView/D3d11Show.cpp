#include "D3d11Show.h"
#include <VersionHelpers.h>
#include "kwan/Tex2DPixelShader.inc"
#include "kwan/Tex2DVertexShader.inc"
#include "kwan/Tex2DPixelShaderLinearSpace.inc"
#include "kwan/IVR_Log.h"
#include "kwan/FindWindowPtr.h"
#pragma comment(lib, "d3d11.lib")

namespace dxshow {
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
                         m_isGamaSpace(U3DColorSpace::Gama),
                         m_w(0),
                         m_h(0),
                         m_isInit(false),
                         m_OrthoMatrixType(DrawerManagerU3D::OrthoMatrixType::T_2D),
                         m_MatrixModifyFlag(false),
                         lastFailedTick(0),
                         m_failedTime(0)
{
    std::vector<HWND> temp_vecHWnds;
    temp_vecHWnds.clear();
    GetHWndsByProcessID(GetCurrentProcessId(), temp_vecHWnds);

    HWND U3dWin = NULL;
    for (const HWND& h : temp_vecHWnds) {
        HWND parent = GetParent(h);
        if (parent == NULL) {
            m_u3dhWnd = h;
        }
        TCHAR name[256];
        GetWindowText(h, name, 256);
        std::wstring temp_windowName = std::wstring(name);
        if (temp_windowName.find(L"Unity") != std::wstring::npos) {
            U3dWin = h;
        }
    }
    if (U3dWin != NULL) //如果U3D的编辑器窗口不是NULL,重新赋值
        m_u3dhWnd = U3dWin;
}

D3d11Show::~D3d11Show()
{
    RealeaseD3d();
    CloseHandle(m_hSemaphore);
    m_hSemaphore = NULL;
}

int D3d11Show::InitD3D()
{
    //创建设备和上下文
    if (m_isInit) {
        return 0;
    }
    if (m_failedTime > 20) {
        char charBuf[512];
        sprintf_s(charBuf, 512, "InitD3D() failed more than 20 ,returning ... ");
        IvrLog::Inst()->Log(std::string(charBuf));
        return -2;
    }

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
        char charBuf[512];
        sprintf_s(charBuf, 512, "InitD3D():D3D11CreateDevice(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf));
        m_failedTime++;
        m_isInit = false;
        return InitD3D();
    }
    //4X多重采样质量等级
    UINT m4xMsaaQuality(0);
    m_sDevice->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        4,
        &m4xMsaaQuality);
    //准备交换链属性
    DXGI_SWAP_CHAIN_DESC sd = {0};
    sd.BufferDesc.Width = m_w;
    sd.BufferDesc.Height = m_h;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    sd.SampleDesc.Count = 4;
    sd.SampleDesc.Quality = m4xMsaaQuality - 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = m_ViewhWnd;
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
        //MessageBox(NULL, L"Create SwapChain failed!", L"error", MB_OK);
        char charBuf[512];
        sprintf_s(charBuf, 512, "InitD3D():CreateSwapChain(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf));
        m_failedTime++;
        m_isInit = false;
        return InitD3D();
    }
    dxgiFactory->Release();
    dxgiAdapter->Release();
    dxgiDevice->Release();

    //创建渲染目标视图
    ID3D11Texture2D* backBuffer(NULL);
    m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    hr = m_sDevice->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView);
    if (FAILED(hr)) {
        // MessageBox(NULL, L"Create RenderTargetView failed!", L"error", MB_OK);
        char charBuf[512];
        sprintf_s(charBuf, 512, "InitD3D():CreateRenderTargetView(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf));
        m_failedTime++;
        m_isInit = false;
        return InitD3D();
    }
    if (backBuffer)
        backBuffer->Release();

    // 将视图绑定到输出合并器阶段
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, 0);

    // 设置视口
    D3D11_VIEWPORT vp = {0};
    vp.TopLeftX = 0.f;
    vp.TopLeftY = 0.f;
    vp.Width = static_cast<float>(m_w);
    vp.Height = static_cast<float>(m_h);
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;

    m_deviceContext->RSSetViewports(1, &vp);

    // shaders
    hr = m_sDevice->CreateVertexShader(g_Tex2DVertexShader, sizeof(g_Tex2DVertexShader), nullptr, &solidColorVS_);
    if (FAILED(hr)) {
        char charBuf[512];
        sprintf_s(charBuf, 512, "InitD3D():CreateVertexShader(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf));
        m_failedTime++;
        m_isInit = false;
        return InitD3D();
    }
    if (m_isGamaSpace == U3DColorSpace::Gama) {
        hr = m_sDevice->CreatePixelShader(g_Tex2DPixelShader, sizeof(g_Tex2DPixelShader), nullptr, &solidColorPS_);
        if (FAILED(hr)) {
            //MessageBox(NULL, L"创建像素着色器失败!", L"error", MB_OK);
            char charBuf[512];
            sprintf_s(charBuf, 512, "InitD3D():CreatePixelShader(...) failed with error %x", hr);
            IvrLog::Inst()->Log(std::string(charBuf));
            m_failedTime++;
            m_isInit = false;
            return InitD3D();
        }
    }
    else {
        hr = m_sDevice->CreatePixelShader(g_Tex2DPixelShaderLinearSpace, sizeof(g_Tex2DPixelShaderLinearSpace), nullptr, &solidColorPS_);
        if (FAILED(hr)) {
            //MessageBox(NULL, L"创建像素着色器失败!", L"error", MB_OK);
            char charBuf[512];
            sprintf_s(charBuf, 512, "InitD3D():CreatePixelShader(...) failed with error %x", hr);
            IvrLog::Inst()->Log(std::string(charBuf));
            m_failedTime++;
            m_isInit = false;
            return InitD3D();
        }
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
            //MessageBox(NULL, L"创建输入布局失败!", L"error", MB_OK);
            char charBuf[512];
            sprintf_s(charBuf, 512, "InitD3D():CreateInputLayout(...) failed with error %x", hr);
            IvrLog::Inst()->Log(std::string(charBuf));
            m_failedTime++;
            m_isInit = false;
            return InitD3D();
        }
    }

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
        //MessageBox(NULL, L"Create SamplerState failed!", L"error", MB_OK);
        char charBuf[512];
        sprintf_s(charBuf, 512, "InitD3D():CreateSamplerState(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf));
        m_failedTime++;
        m_isInit = false;
        return InitD3D();
    }
    m_isInit = true;
    m_drawer.reset(new DrawerManagerU3D(m_sDevice));

    m_deviceContext->IASetInputLayout(inputLayout_);         //设置顶点格式
    m_deviceContext->VSSetShader(solidColorVS_, 0, 0);       //设置顶点着色器
    m_deviceContext->PSSetShader(solidColorPS_, 0, 0);       //设置像素着色器
    m_deviceContext->PSSetSamplers(0, 1, &colorMapSampler_); //设置纹理采样器

    if (m_u3dhWnd != NULL) {
        SetForegroundWindow(m_u3dhWnd);
    }
    m_failedTime = 0;
    return 1;
}

void D3d11Show::RealeaseD3d(bool isClearhWnd)
{
    if (!m_isInit) return;
    isRendering = false;
    //等待渲染线程返回
    WaitForSingleObject(m_hSemaphore, 300);
    ReleaseSemaphore(m_hSemaphore, 1, NULL);
    m_hSemaphore = NULL;
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

//void D3d11Show::RenderTexture()
//{
//
//}

void D3d11Show::EndRendering()
{
    isRendering = false;
    WaitForSingleObject(m_hSemaphore, 300);
    if (m_ViewhWnd != NULL) {
        ::PostMessage(m_ViewhWnd, WM_QUIT, 0, 0);
    }
    ReleaseSemaphore(m_hSemaphore, 1, NULL);
    m_hSemaphore = NULL;
    RealeaseD3d(false);
}

int D3d11Show::SetupTextureHandle(void* textureHandle, RenderingResources::ResourceViewport type)
{
    if (m_sDevice == NULL || textureHandle == 0)
        return -1;

    if (m_failedTime > 20) {
        char charBuf[512];
        sprintf_s(charBuf, 512, "SetupTextureHandle(...) failed more than 20 ,returning ... ");
        IvrLog::Inst()->Log(std::string(charBuf));
        return -2;
    }

    RenderingResources rs = RenderingResources(m_sDevice, (ID3D11Texture2D*)textureHandle, type);
    if (!rs.isValuable) {
        m_failedTime++;
        return SetupTextureHandle(textureHandle, type);
    }
    m_drawer->PushResources(std::move(rs));
    m_failedTime = 0;
    return 1;
}

void D3d11Show::SwichProjector(DrawerManagerU3D::OrthoMatrixType type)
{
    m_OrthoMatrixType = type;
    m_MatrixModifyFlag = true;
}

void D3d11Show::SetGamaSpace(U3DColorSpace space)
{
    m_isGamaSpace = space;
}

int D3d11Show::StartRenderingView(HWND hWnd, int w, int h, int count, ...)
{
    if (!m_mutex.try_lock())
    {
        return 0;
    }
    IvrLog::Inst()->Log("StartRenderingView called");

    if (!IsWindows10OrGreater()) {
        IvrLog::Inst()->Log("You need at least Windows 10");
        if (hWnd != NULL) {
            ::PostMessage(hWnd, WM_QUIT, 0, 0);
        }
        return -6;
    }
    isRendering = false;
    if (m_hSemaphore != NULL) {
        //等渲染待线程返回
        DWORD dw = WaitForSingleObject(m_hSemaphore, 500);
        switch (dw) {
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
            IvrLog::Inst()->Log("StartRenderingView thread timeout!");
            return -5;
            break;
        case WAIT_FAILED:
            IvrLog::Inst()->Log("StartRenderingView thread WAIT_FAILED!");
            return -5;
            break;
        }
    }
    else {
        m_hSemaphore = CreateSemaphoreA(NULL, 1, 1, m_SemaphoreName);
        WaitForSingleObject(m_hSemaphore, 100);
    }

    if (hWnd == NULL) {
        return -2;
    }

    if (hWnd != m_ViewhWnd) {
        if (m_drawer != nullptr) m_drawer.reset();
        RealeaseD3d(false);
    }

    m_ViewhWnd = hWnd;
    m_w = w;
    m_h = h;

    currentTexturePTR.clear();
    va_list arg_ptr;
    void* nArgValue;
    va_start(arg_ptr, count);
    for (int i = 0; i < count; i++) {
        nArgValue = va_arg(arg_ptr, void*);
        if (nArgValue == nullptr) {
            return -4;
        }
        currentTexturePTR.push_back(nArgValue);
    }

    if (InitD3D() < 0) {
        return -3;
    }
    m_drawer->ClearResources();
    for (int i = 0; i < count; i++) {
        if (count == 1) {
            if (SetupTextureHandle(currentTexturePTR[i], RenderingResources::ResourceViewport::FULL_VIEW) < 0) {
                return -4;
            }
        }
        else {
            if (SetupTextureHandle(currentTexturePTR[i], (RenderingResources::ResourceViewport)(i + 1)) < 0) {
                return -4;
            }
        }
    }
    va_end(arg_ptr);
    m_MatrixModifyFlag = true;

    auto lambdaRenderThread = [&]() {
        IvrLog::Inst()->Log("Render begin!");
        int temp_resultCode = 0;
        isRendering = true;
        const int constFps = 60;
        while (isRendering) {
            std::this_thread::yield();
            if (!IsWindow(m_ViewhWnd)) {
                EndRendering();
            }
            float timeInOneFps = 1000.0f / constFps;
            DWORD timeBegin = GetTickCount();
            if (!IsIconic(m_ViewhWnd)) {
                if (m_MatrixModifyFlag) {
                    m_drawer->UpdateAllMatrix(m_OrthoMatrixType);
                    m_MatrixModifyFlag = false;
                }
                if (m_sDevice == nullptr) {
                    isRendering = false;
                    temp_resultCode = -1;
                    return -1;
                }
                ID3D11DeviceContext* ctx = NULL;
                m_sDevice->GetImmediateContext(&ctx);
                //渲染
                float clearColor[4] = {0.0f, 0.0f, 0.25f, 1.0f};            //背景颜色
                ctx->ClearRenderTargetView(m_renderTargetView, clearColor); //清空视口
                m_drawer->RenderAllResource(ctx);                           //渲染所有物体
                HRESULT hr = m_swapChain->Present(1, 0);
                ctx->Release();
                if (FAILED(hr)) {
                    if (GetTickCount() - lastFailedTick < 1000.0f) {
                        IvrLog::Inst()->Log("m_swapChain->Present Failed twice in a second!");
                    }
                    lastFailedTick = GetTickCount();
                    isRendering = false;
                    char buff[64] = {};
                    sprintf_s(buff, "m_swapChain->Present(1, 0) failed with error 0x%08X", hr);
                    IvrLog::Inst()->Log(buff);
                    isRendering = false;
                    temp_resultCode = -1;
                }
            }
            DWORD timeTotal = GetTickCount() - timeBegin;
            if (timeTotal < timeInOneFps)
                Sleep(DWORD(timeInOneFps - timeTotal));
        }
        ReleaseSemaphore(m_hSemaphore, 1, NULL);
        char buff[64] = {};
        sprintf_s(buff, "Render end! result code = %d", temp_resultCode);
        IvrLog::Inst()->Log(buff);
        switch (temp_resultCode) {
        case -1:
            RealeaseD3d(false);
            if (currentTexturePTR.size() == 1) {
                StartRenderingView(m_ViewhWnd, m_w, m_h, 1, currentTexturePTR[0]);
            }
            else if (currentTexturePTR.size() == 2) {
                StartRenderingView(m_ViewhWnd, m_w, m_h, 2, currentTexturePTR[0], currentTexturePTR[1]);
            }
            break;
        default:
            break;
        }
    };
    m_rthread = std::thread(lambdaRenderThread);
    m_rthread.detach();
    //std::thread(lambdaRenderThread).detach();
    m_mutex.unlock();
    return 1;
}
} // namespace dxshow
