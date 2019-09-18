#pragma once
#include <windows.h>
#include <vector>

typedef struct EnumHWndsArg
{
    std::vector<HWND> *vecHWnds;
    DWORD dwProcessId;
} EnumHWndsArg, *LPEnumHWndsArg;

void GetHWndsByProcessID(DWORD processID, std::vector<HWND> &vecHWnds);
