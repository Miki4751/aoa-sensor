#include "aoa_sensor.h"
#include <EEPROM.h>

// EEPROM地址定义
#define EEPROM_CONFIG_ADDR 0
#define EEPROM_CONFIG_SIZE sizeof(AOAConfig)
#define EEPROM_MAGIC_ADDR (EEPROM_CONFIG_ADDR + EEPROM_CONFIG_SIZE)
#define EEPROM_MAGIC_VALUE 0xA5A5

// 全局配置变量
AOAConfig config;
AOAConfig default_aoa_config = {
    .adc_pin = PA5,
    .aoa_range = 360.0f,
    .aoa_mid = 0.0f,
    .aoa_offset = 0.0f,
    .aoa_polarity = 1,
    .msp_rate = 50,
    .msp_enabled = true,
    .debug_mode = false
};

// 滤波固定参数
#define FILTER_SAMPLES 10
#define FILTER_ALPHA 0.3f

// 滤波变量
static uint16_t filter_buffer[FILTER_SAMPLES];
static uint8_t filter_index = 0;
static float filtered_aoa = 0.0f;

// AOA数据更新相关变量
static uint32_t last_aoa_update_time = 0;
static uint16_t current_adc_value = 0;
static float current_aoa_value = 0.0f;
static float current_voltage_value = 0.0f;
static bool aoa_data_ready = false;

void config_init(void)
{
    
    // 复制默认配置
    memcpy(&config, &default_aoa_config, sizeof(AOAConfig));

    // 尝试从EEPROM加载配置
    config_load();

    // 如果配置无效，使用默认配置
    if (!config_validate())
    {
        config_reset();
    }

    // 初始化滤波缓冲区
    for (int i = 0; i < FILTER_SAMPLES; i++)
    {
        filter_buffer[i] = 2048; // 中值
    }
}

void config_load(void)
{
    uint16_t magic;
    EEPROM.get(EEPROM_MAGIC_ADDR, magic);
    if (magic != EEPROM_MAGIC_VALUE) return;
    EEPROM.get(EEPROM_CONFIG_ADDR, config);
}

void config_save(void)
{
    EEPROM.put(EEPROM_CONFIG_ADDR, config);
    uint16_t magic = EEPROM_MAGIC_VALUE;
    EEPROM.put(EEPROM_MAGIC_ADDR, magic);
}

void config_reset(void)
{
    memcpy(&config, &default_aoa_config, sizeof(AOAConfig));
    config_save();
}

bool config_validate(void)
{
    if (config.adc_pin  < 0) return false;
    if (config.aoa_range <= 0) return false;
    if (config.msp_rate == 0 || config.msp_rate > 100) return false;
    return true;
}

void set_adc_pin(uint8_t pin)
{

    config.adc_pin = pin;
}

void set_aoa_range(float range)
{
    if (range > 0)
    {
        config.aoa_range = range;
    }
}

void set_msp_rate(uint8_t rate)
{
    if (rate > 0 && rate <= 100)
    {
        config.msp_rate = rate;
    }
}

void set_aoa_offset(float offset)
{
    config.aoa_offset = offset;
}

void set_aoa_polarity(int8_t polarity)
{
    if (polarity == 1 || polarity == -1)
    {
        config.aoa_polarity = polarity;
    }
}

void set_debug_mode(bool debug_mode)
{
    config.debug_mode = debug_mode;
}

float get_aoa_from_adc(uint16_t adc_value)
{

    float raw_angle = get_raw_angle_from_adc(adc_value);

    // 2. 以中位点为基准，转换为相对角度（可能为负）
    float relative_angle = raw_angle - config.aoa_mid;

    // 3. 处理360°循环（如270°→0°，260°→-10°，350°→-20°）
    if (relative_angle > 180.0f)
    {
        relative_angle -= 360.0f;
    }
    else if (relative_angle < -180.0f)
    {
        relative_angle += 360.0f;
    }

    // 4. 应用偏移量
    relative_angle += config.aoa_offset;

    float aoa_min = -config.aoa_range / 2.0f;
    float aoa_max = config.aoa_range / 2.0f;
    return constrain(relative_angle, aoa_min, aoa_max);
}

float get_raw_angle_from_adc(uint16_t adc_value)
{
    adc_value = constrain(adc_value, 0, 2047);
    float raw_ratio = (float)adc_value / 2047.0f;
    return raw_ratio * config.aoa_range;
}

uint16_t get_adc_raw(void)
{
    return analogRead(config.adc_pin);
}

float get_aoa_filtered(void)
{
    // 读取ADC值
    uint16_t adc_value = get_adc_raw();

    // 添加到滤波缓冲区
    filter_buffer[filter_index] = adc_value;
    filter_index = (filter_index + 1) % FILTER_SAMPLES;
    filter_buffer[filter_index] = adc_value;

    uint32_t sum = 0;
    for (int i = 0; i < FILTER_SAMPLES; i++)
    {
        sum += filter_buffer[i];
    }
    uint16_t avg_adc = sum / FILTER_SAMPLES;

    float aoa = get_aoa_from_adc(avg_adc);
    filtered_aoa = FILTER_ALPHA * aoa + (1.0f - FILTER_ALPHA) * filtered_aoa;

    // 应用角度极性
    return filtered_aoa * config.aoa_polarity;
}

// AOA数据更新函数（按频率调用）
void update_aoa_data(void)
{
    uint32_t current_time = millis();

    current_adc_value = get_adc_raw();
    current_voltage_value = (float)current_adc_value * 3.3f / 2047.0f;

    current_aoa_value = get_aoa_filtered();

    aoa_data_ready = true;
    last_aoa_update_time = current_time;
}

bool is_aoa_data_ready(void)
{
    return aoa_data_ready;
}

// 获取当前AOA值
float get_current_aoa(void)
{
    update_aoa_data();
    return current_aoa_value;
}

// 获取当前ADC值
uint16_t get_current_adc(void)
{
    return current_adc_value;
}

// 获取当前电压值
float get_current_voltage(void)
{
    return current_voltage_value;
}
