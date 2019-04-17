// VoxelShow.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "D3d11Show.h"
#include "Show.h"
#include "MCDevice.h"

using namespace dxlib;

//全局对象
D3d11Show m_d3d11show;

_VSAPI_ int StartView(HWND hWnd, void* textureHandle, int w, int h)
{
    //如果不在我们自己的机器上,那么就直接返回
    if (MCDevice::GetInst()->ReadID() == 0) {
        return -1;
    }

    m_d3d11show.StartRenderingView(hWnd, textureHandle, w, h);
    return 1;
}

_VSAPI_ void StopView()
{
    //如果不在我们自己的机器上,那么就直接返回
    if (MCDevice::GetInst()->ReadID() == 0) {
        return;
    }
    m_d3d11show.EndRendering();
}
