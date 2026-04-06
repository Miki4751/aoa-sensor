#ifndef SERIAL_COMMANDS_H
#define SERIAL_COMMANDS_H

#include <Arduino.h>
#include "aoa_sensor.h"
#include "HardwareSerial.h"



#define CMD_BAUDRATE            115200
#define CMD_BUFFER_SIZE         128
#define CMD_MAX_ARGS            8
#define CMD_TIMEOUT             1000

// 命令返回码
enum CmdResult {
    CMD_OK = 0,
    CMD_ERROR = 1,
    CMD_INVALID_ARGS = 2,
    CMD_NOT_FOUND = 3,
    CMD_TIME_OUT = 4
};

// 命令函数指针类型
typedef CmdResult (*CmdFunction)(int argc, char* argv[]);

// 命令结构体
typedef struct {
    const char* name;           // 命令名称
    const char* help;           // 帮助信息
    CmdFunction func;           // 命令函数
} Command;

// 命令声明
CmdResult cmd_help(int argc, char* argv[]);
CmdResult cmd_status(int argc, char* argv[]);
CmdResult cmd_config(int argc, char* argv[]);
//CmdResult cmd_aoa_servo(int argc, char* argv[]);
CmdResult cmd_calibrate(int argc, char* argv[]);
CmdResult cmd_save(int argc, char* argv[]);
CmdResult cmd_load(int argc, char* argv[]);
CmdResult cmd_reset(int argc, char* argv[]);
CmdResult cmd_test(int argc, char* argv[]);
CmdResult cmd_version(int argc, char* argv[]);
CmdResult cmd_debug(int argc, char* argv[]);

// 配置子命令
CmdResult cmd_config_show(int argc, char* argv[]);
CmdResult cmd_config_set(int argc, char* argv[]);
CmdResult cmd_config_get(int argc, char* argv[]);

// 校准子命令
CmdResult cmd_calibrate_zero(int argc, char* argv[]);
CmdResult cmd_calibrate_full(int argc, char* argv[]);
CmdResult cmd_calibrate_auto(int argc, char* argv[]);

 

// 串口命令处理函数
void cmd_init(void);
void cmd_update(void);
void cmd_process_line(char* line);
int cmd_parse_args(char* line, char* argv[], int max_args);
void cmd_print_result(CmdResult result, const char* message = NULL);

// 工具函数
void cmd_print_prompt(void);
void cmd_print_aoa_config(void);
void cmd_print_status(void);
bool cmd_str_to_float(const char* str, float* value);
bool cmd_str_to_int(const char* str, int* value);
bool cmd_str_to_uint16(const char* str, uint16_t* value);
 bool cmd_str_to_bool(const char *str, bool *value);
 
// 命令表
extern Command commands[];

// 全局变量
extern char cmd_buffer[CMD_BUFFER_SIZE];
extern int cmd_buffer_pos;
extern unsigned long cmd_last_time;

#endif // SERIAL_COMMANDS_H
