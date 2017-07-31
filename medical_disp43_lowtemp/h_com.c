#ifndef _COM_C_
#define _COM_C_
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
#include "m_card.h"
#include "m_pfdl.h"
#include "pfdl.h"
#include "m_test_mode.h"
#include "m_usb.h"
//------------------------------------------------------------------------------
//��������
void com_init(void);      //ͨѶ��ʼ�������ڳ�ʼ�������е���

void com_rx_init(void);  //ͨѶ���ճ�ʼ����������ѭ�������е���

void com_rx_data_deal(void);  //ͨѶ�������ݴ����������ѭ�������е���

void com_tx_init(void);         //ͨѶ���ͳ�ʼ����������ѭ�������е���

void com_rx_delaytime(void);  //ͨѶ������ʱ������1ms��ʱ�����е���

void com_tx_delaytime(void);  //ͨѶ������ʱ������1ms��ʱ�����е���

void com_rx_end_delaytime(void);   //ͨѶ���������ʱ������1ms��ʱ�����е���

void com_fault_delaytime(void);    //ͨѶ������ʱ������1s��ʱ�����е���

void com_rx_int(uint8_t rx_data);       //ͨѶ�����жϳ���

void com_tx_int(void);                  //ͨѶ�����жϳ���

unsigned int CRC16(unsigned char *puchmsg, unsigned int usDataLen);    //����CRCУ��ĳ���
//------------------------------------------------------------------------------
//��־����
flag_type flg_com;
//------------------------------------------------------------------------------
//��������
uint8_t guc_com_tx_buffer[9];  //ͨѶ���ͻ�����
uint8_t guc_com_rx_buffer[53];  //ͨѶ���ջ�����  23

int16_t gss_com_tx_delaytimer;     //ͨѶ������ʱ��ʱ��
int16_t gss_com_rx_delaytimer;     //ͨѶ������ʱ��ʱ��

int16_t gss_com_rx_end_delaytimer; //ͨѶ�������־
int16_t gss_com_fault_delaytimer;  //ͨѶ������ʱ��ʱ��

uint8_t guc_com_tx_point;     //ͨѶ��������
uint8_t guc_com_rx_point;     //ͨѶ��������
//------------------------------------------------------------------------------
uint8_t guc_led_buffer[LED_BUF_LEN];    //����ܼĴ���
uint8_t guc_key_buffer[KEY_BUF_LEN];    //�����Ĵ���

uint8_t guc_key_val[KEY_BUF_LEN];       //����ֵ  ����ֻ���� guc_key_val[0]

