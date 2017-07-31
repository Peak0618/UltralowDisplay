#ifndef _MAIN_C_
#define _MAIN_C_
//------------------------------------------------------------------------------
//�����ļ�����
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
//��������
void main_loop(void);    //��ѭ������

void timer_op(void);     //��ʱ��������ѭ�������е���

void led_key_deal(void); //��ʾ�������������10ms��ʱ�����е���

void key_buzz_deal(void);//�������������������ѭ�������е���

void key_buzz_delaytime(void);//����������ʱ������10ms���ٳ����е���

void led_allon_delaytime(void);    //ledȫ����ʱ������100ms��ʱ�����е���

void timer_int(void);    //1ms��ʱ�жϳ���

void iic_int(void);      //iicģ�ⶨʱ�ж˳���

uint8_t TEST(uint8_t luc_data, uint8_t luc_bit);

uint8_t SET(uint8_t luc_bit);

uint8_t CLR(uint8_t luc_bit);

void lamp_output_deal(void);  //����������������ѭ�������е���

//------------------------------------------------------------------------------
//�궨��
//#define   COM_RX_MODE    (P12.0 = 0)    //����ģʽ    �Ƶ�.h
//#define   COM_TX_MODE    (P12.0 = 1)    //����ģʽ

#define   IIC_CLK   P6.0
#define   IIC_DIO   P6.1

#define   H_LEVEL   1
#define   L_LEVEL   0

#define   LAMP1_PIN      P1.0
#define   LAMP2_PIN      P1.5
#define   LAMP3_PIN      P3.0

//------------------------------------------------------------------------------
//��������
flag_type flg_time;
//----------------------------------------------------------
uint8_t   guc_5ms_timer;      //5�����ʱ��
uint8_t   guc_10ms_timer;     //10�����ʱ��
uint8_t   guc_100ms_timer;    //100ms��ʱ��
uint8_t   guc_1s_timer;       //1s��ʱ��
uint8_t   guc_1min_timer;     //1min��ʱ��

uint8_t   guc_1638_write_stage;
uint8_t   guc_1638_read_stage;
uint8_t   guc_iic_wait_cnt;
uint8_t   guc_iic_bit;
uint8_t   guc_iic_index;

uint8_t   guc_key_buzz_buffer;
int16_t   gss_key_buzz_delaytimer;

int16_t   gss_led_allon_delaytimer;

uint8_t   guc_lamp_val;        //������ֵ
uint8_t   guc_IC_tx_ok_count;  //�������͸�����ɹ��Ĵ�������ֹû�з��Ϳ���ָ�������0�����Զ෢����

