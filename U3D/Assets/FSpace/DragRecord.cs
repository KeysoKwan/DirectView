using System;
using UnityEngine;

namespace FSpace
{
    /// <summary>
    /// 一次拖拽记录的数据类
    /// </summary>
    public class DragRecord
    {
        /// <summary>
        /// 构造，依赖FCore中的当前笔状态数据
        /// </summary>
        /// <param name="dragObj">拖拽物体</param>
        internal DragRecord(Transform dragObj, float distance)
        {
            this.dragObj = dragObj;

            //Matrix4x4 rDragObj = new Matrix4x4();
            //rDragObj.SetTRS(dragObj.position, dragObj.rotation, new Vector3(1, 1, 1));
            //rDragObj.SetTRS(hitPoint, dragObj.rotation, new Vector3(1, 1, 1));
            //offset = FCore.penMatrix4x4.inverse * rDragObj;

            //float distance = (hitPoint - FCore.penPosition).magnitude;
            this.BeginGrab(dragObj.gameObject, distance, FCore.penPosition, FCore.penRotation);
        }

        private Vector3 _initialGrabOffset = Vector3.zero;
        private Quaternion _initialGrabRotation = Quaternion.identity;
        private float _initialGrabDistance = 0.0f;
        private float _distanceScale = 1.0f;

        //设置停止拖拽的键值
        private int setStopKey = -1;

        /// <summary>
        /// 拖拽物体
        /// </summary>
        public Transform dragObj;

        /// <summary>
        /// 停止拖拽的事件
        /// </summary>
        public event Action<DragRecord> EventStopDrag = null;

        /// <summary>
        /// 是否控制拖拽物体的旋转(未测，不要使用这个功能)
        /// </summary>
        public bool isCtrlRotation = true;

        /// <summary>
        /// 是否停止拖拽
        /// </summary>
        public bool isStop = false;

        /// <summary>
        /// 距离的缩放控制，用于拉远拉近 distance * distanceScale来控制
        /// </summary>
        public float distanceScale
        {
            get
            {
                return _distanceScale;
            }
            set
            {
                _distanceScale = value;
                if (value < 0.05f)
                    _distanceScale = 0.05f;
                if (value > 2.0f)
                    _distanceScale = 2.0f;
            }
        }

        /// <summary>
        /// 抓取点到笔尖的距离
        /// </summary>
        public float distance
        {
            get
            {
                return _initialGrabDistance;
            }
            set
            {
                _initialGrabDistance = value;
            }
        }

        /// <summary>
        /// 设置由哪个按键抬起来终止拖拽，产生Stop Event.
        /// key值为0,1,2
        /// </summary>
        /// <param name="key">按键值0,1,2</param>
        public DragRecord SetStopWhenKeyUp(int key)
        {
            if (setStopKey != -1)//只能被设置一次
            {
                return this;
            }
            if (key == 0)
                FCore.EventKey0Up += OnKeyUp;
            if (key == 1)
                FCore.EventKey1Up += OnKeyUp;
            if (key == 2)
                FCore.EventKey2Up += OnKeyUp;

            return this;
        }

        /// <summary>
        /// 设置由哪个按键抬起来终止拖拽，产生Stop Event,并且设置回调。
        /// key值为0,1,2
        /// </summary>
        /// <param name="proc"></param>
        /// <param name="key">按键值0,1,2</param>
        /// <returns></returns>
        public DragRecord SetStopWhenKeyUp(Action<DragRecord> proc, int key)
        {
            if (setStopKey != -1)//只能被设置一次
            {
                return this;
            }

            this.EventStopDrag += proc;

            if (key == 0)
                FCore.EventKey0Up += OnKeyUp;
            if (key == 1)
                FCore.EventKey1Up += OnKeyUp;
            if (key == 2)
                FCore.EventKey2Up += OnKeyUp;

            return this;
        }

        /// <summary>
        /// 响应事件的抬起
        /// </summary>
        private void OnKeyUp()
        {
            isStop = true;//标记停止

            //清除事件
            if (setStopKey == 0)
                FCore.EventKey0Up -= OnKeyUp;
            if (setStopKey == 1)
                FCore.EventKey1Up -= OnKeyUp;
            if (setStopKey == 2)
                FCore.EventKey2Up -= OnKeyUp;

            try
            {
                if (this.EventStopDrag != null)
                    this.EventStopDrag(this);//执行时间
            }
            catch (Exception e)
            {
                Debug.LogError("DragRecord.OnKeyUp():执行用户事件异常:" + e.Message);
            }
            this.EventStopDrag = null;
        }

        /// <summary>
        /// 使用当前的笔状态去更新这个拖拽物体的位置
        /// </summary>
        internal void Update()
        {
            if (!isStop)
                this.UpdateGrab(FCore.penPosition, FCore.penRotation, isCtrlRotation);
        }

        private void BeginGrab(GameObject hitObject, float hitDistance, Vector3 inputPosition, Quaternion inputRotation)
        {
            Vector3 inputEndPosition = inputPosition + (inputRotation * (Vector3.forward * hitDistance));

            // Cache the initial grab state.
            dragObj = hitObject.transform;

            //ZSpace官方写法使用了本地旋转
            //_initialGrabOffset = Quaternion.Inverse(hitObject.transform.localRotation) * (hitObject.transform.localPosition - inputEndPosition);
            //_initialGrabRotation = Quaternion.Inverse(inputRotation) * hitObject.transform.localRotation;

            _initialGrabOffset = Quaternion.Inverse(hitObject.transform.rotation) * (hitObject.transform.position - inputEndPosition);
            _initialGrabRotation = Quaternion.Inverse(inputRotation) * hitObject.transform.rotation;

            _initialGrabDistance = hitDistance;
        }

        private void UpdateGrab(Vector3 inputPosition, Quaternion inputRotation, bool isCtrlRotation = true)
        {
            Vector3 inputEndPosition = inputPosition + (inputRotation * (Vector3.forward * _initialGrabDistance * distanceScale));

            // Update the grab object's rotation.
            Quaternion objectRotation = inputRotation * _initialGrabRotation;
            if (isCtrlRotation)//是否修改抓取物体的旋转
            {
                dragObj.rotation = objectRotation;
                //Update the grab object's position.
                Vector3 objectPosition = inputEndPosition + (objectRotation * _initialGrabOffset);
                dragObj.position = objectPosition;
            }
            else
            {
                //如果不修改旋转，那么会由于物体不在中心点，出现物体移动时的跑飞
                Vector3 objectPosition = inputEndPosition + (objectRotation * _initialGrabOffset);
                dragObj.position = inputEndPosition;
            }
        }
    }
}