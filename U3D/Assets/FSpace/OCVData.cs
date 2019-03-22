using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

namespace FSpace
{
    #region DetectPoint

    ///-------------------------------------------------------------------------------------------------
    /// <summary> 一个检测到的小跟踪点. </summary>
    ///
    /// <remarks> Xian Dai, 2017/5/6. </remarks>
    ///-------------------------------------------------------------------------------------------------
    public struct DetectPoint
    {
        /// <summary> 结果点的坐标. </summary>
        public Vector3 targetPoint;

        /// <summary> 这个点的可信度，规定如果小于等于0那么就不算检测到. </summary>
        public int credibility;

        /// <summary> 距离误差. </summary>
        public float d;

        /// <summary> 四个摄像机看这个点的射线方向. </summary>
        public Vector3 direction1;

        public Vector3 direction2;
        public Vector3 direction3;
        public Vector3 direction4;
    };

    public struct DetectPointx4
    {
        private DetectPoint dp1;
        private DetectPoint dp2;
        private DetectPoint dp3;
        private DetectPoint dp4;

        public DetectPoint this[int index]
        {
            get
            {
                if (index == 0)
                    return dp1;
                else if (index == 1)
                    return dp2;
                else if (index == 2)
                    return dp3;
                else if (index == 3)
                    return dp4;
                return new DetectPoint();
            }
            set
            {
                if (index == 0)
                    dp1 = value;
                else if (index == 1)
                    dp2 = value;
                else if (index == 2)
                    dp3 = value;
                else if (index == 3)
                    dp4 = value;
            }
        }
    }

    public struct DetectPointx16
    {
        private DetectPointx4 dp4_1;
        private DetectPointx4 dp4_2;
        private DetectPointx4 dp4_3;
        private DetectPointx4 dp4_4;

        public DetectPoint this[int index]
        {
            get
            {
                int i = index / 4;
                int j = index % 4;

                if (i == 0)
                    return dp4_1[j];
                else if (i == 1)
                    return dp4_2[j];
                else if (i == 2)
                    return dp4_3[j];
                else if (i == 3)
                    return dp4_4[j];
                return new DetectPoint();
            }
            set
            {
                int i = index / 4;
                int j = index % 4;

                if (i == 0)
                    dp4_1[j] = value;
                else if (i == 1)
                    dp4_2[j] = value;
                else if (i == 2)
                    dp4_3[j] = value;
                else if (i == 3)
                    dp4_4[j] = value;
            }
        }
    }

    public struct DetectPointx64
    {
        private DetectPointx16 dp16_1;
        private DetectPointx16 dp16_2;
        private DetectPointx16 dp16_3;
        private DetectPointx16 dp16_4;

        public DetectPoint this[int index]
        {
            get
            {
                int i = index / 16;
                int j = index % 16;

                if (i == 0)
                    return dp16_1[j];
                else if (i == 1)
                    return dp16_2[j];
                else if (i == 2)
                    return dp16_3[j];
                else if (i == 3)
                    return dp16_4[j];
                return new DetectPoint();
            }
            set
            {
                int i = index / 16;
                int j = index % 16;

                if (i == 0)
                    dp16_1[j] = value;
                else if (i == 1)
                    dp16_2[j] = value;
                else if (i == 2)
                    dp16_3[j] = value;
                else if (i == 3)
                    dp16_4[j] = value;
            }
        }
    }

    #endregion DetectPoint

    #region CamRay

    public struct CamRay
    {
        /// <summary> 这个点的可信度，如果为0则忽略 </summary>
        public int credibility;

        /// <summary> 相机的编号. </summary>
        public int camIndex;

        /// <summary> 射线的原点坐标. </summary>
        public Vector3 origin;

        /// <summary> 射线的方向. </summary>
        public Vector3 direction;
    };

    public struct CamRayx4
    {
        private CamRay cr1;
        private CamRay cr2;
        private CamRay cr3;
        private CamRay cr4;

        public CamRay this[int index]
        {
            get
            {
                if (index == 0)
                    return cr1;
                else if (index == 1)
                    return cr2;
                else if (index == 2)
                    return cr3;
                else if (index == 3)
                    return cr4;
                return new CamRay();
            }
            set
            {
                if (index == 0)
                    cr1 = value;
                else if (index == 1)
                    cr2 = value;
                else if (index == 2)
                    cr3 = value;
                else if (index == 3)
                    cr4 = value;
            }
        }
    }

