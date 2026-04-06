#include "serial_commands.h"
#include "aoa_sensor.h"
#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

#define CMD_SERIAL Serial1
// 外部配置变量声明
extern AOAConfig config;

// 全局变量
char cmd_buffer[CMD_BUFFER_SIZE];
int cmd_buffer_pos = 0;
unsigned long cmd_last_time = 0;

// 命令表定义
Command commands[] = {
    {"help", NULL, cmd_help},
    {"status", NULL, cmd_status},
    {"config", NULL, cmd_config},
    {"calibrate", NULL, cmd_calibrate},
    {"save", NULL, cmd_save},
    {"load", NULL, cmd_load},
    {"reset", NULL, cmd_reset},
    {"test", NULL, cmd_test},
    {"debug", NULL, cmd_debug},
    {"version", NULL, cmd_version},
    {NULL, NULL, NULL}
};

void cmd_init(void)
{
    CMD_SERIAL.begin(CMD_BAUDRATE);
    cmd_buffer_pos = 0;
    cmd_last_time = millis();

    CMD_SERIAL.println("\r\n=== AOA v2.0 ===");
    CMD_SERIAL.println("help");
    cmd_print_prompt();
}

void cmd_update(void)
{
    // 检查串口数据
    while (CMD_SERIAL.available())
    {
        char c = CMD_SERIAL.read();

        if (c == '\r' || c == '\n')
        {
            // 命令结束
            if (cmd_buffer_pos > 0)
            {
                cmd_buffer[cmd_buffer_pos] = '\0';
                cmd_process_line(cmd_buffer);
                cmd_buffer_pos = 0;
            }
            cmd_print_prompt();
        }
        else if (c == '\b' || c == 127)
        {
            // 退格键
            if (cmd_buffer_pos > 0)
            {
                cmd_buffer_pos--;
                CMD_SERIAL.print("\b \b");
            }
        }
        else if (c >= 32 && c < 127)
        {
            // 可打印字符
            if (cmd_buffer_pos < CMD_BUFFER_SIZE - 1)
            {
                cmd_buffer[cmd_buffer_pos++] = c;
                CMD_SERIAL.print(c);
            }
        }

        cmd_last_time = millis();
    }

    // 超时清空缓冲区
    if (cmd_buffer_pos > 0 && (millis() - cmd_last_time) > CMD_TIMEOUT)
    {
        cmd_buffer_pos = 0;
        CMD_SERIAL.println("\r\nTOUT");
        cmd_print_prompt();
    }
}

void cmd_process_line(char *line)
{
    char *argv[CMD_MAX_ARGS];
    int argc = cmd_parse_args(line, argv, CMD_MAX_ARGS);

    if (argc == 0)
        return;

    // 查找命令
    for (int i = 0; commands[i].name != NULL; i++)
    {
        if (strcmp(argv[0], commands[i].name) == 0)
        {
            CmdResult result = commands[i].func(argc, argv);
            if (result != CMD_OK)
            {
                cmd_print_result(result);
            }
            return;
        }
    }

    cmd_print_result(CMD_NOT_FOUND, "未知命令");
}

int cmd_parse_args(char *line, char *argv[], int max_args)
{
    int argc = 0;
    char *token = strtok(line, " \t");

    while (token != NULL && argc < max_args)
    {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }

    return argc;
}

void cmd_print_result(CmdResult result, const char *message)
{
    if (message)
    {
        CMD_SERIAL.println(message);
        return;
    }
    switch (result)
    {
    case CMD_ERROR:    CMD_SERIAL.println("ERR"); break;
    case CMD_INVALID_ARGS: CMD_SERIAL.println("ARG ERR"); break;
    case CMD_NOT_FOUND:   CMD_SERIAL.println("UNK CMD"); break;
    case CMD_TIME_OUT:    CMD_SERIAL.println("TOUT"); break;
    default: break;
    }
}

void cmd_print_prompt(void)
{
    CMD_SERIAL.print("\r\nAOA> ");
}

// 命令实现
CmdResult cmd_help(int argc, char *argv[])
{
    CMD_SERIAL.println("\r\nCMDS:");
    for (int i = 0; commands[i].name != NULL; i++)
    {
        CMD_SERIAL.println(commands[i].name);
    }
    return CMD_OK;
}

CmdResult cmd_status(int argc, char *argv[])
{
    cmd_print_status();
    return CMD_OK;
}

CmdResult cmd_config(int argc, char *argv[])
{
    if (argc < 2)
    {
        return cmd_config_show(argc, argv);
    }

    if (strcmp(argv[1], "show") == 0)
    {
        return cmd_config_show(argc, argv);
    }
    else if (strcmp(argv[1], "set") == 0)
    {
        return cmd_config_set(argc, argv);
    }
    else if (strcmp(argv[1], "get") == 0)
    {
        return cmd_config_get(argc, argv);
    }
    else
    {
        return CMD_INVALID_ARGS;
    }
}

