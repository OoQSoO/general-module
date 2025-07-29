#include "DS1302.h"
#define DATA 2
#define DATA_H rt_pin_write( DATA , PIN_HIGH );
#define DATA_L rt_pin_write( DATA , PIN_LOW );    
#define RST  4
#define RST_H rt_pin_write( RST , PIN_HIGH );
#define RST_L rt_pin_write( RST , PIN_LOW );
#define SCLK 7
#define SCLK_H rt_pin_write( SCLK , PIN_HIGH );
#define SCLK_L rt_pin_write( SCLK , PIN_LOW );
 
static struct TIMEData TimeData = {0};
static u8 read_time[7];

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

/**
 * @brief 十进制转为十六进制
 * 
 * @param num 
 * @return u8 
 */
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

/**
 * @brief CE,SCLK端口初始化
 * 
 */
static void ds1302_gpio_init(void)
{
    rt_pin_mode(RST,PIN_MODE_OUTPUT);
    rt_pin_mode(SCLK,PIN_MODE_OUTPUT);
}

/**
 * @brief 向DS1302发送一字节数据
 * 
 * @param data 
 */
static void ds1302_write_onebyte(u8 data)//
{
    rt_pin_mode(DATA,PIN_MODE_OUTPUT);
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

/**
 * @brief 向指定寄存器地址发送数据
 * 
 * @param address 寄存器地址
 * @param data    数据
 */
static void ds1302_wirte_rig(u8 address,u8 data)
{
    u8 temp1=address;
    u8 temp2=data;
    RST_L;SCLK_L;delay_us(1);
    RST_H;delay_us(2);
    ds1302_write_onebyte(temp1);
    ds1302_write_onebyte(temp2);
    RST_L;SCLK_L;delay_us(2);
}

/**
 * @brief 从指定地址读取一字节数据
 * 
 * @param address 寄存器地址
 * @return u8 数据
 */
static u8 ds1302_read_rig(u8 address)
{
    u8 temp3=address;
    u8 count=0;
    u8 return_data=0x00;
    RST_L;SCLK_L;delay_us(3);
    RST_H;delay_us(3);
    ds1302_write_onebyte(temp3);
    rt_pin_mode(DATA,PIN_MODE_INPUT);
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

/**
 * @brief 获取rtc时间
 * 
 */
static void ds1032_read_time(void)
{
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

/**
 * @brief rtc时间转为十进制
 * 
 */
static void ds1032_read_realTime(void)
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

void ds1302_init(void)
{
    ds1302_gpio_init();
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

    if(time->tm_sec >= 60) time->tm_sec = 59;
    sec = convertToHexFormat(time->tm_sec);

    if(time->tm_wday == 0) time->tm_wday = 7;
    wday  = convertToHexFormat(time->tm_wday);


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

void ds1032_get_Time(const TIMEData *data )
{
    ds1032_read_realTime();
    data = &TimeData;
}