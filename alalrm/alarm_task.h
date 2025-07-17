#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include <stdint.h>
#include "typedef.h"

#define ALARM_QUEUESIZE 4

enum{
    SM_KEY1 = (0x1 << 1),
    SM_KEY2 = (0x1 << 2),
    SM_KEY3 = (0x1 << 3),
    OFFLINE_PLAYER_ALL_EVENT = 0x1FFFF,
};

enum{
    ALARM_FEED = 0  ,   // 定时器回调使用
    ALARM_UPDATE    ,
    ALARM_MODIFY    ,
};

typedef struct state_machine
{
    void *argc; // 初始化时设置,
	void *(*func)(void *def_arg, void *runtime_arg);
} state_machine;

typedef struct alarm_manager
{
    struct rt_timer  feedtime ;  // 投喂定时器句柄
    struct rt_timer  twodayime ; // 保底定时器句柄
    time_t cur_sec ;    // 当前秒数
    time_t wakeup ;     // 唤醒秒数
    time_t interval ;   // 定时器毫秒数
    time_t prv_sec ;    // 上一个闹钟的秒数
    u8 hour ;
    u8 min ;
    u8 sec ;
    char feed_flag ;
    Nax_TimeList_p alarm_list ;    // 闹钟链表
}alarm_manager_t, *palarm_manager_t;

/**
 * @brief 闹钟状态机初始化
 * 
 * @return int 
 */
extern int alarm_init(void);
#endif