//------------------------------------------------------------------------------
//��������
//------------------------------------------------------------------------------
void com_init(void) //ͨѶ��ʼ�������ڳ�ʼ�������е���
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
    
    bflg_write_led = 1;  //��д�������ʾ��־
    
    bflg_led_allon_delaytime = 1;
    gss_led_allon_delaytimer = 0;
    //----------------------------------
    
    bflg_com_rx_delaytime = 1;
    gss_com_rx_delaytimer = 5;
    
    bflg_com_tx_delaytime = 0;
    gss_com_tx_delaytimer = 0;   
}
//------------------------------------------------------------------------------
void com_rx_init(void)   //ͨѶ���ճ�ʼ����������ѭ�������е���  //peak ÿ���������ʱ����һ��
{
    uint8_t i;
    
    for (i = 0; i < 53; i++)      //23
    {
        guc_com_rx_buffer[i] = 0;  //����ջ�����
    }
    
    bflg_com_rx_busy = 0;
    guc_com_rx_point = 0;
}
//------------------------------------------------------------------------------
void com_rx_data_deal(void)   //ͨѶ�������ݴ����������ѭ�������е���
{
    uint8_t i, msb_byte = 0,lsb_byte = 0;
    uint16_t lus_tmp;
    word_type com_crc;
    //------------------------------------------------------
    if ((guc_com_rx_buffer[1] == 0x03)) //����ǲ�ѯ����
    {
        com_crc.uword = CRC16(guc_com_rx_buffer, 6);
        if ((com_crc.ubyte.low == guc_com_rx_buffer[6])
         && (com_crc.ubyte.high == guc_com_rx_buffer[7]))
        {
            bflg_com_tx_delaytime = 1;       //�÷�����ʱ��־
            gss_com_tx_delaytimer = 10;      //������ʱ��ʱ����ֵ
            
            bflg_com_rx_delaytime = 0;       //�巢����ʱ��־
            gss_com_rx_delaytimer = 0;       //������ʱ��ʱ������
            
            bflg_com_fault = 0;              //��ͨѶ���ϱ�־
            gss_com_fault_delaytimer = 0;    //��ͨѶ���ϼ�ʱ��
            //----------------------------------------------
            //���ݴ���
            bflg_read_write_data = 0;   //��ǰ�Ƕ�����
            
            //bflg_read_key = 1;       //�ö�������־
        }
    }
    else if (guc_com_rx_buffer[1] == 0x10)   //����Ƕ���趨����
    {
        lus_tmp = guc_com_rx_buffer[6] + 7;
        
        com_crc.uword = CRC16(guc_com_rx_buffer, lus_tmp);
        if ((com_crc.ubyte.low == guc_com_rx_buffer[lus_tmp])
         && (com_crc.ubyte.high == guc_com_rx_buffer[lus_tmp + 1]))
        {
            bflg_com_tx_delaytime = 1;  //�÷�����ʱ��־
            gss_com_tx_delaytimer = 10; //������ʱ��ʱ����ֵ
            
            bflg_com_rx_delaytime = 0;       //�巢����ʱ��־
            gss_com_rx_delaytimer = 0;       //������ʱ��ʱ������
            
            bflg_com_fault = 0;              //��ͨѶ���ϱ�־
            gss_com_fault_delaytimer = 0;    //��ͨѶ���ϼ�ʱ��
            //----------------------------------------------
            //���ݴ���
            bflg_read_write_data = 1;   //��ǰ��д����
            //----------------------------------------------
            if (bflg_led_allon_delaytime == 0)
            {
                for (i = 0; i < 12; i++)
                {
                    guc_led_buffer[i + 1] = guc_com_rx_buffer[i + 7];
                }
                guc_erasure_IC_all_data_order = guc_com_rx_buffer[19];     //peak ԭ��[19]û�ã�������Ϊע��ICָ��
                guc_lamp_val = guc_com_rx_buffer[20];
                
                bflg_write_led = 1;     //��д�������ʾ��־
            }

            for(i = 0; i<31; i++)       //��ȡusb�����õ�����
            {
                guc_usb_buffer[i] = guc_com_rx_buffer[i + 20];  //��ȡ��usb�������
            }
        }
    }
}
//------------------------------------------------------------------------------
void com_tx_init(void)   //ͨѶ���ͳ�ʼ����������ѭ�������е���
{
    word_type com_crc;      
    uint8_t  msb_byte = 0, lsb_byte = 0;
    //------------------------------------------------------
    if (bflg_read_write_data == 0) //����Ƕ�����  //peak ��ȡ������ֵ
    {
        guc_com_tx_buffer[0] = guc_com_rx_buffer[0];
        guc_com_tx_buffer[1] = guc_com_rx_buffer[1];
        guc_com_tx_buffer[2] = 4;  //�ֽڸ���4
        //--------------------------------------------------
        guc_com_tx_buffer[3] = guc_key_val[0];                //����ֵ
       
        //guc_com_tx_buffer[4] = guc_key_val[1];   //peak ԭ��   
        //guc_com_tx_buffer[5] = guc_key_val[2];         
        //guc_com_tx_buffer[6] = guc_key_val[3]; 
        //���洢��оƬ�ڲ�������_IC������
        PFDL_Init();                
        PFDL_Read(0x801,1,&msb_byte);      //���ﵽ�ĵ�ַ���ֽ�
        PFDL_Read(0x802,1,&lsb_byte);      //���ﵽ�ĵ�ַ���ֽ�
        flash_address = 0;
        flash_address = (((flash_address | msb_byte)<<8) | lsb_byte); 
        PFDL_Close();
        msb_byte |= (uint8_t)(flash_address >> 8);
        lsb_byte |= (uint8_t)flash_address;      
        guc_com_tx_buffer[4] = msb_byte;
        guc_com_tx_buffer[5] = lsb_byte;                   //IC��ע��ĵ�ַ 

        //buffer[6] 7:���� 6�����汾(������ʽ��ʾ���Ϊ0) 5��δ4:δ3:δ2:usb����1:usb״̬0��usb״̬
        guc_com_tx_buffer[6]  = 0x00;  //Ҫ��ʼ������ֹ��Ϊ0x40ʹ����������ģʽ
        if(guc_IC_access_state == 1)
        {
            guc_IC_access_state = 0;
            guc_com_tx_buffer[6]  |= 0x80;                //ʹ��buffer[6]�����λ������ָ��
        }
        else
        {
            guc_com_tx_buffer[6]  &= 0x7F;                 
        }
        
        guc_com_tx_buffer[6] |= (uint8_t)(guc_Udisk_state&0x03);       //ʹ��buffer[6]�ĵ�0��1��λ��ʾU��״̬
        if(guc_usb_state != 0x00)    
        {
            guc_com_tx_buffer[6] |= 0x04;                              //ʹ�õ�2λ��ʾusb���Ƿ��й���
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
    else  //�����д���� //��������ܡ�LED��ʾ
    {
        guc_com_tx_buffer[0] = guc_com_rx_buffer[0];  //peak���豸��ַ
        guc_com_tx_buffer[1] = guc_com_rx_buffer[1];  //������
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
void com_rx_delaytime(void)     //ͨѶ������ʱ������1ms��ʱ�����е���
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
void com_tx_delaytime(void)     //ͨѶ������ʱ������1ms��ʱ�����е���
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
�������ܣ��˺����ж�ÿһ���ֽ��Ƿ�ʱ����Ϊgss_com_rx_end_delaytimer = 0 ����com_rx_int()��
          ����ʵ���ô�Ӧ�����ж�һ֡�����Ƿ�ʱ�ɣ���

����λ�ã�1ms��ʱ����-------------------ok
********************************************************************************************************************************/
void com_rx_end_delaytime(void) //ͨѶ���������ʱ������1ms��ʱ�����е���
{
    if (bflg_com_rx_busy == 1)  //�����ǰ����æ
    {
        gss_com_rx_end_delaytimer++;
        if (gss_com_rx_end_delaytimer >= 50) // >= 2   //����
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
void com_fault_delaytime(void)     //ͨѶ������ʱ������1s��ʱ�����е���
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
�������ܣ��������ݣ��˺�����ϵͳ�����ж��У�����ж�ÿ�ж�һ���ǽ���һ���ֽ�

����λ�ã���ϵͳ���ڽ����ж���-------------------ok
********************************************************************************************************************************/
void com_rx_int(uint8_t rx_data)   //ͨѶ�����жϳ���
{
    uint8_t luc_com_rx_byte;  //�����ֽ���ʱ������
    
    gss_com_rx_end_delaytimer = 0; //���������ʱ��ʱ��
    //--------------------------------------------------
    luc_com_rx_byte = rx_data;
    
    if ((bflg_com_rx_busy == 0) && (luc_com_rx_byte == 0x00))    //����޽���æ���Ҵӵ�ַ��ȷ
    {
        bflg_com_rx_busy = 1;
        
        guc_com_rx_buffer[0] = luc_com_rx_byte;
        guc_com_rx_point = 1;
    }
    else if (bflg_com_rx_busy == 1)     //�������æ
    {
        guc_com_rx_buffer[guc_com_rx_point++] = luc_com_rx_byte;
        
        if ((guc_com_rx_buffer[1] == 0x03) && (guc_com_rx_point >= 8))     //����ǲ�ѯ����ҽ����������
        {
            guc_com_rx_point = 0;  //������ֽ�����
            bflg_com_rx_busy = 0;  //�����æ��־
            bflg_com_rx_ok = 1;    //�ý��ճɹ���־
            
            bflg_com_rx_delaytime = 1;   //�ý�����ʱ��־
            gss_com_rx_delaytimer = 50;  //������ʱ��ʱ����ֵ
        }
        else if ((guc_com_rx_buffer[1] == 0x10) && (guc_com_rx_point >= 53))  //23   //����Ƕ���趨����ҽ��յ��ֽ�������֡
        {
            guc_com_rx_point = 0;   
            bflg_com_rx_busy = 0;   
            bflg_com_rx_ok = 1;    
            
            bflg_com_rx_delaytime = 1;  
            gss_com_rx_delaytimer = 50;  
        }
        else if ((guc_com_rx_buffer[1] == 0xAD) && (guc_com_rx_point >= 1))     //����ǲ���ģʽ
        {
            bflg_test_mode = 1;       //����ģʽ  �������ģʽ����Ϊ����ģʽ

            guc_com_rx_point = 0;     //������ֽ�����
            bflg_com_rx_busy = 0;     //�����æ��־

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
void com_tx_int(void)    //ͨѶ�����жϳ���
{
    if (bflg_com_tx_busy == 1)
    {                                   //��������ָ�� buff[6]�����λ������ָ��
        if (bflg_read_write_data == 0)  //����Ƕ����� peak ��������ʾ���ȡ����ֵ����ʱ��ʾ�������ط��ذ���ֵ
        {
            if (guc_com_tx_point < 9)
            {
                TXD1 = guc_com_tx_buffer[guc_com_tx_point++];    //��������
            }
            else
            {
                guc_com_tx_point = 0;   //�巢���ֽ�����
                bflg_com_tx_busy = 0;   //�巢��æ��־
                bflg_com_tx_ok = 1;
                
                bflg_com_rx_delaytime = 1;   //��comͨѶ������ʱ��־
                gss_com_rx_delaytimer = 5;   //comͨѶ������ʱ��ʱ����ֵ
            }
        }
        else    //�����д���� peak��������ʾ��д�������ʾ������
        {
            if (guc_com_tx_point < 8)
            {
                TXD1 = guc_com_tx_buffer[guc_com_tx_point++];   //��������
            }
            else
            {
                guc_com_tx_point = 0;   //�巢���ֽ�����
                bflg_com_tx_busy = 0;   //�巢��æ��־
                bflg_com_tx_ok = 1;
                
                bflg_com_rx_delaytime = 1;   //��ͨѶ������ʱ��־
                gss_com_rx_delaytimer = 5;   //ͨѶ������ʱ��ʱ����ֵ
            }
        }
    }
}
//------------------------------------------------------------------------------
unsigned int CRC16(unsigned char *puchmsg, unsigned int usDataLen)    //����CRCУ��ĳ������
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
