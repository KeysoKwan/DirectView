#include "windows.h"
#include "MonitorAdapter.h"
#include "RLock.h"

namespace GCmointor {

#define MAX_MONITOR_NAME 256

std::vector<MONITORINFOEX> g_hMonitorGroup;
std::mutex XDD_Mutex;

int CALLBACK MonitorEnumProc(HMONITOR hMonitor,
                             HDC hdc,
                             LPRECT lpRMonitor,
                             LPARAM dwData)
{
    MONITORINFOEX mixTemp;
    memset(&mixTemp, 0, sizeof(MONITORINFOEX));
    mixTemp.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(hMonitor, &mixTemp);
    g_hMonitorGroup.push_back(mixTemp);
    return 1;
}

// 根据Model判断EDID数据是否正确
BOOL XDD_IsCorrectEDID(
    IN const BYTE *pEDIDBuf, // EDID数据缓冲区
    IN DWORD dwcbBufSize,    // 数据字节大小
    IN LPCWSTR lpModel       // 型号
)
{
    // 参数有效性
    if (pEDIDBuf == NULL || dwcbBufSize < 24 || lpModel == NULL) {
        return FALSE;
    }

    // 判断EDID头
    if (pEDIDBuf[0] != 0x00 || pEDIDBuf[1] != 0xFF || pEDIDBuf[2] != 0xFF || pEDIDBuf[3] != 0xFF || pEDIDBuf[4] != 0xFF || pEDIDBuf[5] != 0xFF || pEDIDBuf[6] != 0xFF || pEDIDBuf[7] != 0x00) {
        return FALSE;
    }

    // 厂商名称 2个字节 可表三个大写英文字母
    // 每个字母有5位 共15位不足一位 在第一个字母代码最高位补 0” 字母 A”至 Z”对应的代码为00001至11010
    // 例如 MAG”三个字母 M代码为01101 A代码为00001 G代码为00111 在M代码前补0为001101
    // 自左向右排列得2字节 001101 00001 00111 转化为十六进制数即为34 27
    DWORD dwPos = 8;
    wchar_t wcModelBuf[9] = {0};
    char byte1 = pEDIDBuf[dwPos];
    char byte2 = pEDIDBuf[dwPos + 1];
    wcModelBuf[0] = ((byte1 & 0x7C) >> 2) + 64;
    wcModelBuf[1] = ((byte1 & 3) << 3) + ((byte2 & 0xE0) >> 5) + 64;
    wcModelBuf[2] = (byte2 & 0x1F) + 64;
    swprintf_s(wcModelBuf + 3, sizeof(wcModelBuf) / sizeof(wchar_t) - 3, L"%X%X%X%X", (pEDIDBuf[dwPos + 3] & 0xf0) >> 4, pEDIDBuf[dwPos + 3] & 0xf, (pEDIDBuf[dwPos + 2] & 0xf0) >> 4, pEDIDBuf[dwPos + 2] & 0x0f);

    // 比较MODEL是否匹配
    return (_wcsicmp(wcModelBuf, lpModel) == 0) ? TRUE : FALSE;
}

BOOL XDD_GetDeviceEDID(
    IN LPCWSTR lpModel,    // 型号
    IN LPCWSTR lpDriver,   // Driver
    OUT BYTE *pDataBuf,    // 输出EDID数据缓冲区
    IN DWORD dwcbBufSize,  // 输出缓冲区字节大小，不可小于256
    OUT DWORD *pdwGetBytes // 实际获得字节数
)
{
    // 初始化输出参数
    if (pdwGetBytes != NULL) {
        *pdwGetBytes = 0;
    }

    // 参数有效性
    if (lpModel == NULL || lpDriver == NULL || pDataBuf == NULL || dwcbBufSize == 0) {
        return FALSE;
    }

    // 打开设备注册表子键
    wchar_t wcSubKey[MAX_PATH] = L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\";
    wcscat_s(wcSubKey, lpModel);
    HKEY hSubKey;
    if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, wcSubKey, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS) {
        return FALSE;
    }

    // 存放EDID数据
    BOOL bGetEDIDSuccess = FALSE;
    BYTE EDIDBuf[256] = {0};
    DWORD dwEDIDSize = sizeof(EDIDBuf);

