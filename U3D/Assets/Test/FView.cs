﻿using DTO;
using FSpace;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.UI;
using xuexue.common.json;
using xuexue.LitJson;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Threading.Tasks;

public class FView : MonoBehaviour
{

    [DllImport("FView")]
    private static extern void StartView(System.IntPtr hWnd, System.IntPtr textureHandle, int w, int h);

    //寻找当前目标窗口的进程
    [DllImport("user32.dll")]
    private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

    //根据窗口句柄获取pid
    [DllImport("User32.dll")]
    private static extern int GetWindowThreadProcessId(IntPtr hwnd, out int ID);

    /// <summary>
    /// 相机采图
    /// </summary>
    private WebCamTexture _camTex;

    /// <summary>
    /// 相机背景图
    /// </summary>
    public RawImage riC920;

    /// <summary>
    /// 渲染结果
    /// </summary>
    public RenderTexture rt;

    /// <summary>
    /// 渲染纹理指针
    /// </summary>
    private IntPtr rtPtr = IntPtr.Zero;

    /// <summary>
    /// 显示窗口句柄
    /// </summary>
    IntPtr _hViewClient = IntPtr.Zero;

    /// <summary>
    /// 一个罗技摄像头姿态的记录
    /// </summary>
    FViewRT fViewRT;

    /// <summary>
    /// 显示窗口进程
    /// </summary>
    Process viewProcess;

    private void Awake()
    {
        //程序一运行就绑定一下
        JsonTypeBind.Bind();
        string fViewJsonPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "FViewTool", "FView.json");

        if (!File.Exists(fViewJsonPath))
        {
            UnityEngine.Debug.LogError("FView.OpenFViewWindows():先使用工具软件进行罗技摄像头的标定！");
        }

        //读取json的配置
        string str = File.ReadAllText(fViewJsonPath);
        fViewRT = JsonMapper.ToObject<FViewRT>(str);

        //使用标定结果设置CamRoot的坐标
        transform.localPosition = fViewRT.viewPosition;
        transform.localRotation = fViewRT.viewRotation;

    }

    // Use this for initialization
    async void Start()
    {
        StartCoroutine(InitCamera());
        await Task.Delay(3000);

        OpenFViewWindows();
    }

    // Update is called once per frame
    void Update()
    {
        if (Input.GetKeyDown(KeyCode.V))
        {
            OpenFViewWindows();
        }

        IntPtr curRtPtr = rt.GetNativeTexturePtr();
        if (curRtPtr != rtPtr && _hViewClient != IntPtr.Zero)
        {
            rtPtr = curRtPtr;
            StartView(_hViewClient, rtPtr, 1920, 1080);
        }
    }

    private void OpenFViewWindows()
    {
        if (viewProcess != null)
        {
            return;
        }
        string _path = Path.Combine(Application.streamingAssetsPath, "ClientWin.exe");
        ProcessStartInfo startInfo = new ProcessStartInfo();
        startInfo.FileName = _path;
        viewProcess = new Process();
        viewProcess.StartInfo = startInfo;
        viewProcess.Start();


        _hViewClient = IntPtr.Zero;
        while (true)
        {
            _hViewClient = FindWindow(null, "ViewClient");
            if (_hViewClient != IntPtr.Zero)
            {
                UnityEngine.Debug.Log("FView.OpenFViewWindows():找到了窗口句柄！");
                int pid = 0;
                GetWindowThreadProcessId(_hViewClient, out pid);
                if (pid == viewProcess.Id)
                {
                    break;
                }
            }
        }

        UnityEngine.Debug.Log("FView.OpenFViewWindows():开始绘图！");

    }

    private void OnApplicationQuit()
    {
        if (viewProcess != null)
        {
            viewProcess.Kill();
        }

    }

    /// <summary>
    /// 打开罗技HD Pro Webcam C920的相机
    /// </summary>
    /// <returns></returns>
    IEnumerator InitCamera()
    {

        //获取授权
        yield return Application.RequestUserAuthorization(UserAuthorization.WebCam);
        if (Application.HasUserAuthorization(UserAuthorization.WebCam))
        {
            var devices = WebCamTexture.devices;
            if (devices.Length > 4)
            {
                string _deviceName = "";
                foreach (var item in devices)
                {
                    if (item.name.EndsWith("C920"))
                    {
                        _deviceName = item.name;
                        break;
                    }
                }
                if (string.IsNullOrEmpty(_deviceName))
                {
                    UnityEngine.Debug.LogError("ScreenControlObj.InitCamera():相机启动失败，没有外接相机");
                }
                else
                {
                    if (_camTex == null)
                    {
                        _camTex = new WebCamTexture(_deviceName, 1280, 720, 15);//设置为1280x720可以减少相机延迟
                        riC920.texture = _camTex;
                        _camTex.Play();
                    }
                    else
                    {
                        riC920.texture = _camTex;
                        _camTex.Play();
                    }
                    //_tex.anisoLevel = 2;
                    UnityEngine.Debug.Log("ScreenControlObj.InitCamera():相机启动");
                }
            }
        }
    }
}