    public struct CamRayx16
    {
        private CamRayx4 cr4_1;
        private CamRayx4 cr4_2;
        private CamRayx4 cr4_3;
        private CamRayx4 cr4_4;

        public CamRay this[int index]
        {
            get
            {
                int i = index / 4;
                int j = index % 4;

                if (i == 0)
                    return cr4_1[j];
                else if (i == 1)
                    return cr4_2[j];
                else if (i == 2)
                    return cr4_3[j];
                else if (i == 3)
                    return cr4_4[j];
                return new CamRay();
            }
            set
            {
                int i = index / 4;
                int j = index % 4;

                if (i == 0)
                    cr4_1[j] = value;
                else if (i == 1)
                    cr4_2[j] = value;
                else if (i == 2)
                    cr4_3[j] = value;
                else if (i == 3)
                    cr4_4[j] = value;
            }
        }
    }

    public struct CamRayx64
    {
        private CamRayx16 cr16_1;
        private CamRayx16 cr16_2;
        private CamRayx16 cr16_3;
        private CamRayx16 cr16_4;

        public CamRay this[int index]
        {
            get
            {
                int i = index / 16;
                int j = index % 16;

                if (i == 0)
                    return cr16_1[j];
                else if (i == 1)
                    return cr16_2[j];
                else if (i == 2)
                    return cr16_3[j];
                else if (i == 3)
                    return cr16_4[j];
                return new CamRay();
            }
            set
            {
                int i = index / 16;
                int j = index % 16;

                if (i == 0)
                    cr16_1[j] = value;
                else if (i == 1)
                    cr16_2[j] = value;
                else if (i == 2)
                    cr16_3[j] = value;
                else if (i == 3)
                    cr16_4[j] = value;
            }
        }
    }

    #endregion CamRay

    ///-------------------------------------------------------------------------------------------------
    /// <summary> 监测出来的摄像机状态. </summary>
    ///
    /// <remarks> Xian Dai, 2017/5/6. </remarks>
    ///-------------------------------------------------------------------------------------------------
    public struct CameraStatus
    {
        /// <summary>这个相机的序号,目前应该是0-3，一共4个. </summary>
        public int index;

        /// <summary> 检测状态，1表示成功定位自身，0表示丢失自身，2表示使用手动输入值. </summary>
        public int status;

        /// <summary> 相机的像素宽度. </summary>
        public int width;

        /// <summary> 相机的像素高度. </summary>
        public int height;

        /// <summary> 自身坐标. </summary>
        public Vector3 Position;

        /// <summary> 自身旋转. </summary>
        public Quaternion rotation;

        /// <summary> 自身旋转(使用u3d的zxy旋转的欧拉角表示). </summary>
        public Vector3 rotationZXY;

        /// <summary> 相机的视场方向向量,4个边角点作出. </summary>
        public Vector3 ViewFieldDirection1;

        public Vector3 ViewFieldDirection2;
        public Vector3 ViewFieldDirection3;
        public Vector3 ViewFieldDirection4;

        /// <summary>
        /// 根据这个结构体内容来设置摄像机的显示状态，坐标/旋转/是否显示
        /// </summary>
        /// <param name="cam">要被设置状态的摄像机</param>
        public void SetCamera(Transform cam)
        {
            if (status == 1 || status == 3)
            {
                cam.gameObject.SetActive(true);
                cam.position = Position;
                cam.rotation = rotation;
            }
            else if (status == 2)
            {
                cam.gameObject.SetActive(true);
                cam.position = Position;
                cam.rotation = Quaternion.Euler(rotationZXY);
            }
            else
            {
                cam.gameObject.SetActive(false);
            }
        }
    }

    ///-------------------------------------------------------------------------------------------------
    /// <summary> A camera statusx 4. </summary>
    ///
    /// <remarks> Dx, 2017/7/18. </remarks>
    ///-------------------------------------------------------------------------------------------------
    public struct CameraStatusx4
    {
        public CameraStatus cam_1;
        public CameraStatus cam_2;
        public CameraStatus cam_3;
        public CameraStatus cam_4;

