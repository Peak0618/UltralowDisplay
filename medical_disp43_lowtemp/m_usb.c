#ifndef _USB_C_
#define _USB_C_
//------------------------------------------------------------------------------
//包含文件声明
#include "r_cg_macrodriver.h"

#include "h_type_define.h"

#include "m_usb.h"
#include "r_cg_serial.h"

//------------------------------------------------------------------------------
//函数声明
void com_init_usb(void);
void com_rx_init_usb(void);
void com_rx_data_deal_usb(void) ;
void com_tx_type_rank_usb(void);
void com_tx_init_usb(void);
void com_rx_delaytime_usb(void);
void com_tx_delaytime_usb(void);
void com_rx_end_delaytime_usb(void);
void com_fault_delaytime_usb(void);
void com_rx_int_usb(uint8_t rx_data) ;
void com_tx_int_usb(void);
uint8_t checksum (uint8_t *buffer, uint8_t size);
void USB_RX_UART0_Start(void);
void USB_RX_UART0_Stop(void);
void USB_TX_UART0_Start(void);
void USB_TX_UART0_Stop(void);
uint8_t func_timer_4302_deal(uint8_t luc_timer_tmp); //时间转换数据处理函数
uint8_t func_4302_timer_deal(uint8_t luc_4302_tmp);  //数据转换时间处理函数




//------------------------------------------------------------------------------
//标志定义
flag_type flg_com_usb,flg_com_usb_1;


//------------------------------------------------------------------------------
//变量定义
uint8_t guc_com_tx_buffer_usb[48];     //通讯发送缓存器
uint8_t guc_com_rx_buffer_usb[7];      //通讯接收缓存器
uint8_t guc_usb_buffer[31];            //显示板从主板中获取的发送给usb板的数据

int16_t gss_com_tx_delaytimer_usb;     //通讯发送延时计时器
int16_t gss_com_rx_delaytimer_usb;     //通讯接收延时计时器

int16_t gss_com_rx_end_delaytimer_usb; //通讯接收完标志
int16_t gss_com_fault_delaytimer_usb;  //通讯故障延时计时器

uint8_t guc_com_tx_point_usb;          //通讯发送索引
uint8_t guc_com_rx_point_usb;          //通讯接收索引


//usb的状态
uint8_t guc_Udisk_state;                //U盘的状态_只使用最低2位
uint8_t guc_usb_state;                  //usb板子的状态是否有故障


/***********************************************************************************************************************************************
整体概述: 与 USB 板子进行通讯，使用串口0和USB板连接通讯；

实际使用: u盘连接USB板子的USB接口，文件会自动存储到U盘中；
***********************************************************************************************************************************************/

//------------------------------------------------------------------------------
//程序内容
/***********************************************************************************************************************************************
函数描述:　串口0的初始化，使一开始就允许接收----从机模式

函数位置:  初始化中 system_init() 中；----------OK
***********************************************************************************************************************************************/
void com_init_usb(void)
{
    bflg_com_rx_delaytime_usb = 1;
    gss_com_rx_delaytimer_usb = 5;
    
    bflg_com_tx_delaytime_usb = 0;
    gss_com_tx_delaytimer_usb = 0;
}

/***********************************************************************************************************************************************
函数描述:　 接收前初始化，在接收串口允许接收数据前，将接收buffer清空；

函数位置:   主循环中------------------------------ok
***********************************************************************************************************************************************/
void com_rx_init_usb(void)   //通讯接收初始化程序，在主循环程序中调用
{
    uint8_t i;
    
    for (i = 0; i < 7; i++)  //13
    {
        guc_com_rx_buffer_usb[i] = 0;  //清接收缓存器
    }
    
    bflg_com_rx_busy_usb = 0;
    guc_com_rx_point_usb = 0;
}

