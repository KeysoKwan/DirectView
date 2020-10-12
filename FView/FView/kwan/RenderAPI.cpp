#include "RenderAPI.h"
#include "../D3d11Show.h"
namespace dxshow {
RenderAPI::RenderAPI() : m_stereoEnabled(false),
                         m_OrthoMatrixType(OrthoMatrixType::T_2D),
                         m_isGamaSpace(U3DColorSpace::Gama)
{
}

RenderAPI::~RenderAPI()
{
}
int RenderAPI::StartRenderingView(HWND hWnd, int swapchainWidth, int swapchainHeight, int count, ...)
{
    return 0;
}
void RenderAPI::SwichProjector(OrthoMatrixType type)
{
}
void RenderAPI::SetGamaSpace(U3DColorSpace space)
{
    m_isGamaSpace = space;
}
bool RenderAPI::UpdateStereoEnabledStatus()
{
    return false;
}
void RenderAPI::EndRendering()
{
}
RenderAPI* CreateRenderAPI(UnityGfxRenderer apiType)
{
    if (apiType == kUnityGfxRendererD3D11) {
        return new D3d11Show();
    }
    else if (apiType == kUnityGfxRendererD3D12) {
        //not finish yet
        return new RenderAPI();
    }
    return nullptr;
}
} // namespace dxshow
