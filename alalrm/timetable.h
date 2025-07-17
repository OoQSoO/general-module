#ifndef __NAX_tt__
#define __NAX_tt__
#include "rtconfig.h"
#include <stdint.h>

#define ALARM_TOTAL_MAX 25  // 闹钟总数(app)
#define JSON_BUFFER_SIZE 1600  //
#define ALARM_PATH "/flash0/alarm.txt"

typedef struct {
    uint32_t time;      // 格式: hhmmss，例如 73015 表示 07:30:15
    uint8_t  repeat;    // 7位，每bit对应一周一日
    uint8_t  enabled;   // sta使能位
    uint8_t  measure;   // 投喂分量
} tt_plan_t;

typedef struct {
    uint32_t time;      // 格式: hhmmss
    uint8_t  enabled;   // sta使能位
    uint8_t  measure;   // 投喂分量
} tt_once_t;

typedef struct {
    union {
        tt_plan_t plan;
        tt_once_t once;
    };
    uint8_t is_once;  // 1 = once, 0 = plan
} tt_entry_t;

typedef struct {
    tt_entry_t list[ALARM_TOTAL_MAX];
    int count;
} tt_list_t;

typedef struct {
    uint32_t time;
    uint8_t  is_once;
    int index;
} tt_trigger_t;


/**
 * @brief 闹钟列表内存初始化
 * 
 */
extern void alarm_Malloc(void);

/**
 * @brief 获取闹钟文件json数据
 * 
 */
extern void alarm_ReadFile(void);

/**
 * @brief 解析json到列表
 * 
 * @return 
 */
extern int alarm_parse_json();

/**
 * @brief 查找最近的闹钟
 * 
 * @param alist 闹钟链表
 * @param now_sec 当前秒数
 * @param weekday 星期
 * @param trigger 闹钟承接
 * @return  
 */
extern int alarm_find_next(const tt_list_t *alist, int now_sec, int weekday, tt_trigger_t *trigger);

/**
 * @brief 关闭单次闹钟
 * 
 * @param alist 闹钟列表
 * @param trigger 需关闭的闹钟
 */
extern inline void alarm_handle_once_trigger(const tt_trigger_t *trigger);

/**
 * @brief 将闹钟列表数据重写回闹钟文件
 * 
 * @return buf
 */
extern void alarm_to_json_raw(void);

#endif
