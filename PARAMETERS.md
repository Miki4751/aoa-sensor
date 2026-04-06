# AOA For Inav9.01 传感器参数说明及调参指南()

## 通信参数

| 参数 | 值 |
|------|-----|
| 串口波特率 | 115200 |
| 数据位 | 8 |
| 停止位 | 1 |
| 校验位 | 无 |

## 参数配置结构体

位于 `include/aoa_sensor.h`：

```cpp
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
```

---

## 参数详解

### 硬件配置

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `adc_pin` | uint8_t | PA5 | ADC引脚号，对应 开发板 编号（PA5=5） |

### 攻角范围配置

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `aoa_range` | float | 360.0 | 总量程，正负对半。例：360 = -180° ~ +180° |
| `aoa_mid` | float | 0.0 | 中位点偏移，用于修正机械安装位置 |
| `aoa_offset` | float | 0.0 | 攻角偏移，软件微调 |
| `aoa_polarity` | int8_t | 1 | 角度极性：1=正向，-1=反向 |

**量程示例：**
| aoa_range | 实际范围 |
|-----------|----------|
| 360 | -180° ~ +180° |
| 60 | -30° ~ +30° |
| 90 | -45° ~ +45° |

### MSP输出配置

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `msp_rate` | uint8_t | 50 | MSP发送频率 (1-100 Hz) |
| `msp_enabled` | bool | true | MSP输出使能 |

### 调试配置

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `debug_mode` | bool | false | 调试模式，开启后串口输出调试信息 |

---


## 调参指南


### 1. 基本调参流程

```bash
# 查看当前配置
config show

# 设置攻角色程范围（根据传感器实际范围）
config set aoa_range 360      # ±180°

# 设置偏移量
config set offset 5.0         # +5° 如果有必要


# 保存配置
save

```

### 2. 校准流程

#### 中位点校准
将传感器放置在中间位置，执行：
```bash
calibrate aoa_mid
save
config show

```

### 3. 极性调整

如果攻角方向相反，请执行以下命令：
```bash
config set aoa_polarity -1
save
```

### 4. MSP频率调整

根据飞控需求调整：
```bash
config set msp_rate 20    # 20Hz
config set msp_rate 50    # 50Hz
config set msp_rate 100   # 100Hz
save
```

### 5. 调试模式

开启后可在串口查看实时数据：
```bash
debug on     # 开启调试输出
debug off    # 关闭
```

---

## 命令速查表

### 基本命令
| 命令 | 说明 | 示例 |
|------|------|------|
| `help` | 显示帮助 | `help` |
| `status` | 系统状态 | `status` |
| `config show` | 显示配置 | `config show` |
| `save` | 保存配置 | `save` |
| `load` | 加载配置 | `load` |
| `reset` | 重置默认 | `reset` |
| `version` | 版本信息 | `version` |

### 校准命令
| 命令 | 说明 | 示例 |
|------|------|------|
| `calibrate aoa_mid` | 中位点校准 | `calibrate aoa_mid` |

### 配置命令
| 命令 | 说明 | 示例 |
|------|------|------|
| `config set aoa_range <v>` | 设置量程 | `config set aoa_range 60` |
| `config set offset <v>` | 设置偏移 | `config set offset 5.0` |
| `config set msp_rate <v>` | MSP频率 | `config set msp_rate 50` |
| `config set aoa_polarity <v>` | 极性 | `config set aoa_polarity 1` |
| `config set debug_mode <on/off>` | 调试模式 | `config set debug_mode on` |
| `config get aoa_range` | 获取量程 | `config get aoa_range` |
| `config get offset` | 获取偏移 | `config get offset` |

### 调试命令
| 命令 | 说明 | 示例 |
|------|------|------|
| `debug` | 查看状态 | `debug` |
| `debug on` | 开启调试 | `debug on` |
| `debug off` | 关闭调试 | `debug off` |
| `test 1` | 运行测试 | `test 1` |

---

## 滤波参数（固定）

滤波参数已固定为宏定义，无需调整：

| 参数 | 值 | 说明 |
|------|-----|------|
| `FILTER_SAMPLES` | 10 | 滤波采样数量 |
| `FILTER_ALPHA` | 0.3 | 滤波系数 (0-1) |

---

## 故障排除

| 问题 | 解决方法 |
|------|----------|
| 攻角方向相反 | `config set aoa_polarity -1` |
| 攻角范围不符 | 调整 `aoa_range` 或重新校准 |
| 数据跳动大 | 硬件问题，滤波参数已优化 |
| MSP无输出 | 检查 `msp_rate` 和 `msp_enabled` |
| 校准后不准 | 重新执行零点/满量程校准 |
