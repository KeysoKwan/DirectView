﻿using System;
using System.Runtime.InteropServices;
namespace FSpace
{
    public class FARSingleton//为了可切换场景，封装一个全局类
    {
        [DllImport("f-ar")]
        private static extern int StartView(System.IntPtr hWnd, System.IntPtr textureHandle, int w, int h);
        [DllImport("f-ar")]
        private static extern int StartView_LR(System.IntPtr hWnd, System.IntPtr LeftTextureHandle, System.IntPtr ReftTextureHandle, int w, int h);
        [DllImport("f-ar")]
        private static extern void SwitchProjector(int type);
        [DllImport("f-ar")]
        private static extern void IsGamaSpace(int cSpace);
        [DllImport("f-ar")]
        private static extern void StopView();

        public static FARSingleton GetInstance()
        {
            if (m_fviewInstance == null) m_fviewInstance = new FARSingleton();
            return m_fviewInstance;
        }
        public enum U3DColorSpace
        {
            Gama = 0,
            Linear = 1
        }
        public enum ProjectorType
        {
            _2D = 0,//2D投影
            LeftRight = 1//左右投影
        }
        /// <summary>
        /// 输入 
        /// hWnd                 创建好的窗口句柄
        /// FullNativePTR    单个全屏的纹理指针
        /// 返回错误代码
        /// errorCode >0 成功
        /// errorCode ==-2 窗口句柄丢失
        /// errorCode ==-3 纹理句柄丢失
        /// </summary>
        public int StartView(IntPtr hWnd, IntPtr FullNativePTR)
        {
            errorCode = StartView(hWnd, FullNativePTR, SwapchainWidth, SwapchanHeight);
            return errorCode;
        }

        /// <summary>
        /// 输入 
        /// hWnd                  创建好的窗口句柄
        /// leftNativePTR      半屏的左眼纹理指针
        ///  rightNativePTR    半屏的右眼纹理指针
        /// 返回错误代码
        /// errorCode >0 成功
        /// errorCode ==-2 窗口句柄丢失
        /// errorCode ==-3 纹理句柄丢失
        /// </summary>
        public int StartView_LR(IntPtr hWnd, IntPtr leftNativePTR, IntPtr rightNativePTR)
        {
            errorCode = StartView_LR(hWnd, leftNativePTR, rightNativePTR, SwapchainWidth, SwapchanHeight);
            return errorCode;
        }

        public void SetColorSpace(U3DColorSpace cSpace)
        {
#if UNITY_EDITOR
            if (UnityEditor.PlayerSettings.colorSpace == UnityEngine.ColorSpace.Linear)
            {
                IsGamaSpace((int)U3DColorSpace.Linear);
            }
            else
                IsGamaSpace((int)U3DColorSpace.Gama);
#else
            IsGamaSpace((int)cSpace);
#endif
        }

        /// <summary>
        ///切换投影方式
        ///调用StartView_LR(...)后，可切换到只显示左画面到投屏窗口或左右一起显示 
        /// -----------         ----------
        /// |   L  |   R  |  or   |     L     | 
        /// -----------         ---------- 
        ///如果只调用StartView(...)，则此函数无效 
        /// </summary>
        public void SwitchProjector(ProjectorType type)
        {
            SwitchProjector((int)type);
        }

        /// <summary>
        ///安全关闭窗口
        /// </summary>
        public void CloseDown()
        {
            StopView();
        }

        public const int SwapchainWidth = 1920;
        public const int SwapchanHeight = 1080;
        private int errorCode = 0;
        private FARSingleton() { }
        ~FARSingleton() { StopView(); }
        private static FARSingleton m_fviewInstance;
    }
}