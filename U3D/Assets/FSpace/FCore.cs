using System;
using System.Collections.Generic;
using UnityEngine;

namespace FSpace
{
    /// <summary>
    /// 整个一体机的主要功能都被放到了这个静态类中
    /// </summary>
    public static class FCore
    {
        /// <summary>
        /// 由于场景比例很小，所以加入一个这个缩放比例，用来调整笔在U3D的坐标位置的缩放。
        /// 当笔的移动距离太小无法适应场景的时候，调整这个缩放值更大一些。
        /// </summary>
        public static float ViewerScale = 1;

        /// <summary>
        /// 整个系统的锚点的RTS
        /// </summary>
        static internal Matrix4x4 anchorRTS = Matrix4x4.TRS(Vector3.zero, Quaternion.Euler(Vector3.zero), Vector3.one);

        /// <summary>
        /// 整个系统锚点的旋转矩阵
        /// </summary>
        static internal Matrix4x4 anchorRMat = Matrix4x4.TRS(Vector3.zero, Quaternion.Euler(Vector3.zero), Vector3.one);

        /// <summary>
        ///  整个系统锚点的旋转
        /// </summary>
        static internal Quaternion anchorRQuat = Quaternion.Euler(Vector3.zero);

        #region opt检测数据

        /// <summary>
        /// 笔的射线
        /// </summary>
        public static Ray penRay
        {
            get
            {
                Ray ray = new Ray(penPosition, penDirection);
                return ray;
            }
        }

        /// <summary>
        /// 当前笔的旋转,其中它自己z轴为正前方.
        /// </summary>
        public static Quaternion penRotation
        {
            get
            {
                return anchorRQuat * BaseSystem.penRotation;
            }
        }

        /// <summary>
        /// 当前笔的坐标
        /// </summary>
        public static Vector3 penPosition
        {
            get
            {
                return (anchorRTS * BaseSystem.penPosition.toV4()).toV3();
            }
        }

        /// <summary>
        /// 笔的射线方向
        /// </summary>
        public static Vector3 penDirection
        {
            get
            {
                return (anchorRMat * BaseSystem.penDirection.toV4()).toV3();
            }
        }

        /// <summary>
        /// 当前眼镜的坐标
        /// </summary>
        public static Vector3 glassPosition
        {
            get
            {
                return (anchorRTS * BaseSystem.glassPosition.toV4()).toV3();
            }
        }

        /// <summary>
        /// 当前眼镜的旋转,其中它自己z轴为正前方.
        /// </summary>
        public static Quaternion glassRotation
        {
            get
            {
                return anchorRQuat * BaseSystem.glassRotation;
            }
        }

        /// <summary>
        /// 当前眼镜的旋转限制在水平.
        /// </summary>
        public static Quaternion glassRotation_horizontal
        {
            get
            {
                return anchorRQuat * BaseSystem.glassRotation_horizontal;
            }
        }

        /// <summary>
        /// 左眼摄像机坐标
        /// </summary>
        public static Vector3 eyeLeftPosition
        {
            get
            {
                return (anchorRTS * BaseSystem.eyeLeftPosition.toV4()).toV3();
            }
        }

        /// <summary>
        /// 右眼摄像机坐标
        /// </summary>
        public static Vector3 eyeRightPosition
        {
            get
            {
                return (anchorRTS * BaseSystem.eyeRightPosition.toV4()).toV3();
            }
        }

        #endregion opt检测数据

        #region 笔按键事件 - 拖拽功能

        /// <summary>
        /// 此时是否按键0（左侧键）已经按下
        /// </summary>
        public static bool isKey0Down
        {
            get { return (OCVData._data.PenKey & 0x01) == 0x01; }
        }

        /// <summary>
        /// 此时是否按键1（靠前键）已经按下
        /// </summary>
        public static bool isKey1Down
        {
            get { return (OCVData._data.PenKey & 0x02) == 0x02; }
        }

        /// <summary>
        /// 此时是否按键2（靠后键）已经按下
        /// </summary>
        public static bool isKey2Down
        {
            get { return (OCVData._data.PenKey & 0x04) == 0x04; }
        }

        /// <summary>
        ///  当前是否在正在拖拽状态
        /// </summary>
        public static bool isDraging
        {
            get
            {
                if (_dictRotate.Count > 0)
                    return true;
                else
                    return false;
            }
        }

        /// <summary>
        /// 最后一次拖拽物体到笔尖的距离
        /// </summary>
        public static float lastDragDistance
        {
            get;
            private set;
        }

        /// <summary>
        /// 添加一个拖拽物体.
        /// </summary>
        /// <param name="go">             要添加的物体. </param>
        /// <param name="distance">       拖拽的碰撞点坐标的距离. </param>
        /// <param name="isCtrlRotation"> (Optional)是否控制这个拖拽物体的旋转(如果不控制物体旋转，
        ///                               那么在中心点和碰撞体中心不一致时会出现物体跑飞) </param>
        /// <returns> 返回一个. </returns>
        public static DragRecord addDragObj(GameObject go, float distance, bool isCtrlRotation = true)
        {
            DragRecord dr = new DragRecord(go.transform, distance);
            dr.isCtrlRotation = isCtrlRotation;

            if (_dictRotate.ContainsKey(go))//如果已经包含这个物体
                deleteDragObj(go);

            lastDragDistance = distance;//记录最后一个距离，画线用
            _dictRotate.Add(go, dr);
            return dr;
        }

