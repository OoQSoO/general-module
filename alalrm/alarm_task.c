/**
 * @file state_machine.c
 * @author zxx
 * @brief 状态机
 * @version 0.1
 * @date 2025-06-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "alarm_task.h"
#include "timetable.h"
// #include "rtthread.h"
#include <rtthread.h>
#include <typedef.h>
#include <rtdef.h>
#include <stdio.h>

static void *alarm_Feed(void *def_arg, void *runtime_arg);
static void *alarm_Update(void *def_arg, void *runtime_arg);

static struct state_machine test[3] = {
    { .argc = NULL  , .func = alarm_Feed }, // 按键初始变量配置 GPIO
    { .argc = NULL  , .func = alarm_Update },
};

static alarm_manager_t alarm_info = {0};
static rt_mq_t alarm_queue = NULL; // 闹钟消息队列
static int *alarm_queuepool = NULL;       // 消息队列存储池


/**
 * @brief 投喂定时器回调
 * 
 * @param param 未使用
 */
static void alarm_FeedTimerCb(void *param)
{
    // 发送投喂消息
    int masg = ALARM_FEED;
    rt_mq_send(&alarm_info.alarm_queue,&masg,sizeof(masg));
    return;
}

/**
 * @brief 投喂闹钟业务处理
 * 
 * @param def_arg 未使用
 * @param runtime_arg 未使用
 * @return NULL 
 */
static void *alarm_Feed(void *def_arg, void *runtime_arg)
{
    int masg = 0;
    N_Deg("\n");
    // 运行电机
    Nax_Motor_Feed(MOTOR_RUN,dataLib_readIntForm(I_FEED_MEASURE));
    // 清空喂食数量
    dataLib_writeIntForm(I_FEED_MEASURE,0,1);

    if(tmp->onec_p)
    {
        // 修改单次闹钟状态位
        tmp->onec_p->sta = 0;
        // 将修改后的闹钟写入文件内,会发送ALARM_UPDATA
        timetable_modifyalarm(NULL);
    }

    return NULL;
}

/**
 * @brief 保底定时器回调
 * 
 * @param param 未使用
 */
static void alarm_FallbackTimerCb(void *param)
{
    // 发送投喂消息
    int masg = ALARM_UPDATE;
    rt_mq_send(&alarm_info.alarm_queue,&masg,sizeof(masg));
    return;
}

/**
 * @brief 重新获取闹钟链表
 * 
 * @param def_arg 未使用
 * @param runtime_arg 未使用
 * @return NULL
 */
static void *alarm_Update(void *def_arg, void *runtime_arg)
{
    alarm_info.alarm_list = timetable_getlist(NULL);
    return NULL;
}

/**
 * @brief 状态机处理线程
 * 
 * @param param 
 */
static void alarm_Task(void *param)
{
    int ret = 0;
    int recv_event = 0;

    // 扫描闹钟
    alarm_ReadFile();

    return ;
}


int alarm_init(void)
{
    // 闹钟内存申请
    alarm_Malloc();
    // 队列内存申请
    alarm_queue = malloc(sizeof(struct rt_messagequeue));
    if(alarm_queue)    memset(alarm_queue,0,sizeof(struct rt_messagequeue));

    // 队列消息存放内存申请
    alarm_queuepool = malloc(sizeof(int)*ALARM_QUEUESIZE);
    if(alarm_queue)    memset(alarm_queue,0,sizeof(int)*ALARM_QUEUESIZE);
    
    // 初始化消息队列
    rt_mq_init(alarm_queue,"alarm_queue",alarm_queuepool,sizeof(int),ALARM_QUEUESIZE,RT_IPC_FLAG_FIFO);
    // 创建线程
    return 0;
}