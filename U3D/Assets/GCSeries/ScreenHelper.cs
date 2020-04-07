using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace GCSeries
{
    public class ScreenHelper
    {
        private static List<string> logCacheList = new List<string>();
        public static void ExceptionHandler(string targetSite, System.Exception ex, string append = "")
        {
            string str = targetSite + ex.Message + append;
            if (logCacheList.Contains(str)) return;
            logCacheList.Add(str);
            //AppLog.AddFormat(targetSite, ex, append);
        }
        public static void ExceptionHandler(string targetSite, string message, string append = "")
        {
            string str = targetSite + message + append;
            if (logCacheList.Contains(str)) return;
            logCacheList.Add(str);
            //AppLog.AddFormat(targetSite, message, append);
        }

        #region part1 投屏模式

        private const uint SIZE_OF_DISPLAYCONFIG_PATH_INFO = 72;
        private const uint SIZE_OF_DISPLAYCONFIG_MODE_INFO = 64;

        private const uint QDC_ALL_PATHS = 1;
        private const uint DISPLAYCONFIG_MODE_INFO_TYPE_TARGET = 2;
        private const uint DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE = 1;
        public const uint SDC_TOPOLOGY_INTERNAL = 0x00000001;
        public const uint SDC_TOPOLOGY_CLONE = 0x00000002;
        public const uint SDC_TOPOLOGY_EXTEND = 0x00000004;
        public const uint SDC_TOPOLOGY_EXTERNAL = 0x00000008;
        public const uint SDC_TOPOLOGY_SUPPLIED = 0x00000010;
        public const uint SDC_USE_DATABASE_CURRENT = (SDC_TOPOLOGY_INTERNAL | SDC_TOPOLOGY_CLONE | SDC_TOPOLOGY_EXTEND | SDC_TOPOLOGY_EXTERNAL);

        private const uint SDC_USE_SUPPLIED_DISPLAY_CONFIG = 0x00000020;
        private const uint SDC_VALIDATE = 0x00000040;
        private const uint SDC_APPLY = 0x00000080;
        private const uint SDC_ALLOW_CHANGES = 0x00000400;

        private const uint SDC_NO_OPTIMIZATION = 0x00000100;
        private const uint SDC_SAVE_TO_DATABASE = 0x00000200;
        private const uint SDC_PATH_PERSIST_IF_REQUIRED = 0x00000800;
        private const uint SDC_FORCE_MODE_ENUMERATION = 0x00001000;
        private const uint SDC_ALLOW_PATH_ORDER_CHANGES = 0x00002000;

        private const int DISP_CHANGE_BADFLAGS = -4;          //标志的无效设置被传送。
        private const int DISP_CHANGE_BADMODE = -2;           //不支持图形模式。
        private const int DISP_CHANGE_BADPARAM = -5;          //一个无效的参数被传递。它可以包括一个无效的标志或标志的组合。
        private const int DISP_CHANGE_FAILED = -1;            //指定图形模式的显示驱动失效。
        private const int DISP_CHANGE_NOTUPDATED = -3;        //在WindowsNT中不能把设置写入注册表。
        private const int DISP_CHANGE_RESTART = 1;            //为使图形模式生效计算机必须重新启动。
        private const int DISP_CHANGE_SUCCESSFUL = 0;         //设备改变成功。

        private const int CDS_FULLSCREEN = 0x4;
        /// <summary>
        /// 这些设置将保存在全局设置区内，因此它们对所有用户都有作用，该标志只有与CDS_UPDATEREGISTRY标志一起设置时才有效。
        /// </summary>
        private const int CDS_GLOBAL = 0x8;
        /// <summary>
        /// 设置保存在注册表中，但不起作用，该标志只有和CDS_UPDATEREGISTRY标志一起指定时才有效。
        /// </summary>
        private const int CDS_NORESET = 0x10000000;
        /// <summary>
        /// 即使请求设置与当前设置相同，也会改变设置。
        /// </summary>
        private const int CDS_RESET = 0x40000000;
        /// <summary>
        /// 该设备将成为原始设备。
        /// </summary>
        private const int CDS_SET_PRIMARY = 0x10;
        /// <summary>
        /// 如果请求的图形模要被设置，则系统进行测试，CDS_FULLSCREEN从本质上讲这种模式是暂时的。（对于Windows NT：如果改变到另一个桌面，或从另一个桌面改变，那么该模式将不被重置）。
        /// </summary>
        private const int CDS_TEST = 0x2;
        /// <summary>
        /// 当前屏幕的图形模式将动态地改变并且在注册表中图形模式将会更新，模式信息存储在USER描述文件中。
        /// </summary>
        private const int CDS_UPDATEREGISTRY = 0x1;
        private const int CDS_VIDEOPARAMETERS = 0x20;

        [DllImport("user32.dll", CharSet = CharSet.Unicode)]
        private static extern long GetDisplayConfigBufferSizes([In] uint flags, [Out] out uint numPathArrayElements,
                                                               [Out] out uint numModeArrayElements);

        [DllImport("user32.dll", CharSet = CharSet.Unicode)]
        private static extern long QueryDisplayConfig([In] uint flags, ref uint numPathArrayElements, IntPtr pathArray,
                                                      ref uint numModeArrayElements, IntPtr modeArray,
                                                      IntPtr currentTopologyId);

        [DllImport("user32.dll", CharSet = CharSet.Unicode)]
        private static extern long SetDisplayConfig(uint numPathArrayElements, IntPtr pathArray, uint numModeArrayElements,
                                                    IntPtr modeArray, uint flags);

        private static int GetModeInfoOffsetForDisplayId(uint displayIndex, IntPtr pModeArray, uint uNumModeArrayElements)
        {
            int offset;
            int modeInfoType;

            // there are always two mode infos per display (target and source)
            offset = (int)(displayIndex * SIZE_OF_DISPLAYCONFIG_MODE_INFO * 2);

            // out of bounds sanity check
            if (offset + SIZE_OF_DISPLAYCONFIG_MODE_INFO >= uNumModeArrayElements * SIZE_OF_DISPLAYCONFIG_MODE_INFO)
            {
                return -1;
            }

            // check which one of the two mode infos for the display is the target
            modeInfoType = Marshal.ReadInt32(pModeArray, offset);
            if (modeInfoType == DISPLAYCONFIG_MODE_INFO_TYPE_TARGET)
            {
                return offset;
            }
            else
            {
                offset += (int)SIZE_OF_DISPLAYCONFIG_MODE_INFO;
            }

            modeInfoType = Marshal.ReadInt32(pModeArray, offset);
            if (modeInfoType == DISPLAYCONFIG_MODE_INFO_TYPE_TARGET)
            {
                return offset;
            }
            // no target mode info found, this should never happen
            else
            {
                return -1;
            }
        }

        public static double GetRefreshRate(uint displayIndex)
        {
            uint uNumPathArrayElements = 0;
            uint uNumModeArrayElements = 0;
            IntPtr pPathArray = IntPtr.Zero;
            IntPtr pModeArray = IntPtr.Zero;
            IntPtr pCurrentTopologyId = IntPtr.Zero;
            long result;
            UInt32 numerator;
            UInt32 denominator;
            double refreshRate;

            // get size of buffers for QueryDisplayConfig
            result = GetDisplayConfigBufferSizes(QDC_ALL_PATHS, out uNumPathArrayElements, out uNumModeArrayElements);
            if (result != 0)
            {
                //   Log.Error("W7RefreshRateHelper.GetRefreshRate: GetDisplayConfigBufferSizes(...) returned {0}", result);
                return 0;
            }

            // allocate memory or QueryDisplayConfig buffers
            pPathArray = Marshal.AllocHGlobal((Int32)(uNumPathArrayElements * SIZE_OF_DISPLAYCONFIG_PATH_INFO));
            pModeArray = Marshal.AllocHGlobal((Int32)(uNumModeArrayElements * SIZE_OF_DISPLAYCONFIG_MODE_INFO));

            // get display configuration
            result = QueryDisplayConfig(QDC_ALL_PATHS,
                                        ref uNumPathArrayElements, pPathArray,
                                        ref uNumModeArrayElements, pModeArray,
                                        pCurrentTopologyId);
            // if failed log error message and free memory
            if (result != 0)
            {
                //   Log.Error("W7RefreshRateHelper.GetRefreshRate: QueryDisplayConfig(...) returned {0}", result);
                Marshal.FreeHGlobal(pPathArray);
                Marshal.FreeHGlobal(pModeArray);
                return 0;
            }

            // get offset for a display's target mode info
            int offset = GetModeInfoOffsetForDisplayId(displayIndex, pModeArray, uNumModeArrayElements);
            // if failed log error message and free memory
            if (offset == -1)
            {
                //Log.Error("W7RefreshRateHelper.GetRefreshRate: Couldn't find a target mode info for display {0}", displayIndex);
                Marshal.FreeHGlobal(pPathArray);
                Marshal.FreeHGlobal(pModeArray);
                return 0;
            }

            // get refresh rate
            numerator = (UInt32)Marshal.ReadInt32(pModeArray, offset + 32);
            denominator = (UInt32)Marshal.ReadInt32(pModeArray, offset + 36);
            refreshRate = (double)numerator / (double)denominator;
            //Log.Debug("W7RefreshRateHelper.GetRefreshRate: QueryDisplayConfig returned {0}/{1}", numerator, denominator);

            // free memory and return refresh rate
            Marshal.FreeHGlobal(pPathArray);
            Marshal.FreeHGlobal(pModeArray);
            return refreshRate;
        }
        public static bool SetRefreshRate(uint displayIndex, double refreshRate)
        {
            uint uNumPathArrayElements = 0;
            uint uNumModeArrayElements = 0;
            IntPtr pPathArray = IntPtr.Zero;
            IntPtr pModeArray = IntPtr.Zero;
            IntPtr pCurrentTopologyId = IntPtr.Zero;
            long result;
            UInt32 numerator;
            UInt32 denominator;
            UInt32 scanLineOrdering;
            UInt32 flags;

            // get size of buffers for QueryDisplayConfig
            result = GetDisplayConfigBufferSizes(QDC_ALL_PATHS, out uNumPathArrayElements, out uNumModeArrayElements);
            if (result != 0)
            {
                // Log.Error("W7RefreshRateHelper.GetRefreshRate: GetDisplayConfigBufferSizes(...) returned {0}", result);
                return false;
            }

            // allocate memory or QueryDisplayConfig buffers
            pPathArray = Marshal.AllocHGlobal((Int32)(uNumPathArrayElements * SIZE_OF_DISPLAYCONFIG_PATH_INFO));
            pModeArray = Marshal.AllocHGlobal((Int32)(uNumModeArrayElements * SIZE_OF_DISPLAYCONFIG_MODE_INFO));

            // get display configuration
            result = QueryDisplayConfig(QDC_ALL_PATHS,
                                        ref uNumPathArrayElements, pPathArray,
                                        ref uNumModeArrayElements, pModeArray,
                                        pCurrentTopologyId);
            // if failed log error message and free memory
            if (result != 0)
            {
                //   Log.Error("W7RefreshRateHelper.GetRefreshRate: QueryDisplayConfig(...) returned {0}", result);
                Marshal.FreeHGlobal(pPathArray);
                Marshal.FreeHGlobal(pModeArray);
                return false;
            }

            // get offset for a display's target mode info
            int offset = GetModeInfoOffsetForDisplayId(displayIndex, pModeArray, uNumModeArrayElements);
            // if failed log error message and free memory
            if (offset == -1)
            {
                //Log.Error("W7RefreshRateHelper.GetRefreshRate: Couldn't find a target mode info for display {0}", displayIndex);
                Marshal.FreeHGlobal(pPathArray);
                Marshal.FreeHGlobal(pModeArray);
                return false;
            }

            // TODO: refactor to private method
            // set proper numerator and denominator for refresh rate
            UInt32 newRefreshRate = (uint)(refreshRate * 1000);
            switch (newRefreshRate)
            {
                case 23976:
                    numerator = 24000;
                    denominator = 1001;
                    scanLineOrdering = DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE;
                    break;
                case 24000:
                    numerator = 24000;
                    denominator = 1000;
                    scanLineOrdering = DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE;
                    break;
                case 25000:
                    numerator = 25000;
                    denominator = 1000;
                    scanLineOrdering = DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE;
                    break;
                case 30000:
                    numerator = 30000;
                    denominator = 1000;
                    scanLineOrdering = DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE;
                    break;
                case 50000:
                    numerator = 50000;
                    denominator = 1000;
                    scanLineOrdering = DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE;
                    break;
                case 59940:
                    numerator = 60000;
                    denominator = 1001;
                    scanLineOrdering = DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE;
                    break;
                case 60000:
                    numerator = 60000;
                    denominator = 1000;
                    scanLineOrdering = DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE;
                    break;
                default:
                    numerator = (uint)(newRefreshRate / 1000);
                    denominator = 1;
                    scanLineOrdering = DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE;
                    break;
            }

            // set refresh rate parameters in display config
            Marshal.WriteInt32(pModeArray, offset + 32, (int)numerator);
            Marshal.WriteInt32(pModeArray, offset + 36, (int)denominator);
            Marshal.WriteInt32(pModeArray, offset + 56, (int)scanLineOrdering);

            // validate new refresh rate
            flags = SDC_VALIDATE | SDC_USE_SUPPLIED_DISPLAY_CONFIG;

            result = SetDisplayConfig(uNumPathArrayElements, pPathArray, uNumModeArrayElements, pModeArray, flags);

            // adding SDC_ALLOW_CHANGES to flags if validation failed
            if (result != 0)
            {
                //Log.Debug("W7RefreshRateHelper.SetDisplayConfig(...): SDC_VALIDATE of {0}/{1} failed", numerator, denominator);
                flags = SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_ALLOW_CHANGES;


            }
            else
            {
                //Log.Debug("W7RefreshRateHelper.SetDisplayConfig(...): SDC_VALIDATE of {0}/{1} succesful", numerator, denominator);
                flags = SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG;

            }

            // configuring display

            result = SetDisplayConfig(uNumPathArrayElements, pPathArray, uNumModeArrayElements, pModeArray, flags);

            // if failed log error message and free memory
            if (result != 0)
            {
                //Log.Error("W7RefreshRateHelper.SetDisplayConfig(...): SDC_APPLY returned {0}", result);
                Marshal.FreeHGlobal(pPathArray);
                Marshal.FreeHGlobal(pModeArray);
                return false;
            }

            // refresh rate change successful
            Marshal.FreeHGlobal(pPathArray);
            Marshal.FreeHGlobal(pModeArray);
            return true;
        }
        /// <summary>
        /// 设置投屏模式
        /// SDC_TOPOLOGY_INTERNAL
        /// SDC_TOPOLOGY_CLONE  克隆
        /// SDC_TOPOLOGY_EXTEND 扩展
        /// SDC_TOPOLOGY_EXTERNAL
        /// </summary>
        /// <param name="displayModel"></param>
        /// <returns></returns>
        public static bool SetProjection(uint displayModel)
        {
            return SetDisplayConfig(0, IntPtr.Zero, 0, IntPtr.Zero, SDC_APPLY | displayModel) == 0;
        }
        /// <summary>
        /// 是否是克隆模式
        /// </summary>
        /// <param name="allDisplayDevice"></param>
        /// <returns></returns>
        public static bool IsCloneProjection(List<DisplayDevice> allDisplayDevice)
        {
            if (allDisplayDevice.Count == 1 && allDisplayDevice[0].m_Monitors.Count > 1)
                return true;
            return false;
        }
        /// <summary>
        /// 是否是扩展模式
        /// </summary>
        /// <param name="allDisplayDevice"></param>
        /// <returns></returns>
        public static bool IsExtendProjection(List<DisplayDevice> allDisplayDevice)
        {
            return allDisplayDevice.Count > 1;
        }
        /// <summary>
        /// 设置克隆模式
        /// </summary>
        /// <returns></returns>
        public static bool SetCloneProjection()
        {
            return SetDisplayConfig(0, IntPtr.Zero, 0, IntPtr.Zero, SDC_APPLY | SDC_TOPOLOGY_CLONE) == 0;
        }
        /// <summary>
        /// 设置扩展模式
        /// </summary>
        /// <returns></returns>
        public static bool SetExtendProjection()
        {
            return SetDisplayConfig(0, IntPtr.Zero, 0, IntPtr.Zero, SDC_APPLY | SDC_TOPOLOGY_EXTEND) == 0;
        }
        /// <summary>
        /// 设置仅电脑
        /// </summary>
        /// <returns></returns>
        public static bool SetInternalProjection()
        {
            return SetDisplayConfig(0, IntPtr.Zero, 0, IntPtr.Zero, SDC_APPLY | SDC_TOPOLOGY_INTERNAL) == 0;
        }
        /// <summary>
        /// 设置仅第二屏
        /// </summary>
        /// <returns></returns>
        public static bool SetExternalProjection()
        {
            return SetDisplayConfig(0, IntPtr.Zero, 0, IntPtr.Zero, SDC_APPLY | SDC_TOPOLOGY_EXTERNAL) == 0;
        }

        #endregion

        #region part2 屏幕和显示设备
        [DllImport("user32.dll")]
        public static extern int ChangeDisplaySettings(ref DEVMODE devMode, int flags);

        //[DllImport("user32.dll")]
        //public static extern int ChangeDisplaySettingsEx(ref DEVMODE devMode, int flags);

        [DllImport("user32.dll")]
        public static extern int ChangeDisplaySettingsEx(string lpszDeviceName, [In] ref DEVMODE lpDevMode, IntPtr hwnd, int dwFlags, IntPtr lParam);

        [DllImport("user32.dll")]
        public static extern bool EnumDisplayDevices(string lpDevice, int iDevNum, ref DISPLAY_DEVICE lpDisplayDevice, int dwFlags);

        [DllImport("user32.dll")]
        public static extern bool EnumDisplaySettingsEx(string lpszDeviceName, int iModeNum, ref DEVMODE lpDevMode, int dwFlags);
        [DllImport("user32.dll")]
        public static extern int EnumDisplaySettings(string deviceName, int modeNum, ref DEVMODE devMode);

        public const Int32 CCHDEVICENAME = 32;
        public const Int32 CCHFORMNAME = 32;

        public enum DEVMODE_SETTINGS
        {
            ENUM_CURRENT_SETTINGS = (-1), //检索显示设备的当前设置。
            ENUM_REGISTRY_SETTINGS = (-2) //检索当前存储在注册表中的显示设备的设置。
        }
        [Flags()]
        public enum DisplayDeviceStateFlags : int
        {
            /// <summary>The device is part of the desktop.</summary>
            AttachedToDesktop = 0x1,
            MultiDriver = 0x2,
            /// <summary>The device is part of the desktop.</summary>
            PrimaryDevice = 0x4,
            /// <summary>Represents a pseudo device used to mirror application drawing for remoting or other purposes.</summary>
            MirroringDriver = 0x8,
            /// <summary>The device is VGA compatible.</summary>
            VGACompatible = 0x10,
            /// <summary>The device is removable; it cannot be the primary display.</summary>
            Removable = 0x20,
            /// <summary>The device has more display modes than its output devices support.</summary>
            ModesPruned = 0x8000000,
            Remote = 0x4000000,
            Disconnect = 0x2000000
        }

        public enum Display_Device_Stateflags
        {
            DISPLAY_DEVICE_ATTACHED_TO_DESKTOP = 0x1,
            DISPLAY_DEVICE_MIRRORING_DRIVER = 0x8,
            DISPLAY_DEVICE_MODESPRUNED = 0x8000000,
            DISPLAY_DEVICE_MULTI_DRIVER = 0x2,
            DISPLAY_DEVICE_PRIMARY_DEVICE = 0x4,
            DISPLAY_DEVICE_VGA_COMPATIBLE = 0x10
        }

        public enum DEVMODE_Flags
        {
            DM_BITSPERPEL = 0x40000,
            DM_DISPLAYFLAGS = 0x200000,
            DM_DISPLAYFREQUENCY = 0x400000,
            DM_PELSHEIGHT = 0x100000,
            DM_PELSWIDTH = 0x80000,
            DM_POSITION = 0x20
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct POINTL
        {
            [MarshalAs(UnmanagedType.I4)]
            public int x;
            [MarshalAs(UnmanagedType.I4)]
            public int y;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct DEVMODE
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string dmDeviceName;

            [MarshalAs(UnmanagedType.U2)]
            public UInt16 dmSpecVersion;

            [MarshalAs(UnmanagedType.U2)]
            public UInt16 dmDriverVersion;

            [MarshalAs(UnmanagedType.U2)]
            public UInt16 dmSize;

            [MarshalAs(UnmanagedType.U2)]
            public UInt16 dmDriverExtra;

            [MarshalAs(UnmanagedType.U4)]
            public DEVMODE_Flags dmFields;

            public POINTL dmPosition;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmDisplayOrientation;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmDisplayFixedOutput;

            [MarshalAs(UnmanagedType.I2)]
            public Int16 dmColor;

            [MarshalAs(UnmanagedType.I2)]
            public Int16 dmDuplex;

            [MarshalAs(UnmanagedType.I2)]
            public Int16 dmYResolution;

            [MarshalAs(UnmanagedType.I2)]
            public Int16 dmTTOption;

            [MarshalAs(UnmanagedType.I2)]
            public Int16 dmCollate;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string dmFormName;

            [MarshalAs(UnmanagedType.U2)]
            public UInt16 dmLogPixels;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmBitsPerPel;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmPelsWidth;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmPelsHeight;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmDisplayFlags;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmDisplayFrequency;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmICMMethod;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmICMIntent;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmMediaType;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmDitherType;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmReserved1;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmReserved2;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmPanningWidth;

            [MarshalAs(UnmanagedType.U4)]
            public UInt32 dmPanningHeight;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct DISPLAY_DEVICE
        {
            [MarshalAs(UnmanagedType.U4)]
            public int cb;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string DeviceName;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string DeviceString;
            [MarshalAs(UnmanagedType.U4)]
            public DisplayDeviceStateFlags StateFlags;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string DeviceID;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string DeviceKey;
        }

        public class DisplayDevice
        {
            public bool m_IsPrimary;
            public string m_DeviceName;
            public DEVMODE m_DEVMODE;
            public DISPLAY_DEVICE m_DISPLAYDEVICE;
            public List<DEVMODE> m_SupportSettings;
            public List<Monitor> m_Monitors = new List<Monitor>();

            public RECT MonitorDpiScaleRect
            {
                get
                {
                    if (m_Monitors.Count > 0)
                    {
                        Monitor monitor = m_Monitors[0];
                        return Scale(m_DEVMODE, monitor.m_DpiInfo);
                    }
                    return new RECT();
                }
            }
        }
        public class Monitor
        {
            public IntPtr m_hMonitor;
            public IntPtr m_hdcMonitor;
            public IntPtr m_lprcMonitor;
            public int m_DpiLevel;
            public string m_Name;
            public string m_DeviceName;
            public string m_FriendlyDeviceName;
            public DISPLAY_DEVICE m_DISPLAYDEVICE;
            public MonitorInfo m_MonitorInfo;
            public DpiInfo m_DpiInfo = new DpiInfo();
            public DisplayConfig m_DisplayConfig;
        }
        public class DisplayConfig
        {
            public DISPLAYCONFIG_TARGET_DEVICE_NAME m_DeviceName;
            public DISPLAYCONFIG_MODE_INFO m_ModeInfo;
        }

        private static void SwitchPrimaryScreen(DisplayDevice newPrimary, DisplayDevice oldPrimary)
        {
            MoveOldPrimary(newPrimary, oldPrimary);
            MoveNewPrimary(newPrimary, oldPrimary);
            CommitChange(newPrimary, oldPrimary);
        }
        private static void MoveOldPrimary(DisplayDevice newPrimary, DisplayDevice oldPrimary)
        {
            DEVMODE ndm3 = NewDevMode();
            ndm3.dmFields = DEVMODE_Flags.DM_POSITION;
            ndm3.dmPosition.x = (int)newPrimary.m_DEVMODE.dmPelsWidth;
            ndm3.dmPosition.y = 0;

            ChangeDisplaySettingsEx(oldPrimary.m_DISPLAYDEVICE.DeviceName, ref ndm3, (IntPtr)null, CDS_UPDATEREGISTRY | CDS_NORESET, IntPtr.Zero);

        }
        private static void MoveNewPrimary(DisplayDevice newPrimary, DisplayDevice oldPrimary)
        {
            DEVMODE ndm4 = NewDevMode();
            ndm4.dmFields = DEVMODE_Flags.DM_POSITION;
            ndm4.dmPosition.x = 0;
            ndm4.dmPosition.y = 0;
            ChangeDisplaySettingsEx(newPrimary.m_DISPLAYDEVICE.DeviceName, ref ndm4, (IntPtr)null, CDS_SET_PRIMARY | CDS_UPDATEREGISTRY | CDS_NORESET, IntPtr.Zero);
        }
        private static void CommitChange(DisplayDevice newPrimary, DisplayDevice oldPrimary)
        {
            DEVMODE ndm5 = NewDevMode();
            ChangeDisplaySettingsEx(oldPrimary.m_DISPLAYDEVICE.DeviceName, ref ndm5, (IntPtr)null, CDS_UPDATEREGISTRY, (IntPtr)null);

            DEVMODE ndm6 = NewDevMode();
            ChangeDisplaySettingsEx(newPrimary.m_DISPLAYDEVICE.DeviceName, ref ndm6, (IntPtr)null, CDS_SET_PRIMARY | CDS_SET_PRIMARY | CDS_UPDATEREGISTRY, IntPtr.Zero);
        }
        private static DEVMODE NewDevMode()
        {
            DEVMODE dm = new DEVMODE();
            dm.dmDeviceName = new String(new char[31]);
            dm.dmFormName = new String(new char[31]);
            dm.dmSize = (ushort)Marshal.SizeOf(dm);
            return dm;
        }
        private static string GetName(Monitor monitor)
        {
            try
            {
                string str = monitor.m_DISPLAYDEVICE.DeviceID;
                string[] strArr = str.Split('\\');
                return strArr[1];
            }
            catch (System.Exception ex)
            {
                ExceptionHandler("WpfEx.Helpers.ScreenEx.GetName", ex);
            }
            return string.Empty;
        }
        private static DisplayConfig GetByMonitorName(List<DisplayConfig> displayConfigs, string monitorName)
        {
            if (string.IsNullOrEmpty(monitorName))
                return null;
            foreach (DisplayConfig config in displayConfigs)
            {
                if (config.m_DeviceName.monitorDevicePath.Contains(monitorName))
                    return config;
            }
            return null;
        }
        private static Monitor GetByDeviceName(List<Monitor> monitors, string deviceName)
        {
            foreach (Monitor monitor in monitors)
            {
                if (monitor.m_DeviceName == deviceName)
                    return monitor;
            }
            return null;
        }
        private static bool ContainPels(List<DEVMODE> list, uint pelsWidth, uint pelsHeight)
        {
            foreach (DEVMODE dv in list)
            {
                if (dv.dmPelsWidth == pelsWidth && dv.dmPelsHeight == pelsHeight)
                    return true;
            }
            return false;
        }

        /// <summary>
        /// win10以上 监视器感知 InitProcessDpiAware / InitThreadDpiAware 将影响输出的值
        /// 友情提示SetWindowPos 参数对应MonitorInfo数据 
        /// </summary>
        /// <returns></returns>
        public static List<DisplayDevice> GetAllDisplayDevice()
        {
            List<Monitor> monitors = GetAllMonitors();
            List<DisplayConfig> displayConfigs = GetAllDisplayConfig();
            List<DisplayDevice> displayDevices = new List<DisplayDevice>();
            bool error = false;
            //Here I am listing all DisplayDevices (Monitors)
            for (int devId = 0; !error; devId++)
            {
                try
                {
                    DisplayDevice displayDevice = new DisplayDevice();

                    DISPLAY_DEVICE device = new DISPLAY_DEVICE();
                    device.cb = Marshal.SizeOf(typeof(DISPLAY_DEVICE));
                    error = !EnumDisplayDevices(null, devId, ref device, 0);
                    if ((device.StateFlags & DisplayDeviceStateFlags.AttachedToDesktop) == DisplayDeviceStateFlags.AttachedToDesktop)
                    {
                        displayDevice.m_DISPLAYDEVICE = device;
                        //再调一次查询
                        int dwMonitorIndex = 0;
                        DISPLAY_DEVICE adapterDevice = new DISPLAY_DEVICE();
                        adapterDevice.cb = Marshal.SizeOf(typeof(DISPLAY_DEVICE));
                        while (EnumDisplayDevices(device.DeviceName, dwMonitorIndex, ref adapterDevice, 0))
                        {
                            dwMonitorIndex++;

                            Monitor monitor = new Monitor();
                            monitor.m_DISPLAYDEVICE = adapterDevice;
                            displayDevice.m_Monitors.Add(monitor);

                            adapterDevice = new DISPLAY_DEVICE();
                            adapterDevice.cb = Marshal.SizeOf(typeof(DISPLAY_DEVICE));
                        }
                        displayDevices.Add(displayDevice);
                    }
                }
                catch (System.Exception ex)
                {
                    ExceptionHandler("WpfEx.Helpers.ScreenEx.GetAllDisplayDevice", ex);
                    error = true;
                }
            }

            foreach (DisplayDevice displayDevice in displayDevices)
            {
                DEVMODE ndm = NewDevMode();
                int res = EnumDisplaySettings(displayDevice.m_DISPLAYDEVICE.DeviceName, (int)DEVMODE_SETTINGS.ENUM_CURRENT_SETTINGS, ref ndm);
                if (res == 0)
                    ExceptionHandler("WpfEx.Helpers.ScreenEx.GetAllDisplayDevice", new Exception("GetAllDisplayDevice->EnumDisplaySettings=" + res + " arg0:" + displayDevice.m_DISPLAYDEVICE.DeviceName));
                displayDevice.m_DEVMODE = ndm;
                displayDevice.m_DeviceName = displayDevice.m_DISPLAYDEVICE.DeviceName;
                displayDevice.m_IsPrimary = ((displayDevice.m_DISPLAYDEVICE.StateFlags & DisplayDeviceStateFlags.PrimaryDevice) == DisplayDeviceStateFlags.PrimaryDevice);

                //displayDevice.m_ScreenWidth = (int)ndm.dmPelsWidth;
                //displayDevice.m_ScreenHeight = (int)ndm.dmPelsHeight;
                //displayDevice.m_XPosition = (int)ndm.dmPosition.x;
                //displayDevice.m_YPosition = (int)ndm.dmPosition.y;

                displayDevice.m_SupportSettings = EnumDeviceSettings(displayDevice.m_DISPLAYDEVICE.DeviceName);
                foreach (Monitor monitor in displayDevice.m_Monitors)
                {
                    monitor.m_Name = GetName(monitor);
                    monitor.m_DpiLevel = GetDpiLevel(monitor);
                    monitor.m_DisplayConfig = GetByMonitorName(displayConfigs, monitor.m_Name);
                    if (monitor.m_DisplayConfig != null)
                        monitor.m_FriendlyDeviceName = monitor.m_DisplayConfig.m_DeviceName.monitorFriendlyDeviceName;
                    Monitor temp = GetByDeviceName(monitors, displayDevice.m_DeviceName);
                    if (temp != null)
                    {
                        monitor.m_hMonitor = temp.m_hMonitor;
                        monitor.m_hdcMonitor = temp.m_hdcMonitor;
                        monitor.m_lprcMonitor = temp.m_lprcMonitor;
                        monitor.m_MonitorInfo = temp.m_MonitorInfo;
                        monitor.m_DeviceName = temp.m_DeviceName;
                        monitor.m_DpiInfo = temp.m_DpiInfo;
                        //displayDevice.m_ZoomX = ndm.dmPelsWidth * 1.0f / (monitor.m_MonitorInfo.rcMonitor.Right - monitor.m_MonitorInfo.rcMonitor.Left);
                        //displayDevice.m_ZoomY = ndm.dmPelsHeight * 1.0f / (monitor.m_MonitorInfo.rcMonitor.Bottom - monitor.m_MonitorInfo.rcMonitor.Top);
                    }
                }
            }

            return displayDevices;
        }
        /// <summary>
        /// outMonitor 对应工作区域  dd.m_DEVMODE 对应屏幕区域
        /// </summary>
        public static void Intersect(int left, int top, int right, int bottom, out List<DisplayDevice> outDDList,
            out DisplayDevice outDD, out Monitor outMonitor, out MonitorInfo outMonitorInfo, out long outArea)
        {

            outDD = null;
            outMonitor = null;
            outMonitorInfo = null;
            outArea = -1;

            RECT rect = new RECT(left, top, right, bottom);
            List<DisplayDevice> list = GetAllDisplayDevice();
            outDDList = list;
            foreach (DisplayDevice dd in list)
            {
                foreach (Monitor info in dd.m_Monitors)
                {
                    if (info.m_MonitorInfo != null)
                    {
                        RECT intersect = RECT.Intersect(rect, info.m_MonitorInfo.rcMonitor);
                        int area = (intersect.Right - intersect.Left) * (intersect.Bottom - intersect.Top);
                        if (area > outArea)
                        {
                            outArea = area;
                            outDD = dd;
                            outMonitor = info;
                            outMonitorInfo = info.m_MonitorInfo;
                        }
                    }

                }
            }
        }

        public static void SetPrimaryScreen(string deviceName)
        {
            List<DisplayDevice> screenList = GetAllDisplayDevice();
            DisplayDevice primaryScreen = GetPrimaryScreen(screenList);
            if (primaryScreen.m_DISPLAYDEVICE.DeviceName == deviceName)
                return;

            DisplayDevice newPrimaryScreen = GetScreen(deviceName);

            SwitchPrimaryScreen(newPrimaryScreen, primaryScreen);

        }
        public static DisplayDevice GetPrimaryScreen(List<DisplayDevice> devices)
        {
            foreach (DisplayDevice d in devices)
            {
                if ((d.m_DISPLAYDEVICE.StateFlags & DisplayDeviceStateFlags.PrimaryDevice) == DisplayDeviceStateFlags.PrimaryDevice)
                {
                    return d;
                }
            }
            return null;
        }
        public static List<DisplayDevice> GetUnPrimaryScreen(List<DisplayDevice> devices)
        {
            List<DisplayDevice> dList = new List<DisplayDevice>();

            foreach (DisplayDevice d in devices)
            {
                if ((d.m_DISPLAYDEVICE.StateFlags & DisplayDeviceStateFlags.PrimaryDevice) != DisplayDeviceStateFlags.PrimaryDevice)
                {
                    dList.Add(d);
                }
            }
            return dList;
        }
        public static DisplayDevice GetScreen(string deviceName)
        {
            List<DisplayDevice> screenList = GetAllDisplayDevice();
            return screenList.Where(p => p.m_DISPLAYDEVICE.DeviceName == deviceName).FirstOrDefault();
        }
        public static DisplayDevice Inside(List<DisplayDevice> list, int monitorX, int monitorY)
        {
            foreach (DisplayDevice dd in list)
            {
                foreach (Monitor monitor in dd.m_Monitors)
                {
                    RECT rect = monitor.m_MonitorInfo.rcMonitor;
                    if (monitorX >= rect.Left && monitorX <= rect.Right
                        && monitorY >= rect.Top && monitorY <= rect.Bottom)
                        return dd;
                }
            }
            return null;
        }
        /// <summary>
        /// 枚举当前显示设备支持的所有显示设置
        /// </summary>
        /// <param name="deviceName"></param>
        /// <returns></returns>
        public static List<DEVMODE> EnumDeviceSettings(string deviceName)
        {
            List<DEVMODE> list = new List<DEVMODE>();
            int index = 0;
            DEVMODE dm = NewDevMode();
            while (0 != EnumDisplaySettings(deviceName, index++, ref dm))
            {
                //if(!ContainPels(list, dm.dmPelsWidth, dm.dmPelsHeight))
                list.Add(dm);
                dm = NewDevMode();
            }
            return list;
        }
        #endregion

        #region part3 监视器信息
        //BOOL CALLBACK MonitorEnumProc（HMONITOR hmonitor，HDC hdcMonitor，LPRC lprcMonitor, DWORD dwData）
        //BOOL EnumDisplayMonitors（HDC hdc，LPCRECT lprcClip，MONITORENUMPROC lpfnEnum，LPARAM dwData）
        //HMONITOR MonitorFromWindow（HWND hwnd，DWORD dwFlags）
        //HMONITOR MonitorFromRect（LPCRECT lprc，DWORD dwFlags）
        //HMONITOR MonitorFromPoint（POINT pt，DWORD dwFlags）
        //BOOL GetMonitorInfo（HMONITOR hMonitor，LPMONITORINFO lpmi）

        public delegate bool MONITORENUMPROC(IntPtr hMonitor, IntPtr hdcMonitor, IntPtr lprcMonitor, IntPtr dwData);
        [StructLayout(LayoutKind.Sequential)]
        public struct POINTSTRUCT
        {
            public int x;
            public int y;
            public POINTSTRUCT(int x, int y)
            {
                this.x = x;
                this.y = y;
            }
        }
        [StructLayout(LayoutKind.Sequential, Size = 8)]
        public struct RECT
        {
            public int Left;
            public int Top;
            public int Right;
            public int Bottom;

            public int Width
            {
                set { Right = Left + value; }
                get { return Right - Left; }
            }
            public int Height
            {
                set { Bottom = Top + value; }
                get { return Bottom - Top; }
            }

            public RECT(int left, int top, int right, int bottom)
            {
                this.Left = left;
                this.Top = top;
                this.Right = right;
                this.Bottom = bottom;
            }
            public bool Contains(int x, int y)
            {
                return Left <= x && x < Right && Top <= y && y < Bottom;
            }
            public bool Contains(RECT rect)
            {
                return (this.Left <= rect.Left) && (rect.Right <= this.Right) && (this.Top <= rect.Top) && (rect.Bottom <= this.Bottom);
            }
            public void Inflate(int x, int y)
            {
                this.Left -= x;
                this.Top -= y;
                this.Right += x;
                this.Bottom += y;
            }
            public void Intersect(RECT rect)
            {
                RECT result = RECT.Intersect(rect, this);

                this.Left = result.Left;
                this.Top = result.Top;
                this.Right = result.Right;
                this.Bottom = result.Bottom;
            }
            public static RECT Intersect(RECT a, RECT b)
            {
                int x1 = Math.Max(a.Left, b.Left);
                int x2 = Math.Min(a.Right, b.Right);
                int y1 = Math.Max(a.Top, b.Top);
                int y2 = Math.Min(a.Bottom, b.Bottom);

                if (x2 >= x1 && y2 >= y1)
                {
                    return new RECT(x1, y1, x2, y2);
                }
                return new RECT();
            }
            public bool IntersectsWith(RECT rect)
            {
                return (rect.Left < this.Right) && (this.Left < (rect.Right)) && (rect.Top < this.Bottom) && (this.Top < rect.Bottom);
            }
            public static RECT Union(RECT a, RECT b)
            {
                int x1 = Math.Min(a.Left, b.Left);
                int x2 = Math.Max(a.Right, b.Right);
                int y1 = Math.Min(a.Top, b.Top);
                int y2 = Math.Max(a.Bottom, b.Bottom);

                return new RECT(x1, y1, x2, y2);
            }
        }
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto, Pack = 4)]
        public class MonitorInfo
        {
            public int cbSize = Marshal.SizeOf(typeof(MonitorInfo));
            public RECT rcMonitor = new RECT();
            public RECT rcWork = new RECT();
            public int dwFlags = 0;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            public char[] szDevice = new char[32];
        }


        [DllImport("User32.dll")]
        private static extern bool EnumDisplayMonitors(IntPtr hdc, IntPtr pprcClip, MONITORENUMPROC monitorEnumProc, IntPtr dwData);

        [DllImport("user32.dll", CharSet = CharSet.Auto, ExactSpelling = true)]
        public static extern int GetSystemMetrics(int nIndex);

        [DllImport("user32.dll", CharSet = CharSet.Auto, ExactSpelling = true)]
        private static extern UInt32 MonitorFromPoint(POINTSTRUCT pt, UInt32 dwFlags);

        [DllImport("User32.dll", CharSet = CharSet.Auto)]
        public static extern bool GetMonitorInfo(IntPtr hmonitor, [In, Out]MonitorInfo info);
        [DllImport("User32.dll", CharSet = CharSet.Auto)]
        public static extern bool GetMonitorInfo(IntPtr hmonitor, IntPtr info);

        private static MONITORENUMPROC MonitorEnumProcFun = MonitorEnumProc;
        private static List<Monitor> Monitors = new List<Monitor>();
        private static bool ShcoreDll = true;

        private static void MonitorDpi(Monitor monitor)
        {
            if (ShcoreDll)
            {
                try
                {
                    uint x, y;
                    int res = GetDpiForMonitor(monitor.m_hMonitor, DpiType.Effective, out x, out y);
                    if (res == (int)S_OK)
                    {
                        monitor.m_DpiInfo.dpiX = x;
                        monitor.m_DpiInfo.dpiY = y;
                    }
                    else
                        ExceptionHandler("ScreenHelper.MonitorEnumProc.MonitorDpi", $"result = {res}");
                }
                catch //(Exception ex)
                {
                    ShcoreDll = false;
                    ExceptionHandler("ScreenHelper.MonitorEnumProc.MonitorDpi", "全局Dpi模式");
                    //ExceptionHandler("ScreenHelper.MonitorEnumProc.MonitorDpi", ex);
                }
            }
            //兼容win 8.1以下
            if (!ShcoreDll)
            {
                IntPtr hdc = GetDC(IntPtr.Zero);
                monitor.m_DpiInfo.dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
                monitor.m_DpiInfo.dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
                ReleaseDC(IntPtr.Zero, hdc);
            }
        }
        private static bool MonitorEnumProc(IntPtr hMonitor, IntPtr hdcMonitor, IntPtr lprcMonitor, IntPtr dwData)
        {
            //RECT rect = (RECT)Marshal.PtrToStructure(lprcMonitor, typeof(RECT));
            MonitorInfo info = new MonitorInfo();
            if (GetMonitorInfo(hMonitor, info))
            {
                Monitor monitor = new Monitor();
                monitor.m_hMonitor = hMonitor;
                monitor.m_hdcMonitor = hdcMonitor;
                monitor.m_lprcMonitor = lprcMonitor;
                monitor.m_MonitorInfo = info;

                MonitorDpi(monitor);

                monitor.m_DeviceName = new string(info.szDevice).Replace('\0', ' ').Trim();
                monitor.m_DeviceName = System.Text.RegularExpressions.Regex.Replace(monitor.m_DeviceName, " ", "");
                Monitors.Add(monitor);
            }
            return true;
        }
        public static void GetAllMonitors(out List<Monitor> monitors)
        {
            EnumDisplayMonitors(IntPtr.Zero, IntPtr.Zero, MonitorEnumProcFun, IntPtr.Zero);
            monitors = new List<Monitor>();
            monitors.AddRange(Monitors);
            Monitors.Clear();
        }
        public static List<Monitor> GetAllMonitors()
        {
            EnumDisplayMonitors(IntPtr.Zero, IntPtr.Zero, MonitorEnumProcFun, IntPtr.Zero);
            List<Monitor> monitors = new List<Monitor>();
            monitors.AddRange(Monitors);
            Monitors.Clear();
            return monitors;
        }

        #endregion

        #region dpi感知 可以针对显示器  (wpf程序可以使用程序清单开启)
        public class DpiInfo
        {
            public float dpiX = 96;
            public float dpiY = 96;

            public float ScalingX
            {
                get { return dpiX / 96; }
            }
            public float ScalingY
            {
                get { return dpiY / 96; }
            }
        }
        public static RECT Scale(DEVMODE dev, DpiInfo info)
        {
            RECT rect = new RECT();
            rect.Left = dev.dmPosition.x;
            rect.Top = dev.dmPosition.y;
            rect.Right = rect.Left + (int)(dev.dmPelsWidth / info.ScalingX);
            rect.Bottom = rect.Top + (int)(dev.dmPelsHeight / info.ScalingY);
            return rect;
        }
        public static ulong S_OK = 0;
        public static ulong E_INVALIDARG = 2147942487;
        public static ulong E_ACCESSDENIED = 2147942405;
        public enum DPI_AWARENESS
        {
            DPI_AWARENESS_INVALID = -1,
            DPI_AWARENESS_UNAWARE = 0,
            DPI_AWARENESS_SYSTEM_AWARE = 1,
            DPI_AWARENESS_PER_MONITOR_AWARE = 2  //这种感知下DPI变更可以响应WM_DPICHANGED消息
        }
        //详情了解 -> https://docs.microsoft.com/zh-cn/windows/win32/hidpi/dpi-awareness-context
        public enum DPI_AWARENESS_CONTEXT
        {
            DPI_AWARENESS_CONTEXT_DEFAULT = 0, // Undocumented
            DPI_AWARENESS_CONTEXT_UNAWARE = -1, // Undocumented
            DPI_AWARENESS_CONTEXT_SYSTEM_AWARE = -2, // Undocumented
            DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE = -3, // Undocumented
            DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = -4, //win10 较早版本的操作系统上不可用
            DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED = -5,
        }
        /// <summary>
        /// Windows 10, version 1607 [desktop apps only]
        /// </summary>
        [DllImport("User32.dll")]
        public static extern DPI_AWARENESS_CONTEXT GetThreadDpiAwarenessContext();
        /// <summary>
        /// Windows 10, version 1607 [desktop apps only]
        /// </summary>
        [DllImport("User32.dll")]
        public static extern DPI_AWARENESS_CONTEXT GetWindowDpiAwarenessContext(IntPtr hwnd);
        /// <summary>
        /// Windows 10, version 1607 [desktop apps only]
        /// </summary>
        [DllImport("User32.dll")]
        public static extern DPI_AWARENESS GetAwarenessFromDpiAwarenessContext(DPI_AWARENESS_CONTEXT value);
        /// <summary>
        /// Windows 10, version 1607 [desktop apps only]
        /// </summary>
        [DllImport("User32.dll")]
        public static extern DPI_AWARENESS_CONTEXT SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT dpiContext);
        [DllImport("User32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool IsValidDpiAwarenessContext(DPI_AWARENESS_CONTEXT value);
        [DllImport("User32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool AreDpiAwarenessContextsEqual(DPI_AWARENESS_CONTEXT dpiContextA, DPI_AWARENESS_CONTEXT dpiContextB);

        [DllImport("User32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool SetProcessDPIAware();
        /// <summary>
        /// win8.1以上 进程仅能成功设置一次
        /// </summary>
        /// <param name="value"></param>
        /// <returns> S_OK:该应用程序的DPI意识已成功设置  E_INVALIDARG:传入的值无效 E_ACCESSDENIED:已经设置过了DPI感知 </returns>
        [DllImport("SHcore.dll")]
        public static extern ulong SetProcessDpiAwareness(DPI_AWARENESS value);
        /// <summary>
        /// win8.1以上
        /// </summary>
        [DllImport("SHcore.dll")]
        public static extern int GetProcessDpiAwareness(IntPtr hWnd, out DPI_AWARENESS value);

        public enum DpiType
        {
            Effective = 0,
            Angular = 1,
            Raw = 2,
        }
        /// <summary>
        /// win8.1以上 返回数据与设置的感知有关 
        /// </summary>
        [DllImport("Shcore.dll")]
        public static extern int GetDpiForMonitor([In]IntPtr hmonitor, [In]DpiType dpiType, [Out]out uint dpiX, [Out]out uint dpiY);
        /// <summary>
        /// 支持最低版本 Windows 10, version 1607 [desktop apps only]
        /// 返回数据取决于窗口的感知
        /// </summary>
        [DllImport("User32.dll")]
        public static extern uint GetDpiForWindow(IntPtr hwnd);

        /// <summary>
        /// 设置线程感知  
        /// c++ :某些win32 api获取值设置前后不一样 比如GetCursorPos
        /// wpf :窗口left top 设置会受到感知影响，通常不会是你所希望的值 建议使用api SetWindowPos设置窗口数据
        /// wpf :窗口width开启感知后会基于dpi缩放了的值而不是 监视器Monitor范围
        /// wpf :所以在开启感知下尽量使用相应api GetWindowRect SetWindowPos 不要用窗口Width Left等属性
        /// SetWindowPos 参数对应MonitorInfo数据
        /// </summary>
        /// <returns></returns>
        public static bool InitThreadDpiAware()
        {
            //线程设置 Windows 10 1607以上
            try
            {
                DPI_AWARENESS_CONTEXT context = GetThreadDpiAwarenessContext();
                DPI_AWARENESS aware = GetAwarenessFromDpiAwarenessContext(context);
                if (aware != DPI_AWARENESS.DPI_AWARENESS_PER_MONITOR_AWARE)
                {
                    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT.DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
                    context = GetThreadDpiAwarenessContext();
                    aware = GetAwarenessFromDpiAwarenessContext(context);
                    if (aware != DPI_AWARENESS.DPI_AWARENESS_PER_MONITOR_AWARE)
                    {
                        ExceptionHandler("ScreenHelper.InitMonitorDpiAware.SetThreadDpiAwarenessContext", $"设置失败 aware={aware.ToString()}");
                        return false;
                    }
                }
            }
            catch //(Exception ex)
            {
                return false;
                //ExceptionHandler("ScreenHelper.InitMonitorDpiAware.SetThreadDpiAwarenessContext", ex);
            }
            return true;
        }
        /// <summary>
        /// 初始化进程感知 仅能设置一次 后续将失败
        /// </summary>
        /// <returns></returns>
        public static bool InitProcessDpiAware()
        {
            try
            {
                //进程设置 兼容8.1上
                DPI_AWARENESS aware;
                GetProcessDpiAwareness(IntPtr.Zero, out aware);
                if (aware != DPI_AWARENESS.DPI_AWARENESS_PER_MONITOR_AWARE)
                {
                    ulong res = SetProcessDpiAwareness(DPI_AWARENESS.DPI_AWARENESS_PER_MONITOR_AWARE);
                    if (res == E_INVALIDARG)
                    {
                        ExceptionHandler("ScreenHelper.InitMonitorDpiAware.SetProcessDpiAwareness", "传入的值无效");
                        return false;
                    }
                    else if (res == E_ACCESSDENIED)
                    {
                        ExceptionHandler("ScreenHelper.InitMonitorDpiAware.SetProcessDpiAwareness", "已经设置过了DPI感知");
                        return false;
                    }
                }
            }
            catch //(Exception ex)
            {
                return false;
                //ExceptionHandler("ScreenHelper.InitMonitorDpiAware.SetProcessDpiAwareness", ex);
            }
            return true;
        }
        #endregion

        #region 桌面dpi  win8.1以下 全局获取方式

        const int HORZRES = 8;
        const int VERTRES = 10;
        const int LOGPIXELSX = 88;
        const int LOGPIXELSY = 90;
        const int DESKTOPVERTRES = 117;
        const int DESKTOPHORZRES = 118;

        [DllImport("user32.dll")]
        static extern IntPtr GetDC(IntPtr ptr);
        [DllImport("gdi32.dll")]
        static extern int GetDeviceCaps(IntPtr hdc, int nIndex);
        [DllImport("user32.dll", EntryPoint = "ReleaseDC")]
        static extern IntPtr ReleaseDC(IntPtr hWnd, IntPtr hDc);

        #endregion

        #region part4 设置dpi和像素
        /// <summary>
        /// 这是查看是否是自定义缩放
        /// 修改需要注销
        /// </summary>
        /// <returns></returns>
        public static bool IsCustomScaling()
        {
            try
            {
                Microsoft.Win32.RegistryKey reg = Microsoft.Win32.Registry.CurrentUser;
                Microsoft.Win32.RegistryKey run = reg.OpenSubKey(@"Control Panel\Desktop", true);
                object v = run.GetValue("Win8DpiScaling");
                run.Close();
                reg.Close();
                if (v == null) return false;
                int nv = (int)v;
                if (nv == 1)
                    return true;
            }
            catch (System.Exception ex)
            {
                ExceptionHandler("WpfEx.Helpers.ScreenEx.IsCustomScaling", ex);
            }
            return false;
        }
        // Get desktop dc
        //desktopDc = GetDC(NULL);
        //// Get native resolution
        //horizontalDPI = GetDeviceCaps(desktopDc, LOGPIXELSX);
        //verticalDPI = GetDeviceCaps(desktopDc, LOGPIXELSY);
        public static int GetDpiLevel(Monitor monitor)
        {
            int res = 0;
            try
            {
                Microsoft.Win32.RegistryKey reg = Microsoft.Win32.Registry.CurrentUser;
                Microsoft.Win32.RegistryKey run = reg.OpenSubKey(@"Control Panel\Desktop\PerMonitorSettings", true);
                foreach (string subKeyName in run.GetSubKeyNames())
                {
                    if (!subKeyName.StartsWith(monitor.m_Name)) continue;
                    Microsoft.Win32.RegistryKey sbuKey = run.OpenSubKey(subKeyName, true);
                    res = (int)sbuKey.GetValue("DpiValue");
                    sbuKey.Close();
                    break;
                }
                run.Close();
                reg.Close();
            }
            catch (System.Exception ex)
            {
                ExceptionHandler("WpfEx.Helpers.ScreenEx.GetDpiLevel", ex);
            }
            return res;
        }
        /// <summary>
        /// 设置显示设备Dpi 推荐值=0
        /// Since I'm only testing three possible values 0, -2 (4294967294), and -1 (4294967295), only doing this three times.
        /// 0 would be the default for that monitor(could be 100% or 150%, there may be others, but 150% is afaik only for high-res modern laptop screens)
        /// -2 would be two options above default (e.g.my lenovo defaults at 150%, but offers 125% above, -1, 100% above that, -2 -- also 175%, 1, and 200%, 2)
        /// -1 would be one option above default (other monitors don't have 5 options like my lenovo, and might be 100%, 150% default, 200% -- hence -1, 0, 1)
        /// </summary>
        /// <param name="monitor">显示设备</param>
        /// <param name="level">[?-?]</param>
        public static bool SetDpiLevel(DisplayDevice device, int level = 0, bool refresh = false)
        {
            //更新设置当前设置保证ScaleFactors下能查询到MonitorID
            DEVMODE ndm = NewDevMode();
            int res = EnumDisplaySettings(device.m_DISPLAYDEVICE.DeviceName, (int)DEVMODE_SETTINGS.ENUM_CURRENT_SETTINGS, ref ndm);
            if (res == 0)
            {
                ExceptionHandler("WpfEx.Helpers.ScreenEx.SetDpiLevel", new Exception("SetDpiLevel->EnumDisplaySettings=" + res + " arg0:" + device.m_DISPLAYDEVICE.DeviceName));
                return false;
            }
            //res = ChangeDisplaySettingsEx(device.m_DISPLAYDEVICE.DeviceName, ref device.m_DEVMODE, (IntPtr)null, 0, IntPtr.Zero);
            res = ChangeDisplaySettingsEx(device.m_DISPLAYDEVICE.DeviceName, ref ndm, (IntPtr)null, 0, IntPtr.Zero);
            foreach (Monitor monitor in device.m_Monitors)
            {
                try
                {
                    string monitorID = string.Empty;
                    Microsoft.Win32.RegistryKey reg = Microsoft.Win32.Registry.LocalMachine;
                    Microsoft.Win32.RegistryKey run = reg.OpenSubKey(@"SYSTEM\CurrentControlSet\Control\GraphicsDrivers\ScaleFactors", true);
                    foreach (string subKeyName in run.GetSubKeyNames())
                    {
                        if (!subKeyName.StartsWith(monitor.m_Name)) continue;
                        monitorID = subKeyName;
                        break;
                    }
                    run.Close();
                    reg.Close();

                    if (string.IsNullOrEmpty(monitorID))
                    {
                        ExceptionHandler("WpfEx.Helpers.ScreenEx.SetDpiLevel", new System.Exception(@"SYSTEM\CurrentControlSet\Control\GraphicsDrivers\ScaleFactors Not Find:" + monitor.m_Name));
                        return false;
                    }

                    reg = Microsoft.Win32.Registry.CurrentUser;
                    run = reg.OpenSubKey(@"Control Panel\Desktop\PerMonitorSettings\" + monitorID, true);
                    if (run == null)
                        run = reg.CreateSubKey(@"Control Panel\Desktop\PerMonitorSettings\" + monitorID);
                    run.SetValue("DpiValue", level);

                    run.Flush();
                    reg.Flush();
                    run.Close();
                    reg.Close();
                }
                catch (System.Exception ex)
                {
                    ExceptionHandler("WpfEx.Helpers.ScreenEx.SetDpiLevel", ex);
                    return false;
                }
            }
            //refresh = true;
            if (refresh)
                res = ChangeDisplaySettingsEx(device.m_DISPLAYDEVICE.DeviceName, ref device.m_DEVMODE, (IntPtr)null, CDS_RESET, IntPtr.Zero);
            return true;
        }
        public static bool SetPels(DisplayDevice device, int width, int height)
        {
            if (device.m_SupportSettings == null) return false;
            for (int i = 0; i < device.m_SupportSettings.Count; i++)
            {
                DEVMODE dm = device.m_SupportSettings[i];
                if (dm.dmPelsWidth != width || dm.dmPelsHeight != height)
                    continue;
                dm.dmFields = DEVMODE_Flags.DM_PELSHEIGHT | DEVMODE_Flags.DM_PELSWIDTH;
                //int res = ChangeDisplaySettings(ref dm, CDS_RESET | CDS_FULLSCREEN);
                ChangeDisplaySettingsEx(device.m_DISPLAYDEVICE.DeviceName, ref dm, (IntPtr)null, CDS_RESET, IntPtr.Zero);
                return true;
            }
            return true;
        }
        #endregion

        #region part5 Connecting and Configuring Display CCD显示配置

        public const int ERROR_SUCCESS = 0;

        public enum QUERY_DEVICE_CONFIG_FLAGS : uint
        {
            QDC_ALL_PATHS = 0x00000001,
            QDC_ONLY_ACTIVE_PATHS = 0x00000002,
            QDC_DATABASE_CURRENT = 0x00000004
        }

        public enum DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY : uint
        {
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER = 0xFFFFFFFF,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HD15 = 0,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SVIDEO = 1,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPOSITE_VIDEO = 2,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPONENT_VIDEO = 3,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DVI = 4,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI = 5,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_LVDS = 6,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_D_JPN = 8,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDI = 9,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EXTERNAL = 10,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED = 11,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EXTERNAL = 12,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED = 13,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDTVDONGLE = 14,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_MIRACAST = 15,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL = 0x80000000,
            DISPLAYCONFIG_OUTPUT_TECHNOLOGY_FORCE_UINT32 = 0xFFFFFFFF
        }

        public enum DISPLAYCONFIG_SCANLINE_ORDERING : uint
        {
            DISPLAYCONFIG_SCANLINE_ORDERING_UNSPECIFIED = 0,
            DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE = 1,
            DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED = 2,
            DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED_UPPERFIELDFIRST = DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED,
            DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED_LOWERFIELDFIRST = 3,
            DISPLAYCONFIG_SCANLINE_ORDERING_FORCE_UINT32 = 0xFFFFFFFF
        }

        public enum DISPLAYCONFIG_ROTATION : uint
        {
            DISPLAYCONFIG_ROTATION_IDENTITY = 1,
            DISPLAYCONFIG_ROTATION_ROTATE90 = 2,
            DISPLAYCONFIG_ROTATION_ROTATE180 = 3,
            DISPLAYCONFIG_ROTATION_ROTATE270 = 4,
            DISPLAYCONFIG_ROTATION_FORCE_UINT32 = 0xFFFFFFFF
        }

        public enum DISPLAYCONFIG_SCALING : uint
        {
            DISPLAYCONFIG_SCALING_IDENTITY = 1,
            DISPLAYCONFIG_SCALING_CENTERED = 2,
            DISPLAYCONFIG_SCALING_STRETCHED = 3,
            DISPLAYCONFIG_SCALING_ASPECTRATIOCENTEREDMAX = 4,
            DISPLAYCONFIG_SCALING_CUSTOM = 5,
            DISPLAYCONFIG_SCALING_PREFERRED = 128,
            DISPLAYCONFIG_SCALING_FORCE_UINT32 = 0xFFFFFFFF
        }

        public enum DISPLAYCONFIG_PIXELFORMAT : uint
        {
            DISPLAYCONFIG_PIXELFORMAT_8BPP = 1,
            DISPLAYCONFIG_PIXELFORMAT_16BPP = 2,
            DISPLAYCONFIG_PIXELFORMAT_24BPP = 3,
            DISPLAYCONFIG_PIXELFORMAT_32BPP = 4,
            DISPLAYCONFIG_PIXELFORMAT_NONGDI = 5,
            DISPLAYCONFIG_PIXELFORMAT_FORCE_UINT32 = 0xffffffff
        }

        public enum DISPLAYCONFIG_MODE_INFO_TYPE : uint
        {
            DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE = 1,
            DISPLAYCONFIG_MODE_INFO_TYPE_TARGET = 2,
            DISPLAYCONFIG_MODE_INFO_TYPE_FORCE_UINT32 = 0xFFFFFFFF
        }

        public enum DISPLAYCONFIG_DEVICE_INFO_TYPE : uint
        {
            DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME = 1,
            DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME = 2,
            DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_PREFERRED_MODE = 3,
            DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME = 4,
            DISPLAYCONFIG_DEVICE_INFO_SET_TARGET_PERSISTENCE = 5,
            DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_BASE_TYPE = 6,
            DISPLAYCONFIG_DEVICE_INFO_FORCE_UINT32 = 0xFFFFFFFF
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct LUID
        {
            public uint LowPart;
            public int HighPart;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DISPLAYCONFIG_PATH_SOURCE_INFO
        {
            public LUID adapterId;
            public uint id;
            public uint modeInfoIdx;
            public uint statusFlags;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DISPLAYCONFIG_PATH_TARGET_INFO
        {
            public LUID adapterId;
            public uint id;
            public uint modeInfoIdx;
            DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY outputTechnology;
            DISPLAYCONFIG_ROTATION rotation;
            DISPLAYCONFIG_SCALING scaling;
            DISPLAYCONFIG_RATIONAL refreshRate;
            DISPLAYCONFIG_SCANLINE_ORDERING scanLineOrdering;
            public bool targetAvailable;
            public uint statusFlags;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DISPLAYCONFIG_RATIONAL
        {
            public uint Numerator;
            public uint Denominator;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DISPLAYCONFIG_PATH_INFO
        {
            public DISPLAYCONFIG_PATH_SOURCE_INFO sourceInfo;
            public DISPLAYCONFIG_PATH_TARGET_INFO targetInfo;
            public uint flags;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DISPLAYCONFIG_2DREGION
        {
            public uint cx;
            public uint cy;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DISPLAYCONFIG_VIDEO_SIGNAL_INFO
        {
            public ulong pixelRate;
            public DISPLAYCONFIG_RATIONAL hSyncFreq;
            public DISPLAYCONFIG_RATIONAL vSyncFreq;
            public DISPLAYCONFIG_2DREGION activeSize;
            public DISPLAYCONFIG_2DREGION totalSize;
            public uint videoStandard;
            public DISPLAYCONFIG_SCANLINE_ORDERING scanLineOrdering;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DISPLAYCONFIG_TARGET_MODE
        {
            public DISPLAYCONFIG_VIDEO_SIGNAL_INFO targetVideoSignalInfo;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DISPLAYCONFIG_SOURCE_MODE
        {
            public uint width;
            public uint height;
            public DISPLAYCONFIG_PIXELFORMAT pixelFormat;
            public POINTL position;
        }

        [StructLayout(LayoutKind.Explicit)]
        public struct DISPLAYCONFIG_MODE_INFO_UNION
        {
            [FieldOffset(0)]
            public DISPLAYCONFIG_TARGET_MODE targetMode;
            [FieldOffset(0)]
            public DISPLAYCONFIG_SOURCE_MODE sourceMode;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DISPLAYCONFIG_MODE_INFO
        {
            public DISPLAYCONFIG_MODE_INFO_TYPE infoType;
            public uint id;
            public LUID adapterId;
            public DISPLAYCONFIG_MODE_INFO_UNION modeInfo;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DISPLAYCONFIG_TARGET_DEVICE_NAME_FLAGS
        {
            public uint value;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DISPLAYCONFIG_DEVICE_INFO_HEADER
        {
            public DISPLAYCONFIG_DEVICE_INFO_TYPE type;
            public uint size;
            public LUID adapterId;
            public uint id;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct DISPLAYCONFIG_TARGET_DEVICE_NAME
        {
            public DISPLAYCONFIG_DEVICE_INFO_HEADER header;
            public DISPLAYCONFIG_TARGET_DEVICE_NAME_FLAGS flags;
            public DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY outputTechnology;
            public ushort edidManufactureId;
            public ushort edidProductCodeId;
            public uint connectorInstance;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string monitorFriendlyDeviceName;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string monitorDevicePath;
        }

        [DllImport("user32.dll")]
        public static extern int GetDisplayConfigBufferSizes(QUERY_DEVICE_CONFIG_FLAGS Flags, out uint NumPathArrayElements, out uint NumModeInfoArrayElements);

        [DllImport("user32.dll")]
        public static extern int QueryDisplayConfig(QUERY_DEVICE_CONFIG_FLAGS Flags, ref uint NumPathArrayElements, [Out] DISPLAYCONFIG_PATH_INFO[] PathInfoArray,
            ref uint NumModeInfoArrayElements, [Out] DISPLAYCONFIG_MODE_INFO[] ModeInfoArray, IntPtr CurrentTopologyId);

        [DllImport("user32.dll")]
        public static extern int DisplayConfigGetDeviceInfo(ref DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName);

        public static List<DisplayConfig> GetAllDisplayConfig()
        {
            List<DisplayConfig> configs = new List<DisplayConfig>();
            uint PathCount, ModeCount;
            int error = GetDisplayConfigBufferSizes(QUERY_DEVICE_CONFIG_FLAGS.QDC_ONLY_ACTIVE_PATHS, out PathCount, out ModeCount);
            if (error != ERROR_SUCCESS)
            {
                ExceptionHandler("WpfEx.Helpers.ScreenEx.GetAllDisplayConfig", new Exception("GetAllDisplayConfig->GetDisplayConfigBufferSizes=" + error));
                return configs;
            }

            DISPLAYCONFIG_PATH_INFO[] DisplayPaths = new DISPLAYCONFIG_PATH_INFO[PathCount];
            DISPLAYCONFIG_MODE_INFO[] DisplayModes = new DISPLAYCONFIG_MODE_INFO[ModeCount];
            error = QueryDisplayConfig(QUERY_DEVICE_CONFIG_FLAGS.QDC_ONLY_ACTIVE_PATHS, ref PathCount, DisplayPaths, ref ModeCount, DisplayModes, IntPtr.Zero);
            if (error != ERROR_SUCCESS)
            {
                ExceptionHandler("WpfEx.Helpers.ScreenEx.GetAllDisplayConfig", new Exception("GetAllDisplayConfig->QueryDisplayConfig=" + error + " PathCount:" + PathCount + " ModeCount:" + ModeCount));
                return configs;
            }
            for (int i = 0; i < ModeCount; i++)
            {
                if (DisplayModes[i].infoType == DISPLAYCONFIG_MODE_INFO_TYPE.DISPLAYCONFIG_MODE_INFO_TYPE_TARGET)
                {
                    LUID adapterId = DisplayModes[i].adapterId;
                    uint targetId = DisplayModes[i].id;

                    DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName = new DISPLAYCONFIG_TARGET_DEVICE_NAME();
                    deviceName.header.size = (uint)Marshal.SizeOf(typeof(DISPLAYCONFIG_TARGET_DEVICE_NAME));
                    deviceName.header.adapterId = adapterId;
                    deviceName.header.id = targetId;
                    deviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_TYPE.DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
                    error = DisplayConfigGetDeviceInfo(ref deviceName);
                    if (error != ERROR_SUCCESS)
                        ExceptionHandler("WpfEx.Helpers.ScreenEx.GetAllDisplayConfig", new Exception("GetAllDisplayConfig->DisplayConfigGetDeviceInfo=" + error + " targetId:" + targetId));
                    else
                    {
                        DisplayConfig config = new DisplayConfig();
                        config.m_DeviceName = deviceName;
                        config.m_ModeInfo = DisplayModes[i];
                        configs.Add(config);
                    }
                }
            }
            return configs;
        }
        #endregion
    }
}
