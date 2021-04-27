#include "D3d11Show.h"
#include <VersionHelpers.h>
#include "kwan/Tex2DPixelShader.inc"
#include "kwan/Tex2DVertexShader.inc"
#include "kwan/Tex2DPixelShaderLinearSpace.inc"
#include "kwan/IVR_Log.h"
#include "kwan/FindWindowPtr.h"
#pragma comment(lib, "d3d11.lib")

namespace dxshow {
D3d11Show::D3d11Show() : m_sDevice(nullptr),
                         m_deviceContext(nullptr),
                         m_swapChain(nullptr),
                         /*         m_renderTargetView(NULL),*/
                         solidColorVS_(nullptr),
                         solidColorPS_(nullptr),
                         inputLayout_(nullptr),
                         colorMapSampler_(nullptr),
                         isRendering(false),
                         OnWindowsResized(true),
                         m_ViewhWnd(nullptr),
                         m_w(0),
                         m_h(0),
                         m_isInit(false),
                         m_MatrixModifyFlag(false),
                         lastFailedTick(0),
                         m_failedTime(0)
{
    char charBuf[128];
    sprintf_s(charBuf, 128, "Current API is DirectX11 ... ");
    IvrLog::Inst()->Log(std::string(charBuf), 0);

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
            break;
        }
    }
    if (U3dWin != NULL) //如果U3D的编辑器窗口不是NULL,重新赋值
        m_u3dhWnd = U3dWin;
}

D3d11Show::~D3d11Show()
{
    RealeaseD3d();
}