        public CameraStatus this[int index]
        {
            get
            {
                if (index == 0)
                    return cam_1;
                else if (index == 1)
                    return cam_2;
                else if (index == 2)
                    return cam_3;
                else if (index == 3)
                    return cam_4;

                throw new Exception("CameraStatusx4的下标index错误！");
                //return new CameraStatus();
            }
            set
            {
                if (index == 0)
                    cam_1 = value;
                else if (index == 1)
                    cam_2 = value;
                else if (index == 2)
                    cam_3 = value;
                else if (index == 3)
                    cam_4 = value;
            }
        }
    }

    ///-------------------------------------------------------------------------------------------------
    /// <summary> U3D相机的数据设置. </summary>
    ///
    /// <remarks> Dx, 2017/11/1. </remarks>
    ///-------------------------------------------------------------------------------------------------
    public struct U3DCamera
    {
        /// <summary> 相机的世界坐标. </summary>
        public Vector3 Position;

        /// <summary> 相机的世界旋转. </summary>
        public Quaternion rotation;

        /// <summary> FOV. </summary>
        public float fov;
    };

    public struct ocvdata
    {
        /// <summary> ocv数据版本. </summary>
        public int ocvdataVersion;

        /// <summary> 4个相机数据. </summary>
        public CameraStatusx4 cam_s;

        /// <summary> 眼镜的检测状态. </summary>
        public int GlassStatus;

        /// <summary> 眼镜坐标. </summary>
        public Vector3 GlassPosition;

        /// <summary> 眼镜坐标(考虑机身倾角). </summary>
        public Vector3 GlassPosition_w;

        /// <summary> 眼镜旋转. </summary>
        public Quaternion GlassRotation;

        /// <summary> 眼镜旋转(考虑机身倾角). </summary>
        public Quaternion GlassRotation_w;

        /// <summary>笔的检测状态. </summary>
        public int PenStatus;

        /// <summary> 笔坐标. </summary>
        public Vector3 PenPosition;

        /// <summary> 笔坐标(考虑机身倾角). </summary>
        public Vector3 PenPosition_w;

        /// <summary> 笔方向. </summary>
        public Vector3 PenDirection;

        /// <summary> 笔方向(考虑机身倾角). </summary>
        public Vector3 PenDirection_w;

        /// <summary> 笔的滚转角. </summary>
        public float PenRoll;

        /// <summary> 整个设备的偏转角度(未使用). </summary>
        public float DevicePitch;

        /// <summary> 笔的按键值,0x01中键,0x02左键,0x04右键. </summary>
        public int PenKey;

        /// <summary> 陀螺仪旋转,只有x，y两轴. </summary>
        public Quaternion GyroscopeRotation;

        /// <summary> 整个机子倾斜角，默认为0. </summary>
        public float slantAngle;

        /// <summary>是否在睡眠状态,0为正常检查,>0为睡眠状态.</summary>
        public int isSleep;

        /// <summary> 当前处理程序的fps. </summary>
        public float fps;

        /// <summary> 相机硬件错误标志 </summary>
        public int cameraDevErrorCode;

        /// <summary> 笔硬件错误标志. </summary>
        public int penDevErrorCode;

        /// <summary> 主控硬件错误标志. </summary>
        public int mcDevErrorCode;

        /// <summary>  处理程序自己软件的错误标志. </summary>
        public int procErrorCode;

        /// <summary> 设备的主板id号. </summary>
        public int devID;

        /// <summary> 16:9摄像机的中央摄像机. </summary>
        public U3DCamera camera_c;

        /// <summary> 16:9摄像机的左边摄像机. </summary>
        public U3DCamera camera_l;

        /// <summary> 16:9摄像机的右边摄像机. </summary>
        public U3DCamera camera_r;

        /// <summary> 比较可靠的点 64个. </summary>
        public DetectPointx64 reliablePoint;

        /// <summary> 最多128个跟踪点. </summary>
        public DetectPointx64 points;

        /// <summary> 用来调试看效果的摄像机射线. </summary>
        public CamRayx64 camRays;

        /// <summary> fView相机坐标. </summary>
        public Vector3 viewPosition;

        /// <summary> fView相机旋转. </summary>
        public Quaternion viewRotation;

        /// <summary> (逆)fView相机坐标. </summary>
        public Vector3 iViewPosition;

