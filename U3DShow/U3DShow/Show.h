#include "windows.h"

#define _VSAPI_ extern "C" __declspec(dllexport)

_VSAPI_ int StartView(HWND hWnd, void* textureHandle, int w, int h);

_VSAPI_ void StopView();