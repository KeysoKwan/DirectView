using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.UI;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Threading.Tasks;

namespace GCSeries
{
    public class Direct3DWin : MonoBehaviour
    {
        /// <summary>
        /// 投屏到一张纹理或两张纹理
        /// </summary>
        public enum WorkMode
        {
            _SingleTexture,
            _DoubleTexture
        }

        /// <summary>
        /// 纹理目标
        /// </summary>
        public RenderTexture renderTexture;
        public RenderTexture renderTextureL;
        public RenderTexture renderTextureR;

        /// <summary>
        /// 当前工作模式
        /// </summary>
        public WorkMode workingMode = WorkMode._SingleTexture;
        private WorkMode _lastWorkingType;

        /// <summary>
        /// 投屏目标显示器名
        /// 如果为空字符串则寻找第一个非GC显示器
        /// </summary>
        public string targetMonitorName;

        private int _farframerate = 60;

        private List<ScreenHelper.DisplayDevice> m_monitors;
        /// <summary>
        /// 投屏窗口目标帧率
        /// </summary>
        public int FARframerate
        {
            get { return _farframerate; }
            set
            {
                _farframerate = value;
                _farframerate = Mathf.Clamp(_farframerate, 30, 150);
                FARDll.fmARSetFramerate(_farframerate);
            }
        }
        /// <summary>
        /// 显示窗口句柄
        /// </summary>
        IntPtr _hViewClient = IntPtr.Zero;

        /// <summary>
        /// 投屏状态值
        /// </summary>
        public enum FAR_Status
        {
            /// <summary>
            /// 成功
            /// </summary>
            FAR_Ok = 1,
            /// <summary>
            /// 没被初始化或正在初始化中
            /// </summary>
            FAR_NotInitialized = 0,
            /// <summary>
            /// 非法硬件设备
            /// </summary>
            FAR_Illegal = -1,
            /// <summary>
            /// 窗口句柄丢失
            /// </summary>
            FAR_InvaliHwndHandle = -2,
            /// <summary>
            /// 渲染设备初始化失败
            /// </summary>
            FAR_D3DFailed = -3,
            /// <summary>
            /// 纹理句柄丢失
            /// </summary>
            FAR_InvaliTexturedHandle = -4,
            /// <summary>
            /// 渲染等待进程超时
            /// </summary>
            FAR_Timeout = -5,
            /// <summary>
            /// windows版本低于win10
            /// </summary>
            FAR_SysNnotSupported = -6,
            /// <summary>
            /// windows版本低于win10
            /// </summary>
            FAR_ProcessConflic = -7
        }

        /// <summary>
        /// 投屏状态值
        /// </summary>
        FAR_Status status = FAR_Status.FAR_NotInitialized;

        private void Awake()
        {
            _lastWorkingType = workingMode;
        }

        void Start()
        {
            //启动投屏窗口
            FARStartRenderingView(workingMode);
        }

        private void Update()
        {
            if (Input.GetKeyDown(KeyCode.V))
            {
                //启动投屏窗口
                // FARStartRenderingView(WorkMode._DoubleTexture);
                FARPreview();
            }
            if (Input.GetKeyDown(KeyCode.Q))
            {
                //关闭投屏窗口
                FARDll.CloseDown();
            }
            if (_lastWorkingType != workingMode)
            {
                //切换到2D画面
                FARStartRenderingView(workingMode);
                _lastWorkingType = workingMode;
            }
        }

