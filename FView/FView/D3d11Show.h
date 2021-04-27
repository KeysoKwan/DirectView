#pragma once

#include <d3d11_2.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h> 
#include "kwan/RenderAPI.h"
#include "kwan/DrawerManager.h"
#include "kwan/RenderingResources.h"
#include "kwan/RLock.h"
#include <thread>

namespace dxshow {
using DrawerManagerU3D = DrawerManager<RenderingResources>;
template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
class D3d11Show : public RenderAPI
{
  public:
    D3d11Show();
    virtual ~D3d11Show();

  public:
    ///输入
    ///HWND hWnd 窗口句柄
    ///int swapchainWidth 交换链路长度
    ///int swapchainHeight 交换链路宽度
    ///int count 缺省参数个数
    virtual int StartRenderingView(HWND hWnd, int swapchainWidth, int swapchainHeight, int count, ...);
    virtual int OnRenderEvent();
    virtual void SwichProjector(OrthoMatrixType type);
    virtual bool UpdateStereoEnabledStatus();
    virtual void EndRendering();

  private:
    void RealeaseD3d(bool isClearhWnd = true);
    int InitD3D();

  public:

    bool isRendering;
    bool OnWindowsResized;

  private:
    ComPtr<ID3D11Device> m_sDevice;
    ComPtr<ID3D11DeviceContext> m_deviceContext;
    ComPtr<IDXGISwapChain> m_swapChain;

    ComPtr<ID3D11VertexShader> solidColorVS_;
    ComPtr<ID3D11PixelShader> solidColorPS_;
    ComPtr<ID3D11InputLayout> inputLayout_;
    ComPtr<ID3D11SamplerState> colorMapSampler_;

    std::unique_ptr<DrawerManagerU3D> m_drawer;

    //新增渲染类智能指针对象
    //std::unique_ptr<DrawerManagerU3D> m_drawer = nullptr;
    //存下当前纹理指针，用于异常处理
    std::vector<void*> currentTexturePTR;

    
    HWND m_ViewhWnd;
    HWND m_u3dhWnd;

    int m_w;
    int m_h;

    bool m_isInit;
    bool m_MatrixModifyFlag;
    //线程与线程安全对象
    std::thread m_renderingThread;

    const char* m_SemaphoreName = "D3D11SHOW_sem";
    std::mutex m_mutex;

    DWORD lastFailedTick;
    size_t m_failedTime;
    std::thread m_rthread;
    
};
} // namespace dxshow
