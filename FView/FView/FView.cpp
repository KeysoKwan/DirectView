// U3DShow.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <ShlObj.h> //SHGetSpecialFolderPath
#include "D3d11Show.h"
#include "FView.h"
#include "MCDevice.h"
#include "./dto/FViewRT.h"

using namespace dxlib;

//全局对象
D3d11Show m_d3d11show;
dto::FViewRT fviewRT;

_VSAPI_ int StartView(HWND hWnd, void* textureHandle, int w, int h)
{
    //如果不在我们自己的机器上,那么就直接返回
    if (MCDevice::GetInst()->ReadID() == 0) {
        return -1;
    }
    return m_d3d11show.StartRenderingView(hWnd, textureHandle, w, h);
}

_VSAPI_ int StartView_LR(HWND hWnd, void* LeftTextureHandle, void* RightTextureHandle, int w, int h)
{
    //如果不在我们自己的机器上,那么就直接返回
    if (MCDevice::GetInst()->ReadID() == 0) {
        return -1;
    }
    return m_d3d11show.StartRenderingView(hWnd, LeftTextureHandle, RightTextureHandle, w, h);
}
_VSAPI_ int SwitchProjector(int type)
{
    //如果不在我们自己的机器上,那么就直接返回
    if (MCDevice::GetInst()->ReadID() == 0) {
        return -1;
    }
    //传两张纹理指针的左右3D投屏下，可切换到只显示左画面到投屏窗口或左右一起显示
    // -----------         ---------
    // |   L  |   R  |  or   |     L     |
    // -----------         ----------
    //如果只传了一张纹理指针，则此函数无效
    type = type % 2;
    m_d3d11show.SwichProjector((DrawerManagerU3D::ProjectionType)type);
    return 1;
}

_VSAPI_ void IsGamaSpace(int space)
{
    //如果不在我们自己的机器上,那么就直接返回
    if (MCDevice::GetInst()->ReadID() == 0) {
        return;
    }
    //切换Gama与Linner色彩空间
    space %= 2;
    m_d3d11show.SetGamaSpace((D3d11Show::U3DColorSpace)space);
}

_VSAPI_ void StopView()
{
    //如果不在我们自己的机器上,那么就直接返回
    if (MCDevice::GetInst()->ReadID() == 0) {
        return;
    }
    //安全退出线程并关闭渲染窗口
    m_d3d11show.EndRendering();
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
