using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace gcARTool
{
    class WindowPos
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct RECT_WIN
        {
            public int Left; //最左坐标
            public int Top; //最上坐标
            public int Right; //最右坐标
            public int Bottom; //最下坐标
        }

        //[DllImport("FView")]
        //public static extern void ShowInExe(System.IntPtr hWnd, System.IntPtr textureHandle, int w, int h);

        //[DllImport("FViewPlus")]
        //public static extern void StopView();



        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void log_callback_delegate([MarshalAs(UnmanagedType.I4)]int n

         );

        //[DllImport("FViewPlus", CallingConvention = CallingConvention.Cdecl)]
        //public extern static void set_log_callback(
        //    [MarshalAs(UnmanagedType.FunctionPtr)]
        //    log_callback_delegate handler
        //);


        //[DllImport("FView")]
        //public static extern float GetFreshRate();

        //寻找当前目标窗口的进程
        [DllImport("user32.dll")]
        public static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        //根据窗口句柄获取pid
        [DllImport("User32.dll")]
        public static extern int GetWindowThreadProcessId(IntPtr hwnd, out int ID);


        [DllImport("user32.dll", EntryPoint = "GetWindowLongA", SetLastError = true)]
        public static extern long GetWindowLong(IntPtr hwnd, int nIndex);

        public static IntPtr SetWindowLong(IntPtr hWnd, int nIndex, int dwNewLong)
        {
            if (IntPtr.Size == 4) {
                return SetWindowLongPtr32(hWnd, nIndex, dwNewLong);
            }
            return SetWindowLongPtr64(hWnd, nIndex, dwNewLong);
        }

        [DllImport("user32.dll", EntryPoint = "SetWindowLong", CharSet = CharSet.Auto)]
        public static extern IntPtr SetWindowLongPtr32(IntPtr hWnd, int nIndex, int dwNewLong);

        [DllImport("user32.dll", EntryPoint = "SetWindowLongPtr", CharSet = CharSet.Auto)]
        public static extern IntPtr SetWindowLongPtr64(IntPtr hWnd, int nIndex, int dwNewLong);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern long SetWindowPos(IntPtr hwnd, long hWndInsertAfter, long x, long y, long cx, long cy, long wFlags);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern bool MoveWindow(IntPtr hwnd, int x, int y, int cx, int cy, bool repaint);

        [DllImport("user32.dll", EntryPoint = "ShowWindow", SetLastError = true)]
        public static extern bool ShowWindow(IntPtr hWnd, uint nCmdShow);


        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern IntPtr SetFocus(IntPtr hWnd);

        [DllImport("user32.dll", CharSet = CharSet.Auto, ExactSpelling = true)]
        public static extern IntPtr GetForegroundWindow();

        [DllImport("user32.dll")]
        public static extern bool IsWindow(IntPtr hwnd);

        /// <summary>
        /// 得到屏幕个数，要注意只有在扩展模式屏幕个数才是2，复制模式屏幕个数仍是1.
        /// </summary>
        /// <returns></returns>
        [DllImport("oglwin")]
        public static extern uint GetMonitorCount();

        [DllImport("user32.dll")]
        public static extern bool EnumDisplayMonitors(IntPtr hdc, IntPtr lprcClip,
       MonitorEnumDelegate lpfnEnum, IntPtr dwData);


        public delegate bool MonitorEnumDelegate(IntPtr hMonitor, IntPtr hdcMonitor, ref Rect lprcMonitor, IntPtr dwData);

        [StructLayout(LayoutKind.Sequential)]
        public struct Rect
        {
            public int left;
            public int top;
            public int right;
            public int bottom;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct MonitorInfo
        {
            public uint size;
            public Rect rcMonitor;
            public Rect rcWork;
            public uint flags;
        }

        /// <summary>
        /// The struct that contains the display information
        /// </summary>
        public class DisplayInfo
        {
            public string Availability { get; set; }
            public string ScreenHeight { get; set; }
            public string ScreenWidth { get; set; }
            public Rect MonitorArea { get; set; }
            public Rect WorkArea { get; set; }
        }

        [DllImport("user32.dll")]
        public static extern bool GetMonitorInfoA(IntPtr monitorHandle, ref MonitorInfo mInfo);

        /// <summary>
        /// Collection of display information
        /// </summary>
        public class DisplayInfoCollection : List<DisplayInfo>
        {
        }

        /// <summary>
        /// Returns the number of Displays using the Win32 functions
        /// </summary>
        /// <returns>collection of Display Info</returns>
        public static DisplayInfoCollection GetDisplays()
        {
            DisplayInfoCollection col = new DisplayInfoCollection();

            EnumDisplayMonitors(IntPtr.Zero, IntPtr.Zero,
                delegate (IntPtr hMonitor, IntPtr hdcMonitor, ref Rect lprcMonitor, IntPtr dwData) {
                    MonitorInfo mi = new MonitorInfo();
                    mi.size = (uint)Marshal.SizeOf(mi);
                    bool success = GetMonitorInfoA(hMonitor, ref mi);
                    if (success) {
                        DisplayInfo di = new DisplayInfo();
                        di.ScreenWidth = (mi.rcMonitor.right - mi.rcMonitor.left).ToString();
                        di.ScreenHeight = (mi.rcMonitor.bottom - mi.rcMonitor.top).ToString();
                        di.MonitorArea = mi.rcMonitor;
                        di.WorkArea = mi.rcWork;
                        di.Availability = mi.flags.ToString();
                        col.Add(di);
                    }
                    return true;
                }, IntPtr.Zero);
            return col;
        }

        private const int SWP_NOOWNERZORDER = 0x200;
        private const int SWP_NOREDRAW = 0x8;
        private const int SWP_NOZORDER = 0x4;
        private const int SWP_SHOWWINDOW = 0x0040;
        private const int WS_EX_MDICHILD = 0x40;
        private const int SWP_FRAMECHANGED = 0x20;
        private const int SWP_NOACTIVATE = 0x10;
        private const int SWP_ASYNCWINDOWPOS = 0x4000;
        private const int SWP_NOMOVE = 0x2;
        private const int SWP_NOSIZE = 0x1;
        private const int GWL_STYLE = -16;
        private const int WS_VISIBLE = 0x10000000;
        private const int WM_CLOSE = 0x10;
        private const int WS_CHILD = 0x40000000;
        private const int WS_CAPTION = 0x00C00000;
        private const int WS_SYSMENU = 0x00080000;
        private const int WS_SIZEBOX = 0x00040000;

        private const int SW_HIDE = 0; //{隐藏, 并且任务栏也没有最小化图标}
        private const int SW_SHOWNORMAL = 1; //{用最近的大小和位置显示, 激活}
        private const int SW_NORMAL = 1; //{同 SW_SHOWNORMAL}
        private const int SW_SHOWMINIMIZED = 2; //{最小化, 激活}
        private const int SW_SHOWMAXIMIZED = 3; //{最大化, 激活}
        private const int SW_MAXIMIZE = 3; //{同 SW_SHOWMAXIMIZED}
        private const int SW_SHOWNOACTIVATE = 4; //{用最近的大小和位置显示, 不激活}
        private const int SW_SHOW = 5; //{同 SW_SHOWNORMAL}
        private const int SW_MINIMIZE = 6; //{最小化, 不激活}
        private const int SW_SHOWMINNOACTIVE = 7; //{同 SW_MINIMIZE}
        private const int SW_SHOWNA = 8; //{同 SW_SHOWNOACTIVATE}
        private const int SW_RESTORE = 9; //{同 SW_SHOWNORMAL}
        private const int SW_SHOWDEFAULT = 10; //{同 SW_SHOWNORMAL}
        private const int SW_MAX = 10; //{同 SW_SHOWNORMAL}

        public static void UpdateWindowPos(IntPtr hWnd, bool isMain, int width, int height)
        {
            DisplayInfoCollection displays = GetDisplays();
            if (displays.Count > 1) {
                foreach (var d in displays) {
                    if (d.Availability == "0" && isMain) {
                        MoveWindow(hWnd, (d.MonitorArea.right - d.MonitorArea.left / 2) - width / 2, (d.MonitorArea.bottom - d.MonitorArea.top) / 2 - height, width, height, true);
                        break;
                    }
                    else if (d.Availability == "1" && !isMain) {
                        ShowWindow(hWnd, SW_MAXIMIZE);
                    }
                }
            }
        }
    }
}
