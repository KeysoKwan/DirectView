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
    private static extern void ShowInExe(System.IntPtr hWnd, System.IntPtr textureHandle, int w, int h);

    //寻找当前目标窗口的进程
    [DllImport("user32.dll")]
    private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);
    //根据窗口句柄获取pid
    [DllImport("User32.dll")]
    private static extern int GetWindowThreadProcessId(IntPtr hwnd, out int ID);


    private WebCamTexture _camTex;

    public RawImage riC920;

    public RenderTexture rt;

    private IntPtr rtPtr = IntPtr.Zero;
    IntPtr VoxenShareWinInptr = IntPtr.Zero;

    /// <summary>
    /// 一个罗技摄像头姿态的记录
    /// </summary>
    FViewRT fViewRT;

    private void Awake()
    {
        //程序一运行就绑定一下
        JsonTypeBind.Bind();

        string fViewJsonPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "FViewTool", "FView.json");
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
        if (curRtPtr != rtPtr)
        {
            rtPtr = curRtPtr;
            UnityEngine.Debug.Log("FView.Update():当前纹理指针赋值!" + rtPtr.ToInt64());
            //调用c++dll
            ShowInExe(VoxenShareWinInptr, rtPtr, 1920, 1080);
        }
    }

    Process viewProcess;

    private void OpenFViewWindows()
    {
        if (viewProcess != null)
        {
            return;
        }
        string _path = Path.Combine(Application.streamingAssetsPath, "ClientWin.exe");
        ProcessStartInfo startInfo = new ProcessStartInfo();
        startInfo.FileName = _path;
        startInfo.Arguments = "OpenInUnity";
        viewProcess = new Process();
        viewProcess.StartInfo = startInfo;
        viewProcess.Start();


        VoxenShareWinInptr = IntPtr.Zero;
        while (true)
        {
            VoxenShareWinInptr = FindWindow(null, "ViewClient");
            if (VoxenShareWinInptr != IntPtr.Zero)
            {
                UnityEngine.Debug.Log("FView.OpenFViewWindows():找到了窗口句柄！");
                int pid = 0;
                GetWindowThreadProcessId(VoxenShareWinInptr, out pid);
                if (pid == viewProcess.Id)
                {
                    break;
                }
            }
        }

        UnityEngine.Debug.Log("FView.OpenFViewWindows():开始绘图！");

    }


    private void OnGUI()
    {
        if (GUILayout.Button("open"))
        {
            OpenFViewWindows();
        }
    }

    private void OnApplicationQuit()
    {
        if (viewProcess != null)
        {
            viewProcess.Kill();
        }

    }

    IEnumerator InitCamera()
    {

        //获取授权
        yield return Application.RequestUserAuthorization(UserAuthorization.WebCam);
        if (Application.HasUserAuthorization(UserAuthorization.WebCam))
        {
            var devices = WebCamTexture.devices;
            //UnityEngine.Debug.Log("devices.Length " + devices.Length);
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

                //var _deviceName = "HD Pro Webcam C920";//devices[0].name;
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