/***********************************************************************************************************************************************
函数描述:　  串口接收数据成功时，对接收的数据进行处理；
             此处检测 [2]是 0x0A，且校验正确，则可发送数据

函数位置:   主循环中( bflg_com_rx_ok_usb==1)------------------------------ok
***********************************************************************************************************************************************/
void com_rx_data_deal_usb(void)   //通讯接收数据处理程序，在主循环程序中调用
{
    uint8_t i,cksum;
    uint16_t lus_tmp;
    word_type com_crc;
    //------------------------------------------------------

    if(guc_com_rx_buffer_usb[2] == 0x05)         //若果是数据请求命令,向usb板发送
    {
        cksum = checksum((guc_com_rx_buffer_usb+2), 4);  
        if(cksum == guc_com_rx_buffer_usb[6])
        {
            if(guc_com_rx_buffer_usb[4] == 1)
            {
                bflg_request_data_type = 1;
            }
            else if(guc_com_rx_buffer_usb[4] == 2)
            {
                bflg_request_time_type = 1;
            }
            else if(guc_com_rx_buffer_usb[4] == 3)
            {
                bflg_request_machine_type = 1;
            } 
            
            /*
            bflg_com_tx_delaytime_usb = 1;       //置发送延时标志
            gss_com_tx_delaytimer_usb = 10;      //发送延时计时器赋值  //peak 接收完成后延时10ms 开始发送
            
            bflg_com_rx_delaytime_usb = 0;       //清发送延时标志
            gss_com_rx_delaytimer_usb = 0;       //发送延时计时器清零
            
            bflg_com_fault_usb = 0;              //清通讯故障标志
            gss_com_fault_delaytimer_usb = 0;    //清通讯故障计时器
            */
        }
    }
    else if(guc_com_rx_buffer_usb[2] == 0x07)          //如果是U盘状态汇报，向主控板发送
    {
        cksum = checksum((guc_com_rx_buffer_usb+2), 4);  
        if(cksum == guc_com_rx_buffer_usb[6])
        {
            guc_Udisk_state = guc_com_rx_buffer_usb[4]; //U盘状态     =0空闲/1忙/2完成/失败
            guc_usb_state   = guc_com_rx_buffer_usb[5]; //usb板状态=0正常/1~ff故障
        }
    }
    else if((guc_com_rx_buffer_usb[2] & 0x80) == 0x80)            //如果是usb板的应答
    {
        cksum = checksum((guc_com_rx_buffer_usb+2), 3);  
        if(cksum == guc_com_rx_buffer_usb[5])
        {
           /* 
            if(guc_com_rx_buffer_usb[2] = 0x81)  //数据帧应答
            {
                if(guc_com_rx_buffer_usb[4] == 0x00)      //正确
                if(guc_com_rx_buffer_usb[4] == 0xff)      //错误
            }
            if(guc_com_rx_buffer_usb[2] = 0x82)  //事件帧应答
            if(guc_com_rx_buffer_usb[2] = 0x83)  //时间帧应答
            if(guc_com_rx_buffer_usb[2] = 0x84)  //控制帧应答
            if(guc_com_rx_buffer_usb[2] = 0x86)  //机型帧应答
            */

        }
    } 

    bflg_com_rx_delaytime_usb = 1;        //peak 处理完后延时一段时间后等待接收 ??
    gss_com_rx_delaytimer_usb = 50;       
}

/***********************************************************************************************************************************************
函数描述:　选择发送什么内容:1、回应请求帧(数据/时间/机型)  2、事件报警  3、控制命令
           有需要发送的才进行发送；---发送的发起位置
           
函数位置:  主循环中------------------------------------------ok
***********************************************************************************************************************************************/
void com_tx_type_rank_usb(void)

