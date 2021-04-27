// U3DShow.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <ShlObj.h> //SHGetSpecialFolderPath
#include "D3d11Show.h"
#include "FView.h"
#include "MCDevice.h"
#include "./dto/FViewRT.h"
#include "kwan/RenderAPI.h"
#include "kwan/MonitorAdapter.h"
#include "kwan/IUnityGraphics.h"

using namespace dxlib;

//全局对象
std::unique_ptr<dxshow::RenderAPI> m_currentAPI;
dto::FViewRT fviewRT;

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);
static IUnityInterfaces* s_UnityInterfaces = NULL;
static IUnityGraphics* s_Graphics = NULL;

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    s_UnityInterfaces = unityInterfaces;
    s_Graphics = s_UnityInterfaces->Get<IUnityGraphics>();
    s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

#if SUPPORT_VULKAN
    if (s_Graphics->GetRenderer() == kUnityGfxRendererNull)
    {
        extern void RenderAPI_Vulkan_OnPluginLoad(IUnityInterfaces*);
        RenderAPI_Vulkan_OnPluginLoad(unityInterfaces);
    }
#endif // SUPPORT_VULKAN

    // Run OnGraphicsDeviceEvent(initialize) manually on plugin load
    OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

static UnityGfxRenderer s_DeviceType = kUnityGfxRendererNull;

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
    // Create graphics API implementation upon initialization
    if (eventType == kUnityGfxDeviceEventInitialize)
    {
        s_DeviceType = s_Graphics->GetRenderer();
        m_currentAPI.reset(dxshow::CreateRenderAPI(s_DeviceType));      
    }

    // Cleanup graphics API implementation upon shutdown
    if (eventType == kUnityGfxDeviceEventShutdown)
    {
        s_DeviceType = kUnityGfxRendererNull;
    }
}


/// <returns>
///  返回错误代码
/// errorCode >0 成功
/// errorCode ==-1 如果不在我们自己的机器上
/// errorCode ==-2 窗口句柄丢失
/// errorCode ==-3 渲染设备初始化失败
/// errorCode ==-4 纹理句柄丢失
/// errorCode ==-5 渲染等待进程超时
/// errorCode ==-6 windows版本低于win10
/// errorCode ==-7 当前显卡不支持 
/// </returns>
_VSAPI_ int fmARStartViewDX11(HWND hWnd, void* textureHandle, void* RightTextureHandle, int w, int h)
{
    ////如果不在我们自己的机器上,那么就直接返回
    //if (MCDevice::GetInst()->ReadID() == 0) {
    //    return -1;
    //}
    if (RightTextureHandle == NULL || RightTextureHandle == nullptr)
        return m_currentAPI->StartRenderingView(hWnd, w, h, 1, textureHandle);
    else
        return m_currentAPI->StartRenderingView(hWnd, w, h, 2, textureHandle, RightTextureHandle);
}

//_VSAPI_ int fmARStartView_LRDX11(HWND hWnd, void* LeftTextureHandle, void* RightTextureHandle, int w, int h)
//{
//    //如果不在我们自己的机器上,那么就直接返回
//    if (MCDevice::GetInst()->ReadID() == 0) {
//        return -1;
//    }
//    return m_d3d11show.StartRenderingView(hWnd, w, h, 2, LeftTextureHandle, RightTextureHandle);
//}
// --------------------------------------------------------------------------
// GetRenderEventFunc, an example function we export which is used to get a rendering event callback function.
static void UNITY_INTERFACE_API OnRenderEvent(int eventID)
{
    // Unknown / unsupported graphics device type? Do nothing
    if (m_currentAPI == NULL)
        return;
    m_currentAPI->OnRenderEvent();
}

_VSAPI_ UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
{
    return OnRenderEvent;
}


_VSAPI_ int fmARSwitchProjector(int type)
{
    //如果不在我们自己的机器上,那么就直接返回
    /* if (MCDevice::GetInst()->ReadID() == 0) {
        return -1;
    }*/
    //传两张纹理指针的左右3D投屏下，可切换到只显示左画面到投屏窗口或左右一起显示
    // -----------       ---------
    // |  L |  R |  or   |   L   |
    // -----------       ---------
    //如果只传了一张纹理指针，则此函数无效
    type = type % 3;
    m_currentAPI->SwichProjector((dxshow::OrthoMatrixType)type);
    return 1;
}

