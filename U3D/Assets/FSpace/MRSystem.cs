using System;
using UnityEngine;

namespace FSpace
{
    /// <summary>
    /// 这个类能够更新OCV数据，这个脚本的优先级可能要放到最前比较好。同时让它作为整个系统的锚点。当开启了自动倾斜的时候，这个系统以底边
    /// </summary>
    public class MRSystem : MonoBehaviour
    {
        /// <summary>
        /// 是否应用屏幕的自动倾斜
        /// </summary>
        [Tooltip("是否应用屏幕的自动倾斜")]
        public bool isAutoSlant = true;

        /// <summary>
        /// 整个系统的缩放比例
        /// </summary>
        [Tooltip("整个系统的缩放比例")]
        public float ViewerScale = 1.0f;

        /// <summary>
        /// 设置一个深度k(实验性的)
        /// </summary>
        private float zK = 1.0f;

        /// <summary>
        /// 内部调试，不要设置为true.
        /// </summary>
        internal static bool isOptDebug = false;

        private float _viewerScale = 1.0f;
        private bool _isAutoSlant = true;

        private void Awake()
        {
            UpdateInspector();

            OCVData.InitData();
            OCVData.UpdateOCVdata();
        }

        private void FixedUpdate()
        {
            this.Update();
        }

        private void Update()
        {
            // 响应检视面板改动
            UpdateInspector();

            //写入u3d到ocv的数据
            //OCVData.writeU3Ddata();

            //设置当前的TRS
            FCore.anchorRTS.SetTRS(transform.position, transform.rotation, transform.lossyScale);
            FCore.anchorRMat.SetTRS(Vector3.zero, transform.rotation, transform.lossyScale);
            FCore.anchorRQuat = transform.rotation;

            //读取新的OCV数据
            OCVData.UpdateOCVdata();

            //读取新的数据之后将底边设置为0坐标（isOptDebug设置为ture的时候则不改变底边坐标）
            SetBottonZero();

            //更新倾斜的屏幕点
            BaseSystem.updateScreenPoint();

            //更新按键状态以便发出事件
            FCore.updateKeyEvent();

            //更新一下记录的拖拽物体的状态好了
            FCore.updateDragObj();

            //设置自己是活动用户
            OCVData.fmSetActiveUser();

            //这个物体作为父物体应该也应用一个倾斜,对它的第一级子物体赋旋转,
            //但是不能对它自身赋旋转,因为它自身的旋转要作为整个系统的锚点参与计算
            for (int i = 0; i < transform.childCount; i++)
            {
                transform.GetChild(i).localRotation = FCore.slantRotate;
            }
        }

        private void OnDrawGizmos()
        {
            //响应检视面板isAutoSlant的设置
            UpdateInspector();

            if (isOptDebug)
            {
            }
            else
            {
                //更新倾斜的屏幕点
                BaseSystem.updateScreenPoint();

                //设置当前的TRS
                FCore.anchorRTS.SetTRS(transform.position, transform.rotation, transform.lossyScale);
                FCore.anchorRMat.SetTRS(Vector3.zero, transform.rotation, transform.lossyScale);
                FCore.anchorRQuat = transform.rotation;

                //画位置要改
                Debug.DrawLine(FCore.screenPointLeftTop, FCore.screenPointRightTop, Color.red);
                Debug.DrawLine(FCore.screenPointRightTop, FCore.screenPointRightBotton, Color.red);
                Debug.DrawLine(FCore.screenPointRightBotton, FCore.screenPointLeftBotton, Color.red);
                Debug.DrawLine(FCore.screenPointLeftTop, FCore.screenPointLeftBotton, Color.red);
            }
        }

        private void SetBottonZero()
        {
            //在调试模式下应该强制关闭自动倾斜
            if (isOptDebug)
            {
                FCore.isAutoSlant = false;
            }
            else
            {
                //眼镜和笔的坐标都上抬0.151m,让底边在0点。
                OCVData._data.PenPosition.y += 0.151f;
                OCVData._data.GlassPosition.y += 0.151f;

                Vector3 p1 = OCVData._data.PenPosition + OCVData._data.PenDirection;
                p1.z = p1.z * zK;

                OCVData._data.PenPosition.z = OCVData._data.PenPosition.z * zK;
                OCVData._data.GlassPosition.z = OCVData._data.GlassPosition.z * zK;
                OCVData._data.PenDirection = p1 - OCVData._data.PenPosition;
            }
        }

        /// <summary>
        /// 响应检视面板改动
        /// </summary>
        private void UpdateInspector()
        {
            if (Math.Abs(_viewerScale - ViewerScale) > 1e-4)
            {
                _viewerScale = ViewerScale;
                FCore.ViewerScale = ViewerScale;
            }

            if (_isAutoSlant != isAutoSlant)
            {
                _isAutoSlant = isAutoSlant;
                FCore.isAutoSlant = isAutoSlant;
            }

            ViewerScale = FCore.ViewerScale;
            isAutoSlant = FCore.isAutoSlant;
        }
    }
}