{
    //没有延时等待发送的/没有正在要发送的/没有正在忙着发送的 此时才能是否可以发送下个一类型
    if((bflg_com_tx_delaytime_usb != 1)&& (bflg_com_allow_tx_usb != 1)&&(bflg_com_tx_busy_usb == 0)) 
    {
        if(bflg_request_data_type == 1)          //数据请求帧
        { 
            bflg_request_data_type = 0;
            bflg_init_data_type = 1;
            
            bflg_com_tx_delaytime_usb = 1;       //置发送延时标志
            gss_com_tx_delaytimer_usb = 10;      //发送延时计时器赋值  //peak 接收完成后延时10ms 开始发送
            /*
            bflg_com_rx_delaytime_usb = 0;       //清发送延时标志
            gss_com_rx_delaytimer_usb = 0;       //发送延时计时器清零

            bflg_com_fault_usb = 0;              //清通讯故障标志
            gss_com_fault_delaytimer_usb = 0;    //清通讯故障计时器
            */
        }
        else if((bflg_request_time_type == 1)||((guc_usb_buffer[0] & 0x20) == 0x20))  //时间请求帧
        {   
            guc_usb_buffer[0] &= 0xdf;          //第5位(是否有时间修改设置)清0
            
            bflg_request_time_type = 0;         //usb有时间请求和主板有修改设置时间发同一数据      
            bflg_init_time_type = 1;          
            
            bflg_com_tx_delaytime_usb = 1;       
            gss_com_tx_delaytimer_usb = 10; 
        }
        else if(bflg_request_machine_type == 1)  //机型请求帧
        {
            bflg_request_machine_type = 0;
            bflg_init_machine_type = 1;

            bflg_com_tx_delaytime_usb = 1;      
            gss_com_tx_delaytimer_usb = 10; 
        }
        else if((guc_usb_buffer[0] & 0x80) == 0x80)     
        {
            guc_usb_buffer[0] &= 0x7f;               //第7位(是否有事件发生)清0
            bflg_init_event_type = 1;

            bflg_com_tx_delaytime_usb = 1;      
            gss_com_tx_delaytimer_usb = 10; 
        }
        else if((guc_usb_buffer[0] & 0x40) == 0x40)  //发送控制帧命令的内容 --从主板传输的
        {
            guc_usb_buffer[0] &= 0xbf;               //第6位(是否有控制指令)清0
            bflg_init_control_type = 1;
            
            bflg_com_tx_delaytime_usb = 1;      
            gss_com_tx_delaytimer_usb = 10; 
        }
    }
}
/***********************************************************************************************************************************************
函数描述:　 串口发送前的初始化，将要发送的数据放到发送buffer中；

函数位置:   主循环中 (bflg_com_allow_tx_usb == 1)---------------------------ok
***********************************************************************************************************************************************/
void com_tx_init_usb(void)   //通讯发送初始化程序，在主循环程序中调用
{
    word_type com_crc;
    word_type lss_tmp;

    if(bflg_init_data_type == 1)          //回复数据请求帧 17_bytes
    {
        bflg_init_data_type = 0;
        bflg_data_type_being_tx = 1;    
        
        guc_com_tx_buffer_usb[0] = 0x68;  //帧头
        guc_com_tx_buffer_usb[1] = 0x96;  
        guc_com_tx_buffer_usb[2] = 0x01;  //帧类型
        guc_com_tx_buffer_usb[3] = 0x0c;  //负载长度
        
        guc_com_tx_buffer_usb[4]  = guc_usb_buffer[1]; 
        guc_com_tx_buffer_usb[5]  = guc_usb_buffer[2];
        guc_com_tx_buffer_usb[6]  = guc_usb_buffer[3];
        guc_com_tx_buffer_usb[7]  = guc_usb_buffer[4];
        guc_com_tx_buffer_usb[8]  = guc_usb_buffer[5];
        guc_com_tx_buffer_usb[9]  = guc_usb_buffer[6];
        guc_com_tx_buffer_usb[10] = guc_usb_buffer[7];
        guc_com_tx_buffer_usb[11] = guc_usb_buffer[8];
        guc_com_tx_buffer_usb[12] = guc_usb_buffer[9];
        guc_com_tx_buffer_usb[13] = guc_usb_buffer[10];
        guc_com_tx_buffer_usb[14] = guc_usb_buffer[11];
        guc_com_tx_buffer_usb[15] = guc_usb_buffer[12];
        
        guc_com_tx_buffer_usb[16] = checksum((guc_com_tx_buffer_usb+2), 14);   //累加和校验
    }
    else if(bflg_init_time_type == 1)     //回复时间请求帧 11_bytes
    {
        bflg_init_time_type = 0;
        bflg_time_type_being_tx = 1;  

        guc_com_tx_buffer_usb[0] = 0x68;  //帧头
        guc_com_tx_buffer_usb[1] = 0x96;  
        guc_com_tx_buffer_usb[2] = 0x03;  //帧类型
        guc_com_tx_buffer_usb[3] = 0x06;  //负载长度

        guc_com_tx_buffer_usb[4] = func_timer_4302_deal(guc_usb_buffer[19]); //--- 待确认 17-->0x17
        guc_com_tx_buffer_usb[5] = func_timer_4302_deal(guc_usb_buffer[20]);
        guc_com_tx_buffer_usb[6] = func_timer_4302_deal(guc_usb_buffer[21]);
        guc_com_tx_buffer_usb[7] = func_timer_4302_deal(guc_usb_buffer[22]);
        guc_com_tx_buffer_usb[8] = func_timer_4302_deal(guc_usb_buffer[23]);
        guc_com_tx_buffer_usb[9] = func_timer_4302_deal(guc_usb_buffer[24]);

        guc_com_tx_buffer_usb[10] = checksum((guc_com_tx_buffer_usb+2), 8);   
    }
    else if(bflg_init_machine_type == 1)    //回复机型请求帧 7_bytes
    {
        bflg_init_machine_type = 0;
        bflg_machine_type_being_tx = 1;    

        guc_com_tx_buffer_usb[0] = 0x68;    //帧头
        guc_com_tx_buffer_usb[1] = 0x96;  
        guc_com_tx_buffer_usb[2] = 0x06;    //帧类型
        guc_com_tx_buffer_usb[3] = 0x02;    //负载长度

        guc_com_tx_buffer_usb[4] = guc_usb_buffer[29];
        guc_com_tx_buffer_usb[5] = guc_usb_buffer[30];

        guc_com_tx_buffer_usb[6] = checksum((guc_com_tx_buffer_usb+2), 4);   
    }
    else if(bflg_init_event_type == 1)      //事件报警 11bytes
    {
        bflg_init_event_type = 0;
        bflg_event_type_being_tx = 1;    

        guc_com_tx_buffer_usb[0] = 0x68;    //帧头
        guc_com_tx_buffer_usb[1] = 0x96;  
        guc_com_tx_buffer_usb[2] = 0x02;    //帧类型
        guc_com_tx_buffer_usb[3] = 0x06;    //负载长度

        guc_com_tx_buffer_usb[4] = guc_usb_buffer[13];
        guc_com_tx_buffer_usb[5] = guc_usb_buffer[14];
        guc_com_tx_buffer_usb[6] = guc_usb_buffer[15];
        guc_com_tx_buffer_usb[7] = guc_usb_buffer[16];
        guc_com_tx_buffer_usb[8] = guc_usb_buffer[17];
        guc_com_tx_buffer_usb[9] = guc_usb_buffer[18];

        guc_com_tx_buffer_usb[10] = checksum((guc_com_tx_buffer_usb+2), 8);
    }
    else if(bflg_init_control_type == 1)    //控制命令 9字节
    {
        bflg_init_control_type = 0;
        bflg_control_type_being_tx = 1;   

        guc_com_tx_buffer_usb[0] = 0x68;    //帧头
        guc_com_tx_buffer_usb[1] = 0x96;  
        guc_com_tx_buffer_usb[2] = 0x04;    //帧类型
        guc_com_tx_buffer_usb[3] = 0x04;    //负载长度

        guc_com_tx_buffer_usb[4] = guc_usb_buffer[25];    
        guc_com_tx_buffer_usb[5] = guc_usb_buffer[26];    
        guc_com_tx_buffer_usb[6] = guc_usb_buffer[27];    
        guc_com_tx_buffer_usb[7] = guc_usb_buffer[28];   

        guc_com_tx_buffer_usb[8] = checksum((guc_com_tx_buffer_usb+2), 6);
    }
    
    guc_com_tx_point_usb = 0;
    bflg_com_tx_busy_usb = 1;
    TXD0 = guc_com_tx_buffer_usb[guc_com_tx_point_usb++];
} 