        ///<summary>
        ///通过f-ar接口读取屏幕信息后设置窗口位置
        ///</summary>
        ///<param name="farwin">投屏窗口句柄</param>
        ///<param name="preview">是否为3D预览窗口</param>
        public void UpdateWindowPos(IntPtr farwin, bool preview)
        {
            //可先切换到扩展模式
            //SetDisplayConfig(0, IntPtr.Zero, 0, IntPtr.Zero, (SDC_APPLY | SDC_TOPOLOGY_EXTEND));
            //更新物理显示器列表
            int temp_MonitorCount = m_monitors.Count;
            if (temp_MonitorCount < 0)
                UnityEngine.Debug.LogError("Direct3DWin.UpdateWindowsPos() failed with error : fmARGetMonitorCount " + temp_MonitorCount);

            List<FARDll.GCinfo> tempList = new List<FARDll.GCinfo>();
            bool temp_ContainGcMonitor = false;
            for (int k = 0; k < temp_MonitorCount; k++)
            {
                FARDll.GCinfo gcinfo = new FARDll.GCinfo();
                gcinfo.DeviceName = m_monitors[k].m_Monitors[0].m_FriendlyDeviceName;
                gcinfo.isGCmonitor = gcinfo.DeviceName.Contains("HAIER") ? true : false;
                gcinfo.RCleft = m_monitors[k].m_Monitors[0].m_MonitorInfo.rcMonitor.Left;
                gcinfo.RCright = m_monitors[k].m_Monitors[0].m_MonitorInfo.rcMonitor.Right;
                gcinfo.RCtop = m_monitors[k].m_Monitors[0].m_MonitorInfo.rcMonitor.Top;
                gcinfo.RCbottom = m_monitors[k].m_Monitors[0].m_MonitorInfo.rcMonitor.Bottom;
                tempList.Add(gcinfo);
                temp_ContainGcMonitor |= gcinfo.isGCmonitor;
            }
            FARDll.Rect r;
            FARDll.GetWindowRect(FARDll.GetProcessWnd(), out r);
            Vector2 temp_U3Dcenter = new Vector2((r.Right - r.Left) / 2 + r.Left, (r.Bottom - r.Top) / 2 + r.Top);
            for (int k = 0; k < tempList.Count; k++)
            {
                //如果是3D预览模式
                if (preview)
                {
                    //if (tempList[k].isGCmonitor)
                    //{
                    //    int offset = 100;
                    //    FARDll.MoveWindow(farwin, tempList[k].RCleft + offset, tempList[k].RCtop + offset, tempList[k].RCright - tempList[k].RCleft - offset*2, tempList[k].RCbottom - tempList[k].RCtop - offset * 2, true);
                    //    FARDll.fmAROnWindowResized(true, tempList[k].RCright - tempList[k].RCleft - offset * 2, tempList[k].RCbottom - tempList[k].RCtop - offset * 2);
                    //    return;
                    //}
                    int[] offset = new int[2] { 300, 100 };
                    FARDll.MoveWindow(farwin, tempList[0].RCleft + offset[0], tempList[0].RCtop + offset[1], tempList[0].RCright - tempList[0].RCleft - offset[0] * 2, tempList[0].RCbottom - tempList[0].RCtop - offset[1] * 2, true);
                    return;
                }
                //LogGcInfo(tempList[k]);
                else if (targetMonitorName != string.Empty)
                {
                    if (tempList[k].DeviceName.Contains(targetMonitorName))
                    {
                        UnityEngine.Debug.Log("Direct3DWin.UpdateWindows():投屏显示器：" + tempList[k].DeviceName);
                        FARDll.MoveWindow(farwin, tempList[k].RCleft, tempList[k].RCtop, tempList[k].RCright - tempList[k].RCleft, tempList[k].RCbottom - tempList[k].RCtop, true);
                        return;
                    }
                }
                else
                {
                    //显示器列表包含GC显示器
                    if (temp_ContainGcMonitor)
                    {
                        //获取一个非GC显示器
                        if (!tempList[k].isGCmonitor)
                        {
                            UnityEngine.Debug.Log("Direct3DWin.UpdateWindows():投屏显示器：" + tempList[k].DeviceName);
                            FARDll.MoveWindow(farwin, tempList[k].RCleft, tempList[k].RCtop, tempList[k].RCright - tempList[k].RCleft, tempList[k].RCbottom - tempList[k].RCtop, true);
                            return;
                        }
                    }
                    else//列表中没有GC显示器，通过主程序当前窗口位置设置投屏窗口
                    {
                        Vector2 temp_ScreenCenter = new Vector2((tempList[k].RCright - tempList[k].RCleft) / 2 + tempList[k].RCleft,
                                                                (tempList[k].RCbottom - tempList[k].RCtop) / 2 + tempList[k].RCtop);
                        if (Vector2.Distance(temp_U3Dcenter, temp_ScreenCenter) < Vector2.Distance(temp_ScreenCenter, new Vector2(tempList[k].RCright, tempList[k].RCtop)))
                        {
                            int tempk = k + 1;
                            if (tempk >= temp_MonitorCount)
                                tempk = 0;
                            FARDll.MoveWindow(farwin, tempList[tempk].RCleft, tempList[tempk].RCtop, tempList[tempk].RCright - tempList[tempk].RCleft, tempList[tempk].RCbottom - tempList[tempk].RCtop, true);
                            return;
                        }

                    }
                }
            }
            UnityEngine.Debug.LogError("Direct3DWin.UpdateWindows() failed with error : 没有寻找到对应的显示器");
        }

