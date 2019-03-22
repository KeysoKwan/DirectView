using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;
using xuexue.LitJson;

namespace DTO
{
    [xuexueJsonClass]
    class FViewRT
    {
        /// <summary> fView相机坐标. </summary>
        public Vector3 viewPosition;

        /// <summary> fView相机旋转. </summary>
        public Quaternion viewRotation;

        /// <summary> (逆)fView相机坐标. </summary>
        public Vector3 iViewPosition;

        /// <summary> (逆)fView相机旋转. </summary>
        public Quaternion iViewRotation;

    }
}