/***********************************************************************************************************************************************
函数描述:　 多长时间之后 允许接收

函数位置:   1ms定时中------------------------------ok
***********************************************************************************************************************************************/
void com_rx_delaytime_usb(void)     //通讯接收延时程序，在1ms定时程序中调用
{
    if (bflg_com_rx_delaytime_usb == 1)
    {
        gss_com_rx_delaytimer_usb--;
        if (gss_com_rx_delaytimer_usb <= 0)
        {
            gss_com_rx_delaytimer_usb = 0;
            bflg_com_rx_delaytime_usb = 0;
            
            bflg_com_allow_rx_usb = 1;
        }
    }
}

/***********************************************************************************************************************************************
函数描述:　 多长时间后允许发送 

函数位置:   1ms定时中------------------------------ok
***********************************************************************************************************************************************/
void com_tx_delaytime_usb(void)     //通讯发送延时程序，在1ms定时程序中调用
{
    if (bflg_com_tx_delaytime_usb == 1)
    {
        gss_com_tx_delaytimer_usb--;
        if (gss_com_tx_delaytimer_usb <= 0)
        {
            gss_com_tx_delaytimer_usb = 0;
            bflg_com_tx_delaytime_usb = 0;
            
            bflg_com_allow_tx_usb = 1;
        }
    }
}