int D3d11Show::InitD3D()
{
    //创建设备和上下文
    if (m_isInit) {
        return 0;
    }
    if (m_failedTime > 500) {
        char charBuf[128];
        sprintf_s(charBuf, 128, "InitD3D() failed more than 500 ,returning ... ");
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        return -2;
    }

    D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1};
    D3D_FEATURE_LEVEL myFeatureLevel;
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr = D3D11CreateDevice(
        NULL, // 默认显示适配器
        D3D_DRIVER_TYPE_HARDWARE,
        0, // 不使用软件设备
        createDeviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        m_sDevice.GetAddressOf(),
        &myFeatureLevel,
        m_deviceContext.GetAddressOf());

    if (FAILED(hr)) {
        char charBuf[128];
        sprintf_s(charBuf, 128, "InitD3D():D3D11CreateDevice(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        m_failedTime++;
        m_isInit = false;
        if (hr == DXGI_ERROR_DEVICE_REMOVED)
            return InitD3D();
        else
            return -3;
    }

    ComPtr<IDXGIDevice> dxgiDevice(nullptr);
    hr = m_sDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)(dxgiDevice.GetAddressOf()));
    if (FAILED(hr)) {
        //MessageBox(NULL, L"Create SwapChain failed!", L"error", MB_OK);
        char charBuf[128];
        sprintf_s(charBuf, 128, "m_sDevice->QueryInterface(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        m_failedTime++;
        m_isInit = false;
        return -3;
    }
    ComPtr<IDXGIAdapter> dxgiAdapter(nullptr);
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)(dxgiAdapter.GetAddressOf()));
    if (FAILED(hr)) {
        //MessageBox(NULL, L"Create SwapChain failed!", L"error", MB_OK);
        char charBuf[128];
        sprintf_s(charBuf, 128, "dxgiDevice->GetParent(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        m_failedTime++;
        m_isInit = false;
        return -3;
    }
    //**************判断显卡型号*****************
    DXGI_ADAPTER_DESC temp_adapterDesc;
    dxgiAdapter->GetDesc(&temp_adapterDesc);
    std::wstring description = temp_adapterDesc.Description;
    if (description.find(L"NVIDIA") != description.npos) {
        //从显卡描述中提取版本号
        auto GetAdapterSeriesNum = [description]() {
            unsigned int series = 0;
            for (size_t i = 0; i < description.length(); i++) {
                size_t begin = 0;
                size_t size = 0;
                if (description[i] >= '0' && description[i] <= '9') {
                    begin = i;
                    for (size_t k = begin; k < description.length(); k++) {
                        i++;
                        if (description[k] >= '0' && description[k] <= '9') {
                            size = k - begin + 1;
                        }
                        else {
                            break;
                        }
                    }
                    std::wstring subdes = description.substr(begin, size);
                    series = _wtoi(subdes.c_str());
                }
            }
            return series;
        };
        if (GetAdapterSeriesNum() < 900) //版本小于900，返回-7错误
        {
            TCHAR charBuf[128];
            wsprintf(charBuf, L"Adapter is not suppor error = %s", description.c_str());
            IvrLog::Inst()->Log(std::wstring(charBuf), 4);
            return -7;
        }
    }
    //*******************************************

    //4X多重采样质量等级
    UINT m4xMsaaQuality(0);
    m_sDevice->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        4,
        &m4xMsaaQuality);

    // Allocate a descriptor.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
    swapChainDesc.BufferDesc.Width = m_w;
    swapChainDesc.BufferDesc.Height = m_h;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swapChainDesc.SampleDesc.Count = 4;
    swapChainDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = m_ViewhWnd;
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    //创建交换链
    ComPtr<IDXGIFactory> dxgiFactory(nullptr);
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)dxgiFactory.GetAddressOf());

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc;
    ZeroMemory(&fullScreenDesc, sizeof(fullScreenDesc));
    fullScreenDesc.RefreshRate.Numerator = 60;
    fullScreenDesc.RefreshRate.Denominator = 1;
    fullScreenDesc.Windowed = TRUE;

    m_deviceContext->ClearState();
    m_deviceContext->Flush();
    //hr = dxgiFactory->CreateSwapChain(m_sDevice, &sd, &m_swapChain);
    hr = dxgiFactory->CreateSwapChain(m_sDevice.Get(), &swapChainDesc, &m_swapChain);
    if (FAILED(hr)) {
        //MessageBox(NULL, L"Create SwapChain failed!", L"error", MB_OK);
        char charBuf[128];
        sprintf_s(charBuf, 128, "InitD3D():CreateSwapChain(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        m_failedTime++;
        m_isInit = false;
        if (hr == DXGI_ERROR_DEVICE_REMOVED)
            return InitD3D();
        else
            return -3;
    }
    // shaders
    hr = m_sDevice->CreateVertexShader(g_Tex2DVertexShader, sizeof(g_Tex2DVertexShader), nullptr, solidColorVS_.GetAddressOf());
    if (FAILED(hr)) {
        char charBuf[128];
        sprintf_s(charBuf, 128, "InitD3D():CreateVertexShader(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        m_failedTime++;
        m_isInit = false;
        return InitD3D();
    }
    if (m_isGamaSpace == U3DColorSpace::Gama) {
        hr = m_sDevice->CreatePixelShader(g_Tex2DPixelShader, sizeof(g_Tex2DPixelShader), nullptr, solidColorPS_.GetAddressOf());
        if (FAILED(hr)) {
            //MessageBox(NULL, L"创建像素着色器失败!", L"error", MB_OK);
            char charBuf[128];
            sprintf_s(charBuf, 128, "InitD3D():CreatePixelShader(...) failed with error %x", hr);
            IvrLog::Inst()->Log(std::string(charBuf), 4);
            m_failedTime++;
            m_isInit = false;
            if (hr == DXGI_ERROR_DEVICE_REMOVED)
                return InitD3D();
            else
                return -3;
        }
    }
    else {
        hr = m_sDevice->CreatePixelShader(g_Tex2DPixelShaderLinearSpace, sizeof(g_Tex2DPixelShaderLinearSpace), nullptr, solidColorPS_.GetAddressOf());
        if (FAILED(hr)) {
            //MessageBox(NULL, L"创建像素着色器失败!", L"error", MB_OK);
            char charBuf[128];
            sprintf_s(charBuf, 128, "InitD3D():CreatePixelShader(...) failed with error %x", hr);
            IvrLog::Inst()->Log(std::string(charBuf), 4);
            m_failedTime++;
            m_isInit = false;
            if (hr == DXGI_ERROR_DEVICE_REMOVED)
                return InitD3D();
            else
                return -3;
        }
    }

    // input layout
    if (solidColorVS_) {
        D3D11_INPUT_ELEMENT_DESC s_DX11InputElementDesc[] =
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };
        hr = m_sDevice->CreateInputLayout(s_DX11InputElementDesc, 2, g_Tex2DVertexShader, sizeof(g_Tex2DVertexShader), inputLayout_.GetAddressOf());
        if (FAILED(hr)) {
            //MessageBox(NULL, L"创建输入布局失败!", L"error", MB_OK);
            char charBuf[128];
            sprintf_s(charBuf, 128, "InitD3D():CreateInputLayout(...) failed with error %x", hr);
            IvrLog::Inst()->Log(std::string(charBuf), 4);
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

    hr = m_sDevice->CreateSamplerState(&colorMapDesc, colorMapSampler_.GetAddressOf());

    if (FAILED(hr)) {
        //MessageBox(NULL, L"Create SamplerState failed!", L"error", MB_OK);
        char charBuf[128];
        sprintf_s(charBuf, 128, "InitD3D():CreateSamplerState(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        m_failedTime++;
        m_isInit = false;
        if (hr == DXGI_ERROR_DEVICE_REMOVED)
            return InitD3D();
        else
            return -3;
    }
    m_isInit = true;

    m_deviceContext->IASetInputLayout(inputLayout_.Get());                 //设置顶点格式
    m_deviceContext->VSSetShader(solidColorVS_.Get(), 0, 0);               //设置顶点着色器
    m_deviceContext->PSSetShader(solidColorPS_.Get(), 0, 0);               //设置像素着色器
    m_deviceContext->PSSetSamplers(0, 1, colorMapSampler_.GetAddressOf()); //设置纹理采样器

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

    m_drawer = nullptr;
    colorMapSampler_ = nullptr;
    solidColorVS_ = nullptr;
    solidColorPS_ = nullptr;
    inputLayout_ = nullptr;
    m_swapChain = nullptr;
    m_deviceContext = nullptr;
    m_sDevice = nullptr;

    if (isClearhWnd) {
        if (m_ViewhWnd) m_ViewhWnd = NULL;
    }
    m_isInit = false;
}

void D3d11Show::EndRendering()
{
    if (isRendering) {
        RealeaseD3d(false);
    }
    if (m_ViewhWnd != NULL) {
        ::PostMessage(m_ViewhWnd, WM_QUIT, 0, 0);
    }
}

void D3d11Show::SwichProjector(OrthoMatrixType type)
{
    m_OrthoMatrixType = type;
    m_MatrixModifyFlag = true;

    ////立体渲染下不允许切换左右视图
    //if (m_OrthoMatrixType != OrthoMatrixType::T_Stereopic) {
    //    m_OrthoMatrixType = type;
    //    m_MatrixModifyFlag = true;
    //}
}

bool D3d11Show::UpdateStereoEnabledStatus()
{
    D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1};
    D3D_FEATURE_LEVEL myFeatureLevel;
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    ComPtr<ID3D11Device> device;
    HRESULT hr = D3D11CreateDevice(
        NULL, // 默认显示适配器
        D3D_DRIVER_TYPE_HARDWARE,
        0, // 不使用软件设备
        createDeviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        device.GetAddressOf(),
        &myFeatureLevel,
        nullptr);

    if (FAILED(hr)) {
        char charBuf[128];
        sprintf_s(charBuf, 128, "UpdateStereoEnabledStatus():D3D11CreateDevice(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        m_failedTime++;
        m_isInit = false;
        return false;
    }

    ComPtr<IDXGIDevice1> dxgiDevice;
    hr = device->QueryInterface(__uuidof(IDXGIDevice1), (void**)dxgiDevice.GetAddressOf());
    if (FAILED(hr)) {
        //MessageBox(NULL, L"Create SamplerState failed!", L"error", MB_OK);
        char charBuf[128];
        sprintf_s(charBuf, 128, "UpdateStereoEnabledStatus():QueryInterface(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        return false;
    }
    ComPtr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
    if (FAILED(hr)) {
        char charBuf[128];
        sprintf_s(charBuf, 128, "UpdateStereoEnabledStatus():GetAdapter(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        return false;
    }

    ComPtr<IDXGIFactory2> dxgiFactory;
    hr = dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
    if (FAILED(hr)) {
        char charBuf[128];
        sprintf_s(charBuf, 128, "UpdateStereoEnabledStatus():GetParent(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        return false;
    }
    bool temp_stereoEnabled = dxgiFactory->IsWindowedStereoEnabled() ? true : false;
    return temp_stereoEnabled;
}

int D3d11Show::StartRenderingView(HWND hWnd, int w, int h, int count, ...)
{
    if (!Rlock(&m_mutex).LockSuccessed()) {
        IvrLog::Inst()->Log("StartRenderingView called,but some other is rendering", 1);
        return 0;
    }
    IvrLog::Inst()->Log("StartRenderingView called", 0);

    if (!IsWindows10OrGreater()) {
        IvrLog::Inst()->Log("You need at least Windows 10", 4);
        m_ViewhWnd = hWnd;
        if (hWnd != NULL) {
            ::PostMessage(hWnd, WM_QUIT, 0, 0);
        }
        return -6;
    }

    if (!IsWindow(hWnd)) {
        IvrLog::Inst()->Log("StartRenderingView(...) failed with error = Invaild HWND", 4);
        return -2;
    }

    if (hWnd != m_ViewhWnd) {
        RealeaseD3d(false);
    }

    m_ViewhWnd = hWnd;

    currentTexturePTR.clear();
    va_list arg_ptr;
    void* nArgValue;
    va_start(arg_ptr, count);
    for (int i = 0; i < count; i++) {
        nArgValue = va_arg(arg_ptr, void*);
        if (nArgValue == nullptr) {
            IvrLog::Inst()->Log("StartRenderingView(...) failed with error = Invaild texture handle", 4);
            return -4;
        }
        currentTexturePTR.push_back(nArgValue);
    }

    int hresult = InitD3D();
    if (hresult < 0) {
        return hresult;
    }

    va_end(arg_ptr);
    m_MatrixModifyFlag = true;

    IvrLog::Inst()->Log("Render begin!", 0);
    int temp_resultCode = 0;

    //m_drawer = ComPtr<DrawerManagerU3D>(new DrawerManagerU3D(m_sDevice.Get(), m_swapChain.Get()));
    std::unique_ptr<DrawerManagerU3D> temp_drawer(new DrawerManagerU3D(m_sDevice.Get(), m_swapChain.Get())); //local value
    m_drawer = std::move(temp_drawer);
    isRendering = true;
    OnWindowsResized = true;

    HANDLE resizedSignal = CreateEvent(NULL, FALSE, FALSE, L"FARwinResizeSignal");

    ///init texture handle
    for (int i = 0; i < currentTexturePTR.size(); i++) {
        if (currentTexturePTR.size() == 1) {
            RenderingResources rs = RenderingResources(m_sDevice.Get(), (ID3D11Texture2D*)currentTexturePTR[i], RenderingResources::ResourceViewport::FULL_VIEW);
            if (!rs.isValuable) { //setup RenderingResources failed
                m_failedTime++;
                isRendering = false;
                temp_resultCode = -1;
                OnWindowsResized = true;
            }
            else //setup RenderingResources successed
                m_drawer->PushResources(std::move(rs));
        }
        else {
            RenderingResources rs = RenderingResources(m_sDevice.Get(), (ID3D11Texture2D*)currentTexturePTR[i], (RenderingResources::ResourceViewport)(i + 1));
            if (!rs.isValuable) { //setup RenderingResources failed
                m_failedTime++;
                isRendering = false;
                temp_resultCode = -1;
                OnWindowsResized = true;
                char buff[128] = {};
                sprintf_s(buff, "Setup RenderingResources failed.");
                IvrLog::Inst()->Log(buff, 4);
            }
            else //setup RenderingResources successed
                m_drawer->PushResources(std::move(rs));
        }

        SetEvent(resizedSignal);
    }
    return 1;
}

int D3d11Show::OnRenderEvent()
{
    if (!isRendering) return -1;

    if (!IsWindow(m_ViewhWnd)) {
        EndRendering();
    }

    int temp_resultCode = 0;

    float timeInOneFps = 1000.0f / TargetFrameRate;
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
        if (OnWindowsResized) {
            RECT rect;
            GetWindowRect(m_ViewhWnd, &rect);
            m_w = rect.right - rect.left;
            m_h = rect.bottom - rect.top;
            //if (m_stereoEnabled) {
            //    m_stereoEnabled = UpdateStereoEnabledStatus();
            //    if (!m_stereoEnabled) {
            //        char buff[128] = {};
            //        sprintf_s(buff, "Stereopic is disable in current hardware,falling back to normal swapchain.");
            //        IvrLog::Inst()->Log(buff, 3);
            //    }
            //}
            m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
            if (m_drawer->UpdateRenderingDependent(m_stereoEnabled) < 0) {
                temp_resultCode = -1;
                OnWindowsResized = true;
            }
            D3D11_VIEWPORT vp = {0};
            vp.TopLeftX = 0.f;
            vp.TopLeftY = 0.f;

            vp.Width = static_cast<float>(m_w);
            vp.Height = static_cast<float>(m_h);
            char buff[128] = {};
            sprintf_s(buff, "OnWindowsResized,width = %d  height = %d , scale = %d ", m_w, m_h, GetSystemMetrics(SM_CXSCREEN));
            IvrLog::Inst()->Log(buff, 0);
            vp.MinDepth = 0.f;
            vp.MaxDepth = 1.f;
            m_deviceContext->RSSetViewports(1, &vp);
            OnWindowsResized = false;
        }
        m_drawer->RenderAllResource(m_deviceContext.Get()); //渲染所有物体

        HRESULT hr = m_swapChain->Present(0, 0);// 不等垂直同步
        if (FAILED(hr)) {
            if (GetTickCount() - lastFailedTick < 1000.0f) {
                IvrLog::Inst()->Log("m_swapChain->Present Failed twice in a second!", 3);
            }
            lastFailedTick = GetTickCount();
            isRendering = false;
            char buff[128] = {};
            sprintf_s(buff, "m_swapChain->Present(1, 0) failed with error 0x%08X", m_sDevice->GetDeviceRemovedReason());
            IvrLog::Inst()->Log(buff, 4);
            temp_resultCode = -1;
            OnWindowsResized = true;
        }

        switch (temp_resultCode) {
        case -1:
            RealeaseD3d(false);
            if (currentTexturePTR.size() == 1) {
                void* arg0 = currentTexturePTR[0];
                StartRenderingView(m_ViewhWnd, m_w, m_h, 1, arg0);
            }
            else if (currentTexturePTR.size() == 2) {
                void* arg0 = currentTexturePTR[0];
                void* arg1 = currentTexturePTR[1];
                StartRenderingView(m_ViewhWnd, m_w, m_h, 2, arg0, arg1);
            }
            break;
        default:
            break;
        }
    }
    /* DWORD timeTotal = GetTickCount() - timeBegin;
                if (timeTotal < timeInOneFps)
                    Sleep(DWORD(timeInOneFps - timeTotal));*/

    return 0;
}
} // namespace dxshow
