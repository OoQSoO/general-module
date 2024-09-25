#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typedef.h>
#include <rtdevice.h>
#include <time.h>
#include "rtthread.h"
#define DATA 2
#define DATA_H rt_pin_write( DATA , PIN_HIGH );
#define DATA_L rt_pin_write( DATA , PIN_LOW );    
#define RST  4
#define RST_H rt_pin_write( RST , PIN_HIGH );
#define RST_L rt_pin_write( RST , PIN_LOW );
#define SCLK 7
#define SCLK_H rt_pin_write( SCLK , PIN_HIGH );
#define SCLK_L rt_pin_write( SCLK , PIN_LOW );
// #define CE_L GPIO_ResetBits(GPIOC,GPIO_Pin_11)//拉低使能位
// #define CE_H GPIO_SetBits(GPIOC,GPIO_Pin_11)//拉高使能位
// #define SCLK_L GPIO_ResetBits(GPIOC,GPIO_Pin_12)//拉低时钟线
// #define SCLK_H  GPIO_SetBits(GPIOC,GPIO_Pin_12)//拉高时钟线
// #define DATA_L  GPIO_ResetBits(GPIOC,GPIO_Pin_10)//拉低数据线
// #define DATA_H  GPIO_SetBits(GPIOC,GPIO_Pin_10)//拉高数据线
 
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
// extern struct TIMEData TimeData;//全局变量
// void ds1302_gpio_init();//ds1302端口初始化
// void ds1302_write_onebyte(u8 data);//向ds1302发送一字节数据
// void ds1302_wirte_rig(u8 address,u8 data);//向指定寄存器写一字节数据
// u8 ds1302_read_rig(u8 address);//从指定寄存器读一字节数据
// void ds1032_init();//ds1302初始化函数
// void ds1032_DATAOUT_init();//IO端口配置为输出
// void ds1032_DATAINPUT_init();//IO端口配置为输入
// void ds1032_read_time();//从ds1302读取实时时间（BCD码）
// void ds1032_read_realTime();//将BCD码转化为十进制数据

 
static struct TIMEData TimeData = {0};
u8 read_time[7];
 
 /*
	MCLK:26MHz, delay(1): about 25us
				delay(10):about 125us
				delay(100):about 850us
 */
static void delay_us(INT32 num)
{
    volatile INT32 i, j;

    for(i = 0; i < num; i ++)
    {
        for(j = 0; j < 100; j ++)
            ;
    }

}

static u8 convertToHexFormat(int num) {
    u8 result = 0,digit = 0;
    u8 multiplier = 1;
    while (num > 0) {
        digit = num % 10;
        result += digit * multiplier;
        num /= 10;
        multiplier *= 16;
    }
    return result;
}

static void ds1302_gpio_init()//CE,SCLK端口初始化
{
    // GPIO_InitTypeDef GPIO_InitStructure;
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; //PC.11  CE
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
    // GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC.11
    // GPIO_ResetBits(GPIOC,GPIO_Pin_11); 
    rt_pin_mode(RST,PIN_MODE_OUTPUT);
    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC.12  SCLK
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
    // GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC.12
    // GPIO_ResetBits(GPIOC,GPIO_Pin_12); 
    rt_pin_mode(SCLK,PIN_MODE_OUTPUT);
}
 
static void ds1032_DATAOUT_init()//配置双向I/O端口为输出态
{
    // GPIO_InitTypeDef GPIO_InitStructure;
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC.10  DATA
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    // GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC.10
    // GPIO_ResetBits(GPIOC,GPIO_Pin_10);
    rt_pin_mode(DATA,PIN_MODE_OUTPUT);
}
 
static void ds1032_DATAINPUT_init()//配置双向I/O端口为输入态
{
    // GPIO_InitTypeDef GPIO_InitStructure;
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC.10 DATA
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    // GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC.10
    rt_pin_mode(DATA,PIN_MODE_INPUT);
}
 
static void ds1302_write_onebyte(u8 data)//向DS1302发送一字节数据
{
    ds1032_DATAOUT_init();
    u8 count=0;
    SCLK_L;
    for (count=0;count<8;count++)
    {	
        SCLK_L;
        if(data&0x01)
        {DATA_H;}
        else{DATA_L;}//先准备好数据再发送
        SCLK_H;//拉高时钟线，发送数据
        data>>=1;
    }
}
 
static void ds1302_wirte_rig(u8 address,u8 data)//向指定寄存器地址发送数据
{
    u8 temp1=address;
    u8 temp2=data;
    RST_L;SCLK_L;delay_us(1);
    RST_H;delay_us(2);
    ds1302_write_onebyte(temp1);
    ds1302_write_onebyte(temp2);
    RST_L;SCLK_L;delay_us(2);
}
 