        ///启动投屏窗口进程
        ///<param name="preview">是否为3D预览窗口</param>
        private IEnumerator CreateFARWindow(WorkMode _workmode, bool preview)
        {
            m_monitors = new List<ScreenHelper.DisplayDevice>();
            m_monitors = ScreenHelper.GetAllDisplayDevice();

            //为了与主渲染进程不产生冲突，等待下一帧结束
            yield return new WaitForEndOfFrame();
            _hViewClient = FARDll.FindWindow(null, "ClientWinCpp");
            status = FAR_Status.FAR_NotInitialized;
            if (_hViewClient == IntPtr.Zero)
            {
                string _path = Path.Combine(Application.streamingAssetsPath, "ClientWin.exe");
                ProcessStartInfo startInfo = new ProcessStartInfo();
                startInfo.FileName = _path;
                if (preview) { startInfo.Arguments = "-withedge"; }
                FARDll.viewProcess = new Process();
                FARDll.viewProcess.StartInfo = startInfo;
                FARDll.viewProcess.Start();
                FARDll.viewProcess.WaitForInputIdle();
                _hViewClient = FARDll.FindWindow(null, "ClientWinCpp");
            }
            if (FARDll.viewProcess != null)
            {
                if (_hViewClient != IntPtr.Zero)
                {
                    UnityEngine.Debug.Log("FAR.CreateFARWindow():找到了窗口句柄！");
                    //全屏到非GC显示器
                    UpdateWindowPos(_hViewClient, preview);
                    //设置当前的色彩空间，u3d默认是Gama空间
                    FARDll.SetColorSpace(FARDll.U3DColorSpace.Gama);
                    //开始绘制同屏窗口，如目标纹理指针变更可随时调用
                    switch (_workmode)
                    {
                        case WorkMode._SingleTexture:
                            status = (FAR_Status)FARDll.StartView(_hViewClient, renderTexture.GetNativeTexturePtr(), IntPtr.Zero);
                            break;
                        case WorkMode._DoubleTexture:
                            status = (FAR_Status)FARDll.StartView(_hViewClient, renderTextureL.GetNativeTexturePtr(), renderTextureR.GetNativeTexturePtr());
                            break;
                        default:
                            status = (FAR_Status)FARDll.StartView(_hViewClient, renderTexture.GetNativeTexturePtr(), IntPtr.Zero);
                            break;
                    }
                    if (FARDll.fmARGetStereopicSupport())
                    {
                        //支持立体显示，自动切换到立体显示模式
                        FARDll.SwitchProjector(FARDll.ProjectorType.Stereopic);
                    }
                    //#if UNITY_EDITOR
                    if (preview)
                    {
                        FARDll.SetForegroundWindow(_hViewClient);
                    }
                }
            }
            else
                status = FAR_Status.FAR_ProcessConflic;

            if (status <= 0)
            {
                UnityEngine.Debug.LogError("FAR.CreateFARWindow():投屏启动失败" + status);
                FARDll.CloseDown();
            }
            else
                UnityEngine.Debug.Log("FAR.CreateFARWindow():开始绘图！");
        }

        private void OnApplicationQuit()
        {
            //安全关闭投屏窗口，可根据需求调用
            FARDll.CloseDown();
        }

        /// <summary>
        /// 启动FAR投屏窗口
        /// </summary>
        /// <param name="workmMode">投屏单张纹理或两张纹理</param>
        public void FARStartRenderingView(WorkMode workmMode)
        {
            //FARDll.CloseDown();
            StartCoroutine(CreateFARWindow(workmMode, false));
        }

        /// <summary>
        /// 启动FAR 3D预览窗口
        /// </summary>
        /// <param name="workmMode">投屏单张纹理或两张纹理</param>
        public void FARPreview()
        {
            StartCoroutine(CreateFARWindow(WorkMode._DoubleTexture, true));
        }

        /// <summary>
        /// 打印一个显示器信息
        /// </summary>
        /// <param name="gCinfo"></param>
        public void LogGcInfo(FARDll.GCinfo gCinfo)
        {
            string loginfo = string.Format("DeviceName = {0}  isGcMonitor = {1}\n  RCleft = {2:D}  RCright = {3:D}  RCtop = {4:D}  RCbottom = {5:D}",
                gCinfo.DeviceName, gCinfo.isGCmonitor, gCinfo.RCleft, gCinfo.RCright, gCinfo.RCtop, gCinfo.RCbottom);
            UnityEngine.Debug.Log(loginfo);

        }
    }
}