        /// <summary>
        /// 更新所有拖拽物体的位置状态
        /// </summary>
        internal static void updateDragObj()
        {
            List<GameObject> needDeleteList = null;

            foreach (var item in _dictRotate)
            {
                if (!item.Value.isStop)//如果物体没有停止拖拽那么就继续计算拖拽
                {
                    item.Value.Update();
                }
                else
                {
                    if (needDeleteList == null) { needDeleteList = new List<GameObject>(); }
                    needDeleteList.Add(item.Key);
                }
            }

            //如果物体已经停止拖拽了那么就删掉这个拖拽物体
            if (needDeleteList != null)
                foreach (var item in needDeleteList)
                {
                    deleteDragObj(item);
                }
        }

        /// <summary>
        /// 删除拖拽物体
        /// </summary>
        public static void deleteDragObj(GameObject go)
        {
            if (!object.ReferenceEquals(go, null) && _dictRotate.ContainsKey(go))//如果包含这个物体
            {
                _dictRotate.Remove(go);
            }
        }

        /// <summary>
        /// 清空所有的拖拽物体
        /// </summary>
        public static void clearDragObj()
        {
            _dictRotate.Clear();
        }

        /// <summary>
        /// 拉近拖拽物体
        /// </summary>
        /// <param name="obj">拖拽物体</param>
        /// <param name="percent">拉近的百分率</param>
        public static void pullDragObj(GameObject obj, float percent)
        {
            if (_dictRotate.ContainsKey(obj))
            {
                _dictRotate[obj].distanceScale -= percent;
                //_dictRotate[obj].distance -= _dictRotate[obj].distance * percent;
                lastDragDistance = _dictRotate[obj].distance * _dictRotate[obj].distanceScale;//记录最后一个距离，画线用
            }
        }

        /// <summary>
        /// 推远拖拽物体
        /// </summary>
        /// <param name="obj">拖拽物体</param>
        /// <param name="percent">推远的百分率</param>
        public static void pushDragObj(GameObject obj, float percent)
        {
            if (_dictRotate.ContainsKey(obj))
            {
                _dictRotate[obj].distanceScale += percent;
                //_dictRotate[obj].distance  = _dictRotate[obj].distance * DragDistanceScale;
                lastDragDistance = _dictRotate[obj].distance * _dictRotate[obj].distanceScale;//记录最后一个距离，画线用
            }
        }

        /// <summary>
        /// 记录所有拖拽物体的字典
        /// </summary>
        private static Dictionary<GameObject, DragRecord> _dictRotate = new Dictionary<GameObject, DragRecord>();

        /// <summary>
        /// 事件，按键0按下
        /// </summary>
        public static event Action EventKey0Down;

        /// <summary>
        /// 事件，按键0抬起
        /// </summary>
        public static event Action EventKey0Up;

        /// <summary>
        /// 事件，按键1按下
        /// </summary>
        public static event Action EventKey1Down;

        /// <summary>
        /// 事件，按键1抬起
        /// </summary>
        public static event Action EventKey1Up;

        /// <summary>
        /// 事件，按键2按下
        /// </summary>
        public static event Action EventKey2Down;

        /// <summary>
        /// 事件，按键2抬起
        /// </summary>
        public static event Action EventKey2Up;

        private static bool _isKey0Down_cur;
        private static bool _isKey1Down_cur;
        private static bool _isKey2Down_cur;

        /// <summary>
        /// 用于更新判断发出按键
        /// </summary>
        internal static void updateKeyEvent()
        {
            if (!_isKey0Down_cur && isKey0Down)
            {
                if (EventKey0Down != null) { EventKey0Down(); }
            }
            if (!_isKey1Down_cur && isKey1Down)
            {
                if (EventKey1Down != null) { EventKey1Down(); }
            }
            if (!_isKey2Down_cur && isKey2Down)
            {
                if (EventKey2Down != null) { EventKey2Down(); }
            }
            if (_isKey0Down_cur && !isKey0Down)
            {
                if (EventKey0Up != null) { EventKey0Up(); }
            }
            if (_isKey1Down_cur && !isKey1Down)
            {
                if (EventKey1Up != null) { EventKey1Up(); }
            }
            if (_isKey2Down_cur && !isKey2Down)
            {
                if (EventKey2Up != null) { EventKey2Up(); }
            }

            _isKey0Down_cur = isKey0Down;
            _isKey1Down_cur = isKey1Down;
            _isKey2Down_cur = isKey2Down;
        }

        #endregion 笔按键事件 - 拖拽功能

        #region 笔的功能

        /// <summary>
        /// 笔震动
        /// </summary>
        /// <param name="mode">震动模式</param>
        public static void PenShake(int mode = 0)
        {
            Debug.Log("FCore.PenShake():调用了一次PenShake()函数！");
            OCVData.fmSetPenShake(1);
        }

