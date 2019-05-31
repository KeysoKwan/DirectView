#pragma once

#include "hidapi.h"

#include <Eigen/Geometry>
#include <unsupported/Eigen/EulerAngles>
#include "crc32.h"

namespace dxlib {
///-------------------------------------------------------------------------------------------------
/// <summary>
/// 机身的一个倾斜角度的读取，从这个类里面可以得到是否当前角度发生了变化，如果发生了变化那么可能重新校正笔.
/// </summary>
///
/// <remarks> Dx, 2017/11/28. </remarks>
///-------------------------------------------------------------------------------------------------
class MCDevice
{
  public:
    MCDevice()
    {
        memset(this->buf, 0, sizeof(buf));
        handle = NULL;
        readLength = 32; //读usb数据长度是32

        euleAngle = Eigen::Vector3f(90, 0, 0); //90度的时候才是竖直。
    }

    ~MCDevice()
    {
    }

    static MCDevice* GetInst()
    {
        if (m_pInstance == NULL)
            m_pInstance = new MCDevice();
        return m_pInstance;
    }

  public:
    /// <summary> 陀螺仪的usb读值旋转数据. </summary>
    Eigen::Quaternionf rotation;

    /// <summary> 使用usb读值rotation计算出的欧拉角. </summary>
    Eigen::Vector3f euleAngle;

    /// <summary> 主机ID. </summary>
    unsigned int ID = 0;

    /// <summary> 主机ID生成用的byte. </summary>
    unsigned char IDByte[12];

    /// <summary> 主机ID的base64版本. </summary>
    std::string IDBase64;

    ///-------------------------------------------------------------------------------------------------
    /// <summary> Initializes this object. </summary>
    ///
    /// <remarks> Dx, 2019/3/13. </remarks>
    ///
    /// <returns> An int. </returns>
    ///-------------------------------------------------------------------------------------------------
    int init();

    ///-------------------------------------------------------------------------------------------------
    /// <summary> Updates this object. </summary>
    ///
    /// <remarks> Dx, 2019/3/13. </remarks>
    ///-------------------------------------------------------------------------------------------------
    void update();

    ///-------------------------------------------------------------------------------------------------
    /// <summary> 得到这个旋转的文本. </summary>
    ///
    /// <remarks> Dx, 2017/8/21. </remarks>
    ///
    /// <returns> The text. </returns>
    ///-------------------------------------------------------------------------------------------------
    std::string getText();

    ///-------------------------------------------------------------------------------------------------
    /// <summary>发送红外灯开关命令. </summary>
    ///
    /// <remarks> Dx, 2017/11/2. </remarks>
    ///
    /// <param name="mode"> (Optional) The mode. </param>
    ///-------------------------------------------------------------------------------------------------
    void sendLight(bool isON);

    ///-------------------------------------------------------------------------------------------------
    /// <summary> 发送连续闪烁的红外灯开关命令(但是它运行的不是很好，频率较高的时候不太好用). </summary>
    ///
    /// <remarks> Dx, 2018/8/13. </remarks>
    ///
    /// <param name="onTime">  一个周期灯亮的时间ms. </param>
    /// <param name="cycleTime"> 一个周期总的时间ms. </param>
    /// <param name="count">   循环次数. </param>
    ///-------------------------------------------------------------------------------------------------
    void sendLight(unsigned short onTime, unsigned short cycleTime, unsigned char count);

    ///-------------------------------------------------------------------------------------------------
    /// <summary> Sends the 3D lr. </summary>
    ///
    /// <remarks> Dx, 2018/1/8. </remarks>
    ///-------------------------------------------------------------------------------------------------
    void send3D_LR();

    ///-------------------------------------------------------------------------------------------------
    /// <summary> Sends the 2D. </summary>
    ///
    /// <remarks> Dx, 2018/1/24. </remarks>
    ///-------------------------------------------------------------------------------------------------
    void send2D();

    ///-------------------------------------------------------------------------------------------------
    /// <summary> 发送读取设备ID参数. </summary>
    ///
    /// <remarks> Dx, 2018/1/24. </remarks>
    ///-------------------------------------------------------------------------------------------------
    bool sendReadID();

    ///-------------------------------------------------------------------------------------------------
    /// <summary> 读主机id,第一次会阻塞. </summary>
    ///
    /// <remarks> Dx, 2019/3/12. </remarks>
    ///
    /// <returns> The identifier. </returns>
    ///-------------------------------------------------------------------------------------------------
    unsigned int ReadID();
    ///-------------------------------------------------------------------------------------------------
    /// <summary> 读主机id,第一次会阻塞. </summary>
    ///
    /// <remarks> Dx, 2019/3/12. </remarks>
    ///
    /// <returns> Base64的主机id. </returns>
    ///-------------------------------------------------------------------------------------------------
    std::string ReadIDBase64();
  private:
    /// <summary> The instance. </summary>
    static MCDevice* m_pInstance;

    /// <summary> 打开的hid设备. </summary>
    hid_device* handle;

    int readLength;

    /// <summary> 当前的读取的状态结果. </summary>
    int res = 0;

    /// <summary> 读usb数据的buffer. </summary>
    unsigned char buf[128];

    /// <summary> 上一次的欧拉角度. </summary>
    Eigen::Vector3f _lastEuleAngle;

    /// <summary> The CRC. </summary>
    CRC32 crc;
};
} // namespace dxlib