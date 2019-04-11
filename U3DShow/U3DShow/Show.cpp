// VoxelShow.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "D3d11Show.h"
#include "Show.h"


D3d11Show m_d3d11show;

_VSAPI_ int ShowInExe(HWND hWnd, void* textureHandle, int w, int h)
{
    m_d3d11show.StartRenderingView(hWnd,textureHandle,w, h);
    return 1;
}

_VSAPI_ void  StopShow()
{
    m_d3d11show.EndRendering();
}
