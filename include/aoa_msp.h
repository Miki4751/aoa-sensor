#ifndef AOA_MSP_H
#define AOA_MSP_H

#include "Arduino.h"

// MSP协议常量（根据INAV定义）
#define MSP2_SENSOR_AOA 0x1F08 // 0x1F01，测距仪数据命令
#define MSP_V2_FRAME_ID 0x3E   // MSPv2帧标识
#define MSP_DIR_TO_FC 0x00     // 数据方向：发送到飞控
#define MSP2_SENSOR_RANGEFINDER 0x1F01

struct MSPHeaderV2
{
    uint8_t startByte;    // '$'
    uint8_t startByte2;   // 'X'
    uint8_t direction;    // '>' 发送
    uint8_t flag;         // 标志位 0
    uint16_t command;     // MSP命令ID
    uint16_t payloadSize; // 数据大小
    uint8_t checksum;     // 校验和
};

// 测距仪数据结构（与INAV协议匹配，紧凑对齐）
typedef struct __attribute__((packed))
{
    int16_t aoa;      // 攻角，0.1度为单位
    int16_t sideslip; // 侧滑角，0.1度为单位
} mspSensorAoaData_t;

class MSP_AOA
{
private:
    HardwareSerial &_serial; // 硬件串口引用（如Serial1）
    uint32_t _baudRate;      // 波特率

    // 计算MSPv2帧的CRC校验值（内部辅助函数）

    uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a)
    {
        crc ^= a;
        for (int ii = 0; ii < 8; ++ii)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ 0xD5;
            }
            else
            {
                crc = crc << 1;
            }
        }
        return crc;
    }

    void sendMSPV2(uint16_t command, const uint8_t *payload, uint16_t payloadSize)
    {
        // MSP V2包头
        _serial.write('$');
        _serial.write('X');
        _serial.write('<');
        _serial.write(0); // flag
        _serial.write(command & 0xFF);
        _serial.write((command >> 8) & 0xFF);
        _serial.write(payloadSize & 0xFF);
        _serial.write((payloadSize >> 8) & 0xFF);

        uint8_t crc = 0;
        crc = crc8_dvb_s2(crc, 0);                         // flag 3
        crc = crc8_dvb_s2(crc, command & 0xFF);            // 4
        crc = crc8_dvb_s2(crc, (command >> 8) & 0xFF);     // 5
        crc = crc8_dvb_s2(crc, payloadSize & 0xFF);        // 6
        crc = crc8_dvb_s2(crc, (payloadSize >> 8) & 0xFF); // 7

        // 发送载荷并继续计算CRC
        for (uint16_t i = 0; i < payloadSize; i++)
        {
            _serial.write(payload[i]);
            crc = crc8_dvb_s2(crc, payload[i]);
        }

        // 发送最终的CRC8
        _serial.write(crc);
    }

public:
    // 构造函数：指定使用的硬件串口和波特率
    MSP_AOA(HardwareSerial &serial, uint32_t baudRate = 115200);

    // 初始化MSP通信（启动串口）
    void begin();

    void sendRangefinderData(int32_t distanceMm, uint8_t quality = 255)
    {
        uint8_t payload[5];
        payload[0] = quality;
        payload[1] = distanceMm & 0xFF;
        payload[2] = (distanceMm >> 8) & 0xFF;
        payload[3] = (distanceMm >> 16) & 0xFF;
        payload[4] = (distanceMm >> 24) & 0xFF;
        sendMSPV2(MSP2_SENSOR_RANGEFINDER, payload, sizeof(payload));
    }

    void sendAoaData(float aoaDegrees, float sideslipDegrees = 0.0f)
    {
        mspSensorAoaData_t data;
        data.aoa = (int16_t)(aoaDegrees * 10);
        data.sideslip = (int16_t)(sideslipDegrees * 10);

        sendMSPV2(MSP2_SENSOR_AOA, (uint8_t *)&data, sizeof(data));
    }
};

#endif // MSP_RANGFINDER_H