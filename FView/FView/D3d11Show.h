#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include "kwan/DrawerManager.h"
#include "kwan/RenderingResources.h"
#include <thread>

namespace dxshow {
using DrawerManagerU3D = DrawerManager<RenderingResources>;

class D3d11Show
{
  public:
    D3d11Show();
    ~D3d11Show();

  public:
    enum class U3DColorSpace
    {
        Gama = 0,
        Linear = 1
    };
    void EndRendering();
    ///输入
    ///HWND hWnd 窗口句柄
    ///int swapchainWidth 交换链路长度
    ///int swapchainHeight 交换链路宽度
    ///int count 缺省参数个数
    int StartRenderingView(HWND hWnd, int swapchainWidth, int swapchainHeight, int count, ...);
    void SetupTextureHandle(void* textureHandle, RenderingResources::ResourceViewport type);
    void SwichProjector(DrawerManagerU3D::ProjectionType type);
    void SetGamaSpace(U3DColorSpace space);

  private:
    void RenderTexture();
    void RealeaseD3d(bool isClearhWnd = true);
    void InitD3D();

  public:
    bool isRendering;

  private:
    ID3D11Device* m_sDevice;
    ID3D11DeviceContext* m_deviceContext;
    IDXGISwapChain* m_swapChain;
    ID3D11RenderTargetView* m_renderTargetView;
    ID3D11VertexShader* solidColorVS_;
    ID3D11PixelShader* solidColorPS_;
    ID3D11InputLayout* inputLayout_;
    ID3D11SamplerState* colorMapSampler_;

    //新增渲染类智能指针对象
    std::unique_ptr<DrawerManagerU3D> m_drawer = nullptr;
    //存下当前纹理指针，用于异常处理
    std::vector<void*> currentTexturePTR;

    U3DColorSpace m_isGamaSpace;
    int m_w;
    int m_h;
    HWND m_ViewhWnd;

    bool m_isInit;
    //线程与线程安全对象
    std::thread m_renderingThread;
    HANDLE m_hSemaphore = NULL;
    const char* m_SemaphoreName = "D3D11SHOW_sem";
    //安全释放资源
    template <typename Res>
    inline void SafeRelease(Res* ptr)
    {
        if (ptr != 0) ptr->Release();
        ptr = 0;
    }
};
} // namespace dxshow
