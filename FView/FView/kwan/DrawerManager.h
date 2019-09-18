#ifndef DrawerManager_H
#define DrawerManager_H

#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include "IVR_Log.h"

namespace dxshow {
//渲染管理类
template <typename Resource>
class DrawerManager
{
  public:
    enum class OrthoMatrixType
    {
        T_2D = 0,
        T_3Dleftright = 1
    };

    DrawerManager<Resource>();
    DrawerManager<Resource>(ID3D11Device* d3dDevice);
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

  private:
    struct ProjectBuffer
    {
        DirectX::XMMATRIX _view;
        DirectX::XMMATRIX _ortho;
    };
    ID3D11Device* m_d3dDevice;
    std::vector<Resource> m_resourcesStarck;

    ProjectBuffer m_projectMatrix;
    ID3D11Buffer* m_projectBuffer;

    OrthoMatrixType m_orthoMatrixType;

    template <typename Res>
    inline void SafeRelease(Res* ptr)
    {
        if (ptr != 0) ptr->Release();
    }
};

template <typename Resource>
DrawerManager<Resource>::DrawerManager()
{
    m_resourcesStarck.clear();
}

template <typename Resource>
DrawerManager<Resource>::DrawerManager(ID3D11Device* d3dDevice) : m_d3dDevice(d3dDevice), m_orthoMatrixType(DrawerManagerU3D::OrthoMatrixType::T_2D)
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
    HRESULT hr = m_d3dDevice->CreateBuffer(&commandDesc, NULL, &m_projectBuffer);
    if (FAILED(hr)) {
        /*MessageBox(NULL, L"Create Buffer failed!", L"error", MB_OK);*/
        char charBuf[512];
        sprintf_s(charBuf, 512, "DrawerManager():CreateBuffer(m_projectBuffer) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf));
        return;
    }
}

template <typename Resource>
DrawerManager<Resource>::~DrawerManager()
{
    m_resourcesStarck.clear();
    SafeRelease(m_projectBuffer);
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
        typename std::vector<Resource>::iterator iter = m_resourcesStarck.begin();
        for (iter; iter != m_resourcesStarck.end(); iter++) //遍历渲染整个容器
        {
            (*iter).Render(ctx, 6);
        }
        ctx->VSSetConstantBuffers(1, 1, &m_projectBuffer);
    }
}

template <typename Resource>
void DrawerManager<Resource>::UpdateAllMatrix(OrthoMatrixType type)
{
    using namespace DirectX;
    /* if (type == m_orthoMatrixType)
        return;*/
    if (m_resourcesStarck.size() == 0)
        return;

    m_orthoMatrixType = type;
    ID3D11DeviceContext* ctx = NULL;
    m_d3dDevice->GetImmediateContext(&ctx);
    if (m_resourcesStarck.size() == 1) {
        //   MessageBox(NULL, L"m_resourcesStarck.size() == 1", L"error", MB_OK);
        m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
        m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(2.0f, 2.0f, 0.1f, 5.0f));
    }
    else {
        switch (type) {
        case OrthoMatrixType::T_2D:
            if ((*m_resourcesStarck.begin()).GetResourceVieportType() == RenderingResources::ResourceViewport::LEFT_HALF) {
                m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(-0.5f, 0.0f, -1.0f, 1.0f), XMVectorSet(-0.5f, 0.0f, 1.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
                m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(1.0f, 2.0f, 0.1f, 5.0f));
            }
            else if ((*m_resourcesStarck.begin()).GetResourceVieportType() == RenderingResources::ResourceViewport::RIGHT_HALF) {
                m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0.5f, 0.0f, -1.0, 1.0f), XMVectorSet(0.5f, 0.0f, 1.0, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
                m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(1.0f, 2.0f, 0.1f, 5.0f));
            }
            else {
                m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
                m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(2.0f, 2.0f, 0.1f, 5.0f));
            }
            break;
        case OrthoMatrixType::T_3Dleftright:
            m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
            m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(2.0f, 2.0f, 0.1f, 5.0f));
            break;
        default:
            m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
            m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(1.0f, 2.0f, 0.1f, 5.0f));
            break;
        }
    }

    //m_projectMatrix._view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 1.0f), XMVectorSet(0, 0, 1, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
    //   m_projectMatrix._ortho = XMMatrixTranspose(XMMatrixOrthographicLH(2 * m_resourcesStarck.size(), 2, 0.1, 5));
    ctx->UpdateSubresource(m_projectBuffer, 0, NULL, &m_projectMatrix, 0, 0);
    ctx->VSSetConstantBuffers(1, 1, &m_projectBuffer);

    typename std::vector<Resource>::iterator iter;
    for (iter = m_resourcesStarck.begin(); iter != m_resourcesStarck.end(); iter++) {
        (*iter).UpdateMVPMatrix();
    }
    ctx->Release();
}
} // namespace dxshow
#endif
