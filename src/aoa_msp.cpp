#include "aoa_msp.h"

// 构造函数：绑定硬件串口和波特率
MSP_AOA::MSP_AOA(HardwareSerial &serial, uint32_t baudRate)
    : _serial(serial), _baudRate(baudRate) {}

// 初始化串口
void MSP_AOA::begin()
{
    _serial.begin(_baudRate);
}

