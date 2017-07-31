#ifndef _MAIN_C_
#define _MAIN_C_
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

//peak add
#include "m_card.h"
#include "m_usb.h"
#include "m_test_mode.h"
//------------------------------------------------------------------------------
//函数声明
void main_loop(void);    //主循环程序

void timer_op(void);     //定时程序，在主循环程序中调用

void led_key_deal(void); //显示按键处理程序，在10ms定时程序中调用

void key_buzz_deal(void);//按键蜂鸣处理程序，在主循环程序中调用

void key_buzz_delaytime(void);//按键蜂鸣延时程序，在10ms多少程序中调用

void led_allon_delaytime(void);    //led全亮延时程序，在100ms定时程序中调用

void timer_int(void);    //1ms定时中断程序

void iic_int(void);      //iic模拟定时中端程序

uint8_t TEST(uint8_t luc_data, uint8_t luc_bit);

uint8_t SET(uint8_t luc_bit);

uint8_t CLR(uint8_t luc_bit);

void lamp_output_deal(void);  //灯输出处理程序，在主循环程序中调用

//------------------------------------------------------------------------------
//宏定义
//#define   COM_RX_MODE    (P12.0 = 0)    //接收模式    移到.h
//#define   COM_TX_MODE    (P12.0 = 1)    //发送模式

#define   IIC_CLK   P6.0
#define   IIC_DIO   P6.1

#define   H_LEVEL   1
#define   L_LEVEL   0

#define   LAMP1_PIN      P1.0
#define   LAMP2_PIN      P1.5
#define   LAMP3_PIN      P3.0

//------------------------------------------------------------------------------
//变量定义
flag_type flg_time;
//----------------------------------------------------------
uint8_t   guc_5ms_timer;      //5毫秒计时器
uint8_t   guc_10ms_timer;     //10毫秒计时器
uint8_t   guc_100ms_timer;    //100ms定时器
uint8_t   guc_1s_timer;       //1s定时器
uint8_t   guc_1min_timer;     //1min定时器

uint8_t   guc_1638_write_stage;
uint8_t   guc_1638_read_stage;
uint8_t   guc_iic_wait_cnt;
uint8_t   guc_iic_bit;
uint8_t   guc_iic_index;

uint8_t   guc_key_buzz_buffer;
int16_t   gss_key_buzz_delaytimer;

int16_t   gss_led_allon_delaytimer;

uint8_t   guc_lamp_val;        //报警灯值
uint8_t   guc_IC_tx_ok_count;  //计数发送给主板成功的次数，防止没有发送开锁指令而被清0，所以多发几次

