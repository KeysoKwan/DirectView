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

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void log_callback_delegate([MarshalAs(UnmanagedType.I4)]int n);

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern IntPtr SetFocus(IntPtr hWnd);

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
        [DllImport("user32.dll")]
        private static extern long SetDisplayConfig(uint numPathArrayElements, IntPtr pathArray, uint numModeArrayElements,
                                                    IntPtr modeArray, uint flags);

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

        private const uint SDC_APPLY = 0x00000080;
        public const uint SDC_TOPOLOGY_CLONE = 0x00000002; //复制模式
        public const uint SDC_TOPOLOGY_EXTEND = 0x00000004; //扩展模式

        public static void UpdateWindowPos(System.Windows.Window mainWindow, System.Windows.Window meshWindow)
        {
            //先切换到扩展模式
            SetDisplayConfig(0, IntPtr.Zero, 0, IntPtr.Zero, (SDC_APPLY | SDC_TOPOLOGY_EXTEND));
            DisplayInfoCollection displays = GetDisplays();
            if (displays.Count > 1) {
                //移动主窗口到非当前显示器，网格窗口全屏到当前显示器
                if (mainWindow.Left > displays[1].MonitorArea.left) {
                    mainWindow.Left = displays[0].MonitorArea.left + 100;
                    mainWindow.Top = displays[0].MonitorArea.left + 100;

                    meshWindow.Left = displays[1].MonitorArea.left;
                    meshWindow.Top = displays[1].MonitorArea.top;
                    meshWindow.Width = displays[1].MonitorArea.right - displays[1].MonitorArea.left;
                    meshWindow.Height = displays[1].MonitorArea.bottom - displays[1].MonitorArea.top;
                    meshWindow.WindowState = System.Windows.WindowState.Maximized;
                }
                else {
                    mainWindow.Left = displays[1].MonitorArea.left + 100;
                    mainWindow.Top = displays[1].MonitorArea.top + 100;

                    meshWindow.Left = displays[0].MonitorArea.left;
                    meshWindow.Top = displays[0].MonitorArea.top;
                    meshWindow.Width = displays[0].MonitorArea.right - displays[0].MonitorArea.left;
                    meshWindow.Height = displays[0].MonitorArea.bottom - displays[0].MonitorArea.top;
                    meshWindow.WindowState = System.Windows.WindowState.Maximized;
                }
            }
        }
    }
}
