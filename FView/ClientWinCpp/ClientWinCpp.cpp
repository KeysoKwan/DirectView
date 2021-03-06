﻿// ClientWinCpp.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ClientWinCpp.h"
#include "shellapi.h"

#define MAX_LOADSTRING 100
#define WM_SWITCHWINDOWLONG (WM_USER + 120)
#define WM_U3D WM_USER + 10

#define WM_OPEN_TOUCH_FEEDBACK WM_USER + 15
#define WM_CLOSE_TOUCH_FEEDBACK WM_USER + 16

// 全局变量:
HINSTANCE hInst;                     // 当前实例
WCHAR szTitle[MAX_LOADSTRING];       // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING]; // 主窗口类名
HANDLE hEvent_onWindowResize;        // 当窗口位置改变时事件
HWND g_hU3;                          // 接收到的u3d窗口
bool temp_edge = false;              // 窗口是否带边框，默认不带

// 此代码模块中包含的函数的前向声明:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENTWINCPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENTWINCPP));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIENTWINCPP);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    //Register　the　application　window　for　 receiving　multi-touch　input.
    if (!RegisterTouchWindow(hWnd, 0)) {
        /* MessageBox(hWnd,L"Cannot　 register　application　window　for　touch　input", L"Error", MB_OK);*/
        return FALSE;
    }

    return TRUE;
}

void SetTouchFeedback(HWND hWnd, bool enable)
{
    try {
        BOOL config = enable ? 1 : 0;
        SetWindowFeedbackSetting(hWnd, FEEDBACK_TOUCH_CONTACTVISUALIZATION, 0, 4, &config);
        SetWindowFeedbackSetting(hWnd, FEEDBACK_TOUCH_TAP, 0, 4, &config);
        SetWindowFeedbackSetting(hWnd, FEEDBACK_TOUCH_DOUBLETAP, 0, 4, &config);
        SetWindowFeedbackSetting(hWnd, FEEDBACK_TOUCH_PRESSANDHOLD, 0, 4, &config);
        SetWindowFeedbackSetting(hWnd, FEEDBACK_TOUCH_RIGHTTAP, 0, 4, &config);
    }
    catch (...) {
    }
}


/// 显示/隐藏边框
void ActiveEdge(HWND hwnd, bool active)
{
    if (active)
    {
        SetWindowLong(hwnd, GWL_STYLE, WS_BORDER | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP);
    }
    else
    {
        SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE | WS_TILEDWINDOW);
    }
}


//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    if (message >= WM_MOUSEMOVE && message <= WM_RBUTTONDBLCLK) {
        if (g_hU3 != NULL)
            PostMessage(g_hU3, message + 0x4000, (WPARAM)hWnd, lParam);
    }
    switch (message) {
    case WM_OPEN_TOUCH_FEEDBACK:
        SetTouchFeedback(hWnd, true);
        break;
    case WM_CLOSE_TOUCH_FEEDBACK:
        SetTouchFeedback(hWnd, false);
        break;
    case WM_TOUCH: {
        //A　WM_TOUCH 　message　can　contain　several　messages　from　different　 contacts
        //packed　together.
        unsigned int numInputs = (int)wParam;       //Number　of　actual　contact　 messages
        TOUCHINPUT* ti = new TOUCHINPUT[numInputs]; //　Allocate　the　storage　for
        //the　 parameters　of　the　per-
        //contact　 messages

        //Unpack　message　parameters　into　the 　array　of　TOUCHINPUT　structures,　each
        //representing 　a　message　for　one　single　contact.
        if (GetTouchInputInfo((HTOUCHINPUT)lParam, numInputs, ti, sizeof(TOUCHINPUT))) {
            if (numInputs > 0) {

                long info = MAKELONG(ti[0].x / 100, ti[0].y / 100);
                if (g_hU3 != NULL)
                    PostMessage(g_hU3, message + 0x4000, ti[0].dwFlags, info);
            }
        }
        CloseTouchInputHandle((HTOUCHINPUT)lParam);
        delete[] ti;
    } break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_F11:
            ActiveEdge(hWnd, temp_edge);
            temp_edge = !temp_edge;
            break;
        default:
            break;
        }
        break;
    case WM_U3D:
        g_hU3 = (HWND)wParam;
        PostMessage(g_hU3, WM_U3D, (WPARAM)hWnd, lParam);
        break;
    case WM_SHOWWINDOW:
        //SetWindowLong(hWnd, GWL_STYLE, WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP);
        LPWSTR* szArgList;
        int argCount;
        szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);
        if (szArgList == NULL) {
            SetWindowLong(hWnd, GWL_STYLE, WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP);
            break;
        }
        for (int i = 0; i < argCount; i++) {
            if (wcscmp(szArgList[i], L"-withedge") == 0) {
                temp_edge = true;
            }
        }
        if (!temp_edge)
            SetWindowLong(hWnd, GWL_STYLE, WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP);

        LocalFree(szArgList);
        break;
    case WM_SWITCHWINDOWLONG:

        break;
    case WM_SIZE:
        //当窗口发生位置改变
        hEvent_onWindowResize = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, L"FARwinResizeSignal");
        if (hEvent_onWindowResize)
            SetEvent(hEvent_onWindowResize);
        break;
    case WM_MOVE:
        //当窗口被移动
        //hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"FARwinResizeSignal");
        //if (hEvent)
        //    SetEvent(hEvent);
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 在此处添加使用 hdc 的任何绘图代码...
        EndPaint(hWnd, &ps);
    } break;
    case WM_DESTROY:
        if (g_hU3 != NULL) {
            PostMessage(g_hU3, WM_U3D + 1, (WPARAM)hWnd, lParam);
        }
        UnregisterTouchWindow(hWnd);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