/***********************************************************************************************************************************************
函数描述:　在设定的接收时间当中，没有接收完，则退出接收；  

函数位置:   1ms定时中------------------------------ok
***********************************************************************************************************************************************/
void com_rx_end_delaytime_usb(void) //通讯接收完成延时程序，在1ms定时程序中调用
{
    if (bflg_com_rx_busy_usb == 1)  //如果当前接收忙
    {
        gss_com_rx_end_delaytimer_usb++;
        if (gss_com_rx_end_delaytimer_usb >= 55)    //peak 50--->55  现在发送48个字节
        {
            gss_com_rx_end_delaytimer_usb = 0;
            bflg_com_rx_busy_usb = 0;
            
            bflg_com_rx_end_usb = 1;
            
            bflg_com_rx_delaytime_usb = 1;
            gss_com_rx_delaytimer_usb = 10;
        }
    }
}
/***********************************************************************************************************************************************
函数描述:　  ，，，

函数位置:   1ms定时中------------------------------未调
***********************************************************************************************************************************************/
void com_fault_delaytime_usb(void)     //通讯故障延时程序，在1s定时程序中调用
{
    if (bflg_com_fault_usb == 0)
    {
        gss_com_fault_delaytimer_usb++;
        if (gss_com_fault_delaytimer_usb >= 30)  //30
        {
            gss_com_fault_delaytimer_usb = 0;
            bflg_com_fault_usb = 1;
        }
    }
}

/***********************************************************************************************************************************************
函数描述:　在系统中断接收中断中进行数据的接收  

函数位置:  系统的串口接收中断函数中------------------------------ok
***********************************************************************************************************************************************/
void com_rx_int_usb(uint8_t rx_data)   //通讯接收中断程序 串口接收中断中调用
{
    uint8_t luc_com_rx_byte;  //接收字节临时缓冲区
    
    gss_com_rx_end_delaytimer_usb = 0; //清接收完延时计时器
    //--------------------------------------------------
    luc_com_rx_byte = rx_data;
    
    if ((bflg_com_rx_busy_usb == 0) && (luc_com_rx_byte == 0x68))    //如果无接收忙且帧头正确
    {
        bflg_com_rx_busy_usb = 1;
        
        guc_com_rx_buffer_usb[0] = luc_com_rx_byte;
        guc_com_rx_point_usb = 1;
    }
    else if ((bflg_com_rx_busy_usb == 1))         //如果接收忙
    {
        guc_com_rx_buffer_usb[guc_com_rx_point_usb++] = luc_com_rx_byte;

        if(guc_com_rx_buffer_usb[1] == 0x96)
        {
            if ((guc_com_rx_buffer_usb[2] == 0x05) && (guc_com_rx_point_usb >= 7))      //如果是请求命令且接收完毕
            {
                guc_com_rx_point_usb = 0;         //清接收字节索引
                bflg_com_rx_busy_usb = 0;         //清接收忙标志
                bflg_com_rx_ok_usb = 1;           //置接收成功标志
            }
            else if ((guc_com_rx_buffer_usb[2] == 0x07) && (guc_com_rx_point_usb >= 7))  //如果是U盘状态且接收完毕
            {
                guc_com_rx_point_usb = 0;         
                bflg_com_rx_busy_usb = 0;        
                bflg_com_rx_ok_usb = 1;          
            }
            else if (((guc_com_rx_buffer_usb[2] & 0x80) == 0x80) && (guc_com_rx_point_usb >= 6))//如果是应答且接收完毕
            {
                guc_com_rx_point_usb = 0;        
                bflg_com_rx_busy_usb = 0;        
                bflg_com_rx_ok_usb = 1;          
            }
            else if(guc_com_rx_point_usb >= 7)   //防数值越界
            {
                guc_com_rx_point_usb = 0; 
                bflg_rx_data_error_usb = 1;
            }
        }
        else                                     //防[1]错
        {
            guc_com_rx_point_usb = 0; 
            bflg_rx_data_error_usb = 1;
        }
    }
}

