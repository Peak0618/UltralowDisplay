#ifndef _COM_C_
#define _COM_C_
//------------------------------------------------------------------------------
//包含文件声明
#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_port.h"
#include "r_cg_intc.h"
#include "r_cg_serial.h"
#include "r_cg_adc.h"
#include "r_cg_timer.h"
#include "r_cg_wdt.h"
#include "r_cg_pclbuz.h"
#include "r_cg_userdefine.h"

#include "h_type_define.h"
#include "h_main.h"
#include "h_com.h"
#include "m_card.h"
#include "m_pfdl.h"
#include "pfdl.h"
#include "m_test_mode.h"
#include "m_usb.h"
//------------------------------------------------------------------------------
//函数声明
void com_init(void);      //通讯初始化程序，在初始化程序中调用

void com_rx_init(void);  //通讯接收初始化程序，在主循环程序中调用

void com_rx_data_deal(void);  //通讯接收数据处理程序，在主循环程序中调用

void com_tx_init(void);         //通讯发送初始化程序，在主循环程序中调用

void com_rx_delaytime(void);  //通讯接收延时程序，在1ms定时程序中调用

void com_tx_delaytime(void);  //通讯发送延时程序，在1ms定时程序中调用

void com_rx_end_delaytime(void);   //通讯接收完成延时程序，在1ms定时程序中调用

void com_fault_delaytime(void);    //通讯故障延时程序，在1s定时程序中调用

void com_rx_int(uint8_t rx_data);       //通讯接收中断程序

void com_tx_int(void);                  //通讯发送中断程序

unsigned int CRC16(unsigned char *puchmsg, unsigned int usDataLen);    //进行CRC校验的程序
//------------------------------------------------------------------------------
//标志定义
flag_type flg_com;
//------------------------------------------------------------------------------
//变量定义
uint8_t guc_com_tx_buffer[9];  //通讯发送缓存器
uint8_t guc_com_rx_buffer[53];  //通讯接收缓存器  23

int16_t gss_com_tx_delaytimer;     //通讯发送延时计时器
int16_t gss_com_rx_delaytimer;     //通讯接收延时计时器

int16_t gss_com_rx_end_delaytimer; //通讯接收完标志
int16_t gss_com_fault_delaytimer;  //通讯故障延时计时器

uint8_t guc_com_tx_point;     //通讯发送索引
uint8_t guc_com_rx_point;     //通讯接收索引
//------------------------------------------------------------------------------
uint8_t guc_led_buffer[LED_BUF_LEN];    //数码管寄存器
uint8_t guc_key_buffer[KEY_BUF_LEN];    //按键寄存器

uint8_t guc_key_val[KEY_BUF_LEN];       //按键值  现在只用了 guc_key_val[0]