    // 枚举该子键下的键
    DWORD dwIndex = 0;
    DWORD dwSubKeyLen = sizeof(wcSubKey) / sizeof(wchar_t);
    FILETIME ft;
    while (bGetEDIDSuccess == FALSE && ::RegEnumKeyEx(hSubKey, dwIndex, wcSubKey, &dwSubKeyLen, NULL, NULL, NULL, &ft) == ERROR_SUCCESS) {
        // 打开枚举到的键
        HKEY hEnumKey;
        if (::RegOpenKeyEx(hSubKey, wcSubKey, 0, KEY_READ, &hEnumKey) == ERROR_SUCCESS) {
            // 打开的键下查询Driver键的值
            dwSubKeyLen = sizeof(wcSubKey) / sizeof(wchar_t);
            if (::RegQueryValueEx(hEnumKey, L"Driver", NULL, NULL, (LPBYTE)&wcSubKey, &dwSubKeyLen) == ERROR_SUCCESS && _wcsicmp(wcSubKey, lpDriver) == 0 // Driver匹配
            ) {
                // 打开键Device Parameters
                HKEY hDevParaKey;
                if (::RegOpenKeyEx(hEnumKey, L"Device Parameters", 0, KEY_READ, &hDevParaKey) == ERROR_SUCCESS) {
                    // 读取EDID
                    memset(EDIDBuf, 0, sizeof(EDIDBuf));
                    dwEDIDSize = sizeof(EDIDBuf);
                    if (::RegQueryValueEx(hDevParaKey, L"EDID", NULL, NULL, (LPBYTE)&EDIDBuf, &dwEDIDSize) == ERROR_SUCCESS && XDD_IsCorrectEDID(EDIDBuf, dwEDIDSize, lpModel) == TRUE // 正确的EDID数据
                    ) {
                        // 得到输出参数
                        DWORD dwRealGetBytes = min(dwEDIDSize, dwcbBufSize);
                        if (pdwGetBytes != NULL) {
                            *pdwGetBytes = dwRealGetBytes;
                        }
                        memcpy(pDataBuf, EDIDBuf, dwRealGetBytes);

                        // 成功获取EDID数据
                        bGetEDIDSuccess = TRUE;
                    }

                    // 关闭键Device Parameters
                    ::RegCloseKey(hDevParaKey);
                }
            }

            // 关闭枚举到的键
            ::RegCloseKey(hEnumKey);
        }

        // 下一个子键
        dwIndex += 1;
    }

    // 关闭设备注册表子键
    ::RegCloseKey(hSubKey);

    // 返回获取EDID数据结果
    return bGetEDIDSuccess;
}

BOOL XDD_GetModelDriverFromDeviceID(
    IN LPCWSTR lpDeviceID,      // DeviceID
    OUT std::wstring &strModel, // 输出型号，比如LEN0028
    OUT std::wstring &strDriver // 输出驱动信息，比如{4d36e96e-e325-11ce-bfc1-08002be10318}\0001
)
{
    // 初始化输出参数
    strModel = L"";
    strDriver = L"";

    // 参数有效性
    if (lpDeviceID == NULL) {
        return FALSE;
    }

    // 查找第一个斜杠后的开始位置
    LPCWSTR lpBegin = wcschr(lpDeviceID, L'\\');
    if (lpBegin == NULL) {
        return FALSE;
    }
    lpBegin += 1;

    // 查找开始后的第一个斜杠
    LPCWSTR lpSlash = wcschr(lpBegin, L'\\');
    if (lpSlash == NULL) {
        return FALSE;
    }

    // 得到Model，最长为7个字符
    wchar_t wcModelBuf[8] = {0};
    size_t szLen = lpSlash - lpBegin;
    if (szLen >= 8) {
        szLen = 7;
    }
    wcsncpy_s(wcModelBuf, lpBegin, szLen);

    // 得到输出参数
    strModel = std::wstring(wcModelBuf);
    strDriver = std::wstring(lpSlash + 1);

    // 解析成功
    return TRUE;
}