CmdResult cmd_calibrate(int argc, char *argv[])
{
    if (argc < 2)
    {
        CMD_SERIAL.println("use: calibrate aoa_mid");
        return CMD_INVALID_ARGS;
    }

    if (strcmp(argv[1], "aoa_mid") == 0)
    {
        float sum = 0;
        for (int i = 0; i < 100; i++)
        {
            sum += get_raw_angle_from_adc(get_adc_raw());
            delay(10);
        }
        config.aoa_mid = sum / 100;
        config_save();
        CMD_SERIAL.print("aoa_mid: ");
        CMD_SERIAL.println(config.aoa_mid, 1);
        return CMD_OK;
    }
    return CMD_INVALID_ARGS;
}

CmdResult cmd_save(int argc, char *argv[])
{
    config_save();
    CMD_SERIAL.println("OK");
    return CMD_OK;
}

CmdResult cmd_load(int argc, char *argv[])
{
    config_load();
    CMD_SERIAL.println("OK");
    return CMD_OK;
}

CmdResult cmd_reset(int argc, char *argv[])
{
    config_reset();
    CMD_SERIAL.println("OK");
    return CMD_OK;
}

CmdResult cmd_test(int argc, char *argv[])
{
    if (argc < 2)
    {
        CMD_SERIAL.println("use: test <0|1>");
        return CMD_INVALID_ARGS;
    }

    if (cmd_str_to_bool(argv[1], (bool*)&argc))
    {
        CMD_SERIAL.println("\r\n=== TEST ===");

        if (is_aoa_data_ready())
        {
            CMD_SERIAL.print("ADC: ");
            CMD_SERIAL.println(get_current_adc());
            CMD_SERIAL.print("V: ");
            CMD_SERIAL.print(get_current_voltage(), 3);
            CMD_SERIAL.println("V");
            CMD_SERIAL.print("AOA: ");
            CMD_SERIAL.print(get_current_aoa(), 2);
            CMD_SERIAL.println("deg");
        }
    }
    return CMD_OK;
}

CmdResult cmd_version(int argc, char *argv[])
{
    CMD_SERIAL.println("AOA v1.1 STM32 INAV");
    return CMD_OK;
}

CmdResult cmd_debug(int argc, char *argv[])
{
    if (argc < 2)
    {
        CMD_SERIAL.print("debug: ");
        CMD_SERIAL.println(config.debug_mode ? "ON" : "OFF");
        return CMD_OK;
    }

    bool new_debug_mode;
    if (strcmp(argv[1], "on") == 0 || strcmp(argv[1], "1") == 0)
    {
        new_debug_mode = true;
    }
    else if (strcmp(argv[1], "off") == 0 || strcmp(argv[1], "0") == 0)
    {
        new_debug_mode = false;
    }
    else
    {
        CMD_SERIAL.println("use: debug [on|off|1|0]");
        return CMD_INVALID_ARGS;
    }

    if (config.debug_mode != new_debug_mode)
    {
        set_debug_mode(new_debug_mode);
        config_save();
    }
    CMD_SERIAL.println("OK");
    return CMD_OK;
}

// 配置子命令实现
CmdResult cmd_config_show(int argc, char *argv[])
{
    cmd_print_aoa_config();
    return CMD_OK;
}

CmdResult cmd_config_set(int argc, char *argv[])
{
    if (argc < 4)
    {
        CMD_SERIAL.println("use: config set <param> <value>");
        return CMD_INVALID_ARGS;
    }

    const char *param = argv[2];
    const char *value = argv[3];
    bool changed = false;

    if (strcmp(param, "aoa_range") == 0)
    {
        float v;
        if (cmd_str_to_float(value, &v) && v > 0)
        {
            config.aoa_range = v;
            changed = true;
        }
    }
    else if (strcmp(param, "offset") == 0)
    {
        float v;
        if (cmd_str_to_float(value, &v))
        {
            set_aoa_offset(v);
            changed = true;
        }
    }
    else if (strcmp(param, "msp_rate") == 0)
    {
        int v;
        if (cmd_str_to_int(value, &v) && v > 0 && v <= 100)
        {
            set_msp_rate(v);
            changed = true;
        }
    }
    else if (strcmp(param, "aoa_polarity") == 0)
    {
        int v;
        if (cmd_str_to_int(value, &v) && (v == 1 || v == -1))
        {
            set_aoa_polarity(v);
            changed = true;
        }
    }
    else if (strcmp(param, "debug_mode") == 0)
    {
        bool v;
        if (cmd_str_to_bool(value, &v))
        {
            set_debug_mode(v);
            changed = true;
        }
    }
    else
    {
        CMD_SERIAL.println("UNK param");
        return CMD_INVALID_ARGS;
    }

    if (changed) config_save();
    CMD_SERIAL.println(changed ? "OK" : "SAME");
    return CMD_OK;
}

