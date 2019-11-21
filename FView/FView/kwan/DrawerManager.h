#ifndef DrawerManager_H
#define DrawerManager_H

#include <vector>
#include <d3d11_2.h>
#include <DirectXMath.h>
#include "IVR_Log.h"
#include "RenderAPI.h"

namespace dxshow {
//渲染管理类
template <typename Resource>
class DrawerManager
{
  public:

    DrawerManager<Resource>();
    DrawerManager<Resource>(ID3D11Device* d3dDevice, IDXGISwapChain1* d3dSwapchian);
    ~DrawerManager<Resource>();

    int GetManagerSize() const;

    //添加渲染元素
    bool PushResources(Resource&& in_res);
    //删除末端渲染元素
    bool PopResources();
    //删除指定渲染元素
    bool PopResources(int index);
    //清空渲染元素
    bool ClearResources();
    //渲染到元素
    void RenderAllResource(ID3D11DeviceContext* ctx);
    //更新排列渲染矩阵
    void UpdateAllMatrix(OrthoMatrixType type);
    //更新渲染视图目标
    int UpdateRenderingDependent(bool isStereoipic);

  private:
    struct ProjectBuffer
    {
        DirectX::XMMATRIX _view;
        DirectX::XMMATRIX _ortho;
    };
    //do not released ref_ value
    ID3D11Device* ref_d3dDevice;
    IDXGISwapChain1* ref_d3dSwapchain;
    bool m_stereoEnable;

    ID3D11RenderTargetView* m_renderTargetView;
    ID3D11RenderTargetView* m_renderTargetViewRight;
    ID3D11DepthStencilView* m_d3dDepthStencilView;

    std::vector<Resource> m_resourcesStarck;

    ProjectBuffer m_projectMatrix;
    ID3D11Buffer* m_projectBuffer;
    OrthoMatrixType m_orthoMatrixType;