_VSAPI_ void fmARIsGamaSpace(int space)
{
    //如果不在我们自己的机器上,那么就直接返回
    /* if (MCDevice::GetInst()->ReadID() == 0) {
        return;
    }*/
    //切换Gama与Linner色彩空间
    space %= 2;
    m_currentAPI->SetGamaSpace((dxshow::RenderAPI::U3DColorSpace)space);
}

_VSAPI_ bool fmARGetStereopicSupport()
{
    //如果不在我们自己的机器上,那么就直接返回
    /* if (MCDevice::GetInst()->ReadID() == 0) {
        return;
    }*/
    //切换Gama与Linner色彩空间
    return m_currentAPI->UpdateStereoEnabledStatus();
}

///设置程序目标帧数，最高不会超过屏幕刷新率
_VSAPI_ void fmARSetFramerate(int frameRate)
{
    //如果不在我们自己的机器上,那么就直接返回
    /* if (MCDevice::GetInst()->ReadID() == 0) {
        return;
    }*/

    m_currentAPI->TargetFrameRate = frameRate;
}

_VSAPI_ void fmARStopView()
{
    //如果不在我们自己的机器上,那么就直接返回
    /*  if (MCDevice::GetInst()->ReadID() == 0) {
        return;
    }*/
    //安全退出线程并关闭渲染窗口
    m_currentAPI->EndRendering();
}

// 通过EDID获取屏幕信息
// 返回屏幕坐标列表

//更新物理显示器列表
//return int
//  result >= 0 返回当前屏幕个数
//  result = -1 获取驱动失败
//  result = -2 读取EDID失败
_VSAPI_ int fmARUpdatePhysicalMonitor()
{
    return GCmointor::XDD_GetActiveAttachedMonitor(GCmointor::StackGcinfo);
}
//返回缓存中所有显示器数量
//return int
_VSAPI_ int fmARGetMonitorCount()
{
    return GCmointor::StackGcinfo.size();
}
//输入index返回GCinfo数据
//GCinfo* out_struct需要在外部创建内存
//return int
//  result =  1 数据获取成功
//  result = -1 out_struct为空指针
//  result = -2 index越界
_VSAPI_ int fmARGetMonitorInfoByIndex(GCmointor::GCinfo* out_struct, int index)
{
    if (out_struct == NULL)
        return -1;

    using namespace GCmointor;
    if (index >= 0 && index < StackGcinfo.size()) {
        out_struct->isGCmonitor = StackGcinfo[index].isGCmonitor;
        out_struct->RCleft = StackGcinfo[index].RCleft;
        out_struct->RCright = StackGcinfo[index].RCright;
        out_struct->RCtop = StackGcinfo[index].RCtop;
        out_struct->RCbottom = StackGcinfo[index].RCbottom;
        memcpy_s(out_struct->DeviceName, 18, StackGcinfo[index].DeviceName, 18);
        return 1;
    }
    else
        return -2;
}

///-------------------------------------------------------------------------------------------------
/// <summary> 独一个json文件路径. </summary>
///
/// <remarks> Dx, 2019/5/31. </remarks>
///
/// <param name="fviewFile"> [in,out] If non-null, the
///                          fview file. </param>
///
/// <returns> An int. </returns>
///-------------------------------------------------------------------------------------------------
_VSAPI_ int fmFViewReadJson()
{
    using namespace rapidjson;
    using namespace dxlib::json;

    char szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, szPath))) {
        std::string appdir{szPath};
        appdir.append("\\FViewTool\\FView.json");
        DocumentW doc;
        if (JsonHelper::readFile(appdir, doc) == false) {
            return -1; //打开文件失败
        }
        fviewRT.readJson(doc);
        return 0;
    }
    return -2;
}

_VSAPI_ void fmFViewGetPosition(dto::Vector3* value)
{
    if (value == nullptr) {
        return;
    }
    value->x = fviewRT.viewPosition.x;
    value->y = fviewRT.viewPosition.y;
    value->z = fviewRT.viewPosition.z;
}

_VSAPI_ void fmFViewGetRotation(dto::Quaternion* value)
{
    if (value == nullptr) {
        return;
    }
    value->x = fviewRT.viewRotation.x;
    value->y = fviewRT.viewRotation.y;
    value->z = fviewRT.viewRotation.z;
    value->w = fviewRT.viewRotation.w;
}