CmdResult cmd_config_get(int argc, char *argv[])
{
    if (argc < 3)
    {
        CMD_SERIAL.println("use: config get <param>");
        return CMD_INVALID_ARGS;
    }

    const char *param = argv[2];

    if (strcmp(param, "aoa_range") == 0)
    {
        CMD_SERIAL.println(config.aoa_range, 2);
    }
    else if (strcmp(param, "offset") == 0)
    {
        CMD_SERIAL.println(config.aoa_offset, 2);
    }
    else if (strcmp(param, "msp_rate") == 0)
    {
        CMD_SERIAL.println(config.msp_rate);
    }
    else if (strcmp(param, "aoa_polarity") == 0)
    {
        CMD_SERIAL.println(config.aoa_polarity);
    }
    else if (strcmp(param, "debug_mode") == 0)
    {
        CMD_SERIAL.println(config.debug_mode ? "ON" : "OFF");
    }
    else
    {
        CMD_SERIAL.println("UNK param");
        return CMD_INVALID_ARGS;
    }
    return CMD_OK;
}

// 工具函数实现
void cmd_print_aoa_config(void)
{
    CMD_SERIAL.println("\r\n=== CFG ===");
    CMD_SERIAL.print("AOA: +/-");
    CMD_SERIAL.println(config.aoa_range / 2.0f, 1);
    CMD_SERIAL.print("mid: ");
    CMD_SERIAL.println(config.aoa_mid, 1);
    CMD_SERIAL.print("offset: ");
    CMD_SERIAL.println(config.aoa_offset, 1);
    CMD_SERIAL.print("polarity: ");
    CMD_SERIAL.println(config.aoa_polarity);
    CMD_SERIAL.print("msp_rate: ");
    CMD_SERIAL.println(config.msp_rate);
    CMD_SERIAL.print("msp: ");
    CMD_SERIAL.println(config.msp_enabled ? "ON" : "OFF");
    CMD_SERIAL.print("debug: ");
    CMD_SERIAL.println(config.debug_mode ? "ON" : "OFF");
}

void cmd_print_status(void)
{
    CMD_SERIAL.println("\r\n=== STATUS ===");
    CMD_SERIAL.print("time: ");
    CMD_SERIAL.print(millis() / 1000);
    CMD_SERIAL.println("s");
    CMD_SERIAL.print("ADC: ");
    CMD_SERIAL.println(get_adc_raw());
    CMD_SERIAL.print("V: ");
    CMD_SERIAL.print((float)get_adc_raw() * 3.3f / 2047.0f, 3);
    CMD_SERIAL.println("V");
    CMD_SERIAL.print("AOA: ");
    CMD_SERIAL.print(get_aoa_filtered(), 1);
    CMD_SERIAL.println("deg");
}

bool cmd_str_to_float(const char *str, float *value)
{
    char *endptr;
    *value = strtof(str, &endptr);
    return (endptr != str);
}

bool cmd_str_to_int(const char *str, int *value)
{
    char *endptr;
    *value = strtol(str, &endptr, 10);
    return (endptr != str);
}

bool cmd_str_to_uint16(const char *str, uint16_t *value)
{
    char *endptr;
    long val = strtol(str, &endptr, 10);
    if (endptr != str && val >= 0 && val <= 65535)
    {
        *value = (uint16_t)val;
        return true;
    }
    return false;
}

bool cmd_str_to_bool(const char *str, bool *value)
{
    if (str == NULL || value == NULL)
    {
        return false; // 输入指针无效，转换失败
    }

    // 处理数字形式："0" -> false，"1" -> true
    char *endptr;
    long val = strtol(str, &endptr, 10);
    if (endptr != str && *endptr == '\0')
    { // 确保完全转换（无多余字符）
        if (val == 0)
        {
            *value = false;
            return true;
        }
        else if (val == 1)
        {
            *value = true;
            return true;
        }
    }

    // 处理字符串形式："true"/"false"（不区分大小写）
    if (strcasecmp(str, "true") == 0)
    {
        *value = true;
        return true;
    }
    else if (strcasecmp(str, "false") == 0)
    {
        *value = false;
        return true;
    }

    // 所有情况均不匹配，转换失败
    return false;
}