//------------------------------------------------------------------------------
//��������
//------------------------------------------------------------------------------
void system_init(void)   //ϵͳ��ʼ������
{
    //card_flash_init();       //ic ��flash��ʼ��
    //----------------------------------
    R_TAU0_Channel7_Start();   //peak ��ʱ����ʼ
    //----------------------------------
    //R_PCLBUZ0_Start();
    //----------------------------------
    com_init();     //�����崮��ͨѶ��ʼ������
    com_init_usb(); //��USB��Ĵ��ڳ�ʼ��
    //----------------------------------
    IIC_CLK = H_LEVEL;        //peak IIC����
    IIC_DIO = H_LEVEL;
    guc_1638_write_stage = 1;
    guc_1638_read_stage = 1;
    //----------------------------------
    bflg_key_buzz_delaytime = 1;
    gss_key_buzz_delaytimer = 0;

    //debug
    //R_PCLBUZ0_Start();       //peak  ��������ʼ
    //----------------------------------
    guc_lamp_val = 0xFF;     //peak ������ֵ


    //debug
    //R_TAU0_Channel2_Start();  // ��ʱ��2���ж�
}
//------------------------------------------------------------------------------
void main_loop(void)     //��ѭ������
{
    timer_op();              //��ʱ����

    //--------------------------------------------------------
    if(bflg_test_mode == 1)      //����ǲ���ģʽ
    {   
        //bflg_write_led = 1;
        test_mode_deal();
        //test_mode_error_code_set();       
        test_mode_error_code_deal();
        test_mode_error_code_view(); 
                
        key_buzz_deal();//���������������
    }
    else
    {
        //------------------------------------------------------
        //���ذ�ͨѶ����
        if (bflg_com_allow_rx == 1)    //����������
        {
            bflg_com_allow_rx = 0;
            //------------------------------
            com_rx_init();   //ͨѶ���ճ�ʼ������
            COM_RX_MODE;
            R_UART1_Start();
        }
        //----------------------------------
        if (bflg_com_rx_end == 1) //������ս���
        {
            bflg_com_rx_end = 0;
            //------------------------------
            R_UART1_Stop();
        }
        //----------------------------------
        if (bflg_com_rx_ok == 1)  //������ճɹ�
        {
            bflg_com_rx_ok = 0;
            //------------------------------
            R_UART1_Stop();
            com_rx_data_deal();   //ͨѶ�������ݴ������
        }
        //----------------------------------
        if (bflg_com_allow_tx == 1)    //���������
        {
            bflg_com_allow_tx = 0;
            //------------------------------
            R_UART1_Start();
            COM_TX_MODE;
            com_tx_init();   //ͨѶ���ͳ�ʼ������
        }
        if (bflg_com_tx_ok == 1)  //������ͳɹ�
        {
            bflg_com_tx_ok = 0;
            //------------------------------
            R_UART1_Stop();
        }
        
        //------------------------------------------------------
        //��ʾ��Ĵ���0��USB���ͨѶ����
        if (bflg_com_allow_rx_usb == 1)	  //����������
        {
            bflg_com_allow_rx_usb = 0;
            com_rx_init_usb();	//ͨѶ���ճ�ʼ������
            //COM_RX_MODE;
            //R_UART0_Start();
            USB_RX_UART0_Start();
        }
        //----------------------------------
        if(bflg_rx_data_error_usb == 1) //�������ݴ�
        {
            bflg_rx_data_error_usb = 0;            
            USB_RX_UART0_Stop();
            
            bflg_com_rx_delaytime_usb = 1;       
            gss_com_rx_delaytimer_usb = 50;  
        }
        //----------------------------------
        if (bflg_com_rx_end_usb == 1) //������ս���
        {
            bflg_com_rx_end_usb = 0;
            //R_UART0_Stop();            
            USB_RX_UART0_Stop();
        }
        //----------------------------------
        if (bflg_com_rx_ok_usb == 1)  //������ճɹ�
        {
            bflg_com_rx_ok_usb = 0;
            //R_UART0_Stop();
            USB_RX_UART0_Stop();
            com_rx_data_deal_usb();	 //ͨѶ�������ݴ������
        }
        //----------------------------------
        if (bflg_com_allow_tx_usb == 1)	  //���������
        {
            bflg_com_allow_tx_usb = 0;
            //R_UART0_Start();
            //COM_TX_MODE;
            USB_TX_UART0_Start();
            com_tx_init_usb();	//ͨѶ���ͳ�ʼ������
        }
        if (bflg_com_tx_ok_usb == 1)  //������ͳɹ�
        {
            bflg_com_tx_ok_usb = 0;
            //R_UART0_Stop();
            USB_TX_UART0_Stop();
        }
    
    
    //------------------------------------------------------
    key_buzz_deal();//���������������
    //------------------------------------------------------
    lamp_output_deal();  //������������
    
    //------------------------------------------------------
    //IC��
    card_set_start_stop();
    card_erase_all_data();
    card_reset_repeat_tx_order();
    //------------------------------------------------------
    //�ж�usb�Ƿ��з��ͣ�����������

    com_tx_type_rank_usb();
    
  }
}
//------------------------------------------------------------------------------
void timer_op(void)      //��ʱ��������ѭ�������е���
{
    //------------------------------------------------------
    if (bflg_1ms_reach == 1)  //���1ms��ʱ��
    {
        bflg_1ms_reach = 0;
        guc_100ms_timer--;
        //--------------------------------------------------
        com_rx_delaytime();   //ͨѶ������ʱ����
        
        com_tx_delaytime();   //ͨѶ������ʱ����
        
        com_rx_end_delaytime();    //ͨѶ���������ʱ����   
        //--------------------------------------------------
        //usb_ͨѶ
		com_rx_delaytime_usb();   //ͨѶ������ʱ����
        
        com_tx_delaytime_usb();   //ͨѶ������ʱ����
        
        com_rx_end_delaytime_usb();    //ͨѶ���������ʱ����
        //--------------------------------------------------
        //card_ͨѶ
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
        //5ms��ʱ���ó���
        /*if (bflg_read_key == 1)
        {
            led_key_deal();  //��ʾ�����������
        }
        
        if (bflg_write_led == 1)
        {
            led_key_deal();  //��ʾ�����������
        }*/
        led_key_deal();  //��ʾ�����������
    }
    //------------------------------------------------------
    if (bflg_10ms_reach == 1)
    {
        bflg_10ms_reach = 0;
        guc_1s_timer--;
        //--------------------------------------------------
        //10ms��ʱ���ó���
        key_buzz_delaytime(); //����������ʱ����


    }
    //------------------------------------------------------
    if (guc_100ms_timer == 0)
    {
        guc_100ms_timer = 100;
        //--------------------------------------------------
        //100ms��ʱ���ó���
        led_allon_delaytime();     //ledȫ����ʱ����
        
        if(bflg_test_mode == 1)
        {bflg_write_led = ~bflg_write_led;}
    }
    //------------------------------------------------------
    if (guc_1s_timer == 0)
    {
        guc_1s_timer = 100;
        guc_1min_timer--;
        //--------------------------------------------------
        //1s��ʱ���ó���
        com_fault_delaytime();    //ͨѶ������ʱ����
        //--------------------------------------------------
        //IC��
        card_buzz_interval_delaytimer();
        card_tx_order_interval();
    }
    //------------------------------------------------------
    if (guc_1min_timer == 0)
    {
        guc_1min_timer = 60;
        //--------------------------------------------------
        //1min��ʱ���ó���
        
    }
}
//------------------------------------------------------------------------------
void led_key_deal(void)  //��ʾ�������������10ms��ʱ�����е���
{
    guc_iic_bit = 0;
    IIC_DIO = L_LEVEL;
    
    R_TAU0_Channel6_Start();
}
//------------------------------------------------------------------------------
void key_buzz_deal(void) //�������������������ѭ�������е���
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
                
                R_PCLBUZ0_Start();          //����ʱע������
            }
        }
    }
}
//------------------------------------------------------------------------------
void key_buzz_delaytime(void) //����������ʱ������10ms���ٳ����е���
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
void led_allon_delaytime(void)     //ledȫ����ʱ������100ms��ʱ�����е���
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
void timer_int(void)     //1ms��ʱ�жϳ���
{
    bflg_1ms_reach = 1;       //��1ms����־λ
    //----------------------------------
    guc_5ms_timer++;
    if (guc_5ms_timer >= 5)   //10ms��ʱ��
    {
        guc_5ms_timer = 0;
        bflg_5ms_reach = 1;
    }
    //----------------------------------
    guc_10ms_timer++;
    if (guc_10ms_timer >= 10) //10ms��ʱ��
    {
        guc_10ms_timer = 0;
        bflg_10ms_reach = 1;
    }
    //----------------------------------
}
//------------------------------------------------------------------------------
void iic_int(void)       //iicģ�ⶨʱ�ж˳���
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
void lamp_output_deal(void)   //����������������ѭ�������е���
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