//------------------------------------------------------------------------------
//程序内容
//------------------------------------------------------------------------------
void system_init(void)   //系统初始化程序
{
    //card_flash_init();       //ic 卡flash初始化
    //----------------------------------
    R_TAU0_Channel7_Start();   //peak 定时器开始
    //----------------------------------
    //R_PCLBUZ0_Start();
    //----------------------------------
    com_init();     //和主板串口通讯初始化程序
    com_init_usb(); //和USB板的串口初始化
    //----------------------------------
    IIC_CLK = H_LEVEL;        //peak IIC上拉
    IIC_DIO = H_LEVEL;
    guc_1638_write_stage = 1;
    guc_1638_read_stage = 1;
    //----------------------------------
    bflg_key_buzz_delaytime = 1;
    gss_key_buzz_delaytimer = 0;

    //debug
    //R_PCLBUZ0_Start();       //peak  蜂鸣器开始
    //----------------------------------
    guc_lamp_val = 0xFF;     //peak 报警灯值


    //debug
    //R_TAU0_Channel2_Start();  // 定时器2的中断
}
//------------------------------------------------------------------------------
void main_loop(void)     //主循环程序
{
    timer_op();              //定时程序

    //--------------------------------------------------------
    if(bflg_test_mode == 1)      //如果是测试模式
    {   
        //bflg_write_led = 1;
        test_mode_deal();
        //test_mode_error_code_set();       
        test_mode_error_code_deal();
        test_mode_error_code_view(); 
                
        key_buzz_deal();//按键蜂鸣处理程序
    }
    else
    {
        //------------------------------------------------------
        //主控板通讯程序
        if (bflg_com_allow_rx == 1)    //如果允许接收
        {
            bflg_com_allow_rx = 0;
            //------------------------------
            com_rx_init();   //通讯接收初始化程序
            COM_RX_MODE;
            R_UART1_Start();
        }
        //----------------------------------
        if (bflg_com_rx_end == 1) //如果接收结束
        {
            bflg_com_rx_end = 0;
            //------------------------------
            R_UART1_Stop();
        }
        //----------------------------------
        if (bflg_com_rx_ok == 1)  //如果接收成功
        {
            bflg_com_rx_ok = 0;
            //------------------------------
            R_UART1_Stop();
            com_rx_data_deal();   //通讯接收数据处理程序
        }
        //----------------------------------
        if (bflg_com_allow_tx == 1)    //如果允许发送
        {
            bflg_com_allow_tx = 0;
            //------------------------------
            R_UART1_Start();
            COM_TX_MODE;
            com_tx_init();   //通讯发送初始化程序
        }
        if (bflg_com_tx_ok == 1)  //如果发送成功
        {
            bflg_com_tx_ok = 0;
            //------------------------------
            R_UART1_Stop();
        }
        
        //------------------------------------------------------
        //显示板的串口0和USB板的通讯程序
        if (bflg_com_allow_rx_usb == 1)	  //如果允许接收
        {
            bflg_com_allow_rx_usb = 0;
            com_rx_init_usb();	//通讯接收初始化程序
            //COM_RX_MODE;
            //R_UART0_Start();
            USB_RX_UART0_Start();
        }
        //----------------------------------
        if(bflg_rx_data_error_usb == 1) //接收数据错
        {
            bflg_rx_data_error_usb = 0;            
            USB_RX_UART0_Stop();
            
            bflg_com_rx_delaytime_usb = 1;       
            gss_com_rx_delaytimer_usb = 50;  
        }
        //----------------------------------
        if (bflg_com_rx_end_usb == 1) //如果接收结束
        {
            bflg_com_rx_end_usb = 0;
            //R_UART0_Stop();            
            USB_RX_UART0_Stop();
        }
        //----------------------------------
        if (bflg_com_rx_ok_usb == 1)  //如果接收成功
        {
            bflg_com_rx_ok_usb = 0;
            //R_UART0_Stop();
            USB_RX_UART0_Stop();
            com_rx_data_deal_usb();	 //通讯接收数据处理程序
        }
        //----------------------------------
        if (bflg_com_allow_tx_usb == 1)	  //如果允许发送
        {
            bflg_com_allow_tx_usb = 0;
            //R_UART0_Start();
            //COM_TX_MODE;
            USB_TX_UART0_Start();
            com_tx_init_usb();	//通讯发送初始化程序
        }
        if (bflg_com_tx_ok_usb == 1)  //如果发送成功
        {
            bflg_com_tx_ok_usb = 0;
            //R_UART0_Stop();
            USB_TX_UART0_Stop();
        }
    
    
    //------------------------------------------------------
    key_buzz_deal();//按键蜂鸣处理程序
    //------------------------------------------------------
    lamp_output_deal();  //灯输出处理程序
    
    //------------------------------------------------------
    //IC卡
    card_set_start_stop();
    card_erase_all_data();
    card_reset_repeat_tx_order();
    //------------------------------------------------------
    //判断usb是否有发送，及发送类型

    com_tx_type_rank_usb();
    
  }
}
//------------------------------------------------------------------------------
void timer_op(void)      //定时程序，在主循环程序中调用
{
    //------------------------------------------------------
    if (bflg_1ms_reach == 1)  //如果1ms定时到
    {
        bflg_1ms_reach = 0;
        guc_100ms_timer--;
        //--------------------------------------------------
        com_rx_delaytime();   //通讯接收延时程序
        
        com_tx_delaytime();   //通讯发送延时程序
        
        com_rx_end_delaytime();    //通讯接收完成延时程序   
        //--------------------------------------------------
        //usb_通讯
		com_rx_delaytime_usb();   //通讯接收延时程序
        
        com_tx_delaytime_usb();   //通讯发送延时程序
        
        com_rx_end_delaytime_usb();    //通讯接收完成延时程序
        //--------------------------------------------------
        //card_通讯
        card_tx_20_delaytime();
        card_tx_auto_delaytime();
        card_rx_end_delaytime();
        card_bus_monitor();
    }
    //------------------------------------------------------
    if (bflg_5ms_reach == 1)
    {
        bflg_5ms_reach = 0;
        //--------------------------------------------------
        //5ms定时调用程序
        /*if (bflg_read_key == 1)
        {
            led_key_deal();  //显示按键处理程序
        }
        
        if (bflg_write_led == 1)
        {
            led_key_deal();  //显示按键处理程序
        }*/
        led_key_deal();  //显示按键处理程序
    }
    //------------------------------------------------------
    if (bflg_10ms_reach == 1)
    {
        bflg_10ms_reach = 0;
        guc_1s_timer--;
        //--------------------------------------------------
        //10ms定时调用程序
        key_buzz_delaytime(); //按键蜂鸣延时程序


    }
    //------------------------------------------------------
    if (guc_100ms_timer == 0)
    {
        guc_100ms_timer = 100;
        //--------------------------------------------------
        //100ms定时调用程序
        led_allon_delaytime();     //led全亮延时程序
        
        if(bflg_test_mode == 1)
        {bflg_write_led = ~bflg_write_led;}
    }
    //------------------------------------------------------
    if (guc_1s_timer == 0)
    {
        guc_1s_timer = 100;
        guc_1min_timer--;
        //--------------------------------------------------
        //1s定时调用程序
        com_fault_delaytime();    //通讯故障延时程序
        //--------------------------------------------------
        //IC卡
        card_buzz_interval_delaytimer();
        card_tx_order_interval();
    }
    //------------------------------------------------------
    if (guc_1min_timer == 0)
    {
        guc_1min_timer = 60;
        //--------------------------------------------------
        //1min定时调用程序
        
    }
}
//------------------------------------------------------------------------------
void led_key_deal(void)  //显示按键处理程序，在10ms定时程序中调用
{
    guc_iic_bit = 0;
    IIC_DIO = L_LEVEL;
    
    R_TAU0_Channel6_Start();
}
//------------------------------------------------------------------------------
void key_buzz_deal(void) //按键蜂鸣处理程序，在主循环程序中调用
{
    if (bflg_key_buzz_delaytime == 0)
    {
        if (guc_key_buzz_buffer != guc_key_buffer[0])
        {
            guc_key_buzz_buffer = guc_key_buffer[0];
            
            if ((guc_key_buffer[0] != 0) && (guc_1638_write_stage != 3))
            {
                bflg_key_buzz_delaytime = 1;
                gss_key_buzz_delaytimer = 0;
                
                R_PCLBUZ0_Start();          //调试时注释无声
            }
        }
    }
}
//------------------------------------------------------------------------------
void key_buzz_delaytime(void) //按键蜂鸣延时程序，在10ms多少程序中调用
{
    if (bflg_key_buzz_delaytime == 1)
    {
        gss_key_buzz_delaytimer++;
        if (gss_key_buzz_delaytimer >= 20)
        {
            gss_key_buzz_delaytimer = 0;
            bflg_key_buzz_delaytime = 0;
            
            R_PCLBUZ0_Stop();
        }
    }
}
//------------------------------------------------------------------------------
void led_allon_delaytime(void)     //led全亮延时程序，在100ms定时程序中调用
{
	  if (bflg_led_allon_delaytime == 1)
	  {
	  	  gss_led_allon_delaytimer++;
	  	  if (gss_led_allon_delaytimer >= 10)
	  	  {
	  	  	  gss_led_allon_delaytimer = 0;
	  	  	  bflg_led_allon_delaytime = 0;
	  	  }
	  }
}
//------------------------------------------------------------------------------
void timer_int(void)     //1ms定时中断程序
{
    bflg_1ms_reach = 1;       //置1ms到标志位
    //----------------------------------
    guc_5ms_timer++;
    if (guc_5ms_timer >= 5)   //10ms计时到
    {
        guc_5ms_timer = 0;
        bflg_5ms_reach = 1;
    }
    //----------------------------------
    guc_10ms_timer++;
    if (guc_10ms_timer >= 10) //10ms计时到
    {
        guc_10ms_timer = 0;
        bflg_10ms_reach = 1;
    }
    //----------------------------------
}
//------------------------------------------------------------------------------
void iic_int(void)       //iic模拟定时中端程序
{
    if (bflg_write_led == 1)
    {
        if (guc_1638_write_stage == 1)
        {
            IIC_CLK = ~IIC_CLK;
            
            if (bflg_iic_wait == 0)
            {
                if (IIC_CLK == L_LEVEL)
                {
                    if (guc_iic_bit < 8)
                    {
                        IIC_DIO = TEST(CMD1_1638, guc_iic_bit);
                        
                        guc_iic_bit++;
                    }
                    else
                    {
                        guc_iic_bit = 0;
                        
                        IIC_DIO = L_LEVEL;
                        
                        bflg_iic_wait = 1;
                        guc_iic_wait_cnt = 0;
                    }
                }
            }
            else
            {
                guc_iic_wait_cnt++;
                
                if (guc_iic_wait_cnt == 1)
                {
                    IIC_DIO = L_LEVEL;
                }
                if (guc_iic_wait_cnt >= 3)
                {
                    guc_iic_wait_cnt = 0;
                    bflg_iic_wait = 0;
                    		    
                    R_TAU0_Channel6_Stop();
                    guc_1638_write_stage = 2;
                    
                    IIC_DIO = H_LEVEL;
                }
            }
        }
        else if (guc_1638_write_stage == 2)
        {
            IIC_CLK = ~IIC_CLK;
            
            if (bflg_iic_wait == 0)
            {
                if (IIC_CLK == L_LEVEL)
                {
                    if (guc_iic_bit < 8)
                    {
                        IIC_DIO = TEST(CMD2_1638, guc_iic_bit);
                        
                        guc_iic_bit++;
                    }
                    else
                    {
                        guc_iic_bit = 0;
                        
                        IIC_DIO = L_LEVEL;
                        
                        bflg_iic_wait = 1;
                        guc_iic_wait_cnt = 0;
                    }
                }
            }
            else
            {
                guc_iic_wait_cnt++;
                
                if (guc_iic_wait_cnt == 1)
                {
                    IIC_DIO = L_LEVEL;
                }
                if (guc_iic_wait_cnt >= 3)
                {
                    guc_iic_wait_cnt = 0;
                    bflg_iic_wait = 0;
                    
                    R_TAU0_Channel6_Stop();
                    guc_1638_write_stage = 3;
                    
                    IIC_DIO = H_LEVEL;
                }
            }
        }
        else if (guc_1638_write_stage == 3)
        {
            IIC_CLK = ~IIC_CLK;
            
            if (bflg_iic_wait == 0)
            {
                if (IIC_CLK == L_LEVEL)
                {
                    if (guc_iic_bit < 8)
                    {
                        IIC_DIO = TEST(guc_led_buffer[guc_iic_index], guc_iic_bit);
                        
                        guc_iic_bit++;
                    }
                    else
                    {
                        guc_iic_bit = 0;
                        
                        IIC_DIO = L_LEVEL;
                        
                        guc_iic_index++;
                        if (guc_iic_index >= 13)
                        {
                            guc_iic_index = 0;
                            
                            bflg_iic_wait = 1;
                            guc_iic_wait_cnt = 0;
                        }
                    }
                }
            }
            else
            {
                guc_iic_wait_cnt++;
                
                if (guc_iic_wait_cnt == 1)
                {
                    IIC_DIO = L_LEVEL;
                }
                if (guc_iic_wait_cnt >= 3)
                {
                    guc_iic_wait_cnt = 0;
                    bflg_iic_wait = 0;
                    
                    R_TAU0_Channel6_Stop();
                    guc_1638_write_stage = 4;
                    
                    IIC_DIO = H_LEVEL;
                }
            }
        }
        else if (guc_1638_write_stage == 4)
        {
            IIC_CLK = ~IIC_CLK;
            
            if (bflg_iic_wait == 0)
            {
                if (IIC_CLK == L_LEVEL)
                {
                    if (guc_iic_bit < 8)
                    {
                        IIC_DIO = TEST(CMD4_1638, guc_iic_bit);
                        
                        guc_iic_bit++;
                    }
                    else
                    {
                        guc_iic_bit = 0;
                        
                        IIC_DIO = L_LEVEL;
                        
                        bflg_iic_wait = 1;
                        guc_iic_wait_cnt = 0;
                    }
                }
            }
            else
            {
                guc_iic_wait_cnt++;
                
                if (guc_iic_wait_cnt == 1)
                {
                    IIC_DIO = L_LEVEL;
                }
                if (guc_iic_wait_cnt >= 3)
                {
                    guc_iic_wait_cnt = 0;
                    bflg_iic_wait = 0;
                    
                    R_TAU0_Channel6_Stop();
                    guc_1638_write_stage = 1;
                    
                    IIC_DIO = H_LEVEL;
                    
                    bflg_write_led = 0;
                }
            }
        }
    }
    else// if (bflg_read_key == 1)
    {
        if (guc_1638_read_stage == 1)
        {
            IIC_CLK = ~IIC_CLK;
            
            if (bflg_iic_wait == 0)
            {
                if (IIC_CLK == L_LEVEL)
                {
                    if (guc_iic_bit < 8)
                    {
                        IIC_DIO = TEST(CMD5_1638, guc_iic_bit);
                        
                        guc_iic_bit++;
                    }
                    else
                    {
                        guc_iic_bit = 0;
                        
                        IIC_DIO = L_LEVEL;
                        
                        bflg_iic_wait = 1;
                        guc_iic_wait_cnt = 0;
                    }
                }
            }
            else
            {
                guc_iic_wait_cnt++;
                
                if (guc_iic_wait_cnt >= 3)
                {
                    guc_iic_wait_cnt = 0;
                    bflg_iic_wait = 0;
                    
                    //IIC_DIO = H_LEVEL;
                    
                    //R_TAU0_Channel6_Stop();
                    guc_1638_read_stage = 2;
                    
                    PM6.1 = 1;
                }
            }
        }
        else if (guc_1638_read_stage == 2)
        {
            IIC_CLK = ~IIC_CLK;
            
            if (bflg_iic_wait == 0)
            {
                if (IIC_CLK == H_LEVEL)
                {
                    if (guc_iic_bit < 8)
                    {
                        if (IIC_DIO == H_LEVEL)
                        {
                            guc_key_buffer[guc_iic_index] |= SET(guc_iic_bit);
                        }
                        else
                        {
                            guc_key_buffer[guc_iic_index] &= CLR(guc_iic_bit);
                        }
                        
                        guc_iic_bit++;
                    }
                    else
                    {
                        guc_iic_bit = 0;
                        
                        guc_iic_index++;
                        if (guc_iic_index >= 4)
                        {
                            guc_iic_index = 0;
                            
                            bflg_iic_wait = 1;
                            guc_iic_wait_cnt = 0;
                        }
                    }
                }
            }
            else
            {
                guc_iic_wait_cnt++;
                
                if (guc_iic_wait_cnt == 1)
                {
                    IIC_DIO = L_LEVEL;
                    PM6.1 = 0;
                }
                if (guc_iic_wait_cnt >= 2)
                {
                    guc_iic_wait_cnt = 0;
                    bflg_iic_wait = 0;
                    
                    R_TAU0_Channel6_Stop();
                    guc_1638_read_stage = 1;
                    
                    IIC_DIO = H_LEVEL;
                    
                    //bflg_read_key = 0;
                    
                    guc_key_val[0] = guc_key_buffer[0];
                    guc_key_val[1] = guc_key_buffer[1];
                    guc_key_val[2] = guc_key_buffer[2];
                    guc_key_val[3] = guc_key_buffer[3];
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
uint8_t TEST(uint8_t luc_data, uint8_t luc_bit)
{
    luc_data >>= luc_bit;
    luc_data &= 0x01;
    
    if (luc_data == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
    
}
//------------------------------------------------------------------------------
uint8_t SET(uint8_t luc_bit)
{
    uint8_t luc_data;
    
    luc_data = 0x01;
    luc_data <<= luc_bit;
    
    return luc_data;
}
//------------------------------------------------------------------------------
uint8_t CLR(uint8_t luc_bit)
{
    uint8_t luc_data;
    
    luc_data = 0x01;
    luc_data <<= luc_bit;
    luc_data = (uint8_t) (~luc_data);
    
    return luc_data;
}
//------------------------------------------------------------------------------
void lamp_output_deal(void)   //灯输出处理程序，在主循环程序中调用
{
	  if ((guc_lamp_val & 0x01) == 0)
	  {
	  	  LAMP1_PIN = L_LEVEL;
	  }
	  else
	  {
	  	  LAMP1_PIN = H_LEVEL;
	  }
	  //----------------------------------
	  if ((guc_lamp_val & 0x02) == 0)
	  {
	  	  LAMP2_PIN = L_LEVEL;
	  }
	  else
	  {
	  	  LAMP2_PIN = H_LEVEL;
	  }
	  //----------------------------------
	  if ((guc_lamp_val & 0x04) == 0)
	  {
	  	  LAMP3_PIN = L_LEVEL;
	  }
	  else
	  {
	  	  LAMP3_PIN = H_LEVEL;
	  }
	  //----------------------------------
}
//------------------------------------------------------------------------------
#endif