int XDD_GetActiveAttachedMonitor(std::vector<GCinfo> &out_vec) // 返回GCinfo列表
{
    //加入互斥防止多次Callback
    dxshow::Rlock lock(&XDD_Mutex);
    // 初始化输出参数
    out_vec.clear();
    g_hMonitorGroup.clear();
    // 枚举Adapter下Monitor用变量
    DWORD dwMonitorIndex = 0;
    DISPLAY_DEVICE ddMonTmp;

    // 枚举Adapter
    DWORD dwAdapterIndex = 0;
    DISPLAY_DEVICE ddAdapter;
    ddAdapter.cb = sizeof(ddAdapter);

    //枚举显示器Rect
    ::EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

    //枚举显示器EDID
    while (::EnumDisplayDevices(0, dwAdapterIndex, &ddAdapter, 0) != FALSE) {
        // 枚举该Adapter下的Monitor
        dwMonitorIndex = 0;
        ZeroMemory(&ddMonTmp, sizeof(ddMonTmp));
        ddMonTmp.cb = sizeof(ddMonTmp);
        while (::EnumDisplayDevices(ddAdapter.DeviceName, dwMonitorIndex, &ddMonTmp, 0) != FALSE) {
            // 判断状态是否正确
            if ((ddMonTmp.StateFlags & DISPLAY_DEVICE_ACTIVE) == DISPLAY_DEVICE_ACTIVE && (ddMonTmp.StateFlags & DISPLAY_DEVICE_ATTACHED) == DISPLAY_DEVICE_ATTACHED) {
                std::wstring strModel;
                std::wstring strDriver;
                if (XDD_GetModelDriverFromDeviceID(ddMonTmp.DeviceID, strModel, strDriver) == FALSE) {
                    //获取驱动失败
                    return -1;
                }

                BYTE EDIDBuf[256] = {0};
                DWORD OUTLENGTH = 0;
                if (XDD_GetDeviceEDID(strModel.c_str(), strDriver.c_str(), EDIDBuf, 256, &OUTLENGTH) == FALSE) {
                    //读取EDID失败
                    return -2;
                }
                /*       std::ofstream oFile;
                    oFile.open("C:\\Users\\Admin\\Desktop\\新建文本文档.txt");
                    for (int i = 0; i < OUTLENGTH; i++)
                    {
                        oFile << (char)EDIDBuf[i];
                    }
                    oFile.close();*/

                DWORD index = 71;
                while (index < OUTLENGTH - 10) {
                    //EDID描述符区域寻找硬件ID描述头
                    if (EDIDBuf[index] == '\0' && EDIDBuf[index + 1] == '\0' && EDIDBuf[index + 2] == '\0' && EDIDBuf[index + 3] == 252 && EDIDBuf[index + 4] == '\0') {
                        if (EDIDBuf[index + 5] != 'H' || EDIDBuf[index + 6] != 'A' || EDIDBuf[index + 7] != 'I' || EDIDBuf[index + 8] != 'E' || EDIDBuf[index + 9] != 'R') {
                            // EDID不匹配
                            std::vector<MONITORINFOEX>::iterator ithMoniter = g_hMonitorGroup.begin();
                            for (ithMoniter; ithMoniter != g_hMonitorGroup.end(); ithMoniter++) {
                                if (std::wstring(ddMonTmp.DeviceName).find(std::wstring((*ithMoniter).szDevice)) != std::wstring::npos) {
                                    //非GC显示器，读取Rect数据，入栈
                                    GCinfo info = {false, (*ithMoniter).rcMonitor.left, (*ithMoniter).rcMonitor.right, (*ithMoniter).rcMonitor.top, (*ithMoniter).rcMonitor.bottom};
                                    memcpy_s(info.DeviceName, 18, EDIDBuf + sizeof(BYTE) * (index + 5), 18);
                                    int i = 0;
                                    while (i < 18) {
                                        if (info.DeviceName[i] == '\n') {
                                            info.DeviceName[i] = '\0';
                                            break;
                                        }
                                        i++;
                                    }
                                    out_vec.push_back(info);
                                }
                            }
                        }
                        else //EDID匹配
                        {
                            std::vector<MONITORINFOEX>::iterator ithMoniter = g_hMonitorGroup.begin();
                            for (ithMoniter; ithMoniter != g_hMonitorGroup.end(); ithMoniter++) {
                                if (std::wstring(ddMonTmp.DeviceName).find(std::wstring((*ithMoniter).szDevice)) != std::wstring::npos) {
                                    //是GC显示器，读取Rect数据，入栈
                                    GCinfo info = {true, (*ithMoniter).rcMonitor.left, (*ithMoniter).rcMonitor.right, (*ithMoniter).rcMonitor.top, (*ithMoniter).rcMonitor.bottom};
                                    memcpy_s(info.DeviceName, 18, EDIDBuf + sizeof(BYTE) * (index + 5), 18);
                                    int i = 0;
                                    while (i < 18) {
                                        if (info.DeviceName[i] == '\n') {
                                            info.DeviceName[i] = '\0';
                                            break;
                                        }
                                        i++;
                                    }
                                    out_vec.push_back(info);
                                }
                            }
                        }
                        break;
                    }
                    index++;
                }
            }
            // 下一个Monitor
            dwMonitorIndex += 1;
            ZeroMemory(&ddMonTmp, sizeof(ddMonTmp));
            ddMonTmp.cb = sizeof(ddMonTmp);
        }

        // 下一个Adapter
        dwAdapterIndex += 1;
        ZeroMemory(&ddAdapter, sizeof(ddAdapter));
        ddAdapter.cb = sizeof(ddAdapter);
    }

    return out_vec.size();
}
} // namespace GCmointor