    template <typename Res>
    inline void SafeRelease(Res* ptr)
    {
        if (ptr != nullptr) {
            ptr->Release();
            ptr = nullptr;
        }
    }
};

template <typename Resource>
DrawerManager<Resource>::DrawerManager()
{
    m_resourcesStarck.clear();
}

template <typename Resource>
DrawerManager<Resource>::DrawerManager(ID3D11Device* d3dDevice,
                                       IDXGISwapChain1* d3dSwapchain) : ref_d3dDevice(d3dDevice),
                                                                        ref_d3dSwapchain(d3dSwapchain),
                                                                        m_renderTargetView(nullptr),
                                                                        m_renderTargetViewRight(nullptr),
                                                                        m_d3dDepthStencilView(nullptr),
                                                                        m_stereoEnable(false),
                                                                        m_orthoMatrixType(OrthoMatrixType::T_2D)
{
    using namespace DirectX;

    m_resourcesStarck.clear();
    m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(2.0f, 2.0f, 0.1f, 20.0f));
    m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0, 0, 1, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
    D3D11_BUFFER_DESC commandDesc;
    ZeroMemory(&commandDesc, sizeof(commandDesc));
    commandDesc.Usage = D3D11_USAGE_DEFAULT;
    commandDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    commandDesc.ByteWidth = sizeof(ProjectBuffer);
    commandDesc.CPUAccessFlags = 0;
    HRESULT hr = ref_d3dDevice->CreateBuffer(&commandDesc, NULL, &m_projectBuffer);
    if (FAILED(hr)) {
        char charBuf[128];
        sprintf_s(charBuf, 128, "DrawerManager():CreateBuffer(m_projectBuffer) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        return;
    }
}

template <typename Resource>
DrawerManager<Resource>::~DrawerManager()
{
    m_resourcesStarck.clear();
    SafeRelease(m_projectBuffer);
    SafeRelease(m_renderTargetView);
    SafeRelease(m_renderTargetViewRight);
    SafeRelease(m_d3dDepthStencilView);
    IvrLog::Inst()->Log(std::string("~DrawerManager()"), 0);
}

template <typename Resource>
int DrawerManager<Resource>::GetManagerSize() const
{
    return m_resourcesStarck.size();
}

template <typename Resource>
bool DrawerManager<Resource>::PushResources(Resource&& in_res)
{
    m_resourcesStarck.push_back(std::move(in_res));
    return true;
}

template <typename Resource>
bool DrawerManager<Resource>::PopResources()
{
    if (!m_resourcesStarck.empty()) {
        m_resourcesStarck.erase(m_resourcesStarck.back());
        return true;
    }
    return false;
}

template <typename Resource>
bool DrawerManager<Resource>::PopResources(int index)
{
    if (m_resourcesStarck.size() > index) {
        m_resourcesStarck.erase(m_resourcesStarck.begin() + index);
        return true;
    }
    return false;
}

template <typename Resource>
bool DrawerManager<Resource>::ClearResources()
{
    m_resourcesStarck.clear();

    return true;
}

template <typename Resource>
void DrawerManager<Resource>::RenderAllResource(ID3D11DeviceContext* ctx)
{
    if (ctx != NULL) {

        if (!m_stereoEnable) {
            if (m_renderTargetView == nullptr) {
                UpdateRenderingDependent(m_stereoEnable);
            }
            ctx->OMSetRenderTargets(1, &m_renderTargetView, 0);
            float clearColor[4] = {0.0f, 0.0f, 0.25f, 1.0f};            //背景颜色
            ctx->ClearRenderTargetView(m_renderTargetView, clearColor); //清空视口
            typename std::vector<Resource>::iterator iter = m_resourcesStarck.begin();
            for (iter; iter != m_resourcesStarck.end(); iter++) //遍历渲染整个容器
            {
                (*iter).Render(ctx, 6);
            }
            ctx->VSSetConstantBuffers(1, 1, &m_projectBuffer);
        }
        else if (m_stereoEnable && m_resourcesStarck.size() == 2) {
            if (m_renderTargetViewRight == nullptr) {
                UpdateRenderingDependent(m_stereoEnable);
            }

            //渲染左眼图像
            ctx->OMSetRenderTargets(1, &m_renderTargetView, 0);
            float clearColor[4] = {0.0f, 0.0f, 0.25f, 1.0f};
            ctx->ClearRenderTargetView(m_renderTargetView, clearColor);
            m_resourcesStarck[0].Render(ctx, 6);
            //渲染右眼图像
            ctx->OMSetRenderTargets(1, &m_renderTargetViewRight, 0);
            ctx->ClearRenderTargetView(m_renderTargetViewRight, clearColor);
            m_resourcesStarck[1].Render(ctx, 6);
            ctx->VSSetConstantBuffers(1, 1, &m_projectBuffer);
        }
    }
}

template <typename Resource>
void DrawerManager<Resource>::UpdateAllMatrix(OrthoMatrixType type)
{
    using namespace DirectX;
    /* if (type == m_orthoMatrixType)
        return;*/

    char charBuf[128];
    char temp_MatrixTypeDescript[3][15] = { {"2D"},{"3D_leftright"},{"Stereopic"} };
    sprintf_s(charBuf, 128, "UpdateAllMatrix( %s )  m_resourcesSize = %zd ", temp_MatrixTypeDescript[(int)type], m_resourcesStarck.size());
    IvrLog::Inst()->Log(std::string(charBuf), 0);

    if (m_resourcesStarck.size() == 0)
        return;

    m_orthoMatrixType = type;
    ID3D11DeviceContext* ctx = NULL;
    ref_d3dDevice->GetImmediateContext(&ctx);
    typename std::vector<Resource>::iterator iter;
    if (m_resourcesStarck.size() == 1) {
        //   MessageBox(NULL, L"m_resourcesStarck.size() == 1", L"error", MB_OK);
        m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
        m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(2.0f, 2.0f, 0.1f, 5.0f));
    }
    else {
        switch (type) {
        case OrthoMatrixType::T_2D:
            m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(-0.5f, 0.0f, -1.0f, 1.0f), XMVectorSet(-0.5f, 0.0f, 1.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
            m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(1.0f, 2.0f, 0.1f, 5.0f));        
            for (iter = m_resourcesStarck.begin(); iter != m_resourcesStarck.end(); iter++) {
                (*iter).UpdateMVPMatrix();
            }
            break;
        case OrthoMatrixType::T_3Dleftright:
            m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
            m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(2.0f, 2.0f, 0.1f, 5.0f));
            for (iter = m_resourcesStarck.begin(); iter != m_resourcesStarck.end(); iter++) {
                (*iter).UpdateMVPMatrix();
            }           
            break;
        case OrthoMatrixType::T_Stereopic:
            m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
            m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(2.0f, 2.0f, 0.1f, 5.0f));
            for (iter = m_resourcesStarck.begin(); iter != m_resourcesStarck.end(); iter++) {
                (*iter).ResetToSteropicMatirx(ctx);
            }
            break;
        default:
            m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
            m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(1.0f, 2.0f, 0.1f, 5.0f));
            for (iter = m_resourcesStarck.begin(); iter != m_resourcesStarck.end(); iter++) {
                (*iter).UpdateMVPMatrix();
            }
            break;
        }
    }
    //m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 1.0f), XMVectorSet(0, 0, 1, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
    //   m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(2 * m_resourcesStarck.size(), 2, 0.1, 5));
    ctx->UpdateSubresource(m_projectBuffer, 0, NULL, &m_projectMatrix, 0, 0);
    ctx->VSSetConstantBuffers(1, 1, &m_projectBuffer);
    ctx->Release();
}

