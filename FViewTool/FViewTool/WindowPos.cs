using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace gcARTool
{
    class WindowPos
    {
        [DllImport("user32.dll")]
        private static extern long SetDisplayConfig(uint numPathArrayElements, IntPtr pathArray, uint numModeArrayElements,
                                                    IntPtr modeArray, uint flags);
        private const uint SDC_APPLY = 0x00000080;
        public const uint SDC_TOPOLOGY_CLONE = 0x00000002; //复制模式
        public const uint SDC_TOPOLOGY_EXTEND = 0x00000004; //扩展模式



        public static void UpdateWindowPos(System.Windows.Window mainWindow, System.Windows.Window meshWindow)
        {
            //先切换到扩展模式
            SetDisplayConfig(0, IntPtr.Zero, 0, IntPtr.Zero, (SDC_APPLY | SDC_TOPOLOGY_EXTEND));
            //更新物理显示器列表
            int temp_MonitorCount = fmARUpdatePhysicalMonitor();
            if (temp_MonitorCount < 0)
                throw new Exception("fmARUpdatePhysicalMonitor failed with error :" + temp_MonitorCount);

            for (int k = 0; k < temp_MonitorCount; k++) {
                GCinfo gcinfo = new GCinfo();
                int result = fmARGetMonitorInfoByIndex(ref gcinfo, k);
                if (result > 0) {
                    if (gcinfo.isGCmonitor) {
                        meshWindow.Left = gcinfo.RCleft + 100;
                        meshWindow.Top = gcinfo.RCtop + 100;
                        meshWindow.WindowState = System.Windows.WindowState.Maximized;
                    }
                    else {
                        mainWindow.Left = gcinfo.RCleft + 100;
                        mainWindow.Top = gcinfo.RCtop + 100;
                    }
                }
                else
                    throw new Exception("fmARGetMonitorInfoByIndex failed with error :" + result);
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct GCinfo
        {
            public bool isGCmonitor;
            public int RCleft;
            public int RCright;
            public int RCtop;
            public int RCbottom;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 18)]
            public string DeviceName;
        };
        [DllImport(@"f-ar.dll")]
        public static extern int fmARUpdatePhysicalMonitor();
        [DllImport(@"f-ar.dll")]
        public static extern int fmARGetMonitorCount();
        [DllImport(@"f-ar.dll")]
        public static extern int fmARGetMonitorInfoByIndex(ref GCinfo out_struct, int index);
    }
}
