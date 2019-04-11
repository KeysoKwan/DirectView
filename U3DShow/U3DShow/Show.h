#include "windows.h"

#define _VSAPI_ extern "C" __declspec(dllexport)


_VSAPI_ void StopShow();

_VSAPI_ int ShowInExe(HWND hWnd, void* textureHandle, int w, int h);