template <typename Resource>
inline int DrawerManager<Resource>::UpdateRenderingDependent(bool isStereoipic)
{
    m_stereoEnable = isStereoipic;
    SafeRelease(m_renderTargetView);
    SafeRelease(m_renderTargetViewRight);
    SafeRelease(m_d3dDepthStencilView);
    HRESULT hr = ref_d3dSwapchain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
    if (FAILED(hr))
    {
        char buff[128] = {};
        sprintf_s(buff, "m_swapChain->ResizeBuffers(...) failed with error 0x%08X", hr);
        IvrLog::Inst()->Log(buff, 4);
        return -1;
    }
  
    //创建渲染目标视图
    ID3D11Texture2D* backBuffer(NULL);
    CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(
        D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        0,
        0,
        1);
    ref_d3dSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    hr = ref_d3dDevice->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView);
    if (FAILED(hr)) {
        // MessageBox(NULL, L"Create RenderTargetView failed!", L"error", MB_OK);
        char charBuf[128];
        sprintf_s(charBuf, 128, "UpdateRenderingDependent(...):CreateRenderTargetView(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
    }

    D3D11_TEXTURE2D_DESC backBufferDesc = {0};
    backBuffer->GetDesc(&backBufferDesc);
    if (m_stereoEnable && m_resourcesStarck.size() == 2) {
        CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewRightDesc(
            D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            0,
            1,
            1);
        hr = ref_d3dDevice->CreateRenderTargetView(backBuffer, &renderTargetViewRightDesc, &m_renderTargetViewRight);
        if (FAILED(hr)) {
            // MessageBox(NULL, L"Create RenderTargetView failed!", L"error", MB_OK);
            char charBuf[128];
            sprintf_s(charBuf, 128, "UpdateRenderingDependent(...):CreateRenderTargetView(...) failed with error %x", hr);
            IvrLog::Inst()->Log(std::string(charBuf), 4);
        }
    }

    // Create a descriptor for the depth/stencil buffer.
    D3D11_TEXTURE2D_DESC depthTexDesc;
    ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
    depthTexDesc.Width = backBufferDesc.Width;
    depthTexDesc.Height = backBufferDesc.Height;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTexDesc.CPUAccessFlags = 0;

    // Allocate a 2-D surface as the depth/stencil buffer.
    ID3D11Texture2D* depthStencil;
    ref_d3dDevice->CreateTexture2D(
        &depthTexDesc,
        nullptr,
        &depthStencil);
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = depthTexDesc.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    hr = ref_d3dDevice->CreateDepthStencilView(
        depthStencil,
        &descDSV,
        &m_d3dDepthStencilView);

    if (FAILED(hr)) {
        // MessageBox(NULL, L"Create RenderTargetView failed!", L"error", MB_OK);
        char charBuf[128];
        sprintf_s(charBuf, 128, "UpdateRenderingDependent(...):CreateDepthStencilView(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
    }
    SafeRelease(depthStencil);
    backBuffer->Release();

    return 0;
}
} // namespace dxshow
#endif