//------------------------------------------------------------------------------
//程序内容
//------------------------------------------------------------------------------
void com_init(void) //通讯初始化程序，在初始化程序中调用
{
    //----------------------------------
    guc_led_buffer[0] = 0xC0;

    guc_led_buffer[1] = 0xFF;
    guc_led_buffer[2] = 0xFF;
    
    guc_led_buffer[3] = 0xFF;
    guc_led_buffer[4] = 0xFF;
    guc_led_buffer[5] = 0xFF;
    guc_led_buffer[6] = 0xFF;
    guc_led_buffer[7] = 0xFF;
    guc_led_buffer[8] = 0xFF;
    guc_led_buffer[9] = 0xFF;
    guc_led_buffer[10] = 0xFF;
    guc_led_buffer[11] = 0xFF;
    guc_led_buffer[12] = 0xFF;
    
    bflg_write_led = 1;  //置写数码管显示标志
    
    bflg_led_allon_delaytime = 1;
    gss_led_allon_delaytimer = 0;
    //----------------------------------
    
    bflg_com_rx_delaytime = 1;
    gss_com_rx_delaytimer = 5;
    
    bflg_com_tx_delaytime = 0;
    gss_com_tx_delaytimer = 0;   
}
//------------------------------------------------------------------------------
void com_rx_init(void)   //通讯接收初始化程序，在主循环程序中调用  //peak 每次允许接收时调用一次
{
    uint8_t i;
    
    for (i = 0; i < 53; i++)      //23
    {
        guc_com_rx_buffer[i] = 0;  //清接收缓存器
    }
    
    bflg_com_rx_busy = 0;
    guc_com_rx_point = 0;
}
//------------------------------------------------------------------------------
void com_rx_data_deal(void)   //通讯接收数据处理程序，在主循环程序中调用
{
    uint8_t i, msb_byte = 0,lsb_byte = 0;
    uint16_t lus_tmp;
    word_type com_crc;
    //------------------------------------------------------
    if ((guc_com_rx_buffer[1] == 0x03)) //如果是查询命令
    {
        com_crc.uword = CRC16(guc_com_rx_buffer, 6);
        if ((com_crc.ubyte.low == guc_com_rx_buffer[6])
         && (com_crc.ubyte.high == guc_com_rx_buffer[7]))
        {
            bflg_com_tx_delaytime = 1;       //置发送延时标志
            gss_com_tx_delaytimer = 10;      //发送延时计时器赋值
            
            bflg_com_rx_delaytime = 0;       //清发送延时标志
            gss_com_rx_delaytimer = 0;       //发送延时计时器清零
            
            bflg_com_fault = 0;              //清通讯故障标志
            gss_com_fault_delaytimer = 0;    //清通讯故障计时器
            //----------------------------------------------
            //数据处理
            bflg_read_write_data = 0;   //当前是读数据
            
            //bflg_read_key = 1;       //置读按键标志
        }
    }
    else if (guc_com_rx_buffer[1] == 0x10)   //如果是多个设定命令
    {
        lus_tmp = guc_com_rx_buffer[6] + 7;
        
        com_crc.uword = CRC16(guc_com_rx_buffer, lus_tmp);
        if ((com_crc.ubyte.low == guc_com_rx_buffer[lus_tmp])
         && (com_crc.ubyte.high == guc_com_rx_buffer[lus_tmp + 1]))
        {
            bflg_com_tx_delaytime = 1;  //置发送延时标志
            gss_com_tx_delaytimer = 10; //发送延时计时器赋值
            
            bflg_com_rx_delaytime = 0;       //清发送延时标志
            gss_com_rx_delaytimer = 0;       //发送延时计时器清零
            
            bflg_com_fault = 0;              //清通讯故障标志
            gss_com_fault_delaytimer = 0;    //清通讯故障计时器
            //----------------------------------------------
            //数据处理
            bflg_read_write_data = 1;   //当前是写数据
            //----------------------------------------------
            if (bflg_led_allon_delaytime == 0)
            {
                for (i = 0; i < 12; i++)
                {
                    guc_led_buffer[i + 1] = guc_com_rx_buffer[i + 7];
                }
                guc_erasure_IC_all_data_order = guc_com_rx_buffer[19];     //peak 原来[19]没用，现在作为注销IC指令
                guc_lamp_val = guc_com_rx_buffer[20];
                
                bflg_write_led = 1;     //置写数码管显示标志
            }

            for(i = 0; i<31; i++)       //获取usb板所用的数据
            {
                guc_usb_buffer[i] = guc_com_rx_buffer[i + 20];  //获取给usb板的数据
            }
        }
    }
}
//------------------------------------------------------------------------------
void com_tx_init(void)   //通讯发送初始化程序，在主循环程序中调用
{
    word_type com_crc;      
    uint8_t  msb_byte = 0, lsb_byte = 0;
    //------------------------------------------------------
    if (bflg_read_write_data == 0) //如果是读数据  //peak 读取按键的值
    {
        guc_com_tx_buffer[0] = guc_com_rx_buffer[0];
        guc_com_tx_buffer[1] = guc_com_rx_buffer[1];
        guc_com_tx_buffer[2] = 4;  //字节个数4
        //--------------------------------------------------
        guc_com_tx_buffer[3] = guc_key_val[0];                //按键值
       
        //guc_com_tx_buffer[4] = guc_key_val[1];   //peak 原来   
        //guc_com_tx_buffer[5] = guc_key_val[2];         
        //guc_com_tx_buffer[6] = guc_key_val[3]; 
        //读存储在芯片内部的数据_IC的数量
        PFDL_Init();                
        PFDL_Read(0x801,1,&msb_byte);      //读达到的地址高字节
        PFDL_Read(0x802,1,&lsb_byte);      //读达到的地址低字节
        flash_address = 0;
        flash_address = (((flash_address | msb_byte)<<8) | lsb_byte); 
        PFDL_Close();
        msb_byte |= (uint8_t)(flash_address >> 8);
        lsb_byte |= (uint8_t)flash_address;      
        guc_com_tx_buffer[4] = msb_byte;
        guc_com_tx_buffer[5] = lsb_byte;                   //IC卡注册的地址 

        //buffer[6] 7:开锁 6：检测版本(所以正式显示板必为0) 5：未4:未3:未2:usb故障1:usb状态0：usb状态
        guc_com_tx_buffer[6]  = 0x00;  //要初始化，防止变为0x40使主板进入测试模式
        if(guc_IC_access_state == 1)
        {
            guc_IC_access_state = 0;
            guc_com_tx_buffer[6]  |= 0x80;                //使用buffer[6]的最高位代表开锁指令
        }
        else
        {
            guc_com_tx_buffer[6]  &= 0x7F;                 
        }
        
        guc_com_tx_buffer[6] |= (uint8_t)(guc_Udisk_state&0x03);       //使用buffer[6]的第0、1两位表示U盘状态
        if(guc_usb_state != 0x00)    
        {
            guc_com_tx_buffer[6] |= 0x04;                              //使用第2位表示usb板是否有故障
        }
        else
        {
            guc_com_tx_buffer[6] &= (~0x04);                         
        }
        //--------------------------------------------------
        com_crc.uword = CRC16(guc_com_tx_buffer, 7);
        guc_com_tx_buffer[7] = com_crc.ubyte.low;
        guc_com_tx_buffer[8] = com_crc.ubyte.high;
    }
    else  //如果是写数据 //设置数码管、LED显示
    {
        guc_com_tx_buffer[0] = guc_com_rx_buffer[0];  //peak从设备地址
        guc_com_tx_buffer[1] = guc_com_rx_buffer[1];  //功能码
        guc_com_tx_buffer[2] = guc_com_rx_buffer[2];   
        guc_com_tx_buffer[3] = guc_com_rx_buffer[3];
        guc_com_tx_buffer[4] = guc_com_rx_buffer[4];
        guc_com_tx_buffer[5] = guc_com_rx_buffer[5];
        //--------------------------------------------------
        com_crc.uword = CRC16(guc_com_tx_buffer, 6);
        guc_com_tx_buffer[6] = com_crc.ubyte.low;
        guc_com_tx_buffer[7] = com_crc.ubyte.high;
    }
    
    guc_com_tx_point = 0;
    bflg_com_tx_busy = 1;
    TXD1 = guc_com_tx_buffer[guc_com_tx_point++];
}
//------------------------------------------------------------------------------
void com_rx_delaytime(void)     //通讯接收延时程序，在1ms定时程序中调用
{
    if (bflg_com_rx_delaytime == 1)
    {
        gss_com_rx_delaytimer--;
        if (gss_com_rx_delaytimer <= 0)
        {
            gss_com_rx_delaytimer = 0;
            bflg_com_rx_delaytime = 0;
            
            bflg_com_allow_rx = 1;
        }
    }
}
//------------------------------------------------------------------------------
void com_tx_delaytime(void)     //通讯发送延时程序，在1ms定时程序中调用
{
    if (bflg_com_tx_delaytime == 1)
    {
        gss_com_tx_delaytimer--;
        if (gss_com_tx_delaytimer <= 0)
        {
            gss_com_tx_delaytimer = 0;
            bflg_com_tx_delaytime = 0;
            
            bflg_com_allow_tx = 1;
        }
    }
}
/*******************************************************************************************************************************
函数功能：此函数判断每一个字节是否超时，因为gss_com_rx_end_delaytimer = 0 放在com_rx_int()中
          ？？实际用处应该是判断一帧数据是否超时吧？？

函数位置：1ms定时器中-------------------ok
********************************************************************************************************************************/
void com_rx_end_delaytime(void) //通讯接收完成延时程序，在1ms定时程序中调用
{
    if (bflg_com_rx_busy == 1)  //如果当前接收忙
    {
        gss_com_rx_end_delaytimer++;
        if (gss_com_rx_end_delaytimer >= 50) // >= 2   //调试
        {
            gss_com_rx_end_delaytimer = 0;
            bflg_com_rx_busy = 0;
            
            bflg_com_rx_end = 1;
            
            bflg_com_rx_delaytime = 1;
            gss_com_rx_delaytimer = 10;
        }
    }
}
//------------------------------------------------------------------------------
void com_fault_delaytime(void)     //通讯故障延时程序，在1s定时程序中调用
{
    if (bflg_com_fault == 0)
    {
        gss_com_fault_delaytimer++;
        if (gss_com_fault_delaytimer >= 30)  //30
        {
            gss_com_fault_delaytimer = 0;
            bflg_com_fault = 1;
        }
    }
}
/*******************************************************************************************************************************
函数功能：接收数据，此函数在系统串口中断中，这个中断每中断一次是接收一个字节

函数位置：在系统串口接收中断中-------------------ok
********************************************************************************************************************************/
void com_rx_int(uint8_t rx_data)   //通讯接收中断程序
{
    uint8_t luc_com_rx_byte;  //接收字节临时缓冲区
    
    gss_com_rx_end_delaytimer = 0; //清接收完延时计时器
    //--------------------------------------------------
    luc_com_rx_byte = rx_data;
    
    if ((bflg_com_rx_busy == 0) && (luc_com_rx_byte == 0x00))    //如果无接收忙，且从地址正确
    {
        bflg_com_rx_busy = 1;
        
        guc_com_rx_buffer[0] = luc_com_rx_byte;
        guc_com_rx_point = 1;
    }
    else if (bflg_com_rx_busy == 1)     //如果接收忙
    {
        guc_com_rx_buffer[guc_com_rx_point++] = luc_com_rx_byte;
        
        if ((guc_com_rx_buffer[1] == 0x03) && (guc_com_rx_point >= 8))     //如果是查询命令，且接收数据完毕
        {
            guc_com_rx_point = 0;  //清接收字节索引
            bflg_com_rx_busy = 0;  //清接收忙标志
            bflg_com_rx_ok = 1;    //置接收成功标志
            
            bflg_com_rx_delaytime = 1;   //置接收延时标志
            gss_com_rx_delaytimer = 50;  //接收延时计时器赋值
        }
        else if ((guc_com_rx_buffer[1] == 0x10) && (guc_com_rx_point >= 53))  //23   //如果是多个设定命令，且接收到字节数数据帧
        {
            guc_com_rx_point = 0;   
            bflg_com_rx_busy = 0;   
            bflg_com_rx_ok = 1;    
            
            bflg_com_rx_delaytime = 1;  
            gss_com_rx_delaytimer = 50;  
        }
        else if ((guc_com_rx_buffer[1] == 0xAD) && (guc_com_rx_point >= 1))     //如果是测试模式
        {
            bflg_test_mode = 1;       //测试模式  进入测试模式后则为主机模式

            guc_com_rx_point = 0;     //清接收字节索引
            bflg_com_rx_busy = 0;     //清接收忙标志

            R_UART1_Stop();
            bflg_com_tx_delaytime = 1;    
            gss_com_tx_delaytimer = 100;   
            
        }
        else if(guc_com_rx_point >= 53)
        {
            guc_com_rx_point = 0;   
            bflg_com_rx_busy = 0;   
            bflg_com_rx_ok = 1;    
            
            bflg_com_rx_delaytime = 1;  
            gss_com_rx_delaytimer = 50;  
        }
    }
}
//------------------------------------------------------------------------------
void com_tx_int(void)    //通讯发送中断程序
{
    if (bflg_com_tx_busy == 1)
    {                                   //包含开锁指令 buff[6]的最高位代表开锁指令
        if (bflg_read_write_data == 0)  //如果是读数据 peak 主控向显示板读取按键值，此时显示板向主控返回按键值
        {
            if (guc_com_tx_point < 9)
            {
                TXD1 = guc_com_tx_buffer[guc_com_tx_point++];    //发送数据
            }
            else
            {
                guc_com_tx_point = 0;   //清发送字节索引
                bflg_com_tx_busy = 0;   //清发送忙标志
                bflg_com_tx_ok = 1;
                
                bflg_com_rx_delaytime = 1;   //置com通讯接收延时标志
                gss_com_rx_delaytimer = 5;   //com通讯接收延时计时器赋值
            }
        }
        else    //如果是写数据 peak主控向显示板写数码管显示的内容
        {
            if (guc_com_tx_point < 8)
            {
                TXD1 = guc_com_tx_buffer[guc_com_tx_point++];   //发送数据
            }
            else
            {
                guc_com_tx_point = 0;   //清发送字节索引
                bflg_com_tx_busy = 0;   //清发送忙标志
                bflg_com_tx_ok = 1;
                
                bflg_com_rx_delaytime = 1;   //置通讯接收延时标志
                gss_com_rx_delaytimer = 5;   //通讯接收延时计时器赋值
            }
        }
    }
}
//------------------------------------------------------------------------------
unsigned int CRC16(unsigned char *puchmsg, unsigned int usDataLen)    //进行CRC校验的程序程序
{
    unsigned int uchCRC;
    unsigned int uIndex_x, uIndex_y;
    
    uchCRC = 0xFFFF;
    
    if (usDataLen > 0)
    {
        for (uIndex_x = 0; uIndex_x <= (usDataLen - 1); uIndex_x++)
        {
            uchCRC = uchCRC ^ (unsigned int) (*puchmsg);
            puchmsg++;
            
            for (uIndex_y = 0; uIndex_y <= 7; uIndex_y++)
            {
                if ((uchCRC & 0x0001) != 0)
                {
                    uchCRC = (uchCRC >> 1) ^ 0xA001;
                }
                else
                {
                    uchCRC = uchCRC >> 1;
                }
            }
        }
    }
    else
    {
        uchCRC = 0;
    }
    
    return uchCRC;
}





//------------------------------------------------------------------------------
#endif