/***********************************************************************************************************************************************
函数描述:　 在系统的发送中断中进行数据的发送； 

函数位置:   系统的串口发送中断函数中------------------------------ok
***********************************************************************************************************************************************/
void com_tx_int_usb(void)    //通讯发送中断程序  串口发送中断中调用
{
    if (bflg_com_tx_busy_usb == 1)
    {
        if(bflg_data_type_being_tx == 1)         //发送数据帧
        {
            if (guc_com_tx_point_usb < 17)
            {
                TXD0 = guc_com_tx_buffer_usb[guc_com_tx_point_usb++];  
            }
            else
            {
                bflg_data_type_being_tx = 0;
                guc_com_tx_point_usb = 0;   
                bflg_com_tx_busy_usb = 0;  
                bflg_com_tx_ok_usb = 1;
            }
        }
        else if(bflg_time_type_being_tx == 1)    //发送时间帧
        {
            if (guc_com_tx_point_usb < 11)
            {
                TXD0 = guc_com_tx_buffer_usb[guc_com_tx_point_usb++];  
            }
            else
            {
                bflg_time_type_being_tx = 0;
                guc_com_tx_point_usb = 0;   
                bflg_com_tx_busy_usb = 0;  
                bflg_com_tx_ok_usb = 1;
            }
        }
        else if(bflg_machine_type_being_tx == 1) //发送机型帧
        {
            if (guc_com_tx_point_usb < 7)
            {
                TXD0 = guc_com_tx_buffer_usb[guc_com_tx_point_usb++];  
            }
            else
            {
                bflg_machine_type_being_tx = 0;
                guc_com_tx_point_usb = 0;   
                bflg_com_tx_busy_usb = 0;  
                bflg_com_tx_ok_usb = 1;
            }
        }
        else if(bflg_event_type_being_tx == 1)   //发送报警事件
        {
            if (guc_com_tx_point_usb < 11)
            {
                TXD0 = guc_com_tx_buffer_usb[guc_com_tx_point_usb++];  
            }
            else
            {
                bflg_event_type_being_tx = 0;
                guc_com_tx_point_usb = 0;   
                bflg_com_tx_busy_usb = 0;  
                bflg_com_tx_ok_usb = 1;
            }
        }
        else if(bflg_control_type_being_tx == 1) //发送控制参数
        {
            if (guc_com_tx_point_usb < 9)
            {
                TXD0 = guc_com_tx_buffer_usb[guc_com_tx_point_usb++];  
            }
            else
            {
                bflg_control_type_being_tx = 0;
                guc_com_tx_point_usb = 0;   
                bflg_com_tx_busy_usb = 0;  
                bflg_com_tx_ok_usb = 1;
            }
        }
    }
}
/***********************************************************************************************************************************************
函数描述: 对usb板发送过来的U盘状态进行处理后再发送给主板(否则数据传输量会增大)

函数位置:    -----------------------------未调
***********************************************************************************************************************************************/
void usb_U_disk_state(void)
{
    
}


