#include <Arduino.h>
#include "aoa_sensor.h"
#include "serial_commands.h"
#include "aoa_msp.h"

HardwareSerial Serial1(PA10, PA9);

// 串口命令配置
#define CMD_SERIAL Serial1
#define MSP_SERIAL Serial2
// 系统状态变量
static unsigned long last_status_time = 0;
static unsigned long last_led_time = 0;
static bool led_state = false;
float currentAoa = 0.0f;
// LED指示灯引脚 (PC13)
#define LED_PIN PC13
#define ButtonPin PA5

MSP_AOA mspRangeAoa(MSP_SERIAL);

void setup()
{
    // 初始化LED
    pinMode(LED_PIN, OUTPUT);
    // pinMode(ButtonPin, INPUT);

    digitalWrite(LED_PIN, HIGH); // 初始状态熄灭

    // 初始化配置系统
    config_init();

    // 初始化MSP协议
    mspRangeAoa.begin();

    // 初始化串口命令系统
    cmd_init();

    // 设置ADC分辨率
    analogReadResolution(11); // 11位ADC (0-2047)
    // 启动提示
    CMD_SERIAL.println("=== AOA v2.0 ===");
    CMD_SERIAL.println("LED blink = OK");
    CMD_SERIAL.println(config.adc_pin);

    last_status_time = millis();
    last_led_time = millis();
}

void loop()
{
    // 更新串口命令处理
    cmd_update();

    if (millis() - last_status_time >= 1000 / config.msp_rate)
    {

        // currentR = random(500, 1500);
        // mspRangeAoa.sendRangefinderData(currentR);
        currentAoa = get_current_aoa();
        mspRangeAoa.sendAoaData(currentAoa);
        if (config.debug_mode)
        {

            CMD_SERIAL.println("\r\n=== TX TEST ===");
            CMD_SERIAL.print("ADC: ");
            CMD_SERIAL.println(get_current_adc());
            CMD_SERIAL.print("V: ");
            CMD_SERIAL.print(get_current_voltage(), 3);
            CMD_SERIAL.println("V");
            CMD_SERIAL.print("AOA: ");
            CMD_SERIAL.print(currentAoa, 2);
            CMD_SERIAL.println("deg");

            last_led_time = millis();
        }
        else
        {
            //     currentR = random(500, 1500);
            // mspRangeAoa.sendRangefinderData(currentR);
        }
        last_status_time = millis();
    }

    // if (millis() - last_status_time >= 3000)
    // {
    //     cmd_calibrate("calibrate", "aoa_mid");
    //     last_status_time = millis();
    // }

    // LED闪烁指示系统运行 (每1秒)
    if (millis() - last_led_time >= 1000)
    {
        led_state = !led_state;
        digitalWrite(LED_PIN, led_state ? LOW : HIGH); // 低电平点亮
        last_led_time = millis();
    }

    // 短暂延时避免CPU占用过高
    delay(10);
}
