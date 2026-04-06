#ifndef AOA_SENSOR_H
#define AOA_SENSOR_H

#include <Arduino.h>

// 攻角传感器配置参数结构体
struct AOAConfig {
    uint8_t adc_pin;              // ADC引脚 (PA0-PA7 对应 0-7)

    float aoa_range;              // 量程（总量程，正负对半），如 360 表示 -180° ~ +180°
    float aoa_mid;               // 中位点偏移（机械安装偏移）
    float aoa_offset;            // 攻角偏移（软件校准偏移）
    int8_t aoa_polarity;         // 角度极性：1=正向，-1=反向

    uint8_t msp_rate;             // MSP发送频率 (Hz)
    bool msp_enabled;             // MSP输出使能

    bool debug_mode;              // 调试模式
};

// 默认配置值
extern AOAConfig default_aoa_config;

// 全局配置变量
extern AOAConfig config;

// 配置管理函数
void config_init(void);
void config_load(void);
void config_save(void);
void config_reset(void);
bool config_validate(void);

// 参数设置函数
void set_adc_pin(uint8_t pin);
void set_aoa_range(float range);
void set_msp_rate(uint8_t rate);
void set_aoa_offset(float offset);
void set_aoa_polarity(int8_t polarity);
void set_debug_mode(bool debug_mode);

// 参数获取函数
float get_aoa_from_adc(uint16_t adc_value);
float get_raw_angle_from_adc(uint16_t adc_value);
uint16_t get_adc_raw(void);
float get_aoa_filtered(void);

// AOA数据更新函数（按频率调用）
void update_aoa_data(void);
bool is_aoa_data_ready(void);
float get_current_aoa(void);
uint16_t get_current_adc(void);
float get_current_voltage(void);

#endif // AOA_CONFIG_H
