#pragma once
#include "IVR_Log.h"
#include "IUnityGraphics.h"

namespace dxshow {
enum class OrthoMatrixType
{
    T_2D = 0,
    T_3Dleftright = 1,
    T_Stereopic = 2
};
class RenderAPI
{
  public:
    RenderAPI();
    virtual ~RenderAPI();

  public:
    enum class U3DColorSpace
    {
        Gama = 0,
        Linear = 1
    };

    ///输入
    ///HWND hWnd 窗口句柄
    ///int swapchainWidth 交换链路长度
    ///int swapchainHeight 交换链路宽度
    ///int count 缺省参数个数
    virtual int StartRenderingView(HWND hWnd, int swapchainWidth, int swapchainHeight, int count, ...);
    virtual void SwichProjector(OrthoMatrixType type);
    virtual void SetGamaSpace(U3DColorSpace space);
    virtual bool UpdateStereoEnabledStatus();
    virtual void EndRendering();
    ///是否默认开启3D stereo显示
    bool m_stereoEnabled;
    ///目标帧率
    int TargetFrameRate = 120;

  protected:
    U3DColorSpace m_isGamaSpace;
    ///投影模式
    OrthoMatrixType m_OrthoMatrixType;
    ///安全释放资源
    template <typename Res>
    inline void SafeRelease(Res* ptr)
    {
        if (ptr != nullptr) {
            ptr->Release();
            ptr = nullptr;
        }
    }
};
RenderAPI* CreateRenderAPI(UnityGfxRenderer apiType);
} // namespace dxshow