/***********************************************************************************************************************************************
函数描述:　累加和校验

函数位置:  本.c文件中的函数使用 ---------OK

 FF FF  0A 00 00 00 00 00 01 01 4D 01  5A  查询指令，USB板发给显示板的查询指令
***********************************************************************************************************************************************/
uint8_t checksum (uint8_t *buffer, uint8_t size)    //进行累加和校验
{
    uint8_t cksum=0;
    while (size>0)
    {
        cksum +=*buffer++;  // *(buffer+2)
        size--;
    }
    return cksum;
}

/***********************************************************************************************************************
函数功能: 串口0接收开始函数(为了使这个串口可以用作全双工，接收和发送的开始和接收分开)

函数位置:    ------------------------------未调
***********************************************************************************************************************/
void USB_RX_UART0_Start(void)
{
    //STIF0 = 0U;    /* clear INTST0 interrupt flag */  //清发送中断标志
    //STMK0 = 0U;    /* enable INTST0 interrupt */      //使能发送中断
    SRIF0 = 0U;    /* clear INTSR0 interrupt flag */  //清接收中断标志
    SRMK0 = 0U;    /* enable INTSR0 interrupt */      //使能接收中断
    SREIF0 = 0U;   /* clear INTSRE0 interrupt flag */ //清错误中断标志
    SREMK0 = 0U;   /* enable INTSRE0 interrupt */     //使能错误中断
    //SO0 |= _0001_SAU_CH0_DATA_OUTPUT_1;    /* output level normal */  //串口输出开始为高电平，因为低电平为起始位
    //SOE0 |= _0001_SAU_CH0_OUTPUT_ENABLE;    /* enable UART0 output */ //允许输出
    //SS0 |= _0002_SAU_CH1_START_TRG_ON | _0001_SAU_CH0_START_TRG_ON;    /* enable UART0 receive and transmit */
    SS0 |= _0002_SAU_CH1_START_TRG_ON;        //允许接收或发送，偶数位发送，奇数位接收
}

/***********************************************************************************************************************
函数功能: 串口0接收停止函数

函数位置:    ------------------------------未调
***********************************************************************************************************************/
void USB_RX_UART0_Stop(void)
{
    ST0 |= _0002_SAU_CH1_STOP_TRG_ON;    /* disable UART0 receive and transmit */
    //ST0 |= _0002_SAU_CH1_STOP_TRG_ON | _0001_SAU_CH0_STOP_TRG_ON;    /* disable UART0 receive and transmit */
    //SOE0 &= ~_0001_SAU_CH0_OUTPUT_ENABLE;    /* disable UART0 output */ //失能输出
    //STMK0 = 1U;    /* disable INTST0 interrupt */     //停止输出
    //STIF0 = 0U;    /* clear INTST0 interrupt flag */  //清发送中断标志
    SRMK0 = 1U;    /* disable INTSR0 interrupt */     //失能接收中断
    SRIF0 = 0U;    /* clear INTSR0 interrupt flag */  //清接收中断标志
    SREMK0 = 1U;   /* disable INTSRE0 interrupt */    //失能错误中断
    SREIF0 = 0U;   /* clear INTSRE0 interrupt flag */ //清错误中断标志
}


/***********************************************************************************************************************
函数功能: 串口0发送开始函数

函数位置:    ------------------------------未调
***********************************************************************************************************************/
void USB_TX_UART0_Start(void)
{
    STIF0 = 0U;    /* clear INTST0 interrupt flag */  //清发送中断标志
    STMK0 = 0U;    /* enable INTST0 interrupt */      //使能发送中断
    //SRIF0 = 0U;    /* clear INTSR0 interrupt flag */  //清接收中断标志
    //SRMK0 = 0U;    /* enable INTSR0 interrupt */      //使能接收中断
    //SREIF0 = 0U;   /* clear INTSRE0 interrupt flag */ //清错误中断标志
    //SREMK0 = 0U;   /* enable INTSRE0 interrupt */     //使能错误中断
    SO0 |= _0001_SAU_CH0_DATA_OUTPUT_1;    /* output level normal */  //串口开始为高电平，因为低电平为起始位
    SOE0 |= _0001_SAU_CH0_OUTPUT_ENABLE;    /* enable UART0 output */ //允许输出
    //SS0 |= _0002_SAU_CH1_START_TRG_ON | _0001_SAU_CH0_START_TRG_ON;    /* enable UART0 receive and transmit */
    SS0 |= _0001_SAU_CH0_START_TRG_ON;    /* enable UART0 receive and transmit */   
}

