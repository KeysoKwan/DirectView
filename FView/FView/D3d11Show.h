#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include "kwan/DrawerManager.h"
#include "kwan/RenderingResources.h"
#include <thread>
using namespace DirectX;

using U3DshowResourcesPTR = std::shared_ptr<RenderingResources>;
using DrawerManagerU3D = DrawerManager<shared_ptr<RenderingResources>>;

class D3d11Show
{
  public:
    D3d11Show();
    ~D3d11Show();

  public:
    enum U3DColorSpace
    {
        Gama = 0,
        Linear = 1
    };
    void EndRendering();
    //原来的调用方法
    int StartRenderingView(HWND hWnd, void* textureHandle, int w, int h);
    //为了不改变原有的接口，所以新增了一个方法，输入left right两张纹理指针绘制双屏
    int StartRenderingView(HWND hWnd, void* leftTexturePTR, void* rightTexturePTR, int w, int h);
    void SetupTextureHandle(void* textureHandle, RenderingResources::ResourceViewport type);
    void SwichProjector(DrawerManagerU3D::ProjectionType type);

    void SetGamaSpace(U3DColorSpace space);

  private:
    void RenderTexture();
    void RealeaseD3d(bool isClearhWnd = true);
    void InitD3D(HWND hWnd, int w, int h);
    void DoRenderingView(int count, ...);

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
    unique_ptr<DrawerManagerU3D> m_drawer = NULL;

    U3DColorSpace m_isGamaSpace;
    int m_w;
    int m_h;
    HWND m_ViewhWnd;

    bool m_isInit;
    //线程与线程安全对象
    thread m_renderingThread;
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
