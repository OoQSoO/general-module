#ifndef __DS1302_H__
#define __DS1302_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typedef.h>
#include <rtdevice.h>
#include <time.h>
#include "rtthread.h"

struct TIMEData
{
	u16 year;
	u8  month;
	u8  day;
	u8  hour;
	u8  minute;
	u8  second;
	u8  week;
};//创建TIMEData结构体方便存储时间日期数据

/**
 * @brief 设备初始化,使用rtc前必须先调用此函数
 * 
 */
extern void ds1302_init(void);

/**
 * @brief 更新rtc时间
 * 
 * @param time 
 */
extern void ds1032_writeTime(struct tm *time);

/**
 * @brief 获取rtc时间
 * 
 * @param data 
 */
extern void ds1032_get_Time(const TIMEData *data );

#endif