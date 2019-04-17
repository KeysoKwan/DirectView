#include "MCDevice.h"
#include <thread>
#include "./base64.h"

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")

#define LogE printf

namespace dxlib {

MCDevice* MCDevice::m_pInstance = NULL;

int MCDevice::init()
{
    if (hid_init()) { //初始化hid库
        LogE("MCDevice.init():hid_init() fail ! ");
        return -1;
    }

    //打开设备VID=0x0483，PID=0x5750
    handle = hid_open(0x0483, 0x5751, NULL);
    if (!handle) {
        LogE("MCDevice.init():unable to open device !");
        return 1;
    }

    // Set the hid_read() function to be non-blocking.
    hid_set_nonblocking(handle, 1);

    memset(buf, 0, sizeof(buf));
    // Try to read from the device. There shoud be no
    // data here, but execution should not block.
    res = hid_read(handle, buf, readLength);

    return 0;
}

void MCDevice::update()
{
    if (handle == NULL) { //说明没打开设备
        init();           //那么还是尝试初始化一下
        return;
    }

    if (res == readLength) { //如果已经读取完毕了
        //可以提取数据
        // key = buf[1];
        if (buf[0] == 0x02 && buf[1] == 0x00) {
            long l = (long)(((unsigned long)buf[5] << 24) | ((unsigned long)buf[4] << 16) | ((unsigned long)buf[3] << 8) | (unsigned long)buf[2]);
            rotation.x() = (float)l / 1073741824.0f;
            l = (long)(((unsigned long)buf[9] << 24) | ((unsigned long)buf[8] << 16) | ((unsigned long)buf[7] << 8) | (unsigned long)buf[6]);
            rotation.y() = (float)l / 1073741824.0f;
            l = (long)(((unsigned long)buf[13] << 24) | ((unsigned long)buf[12] << 16) | ((unsigned long)buf[11] << 8) | (unsigned long)buf[10]);
            rotation.z() = (float)l / 1073741824.0f;
            l = (long)(((unsigned long)buf[17] << 24) | ((unsigned long)buf[16] << 16) | ((unsigned long)buf[15] << 8) | (unsigned long)buf[14]);
            rotation.w() = (float)l / 1073741824.0f;

            Eigen::Matrix3f rm = rotation.toRotationMatrix(); //3x3的旋转矩阵
            Eigen::EulerAnglesZYXf ea(rm);                    //旋转矩阵再转欧拉角
            //euleAngle = Math::Rad2Deg(Eigen::Vector3f{(float)M_PI + ea.beta(), ea.gamma(), ea.alpha()}); //这里是为了以前的调整而调整,x有效
        }
        else if (buf[0] == 0xF3 && buf[1] == 0x0D) {
            //收到了ID数据
            //读取主控板 ID返回 32Byte数据： 0xF30x0DID[0-11]CHECKSUM
            this->ID = crc.Crc(buf, 2, 12);
            memcpy_s(this->IDByte, 12, buf + 2, 12);
            //检查一下确保自己计算是正确的
            if (crc.Crc(this->IDByte, 0, 12) != this->ID) {
                //LogE("MCDevice.update():ID计算有误!!");
            }
            this->IDBase64 = base64_encode(this->IDByte, 12);
            //LogI("MCDevice.update():收到了ID数据 ID=%u ,IDBase64=%s ", this->ID, IDBase64.c_str());
        }
        else { //这里是收到了其他的消息
            //std::string msg = byte2str(buf, readLength);
            //dxlib::Debug::LogW("MCDevice.update():收到了其他的消息！%s", msg.c_str());

            //将来在这里加入读取摄像头标定数据
        }
        memset(buf, 0, sizeof(buf));
        res = hid_read(handle, buf, readLength);
    }
    else if (res == -1) {
    }
    else {
        res = hid_read(handle, buf, readLength);
    }
};

///-------------------------------------------------------------------------------------------------
/// <summary>发送红外灯开关命令. </summary>
///
/// <remarks> Dx, 2017/11/2. </remarks>
///
/// <param name="mode"> (Optional) The mode. </param>
///-------------------------------------------------------------------------------------------------
void MCDevice::sendLight(bool isON)
{
    if (handle == NULL) { //说明没打开设备
        init();           //那么还是尝试初始化一下
    }

    if (handle != NULL) {
        unsigned char data[33];
        memset(data, 0, 33);
        data[1] = 0xF2;
        if (isON) {
            data[2] = 0x0C; //打开灯
        }
        else {
            data[2] = 0x0B;
        }
        data[3] = (data[1] + data[2]) & 0xFF;

        int res = hid_write(handle, data, 32);
        //if (isON)
        //    LogW("MCDevice.sendLight(): 发送开灯 %d", res);
        //else
        //    LogW("MCDevice.sendLight(): 发送关灯 %d", res);
    }
    else {
        LogE("MCDevice.sendLight():发送开/关灯失败，USB设备未打开 ！");
    }
}

///-------------------------------------------------------------------------------------------------
/// <summary> 发送连续闪烁的红外灯开关命令(但是它运行的不是很好，频率较高的时候不太好用). </summary>
///
/// <remarks> Dx, 2018/8/13. </remarks>
///
/// <param name="onTime">  一个周期灯亮的时间ms. </param>
/// <param name="cycleTime"> 一个周期总的时间ms. </param>
/// <param name="count">   循环次数. </param>
///-------------------------------------------------------------------------------------------------
void MCDevice::sendLight(unsigned short onTime, unsigned short cycleTime, unsigned char count)
{
    if (handle == NULL) { //说明没打开设备
        init();           //那么还是尝试初始化一下
    }

    if (handle != NULL) {
        unsigned char data[33];
        memset(data, 0, 33);
        data[1] = 0xF2;
        data[2] = 0x0C; //打开灯
        data[3] = 0xFE;
        data[4] = count;          //闪烁次数
        data[5] = cycleTime >> 8; //周期总时间
        data[6] = cycleTime && 0x00FF;
        data[7] = onTime >> 8; //亮的时间
        data[8] = onTime && 0x00FF;
        int res = hid_write(handle, data, 32);
        //if (isON)
        //    Debug::LogW("MCDevice.sendLight(): 发送开灯 %d", res);
        //else
        //    Debug::LogW("MCDevice.sendLight(): 发送关灯 %d", res);
    }
    else {
        LogE("MCDevice.sendLight():发送开灯失败，USB设备未打开 ！");
    }
}

///-------------------------------------------------------------------------------------------------
/// <summary> Sends the 3D lr. </summary>
///
/// <remarks> Dx, 2018/1/8. </remarks>
///-------------------------------------------------------------------------------------------------
void MCDevice::send3D_LR()
{
    if (handle == NULL) { //说明没打开设备
        init();           //那么还是尝试初始化一下
    }

    if (handle != NULL) {
        unsigned char data[33];
        memset(data, 0, 33);
        data[1] = 0xF2;
        data[2] = 0x02;
        data[3] = (data[1] + data[2]) & 0xFF;
        int res = hid_write(handle, data, 32);
        //LogI("MCDevice.send3D_LR():发送设置3D_LR %d", res);
    }
    else {
        LogE("MCDevice.send3D_LR():发送设置3D_LR，USB设备未打开 ！");
    }
}

///-------------------------------------------------------------------------------------------------
/// <summary> Sends the 2D. </summary>
///
/// <remarks> Dx, 2018/1/24. </remarks>
///-------------------------------------------------------------------------------------------------
void MCDevice::send2D()
{
    if (handle == NULL) { //说明没打开设备
        init();           //那么还是尝试初始化一下
    }

    if (handle != NULL) {
        unsigned char data[33];
        memset(data, 0, 33);
        data[1] = 0xF2;
        data[2] = 0x00;
        data[3] = (data[1] + data[2]) & 0xFF;
        int res = hid_write(handle, data, 32);
        //LogI("MCDevice.send2D():发送设置2D %d", res);
    }
    else {
        LogE("MCDevice.send2D():发送设置2D，USB设备未打开 ！");
    }
}

///-------------------------------------------------------------------------------------------------
/// <summary> 发送读取设备ID参数. </summary>
///
/// <remarks> Dx, 2018/1/24. </remarks>
///-------------------------------------------------------------------------------------------------
bool MCDevice::sendReadID()
{
    if (handle == NULL) { //说明没打开设备
        init();           //那么还是尝试初始化一下
    }

    if (handle != NULL) {
        unsigned char data[33];
        memset(data, 0, 33);
        data[1] = 0xF2;
        data[2] = 0x0D;
        data[3] = (data[1] + data[2]) & 0xFF;
        int res = hid_write2(handle, data, 32);
        return true;
    }
    else {
        LogE("MCDevice.sendReadID():发送读取设备ID，USB设备未打开 ！\r\n");
    }
    return false;
}

///-------------------------------------------------------------------------------------------------
/// <summary> 读主机id. </summary>
///
/// <remarks> Dx, 2019/3/12. </remarks>
///
/// <returns> The identifier. </returns>
///-------------------------------------------------------------------------------------------------
unsigned int MCDevice::ReadID()
{
    if (ID != 0) {
        return ID;
    }
    sendReadID();
    for (size_t i = 0; i < 40; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        update();
        if (ID != 0) {
            return ID;
        }
    }

    return 0;
}

std::string MCDevice::ReadIDBase64()
{
    if (!IDBase64.empty()) {
        return IDBase64;
    }
    sendReadID();
    for (size_t i = 0; i < 40; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        update();
        if (!IDBase64.empty()) {
            return IDBase64;
        }
    }
    return "";
}

} // namespace dxlib