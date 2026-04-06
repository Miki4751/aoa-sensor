# STM32 F103 攻角传感器

基于 STM32 F103 的攻角（AOA）传感器，通过 MSP 协议向 INAV 飞控发送数据。

---

## 1. 硬件接线

### 引脚定义

| 功能 | 引脚 | 说明 |
|------|------|------|
| 传感器输入 | PA0-PA7 | 模拟电压输入（默认 PA0） |
| MSP 输出 | PA9 (TX2) | 接飞控 RX |
| MSP 输入 | PA10 (RX2) | 接飞控 TX |
| 调参串口 | PA9/PA10 (TX1) | USB 转串口连电脑 |
| LED | PC13 | 状态指示 |

### 接线图

```
AOA 传感器                    STM32 F103              INAV 飞控
+--------+                  +---------+              +-------+
|  电压  |---- ADC 输入 --->| PA0-PA7 |              |       |
| 输出   |                  |         |              |       |
+--------+                  |         |   MSP 输出 -->| RX    |
                           |  PA9(TX2)|------------->|       |
                           |  PA10(RX2)|<------------| TX    |
                           |         |              |       |
                           |  USB    |              |       |
                           | (PA9/PA10)----> 电脑串口调试 |
                           +---------+              +-------+
```

### 传感器要求

- 输出电压范围：0~3.3V
- 推荐使用电位器或专用攻角传感器模块

---

## 2. 烧录固件

### 方式一：直接下载固件

从 Release 页面下载预编译的 `.bin` 文件：

```
.pio/build/genericSTM32F103C8mini/firmware.bin
```

使用 ST-Link 或串口工具烧录到开发板。

### 方式二：自行编译

```bash
# 克隆仓库
git clone https://github.com/Miki4751/aoa-sensor.git
cd aoa-sensor

# 编译
pio run -e genericSTM32F103C8mini

# 烧录（需要 ST-Link）
pio run -e genericSTM32F103C8mini --target upload
```

---

## 3. 配置调参

### 连接调参软件

1. 用 USB 转串口模块连接电脑与 STM32
2. 打开串口软件（Arduino IDE 串口监视器、PuTTY 等）
3. 波特率：**115200**
4. 发送命令进行配置

### 常用命令

```bash
# 查看帮助
help

# 查看当前配置
config show

# 查看系统状态
status
```

### 配置攻角范围

```bash
# 设置量程（总量程，正负对半）
# 例：60 = -30° ~ +30°
config set aoa_range 60
```

### 校准中位点

1. 将传感器机械中位点放置水平
2. 执行校准命令

```bash
calibrate aoa_mid
save
```

### 极性调整

如果攻角方向相反：

```bash
config set aoa_polarity -1   # 反向
config set aoa_polarity 1     # 正向
save
```

### 调整 MSP 频率

```bash
config set msp_rate 20    # 20Hz
config set msp_rate 50    # 50Hz（默认）
config set msp_rate 100   # 100Hz
save
```

### 调试模式

开启后串口输出实时数据：

```bash
debug on     # 开启
debug off    # 关闭
```

### 保存配置

所有配置需要保存才能生效：

```bash
save
```

---

## 4. INAV 飞控配置

1. 在 INAV 中将对应串口设置为 **MSP** 协议
2. 波特率设为 **115200**
3. 启用自定义传感器支持

攻角数据即可用于飞行姿态控制、失速警告等。

---

## 5. 故障排除

| 问题 | 解决方法 |
|------|----------|
| 串口无输出 | 检查波特率是否为 115200 |
| MSP 无数据 | 检查 PA9/PA10 与飞控连接 |
| 攻角方向反 | 执行 `config set aoa_polarity -1` |
| 数据跳动 | 检查传感器供电稳定性 |
| 配置丢失 | 执行 `save` 保存配置 |