        #endregion 笔的功能

        #region 设置

        /// <summary>
        /// 设置所有显示器屏幕为2D
        /// </summary>
        public static void SetScreen2D()
        {
            OCVData.fmSetflagVRMode(0);
            OCVData.fmSetflagVRModeOther(0);
        }

        /// <summary>
        /// 设置所有显示器屏幕为3D
        /// </summary>
        public static void SetScreen3D()
        {
            OCVData.fmSetflagVRMode(1);
            OCVData.fmSetflagVRModeOther(1);
        }

        /// <summary>
        /// 只设置自己的机器,MR一体机显示器屏幕为3D
        /// </summary>
        public static void SetScreen3DSelf()
        {
            OCVData.fmSetflagVRMode(1);
        }

        /// <summary>
        /// 只设置自己的机器,MR一体机显示器屏幕为2D
        /// </summary>
        public static void SetScreen2DSelf()
        {
            OCVData.fmSetflagVRMode(0);
        }

        /// <summary>
        /// 切换一体机以外的屏幕进3d
        /// </summary>
        public static void SetScreen3DOther()
        {
            OCVData.fmSetflagVRModeOther(1);
        }

        /// <summary>
        /// 切换一体机以外的屏幕进2d
        /// </summary>
        public static void SetScreen2DOther()
        {
            OCVData.fmSetflagVRModeOther(0);
        }

        /// <summary>
        /// 设置双屏显示为扩展模式
        /// </summary>
        public static void SetDualScreenExtend()
        {
            OCVData.fmSetflagDualScreenMode(1);
        }

        /// <summary>
        /// 设置双屏显示为复制模式
        /// </summary>
        public static void SetDualScreenClone()
        {
            OCVData.fmSetflagDualScreenMode(2);
        }

        /// <summary>
        /// 设置双屏显示为恢复上一次操作前状态
        /// </summary>
        public static void SetDualScreenStatusBack()
        {
            OCVData.fmSetflagDualScreenMode(3);
        }

        /// <summary>
        /// 得到当前的双屏状态
        /// </summary>
        public static int getCurDualScreenStatus()
        {
            return OCVData.fmGetCurDualScreenStatus();
        }

        /// <summary>
        /// 瞳距。默认6cm
        /// </summary>
        public static float PupilDistance = 0.062f;

        /// <summary>
        /// 是否自动设置倾斜。这是一个重要的设置，如果设置了自动倾斜，那么整个系统会和实际一体机和桌面的倾斜角度一致。
        /// 适用于一些模拟桌面的场合，能让u3d中的桌面一直水平。
        /// </summary>
        public static bool isAutoSlant = true;

        #endregion 设置

        #region 零平面

        /// <summary>
        /// 屏幕面-左上角坐标
        /// </summary>
        public static Vector3 screenPointLeftTop
        {
            get
            {
                return (anchorRTS * BaseSystem.screenPoint.LeftTop.toV4()).toV3();
            }
        }

        /// <summary>
        /// 屏幕面-右上角坐标
        /// </summary>
        public static Vector3 screenPointRightTop
        {
            get
            {
                return (anchorRTS * BaseSystem.screenPoint.RightTop.toV4()).toV3();
            }
        }

        /// <summary>
        /// 屏幕面-左下角坐标
        /// </summary>
        public static Vector3 screenPointLeftBotton
        {
            get
            {
                return (anchorRTS * BaseSystem.screenPoint.LeftBotton.toV4()).toV3();
            }
        }

        /// <summary>
        /// 屏幕面-右下角坐标
        /// </summary>
        public static Vector3 screenPointRightBotton
        {
            get
            {
                return (anchorRTS * BaseSystem.screenPoint.RightBotton.toV4()).toV3();
            }
        }

        /// <summary>
        /// 屏幕面-中心坐标
        /// </summary>
        public static Vector3 screenCentre
        {
            get
            {
                return (anchorRTS * BaseSystem.screenPoint.Centre.toV4()).toV3();
            }
        }

        /// <summary>
        /// 屏幕面对应的世界旋转
        /// </summary>
        public static Quaternion screenRotation
        {
            get
            {
                return anchorRQuat * slantRotate;
            }
        }

        /// <summary>
        /// 一体机机身的倾斜角
        /// </summary>
        public static float slantAngle
        {
            get
            {
                if (!isAutoSlant)
                {
                    return 0;
                }
                else
                {
                    return OCVData._data.slantAngle;
                }
            }
        }

        /// <summary>
        /// 机身倾斜角所对应旋转变换
        /// </summary>
        internal static Quaternion slantRotate
        {
            get
            {
                return Quaternion.AngleAxis(slantAngle, Vector3.right);
            }
        }

        #endregion 零平面

        #region 公共方法

        internal static Vector4 toV4(this Vector3 v3)
        {
            return new Vector4(v3.x, v3.y, v3.z, 1);
        }

        internal static Vector3 toV3(this Vector4 v4)
        {
            return new Vector3(v4.x, v4.y, v4.z);
        }

        #endregion 公共方法
    }
}