        /// <summary> (逆)fView相机旋转. </summary>
        public Quaternion iViewRotation;

        /// <summary> 下一个. </summary>
        public IntPtr next;
    }

    public unsafe class OCVData
    {
        ///-------------------------------------------------------------------------------------------------
        /// <summary> 初始化. </summary>
        ///
        /// <remarks> Xian Dai, 2017/5/7. </remarks>
        ///
        /// <param name="isStartServer"> (Optional)
        ///                              默认在服务未启动的时候启动服务,
        ///                              通常应该设置为true. </param>
        ///
        /// <returns> 是否初始化成功的状态,0表示成功,非0为失败. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern int fmInit(bool isStartServer = true);

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 关闭系统. </summary>
        ///
        /// <remarks> Xian Dai, 2017/5/7. </remarks>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern void fmClose();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 得到眼镜状态. </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 返回非零表示当前系统有检测到眼镜. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern int fmGetGlassStatus();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 得到眼镜坐标. </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 返回眼镜的坐标. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern Vector3 fmGetGlassPosition();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 眼镜旋转.z轴向前 </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 眼镜旋转的四元数. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern Quaternion fmGetGlassRotation();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 笔的检测状态. </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 返回非零表示当前系统有检测到笔. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern int fmGetPenStatus();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 笔坐标，它是笔尖的坐标. </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 笔尖坐标. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern Vector3 fmGetPenPosition();

        ///-------------------------------------------------------------------------------------------------
        /// <summary>
        /// 笔方向向量(笔实际上只有两点，所以在光学上无法得出三轴方向，所以这里是方向不是旋转)，
        /// 在3D系统中可以自己结合下面的滚转角生成一个四元数旋转。
        /// </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 笔的方向向量. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern Vector3 fmGetPenDirection();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 笔的滚转角，向右手旋转为正方向. </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 笔的滚转角. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern float fmGetPenRoll();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 得到笔的当前按键值状态标志,0x01中键,0x02左键,0x04右键. </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 得到笔的按键状态. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern int fmGetPenKey();

        ///-------------------------------------------------------------------------------------------------
        /// <summary>
        /// 整个一体机机身和桌面的倾斜角，当一体机机身完全垂直的时候它为0度.
        /// </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 机身的倾斜角. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern float fmGetSlantAngle();

        ///-------------------------------------------------------------------------------------------------
        /// <summary>
        /// 设置笔震动，调用此函数会让笔震动一下,目前只有mode =1的模式.连续调用这个函数可以让笔连续震动.
        /// </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <param name="mode"> (Optional) The mode. </param>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern void fmSetPenShake(int mode = 1);

        ///-------------------------------------------------------------------------------------------------
        /// <summary>
        /// 标记自己是活动用户,每5秒内需要调用一次，如果超过10秒没有调用这个函数，那么后台检测系统会停止检测。需要在
        /// 一个定时器里定时调用这个函数。
        /// 这是因为当操作系统中没有运行一个活动的VR程序的时候，后台检测系统会关闭一些工作以减少开销,并且会关闭红外灯.
        /// 当有VR程序标记自己是活动用户时，会马上打开红外灯并且启动检测程序
        /// PS:可以观察设备屏幕两侧红外灯是否亮起，知道检测程序是否开始工作。
        /// </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern void fmSetActiveUser();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> Gets ocv data address. </summary>
        ///
        /// <remarks> Dx, 2019/1/4. </remarks>
        ///
        /// <returns> The ocv data address. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        private static extern IntPtr getOCVDataAddr();

        ///-------------------------------------------------------------------------------------------------
        /// <summary>
        /// 设置屏幕VR模式的flag，0表示屏幕是非vr的正常显示模式，1表示是VR模式的LR格式画面显示。
        /// 这个函数用以通知屏幕切换显示模式.
        /// </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <param name="flag"> The flag. </param>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern void fmSetflagVRMode(int flag);

        ///-------------------------------------------------------------------------------------------------
        /// <summary>
        /// 设置一体机以外的其他屏幕VR模式的flag，0表示屏幕是非vr的正常显示模式，1表示是VR模式的LR格式画面显示。
        /// 这个函数用以通知屏幕切换显示模式.
        /// </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <param name="flag"> The flag. </param>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern void fmSetflagVRModeOther(int flag);

