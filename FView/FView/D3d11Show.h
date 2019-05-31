#pragma once

#include <d3d11.h>
//#include "D3DX11/include/D3DX11.h"
#include <d3dcompiler.h>
//#include "D3DX11/include/xnamath.h"
#include <DirectXMath.h>
using namespace DirectX;

class D3d11Show
{
  public:
    D3d11Show();
    ~D3d11Show();

  public:
    void EndRendering();
    void StartRenderingView(HWND hWnd, void* textureHandle, int w, int h);

  private:
    void RenderTexture(HWND hWnd, void* textureHandle, int w, int h);
    void RealeaseD3d(bool isClearhWnd = true);
    void InitD3D(HWND hWnd, void* textureHandle, int w, int h);
    void DoRenderingView();

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
    ID3D11Buffer* vertexBuffer_;
    ID3D11ShaderResourceView* colorMap_;
    ID3D11SamplerState* colorMapSampler_;

    void* m_textureHandle;
    int m_w;
    int m_h;
    HWND m_ViewhWnd;
};
