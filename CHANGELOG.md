# 修改记录

## 2026-04-05

### AOAConfig 结构体精简优化

#### 删除的配置参数
- `adc_min`, `adc_max` - 使用 `calib_zero_adc/calib_full_adc` 替代
- `calib_mid_adc` - 未使用
- `filter_samples`, `filter_alpha` - 改为固定宏定义
- `version`, `checksum` - 简化校验逻辑

#### 代码变更
- 滤波参数改为 `#define FILTER_SAMPLES 10` 和 `#define FILTER_ALPHA 0.3f`
- `config_validate()` 简化
- `config_load()/config_save()` 移除校验和计算
- 移除 `set_filter_params()` 和 `config_checksum()` 函数

#### EEPROM 存储精简
- 配置结构体从 ~60 字节减少到 ~40 字节

---

### serial_commands.ino 精简优化

#### 1. 删除未使用的代码
- 删除 `cmd_calibrate_auto()` 函数（未被调用）
- 删除 `cmd_str_to_uint16()` 函数（未被使用）
- 删除 `bool begin` 冗余变量
- 删除注释掉的测试循环代码

#### 2. 精简汉字字符串
- 命令帮助信息：删除 help 文本，改为 `NULL`
- 欢迎信息：`AOA传感器系统 v1.0` → `AOA v1.1`
- 错误提示：全部改为英文简写

| 修改前 | 修改后 |
|--------|--------|
| 配置已保存到EEPROM | OK |
| 参数错误: 无效的参数 | ARG ERR |
| 命令错误: 未知命令 | UNK CMD |
| 命令超时 | TOUT |
| 调试模式已开启 | debug: ON |

#### 3. 代码简化
- 统一使用三元运算符简化条件输出
- 减少冗余变量声明
- 简化 `cmd_config_set` 函数逻辑

#### 4. 输出格式精简
- `cmd_print_aoa_config()`: `当前配置` → `CFG`
- `cmd_print_status()`: `系统状态` → `STATUS`
- 所有参数名称和数值改为简短英文

---

## 之前修改

### aoa_msp.h MSP协议更新

#### 结构体变更
```c
// 旧
typedef struct {
    uint8_t quality;
    int16_t aoa;
} mspSensorAOAData;

// 新
typedef struct {
    int16_t aoa;
    int16_t sideslip;
} mspSensorAoaData_t;
```

#### 函数签名变更
```cpp
// 旧
void sendAoaData(float aoaDegrees, uint8_t quality = 255)

// 新
void sendAoaData(float aoaDegrees, float sideslipDegrees = 0.0f)
```