        ///-------------------------------------------------------------------------------------------------
        /// <summary>
        /// 设置双屏的显示模式，目前flag=1表示为扩展模式，flag=2为复制模式，flag=3为恢复上一次操作前的状态.
        /// </summary>
        ///
        /// <remarks> Xian Dai, 2018/3/26. </remarks>
        ///
        /// <param name="flag"> The flag. </param>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern void fmSetflagDualScreenMode(int flag);

        ///-------------------------------------------------------------------------------------------------
        /// <summary>
        /// 得到当前的双屏的显示模式，目前flag=1表示为扩展模式，flag=2为复制模式，flag=3为恢复上一次操作前的状态.
        /// </summary>
        ///
        /// <remarks> Xian Dai, 2018/3/26. </remarks>
        ///
        /// <param name="flag"> The flag. </param>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern int fmGetCurDualScreenStatus();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 一体机检测程序是否在睡眠状态,0为正常检查,大于0为睡眠状态. </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> The is sleep. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern int fmGetIsSleep();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 当前一体机检测程序的工作帧率. </summary>
        ///
        /// <remarks>  Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> The FPS. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern float fmGetFps();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 获取摄像头硬件错误标志. </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 摄像头硬件错误标志 . </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern int fmGetCameraDevErrorCode();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 获取笔硬件错误标志. </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 笔硬件错误标志. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern int fmGetPenDevErrorCode();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 获取主控板硬件错误标志. </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 主控板硬件错误标志. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern int fmGetMCDevErrorCode();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 得到软件是否发生了错误的标志. </summary>
        ///
        /// <remarks> Dx, 2018/10/30. </remarks>
        ///
        /// <returns> 处理软件自己发生了错误的标志. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern int fmGetProcErrorCode();

        ///-------------------------------------------------------------------------------------------------
        /// <summary> 获取主控板硬件id号. </summary>
        ///
        /// <remarks> Xian Dai, 2018/1/23. </remarks>
        ///
        /// <returns> 主控板硬件id号. </returns>
        ///-------------------------------------------------------------------------------------------------
        [DllImport("FSCore")]
        public static extern int fmGetMCDevID();

        /// <summary>
        /// 大家共用的检测数据
        /// </summary>
        public static ocvdata _data = new ocvdata();

        /// <summary>
        /// pOCVData对应的指针
        /// </summary>
        public static ocvdata* _pData = null;

        /// <summary>
        /// 数据起始的地址.
        /// </summary>
        internal static IntPtr pOCVData;

        /// <summary>
        /// 初始化这个数据，在Start()的时候运行一下就ok了
        /// </summary>
        internal static void InitData()
        {
            try
            {
                int res = fmInit(true);
                if (res == 0)
                {
                    Debug.Log("OCVData.InitData():MR系统初始化成功！");
                }
                else
                {
                    Debug.LogWarning("OCVData.InitData():MR系统初始化失败！err=" + res);
                }
                pOCVData = getOCVDataAddr();
                //pU3dData = getU3dDataAddr();
            }
            catch (Exception e)
            {
                Debug.LogError("OCVData.InitData():MR系统初始化异常！e=" + e.Message);
            }
        }

        /// <summary>
        /// 每帧调用，应该在Update中调用
        /// </summary>
        internal static void UpdateOCVdata()
        {
            if (pOCVData != IntPtr.Zero)
            {
                if (_pData == null)
                    _pData = (ocvdata*)pOCVData.ToPointer();
                _data = *_pData; //现在采用指针消除GC

                //_data = (ocvdata)Marshal.PtrToStructure(pOCVData, typeof(ocvdata));
            }
            else
            {
                Debug.LogWarning("OCVData.UpdateOCVdata():pOCVData为NULL，尝试再次InitData!");
                InitData();
            }

            updateCount++;
        }

        /// <summary>
        /// 写u3d数据到检测程序
        /// </summary>
        internal static void writeU3Ddata()
        {
            //将结构体拷到分配好的内存空间
            //Marshal.StructureToPtr(_u3ddata, pU3dData, false);
        }

        /// <summary>
        /// 数据更新的计数
        /// </summary>
        public static long updateCount;
    }
}