u8 ds1302_read_rig(u8 address)//从指定地址读取一字节数据
{
    u8 temp3=address;
    u8 count=0;
    u8 return_data=0x00;
    RST_L;SCLK_L;delay_us(3);
    RST_H;delay_us(3);
    ds1302_write_onebyte(temp3);
    ds1032_DATAINPUT_init();//配置I/O口为输入
    delay_us(2);
    for (count=0;count<8;count++)
    {
        delay_us(2);//使电平持续一段时间
        return_data>>=1;
        SCLK_H;delay_us(4);//使高电平持续一段时间
        SCLK_L;delay_us(14);//延时14us后再去读取电压，更加准确
        if (rt_pin_read(DATA)) {
            return_data=return_data|0x80;
        }
    }
    delay_us(2);
    RST_L;DATA_L;
    return return_data;
}
 
void ds1032_init()
{
    ds1302_gpio_init();
    ds1302_wirte_rig(0x8e,0x00);//关闭写保护
    ds1302_wirte_rig(0x80,0x45);//seconds37秒
    ds1302_wirte_rig(0x82,0x59);//minutes58分
    ds1302_wirte_rig(0x84,0x23);//hours23时
    ds1302_wirte_rig(0x86,0x20);//date30日
    ds1302_wirte_rig(0x88,0x09);//months9月
    ds1302_wirte_rig(0x8a,0x05);//days星期日
    ds1302_wirte_rig(0x8c,0x24);//year2020年
    ds1302_wirte_rig(0x8e,0x80);//关闭写保护
}
 


static void ds1032_read_time()
{
    ds1302_gpio_init();
    ds1302_wirte_rig(0x8e,0x00);//关闭写保护
    read_time[0]=ds1302_read_rig(0x81);//读秒
    read_time[1]=ds1302_read_rig(0x83);//读分
    read_time[2]=ds1302_read_rig(0x85);//读时
    read_time[3]=ds1302_read_rig(0x87);//读日
    read_time[4]=ds1302_read_rig(0x89);//读月
    read_time[5]=ds1302_read_rig(0x8B);//读星期
    read_time[6]=ds1302_read_rig(0x8D);//读年
    ds1302_wirte_rig(0x8e,0x80);//关闭写保护

}
 
static void ds1032_read_realTime()
{
    ds1032_read_time();  //BCD码转换为10进制
    TimeData.second=(read_time[0]>>4)*10+(read_time[0]&0x0f);
    TimeData.minute=((read_time[1]>>4)&(0x07))*10+(read_time[1]&0x0f);
    TimeData.hour=(read_time[2]>>4)*10+(read_time[2]&0x0f);
    TimeData.day=(read_time[3]>>4)*10+(read_time[3]&0x0f);
    TimeData.month=(read_time[4]>>4)*10+(read_time[4]&0x0f);
    TimeData.week=read_time[5];
    TimeData.year=(read_time[6]>>4)*10+(read_time[6]&0x0f)+2000;
}

void ds1032_writeTime(struct tm *time)
{
    // struct tm ttime = {0};
    u8 year = 0,mon = 0,mday = 0,hour = 0,min = 0,sec = 0,wday = 0;
    year = convertToHexFormat(time->tm_year-100);
    mon  = convertToHexFormat(time->tm_mon++);
    mday = convertToHexFormat(time->tm_mday);

    hour = convertToHexFormat(time->tm_hour);
    min = convertToHexFormat(time->tm_min);
    sec = convertToHexFormat(time->tm_sec);

    wday  = convertToHexFormat(time->tm_wday);


    ds1302_gpio_init();
    ds1302_wirte_rig(0x8e,0x00);//关闭写保护
    ds1302_wirte_rig(0x8c,(year&0xFF));//year2020年
    ds1302_wirte_rig(0x88,(mon&0xFF));//months9月
    ds1302_wirte_rig(0x86,(mday&0xFF));//date30日
    ds1302_wirte_rig(0x84,(hour&0xFF));//hours23时
    ds1302_wirte_rig(0x82,(min&0xFF));//minutes58分
    ds1302_wirte_rig(0x80,(sec&0xFF));//seconds37秒
    ds1302_wirte_rig(0x8a,(wday&0xFF));//days星期日
    ds1302_wirte_rig(0x8e,0x80);//关闭写保护
    ds1032_read_realTime();
}

void ds1032_get_Time(u8 *hour,u8 *min,u8 *sec)
{
    ds1032_read_realTime();
    *hour =  TimeData.hour;
    *min = TimeData.minute;
    *sec = TimeData.second;
}