/***********************************************************************************************************************
函数功能: 串口0发送停止函数

函数位置:    ------------------------------未调
***********************************************************************************************************************/
void USB_TX_UART0_Stop(void)
{
    ST0 |=  _0001_SAU_CH0_STOP_TRG_ON;    /* disable UART0 receive and transmit */
    //ST0 |= _0002_SAU_CH1_STOP_TRG_ON | _0001_SAU_CH0_STOP_TRG_ON;    /* disable UART0 receive and transmit */
    SOE0 &= ~_0001_SAU_CH0_OUTPUT_ENABLE;    /* disable UART0 output */ //停止输出
    STMK0 = 1U;    /* disable INTST0 interrupt */     //失能发送中断
    STIF0 = 0U;    /* clear INTST0 interrupt flag */  //清发送中断标志
    //SRMK0 = 1U;    /* disable INTSR0 interrupt */     //失能接收中断
    //SRIF0 = 0U;    /* clear INTSR0 interrupt flag */  //清接收中断标志
    //SREMK0 = 1U;   /* disable INTSRE0 interrupt */    //失能错误中断
    //SREIF0 = 0U;   /* clear INTSRE0 interrupt flag */ //清错误中断标志
}



//------------------------------------------------------------------------------
//最新添加

/*******************************************************************************
函数描述:　增加补码变为原码的函数,主要用于发送数据时使用

函数位置:  --------------------------------未调
*******************************************************************************/



/*******************************************************************************
函数描述:　小端模式切换

函数位置:  --------------------------------未调
*******************************************************************************/




/*******************************************************************************
函数描述:　从RTC中读取年月日时分秒中移植过来的

函数位置:  --------------------------------未调
*******************************************************************************/
uint8_t func_timer_4302_deal(uint8_t luc_timer_tmp) //时间转换数据处理函数
{                                                             //peak 将要写入的时间转换为数据
	  uint8_t luc_tmp;                                        // 17 ----> 0x17
	  
	  luc_tmp = 0;
	  if (luc_timer_tmp >= 40)
	  {
	  	  luc_tmp |= 0x40;
	  	  luc_timer_tmp -= 40;
	  }
	  if (luc_timer_tmp >= 20)
	  {
	  	  luc_tmp |= 0x20;
	  	  luc_timer_tmp -= 20;
	  }
	  if (luc_timer_tmp >= 10)
	  {
	  	  luc_tmp |= 0x10;
	  	  luc_timer_tmp -= 10;
	  }
	  luc_tmp |= luc_timer_tmp;
	  
	  return luc_tmp;
}
//------------------------------------------------------------------------------
uint8_t func_4302_timer_deal(uint8_t luc_4302_tmp)  //数据转换时间处理函数
{                                                             //peak将读取的数据转换为时间_BCD码
	  uint8_t luc_tmp;                                        // 0x17 ----> 17
	  
	  luc_tmp = 0;
	  if ((luc_4302_tmp & 0x40) != 0)
	  {
	  	  luc_tmp += 40;
	  }
	  if ((luc_4302_tmp & 0x20) != 0)
	  {
	  	  luc_tmp += 20;
	  }
	  if ((luc_4302_tmp & 0x10) != 0)
	  {
	  	  luc_tmp += 10;
	  }
	  luc_4302_tmp &= 0x0F;
	  luc_tmp += luc_4302_tmp;
	  
	  return luc_tmp;
}


             
#endif   
/***************************************END OF THE FILE************************************************/
