/********************************************************************************************************************************
overview:
           0)��IC��Ϊ��˫�� ��
           1)ģ�⴮���շ�
           2)ICע����   
           3)��������     
           
��ʱ��2:      1)�������ս�RX;
           2)�������ͽ�TX;
�ⲿ�ж�5��   ���ڼ�⴮�ڵ���ʼλ    

ͨѶ���̣�
         1����ʾ�巢�ͼ���ָ�� ------�򿨰巢�ͻ�Ӧһ������--06 02 00 00 fb 03(ֻ�ڵ�һ�μ���ʱ��Ӧ��
                                     ֮��ķ�ֹ�򿨰���ߵ��ظ����ͼ���ָ��򿨰岻�ٻ�Ӧ )
         2������IC������ʱ            ------�򿨰巢��IC�������к�;
                                     һ֡��������ʱ��15���ֽڷ���15.6ms 
                                     ����֮֡��ļ����66.8ms
���ڸ�ʽ��1λֹͣλ��8λ���ݡ�1λֹͣλ����У�顢9600                                     
*********************************************************************************************************************************/


//������ͷ�ļ�
//----------------------------------------------------------------
#include "r_cg_macrodriver.h"
#include "r_cg_port.h"
#include "r_cg_intc.h"
#include "r_cg_timer.h"


#include "h_type_define.h"
#include "m_card.h"
#include "m_pfdl.h"
#include "h_com.h"
#include "pfdl.h"

#include "r_cg_pclbuz.h"
#include "h_main.h"
#include "m_test_mode.h"
//--------------------------------------------------------------------
//��������
void card_interrupt_check(void);
void card_receive_data(uint8_t length);
void card_flash_init(void);
void card_erase_all_data(void);
uint8_t card_rx_data_deal(void);
void card_send_data(uint8_t *txbuff,uint8_t length);
void card_tx_20_delaytime(void);
void card_tx_auto_delaytime(void);
void card_tx_order_interval(void);
void card_reset_repeat_tx_order(void);
void card_rx_end_delaytime(void);  
void card_rx_init(void);
void card_tx_init(void);
void card_set_start_stop(void);  
void card_buzz_interval_delaytimer(void);
void card_bus_monitor(void);
uint8_t IC_BCC(uint8_t *SerBfr);


//-------------------------------------------------------------
//�궨��
#define   CARD_RX_PIN      P1.6    //ģ�⴮�ڵ���������
#define   CARD_TX_PIN      P1.7    //ģ�⴮�ڵ��������

#define   CARD_RXBUF_LEN   15
#define   CARD_TXBUF_LEN   10

//9600  -- ��IC����С��ʹ�õĲ�����Ϊ9600
#define   BIT_RX_T        833    //ʵ�ʼ��� = 833.33.. =1/9600 us           9600������1bit��ʱ����Ӧ��ʱ���ļ���ֵ   
#define   BIT_RX_T_1_2    416    //ʵ�ʼ��� = 416.66.. =(  1/9600)/2 us     9600������1/2bit��ʱ����Ӧ��ʱ���ļ���ֵ




//����ֵ_�°� --����δ��
#define   KEY_NONE            0x00
#define   KEY_SET             0x20
#define   KEY_UP              0x08
#define   KEY_DOWN            0x10
#define   KEY_BUZ             0x40 
#define   KEY_SET_AND_UP      0x28 
#define   KEY_SET_AND_DOWN    0x30 
#define   KEY_SET_AND_BUZ     0x60 
#define   KEY_UP_AND_DOWN     0x18 
#define   KEY_UP_AND_BUZ      0x48 
#define   KEY_DOWN_AND_BUZ    0x50 


//---------------------------------------------------------------
//����ı��� 
flag_type card_com,card_com_2;


uint8_t   guc_card_rx_byte;                   //���յĵ����ֽ�
uint8_t   guc_card_rx_bit_index;              //�ֽڵ�λ���Ƚ�������
uint8_t   guc_card_rx_byte_index;             //����֡�ֽڳ��Ƚ�������
uint8_t   guc_card_rx_buffer[CARD_RXBUF_LEN]= {0}; //���յ����ݻ���
uint8_t   gss_card_rx_end_delaytimer;         //�趨�ĳ�ʱʱ��

uint8_t   guc_card_tx_byte;                    //���͵ĵ����ֽ�
uint8_t   guc_card_tx_bit_index;               //�ֽڵ�λ���ȷ�������
uint8_t   guc_card_tx_byte_index;              //����֡�ֽڳ��ȷ�������
uint8_t   guc_card_tx_temp;                    //������ʱ����

uint8_t   gss_card_tx_20_delaytimer;           //����20����ʱ������
uint8_t   gss_card_tx_auto_delaytimer;         //������20��೤ʱ�䷢���Զ�����
int8_t    gss_card_tx_order_interval_delaytime;  //���·���һ�鼤��ָ�����ʱ����ֹ��;�ϵ�

uint8_t   guc_IC_access_state;                 // ����״̬                  

uint8_t   guc_card_buzz_interval_delaytime;
uint8_t   guc_erasure_IC_all_data_order;       //�������巢�͵����ݣ���������ָ���Ƿ�Ҫע��IC����

uint8_t   guc_card_bus_monitor_time;           //������߿��е�ʱ��

uint8_t   guc_card_tx_cnt;                     //������һ��ָ��ļ���

uint8_t const  guc_card_20_order = 0x20;       //������������ 0x20��ģ�齫ȷ��ͨ�Ų����ʣ�����Ӧ 0x06
                                               //�����Ƚ�����һ��������ģ�鲻��Ӧ�κ��������͵�����

uint8_t const  guc_card_tx_buffer[CARD_TXBUF_LEN] = {  0x0a,0x02,0x4e,0x04,0x05,
                                                       0x03,0x26,0x00,0x9d,0x03};   //�����Զ���������
                                                
                                                  
                                                  
                                               

//--------------------------------------------------------------------
//���������
// ��1��λ
const uint8_t SET_bit_table[8]=
{
    SET0,
    SET1,
    SET2,
    SET3,
    SET4,
    SET5,
    SET6,
    SET7
};
//--------------------------------------------------------------------
// ��0��λ
const uint8_t CLR_bit_table[8]=
{
    CLR0,
    CLR1,
    CLR2,
    CLR3,
    CLR4,
    CLR5,
    CLR6,
    CLR7
};


/***********************************************************************************************************************************
��������: ��⵽���½����ж�ʱ�Ĵ���,�������Ǽ�⵽���ڵ���ʼλ


����λ��:�ж�5��ϵͳ��������-------------------------------------ok
************************************************************************************************************************************/
void card_interrupt_check(void)
{                   
                              //ֻ����ʼλ���ⲿ�жϲ�Ϊ��ȷ
    TDR02 = BIT_RX_T_1_2;     //��ʱ���ж�����Ϊ�ò�����1/2λ��ʱ��
    R_TAU0_Channel2_Start();  //����ʱ���жϣ�����TCR������ʼ��ʱ
    
    R_INTC5_Stop();           //���ⲿ�ж�
    
    //card_rx_init();                //�ڲ���ʼ������̫�࣬ ��ӻ�Ӱ��ʱ�򡣡� 
    bflg_card_receive_allow = 1;     //������ձ�־
}

/*********************************************************************************************************************************
��������: �����ݽ��н���

ʵʩ��ʩ��     ��⵽�½���(��ʼλ)���жϣ�Ȼ����ⲿ�жϣ��Ըò�����1/2λ��ʱ�����ʱ���жϣ�
          Ȼ���޸��Ըò����ʵ�1λʱ��Ϊ��ʱ���жϵ����ڣ�
            
����λ��: ϵͳ��ʱ��2�ж�-----------------------------------------------ok
**********************************************************************************************************************************/
void card_receive_data(uint8_t length)
{       
    bflg_rx_valid_level = CARD_RX_PIN;         //��ȡ���ŵ�ƽ
        
    //---------------------------------------------------------------------------------
    //�����ֽڵĽ���                                                       //ִ��˳��_2
    if (bflg_card_rx_startbit == 1)            // ��ʼ����һ���ֽڵ�ÿһλ     
    {   
        if (bflg_rx_valid_level == 0)          //bit=0
        {
            guc_card_rx_byte &= CLR_bit_table[guc_card_rx_bit_index++];
        }
        else                                   //bit=1
        {
            guc_card_rx_byte |= SET_bit_table[guc_card_rx_bit_index++];  
        }
        //------------------------------------------------
        if (guc_card_rx_bit_index >= 8)       // ��9����
        {
            guc_card_rx_bit_index = 0;
            bflg_card_rx_startbit = 0;   
            bflg_card_rx_stopbit = 1;         //��ʾ������һ���ֽ�
        }
    }   
    //---------------------------------------------------------------------------------
    //�趨���ȵ�����֡����                                                    //ִ��˳��_3
    else if (bflg_card_rx_stopbit == 1)      //����֡����                     
    {       
        if (bflg_rx_valid_level == 1)        //ֹͣλ��ȷ
        {
            bflg_card_rx_stopbit = 0;
            bflg_card_rx_startbit = 0;
            guc_card_rx_buffer[guc_card_rx_byte_index++] = guc_card_rx_byte;
            
            if (guc_card_rx_byte_index >= length)   //15  
            {   
                bflg_card_rx_ok = 1;                //��ͨѶ���ս�����־  
                bflg_card_receive_allow = 0;
                bflg_card_rx_busy = 0;              //�����æ��־
            } 
            else
            {
                R_TAU0_Channel2_Stop();              //����1byte���ڴ�Ϊ���ܿ�����Ӧ
                R_INTC5_Start();  
            }
        }
        else                                        //ֹͣλ���󣬲�Ϊ��
        {
            bflg_card_stopbit_error = 1;
            bflg_card_receive_allow = 0;
            bflg_card_rx_busy = 0;  
        }
    }      
    //---------------------------------------------------------------------------------
    //��ʼλ�ж�                                                         //ִ��˳��_1
    else                                                                     
    {
        if (bflg_rx_valid_level == 0)            //��ʼλ��ȷ
        {
            TDR02 = BIT_RX_T;                    //�����趨��ʱ�ж�����
            TS0 |= _0004_TAU_CH2_START_TRG_ON;   //������ʱ�����ɽ�TCR����װ��TDR��ֵ�������¼���
            
            bflg_card_rx_startbit = 1;           //��ʼλ��ȷ����ʼ����
            bflg_card_rx_busy = 1;
        }
        else                                     //��ʼλ����
        {   
            bflg_card_startbit_error = 1;
            bflg_card_receive_allow = 0;
        }
    }
}

/*************************************************************************************************************************************************
��������:  ����оƬ�ڲ���������ĸտ�ʼʹ��ʱ���趨

�������: �趨
          1) �������һλ 0xfff �е�ֵ�Ƿ���0xad�жϴ�flash�Ƿ��ʼ��������
          2) �����ڶ�λ   0xffe �趨Ϊ���浱ǰ�Ѿ�����ĵ�ַ�ĵ��ֽڣ�δ����0
          3) ��������λ   0xffd �趨Ϊ���浱ǰ�Ѿ�����ĵ�ַ�ĸ��ֽڣ�δ����0
          ��ַ��Χ: 0~0x0fff
          
ע��:      1)��ַ����Ե�ַ,��Χ��: 0x00 ~ 0x0fff = 4K
         2)��block�ķ�Χ  :           0 ~ 3

��0: �洢IC���к�ʹ�ã� ��Χ         0x000--0x3ff                               
��1: �洢IC���к�ʹ�ã� ��Χ         0x400--0x7ff  ��Ϊһ��IC�����к���4���ֽڣ��������洢0x7ff/4 = 512�ţ���������99

��2: ���Ѵ���ֽ���(IC�����к�)��  ��Χ              0x800--0xbff(��д�����Ȳ��������ǰ���ģ�������һ���������)  
                                 ʹ��:�õ�ַ0x801�洢�Ѵ��ֽ������ĸ��ֽڣ�         
                                      �õ�ַ0x802�洢�Ѵ��ֽ������ĵ��ֽ�(����500��һ���ֽڴ洢����)
��3: ��ǳ�ʼ����        ��Χ      0xc00--0xfff  
                          ��ʹ��:��ַ0xfff  --д�̶����ֵ0xad       

����λ��:  ��ʼ�� system_init() ��-----------------------------------------ok
**************************************************************************************************************************************************/
void card_flash_init(void)
{   
    uint8_t read_data = 0,write_data = 0xad,address = 0;

    PFDL_Init(); 
    PFDL_Read(0xfff,1,&read_data);       //��ȡ���һλ������

    if(read_data != 0xad)                //ֵ���Ǳ���ѳ�ʼ�������ʼ��
    {
        PFDL_Erase(0x00);                //�������еĿ�
        PFDL_Erase(0x01);
        PFDL_Erase(0x02);
        PFDL_Erase(0x03);
       
        PFDL_Write(0xfff,1,&write_data);
        
        PFDL_Write(0x801,1,&address);    //��ǰ��ַʹ�õ�ַΪ0����δ������
        
        PFDL_Write(0x802,1,&address);    
    }

    PFDL_Close(); 
}

/*********************************************************************************************************************************
��������:   �������ذ巢�͵�ָ��������

����λ��:   ������-----------��ʱλ��---ok
**********************************************************************************************************************************/
void card_erase_all_data(void)
{
    uint8_t write_data = 0xad,address = 0;
    
    if(guc_erasure_IC_all_data_order == 1)
    {
        guc_erasure_IC_all_data_order = 0;
        
        PFDL_Init(); 
        
        PFDL_Erase(0x00);                 //�������еĿ�
        PFDL_Erase(0x01);
        PFDL_Erase(0x02);
        PFDL_Erase(0x03);
       
        PFDL_Write(0xfff,1,&write_data);       
        PFDL_Write(0x801,1,&address);    //��ǰ��ַʹ�õ�ַΪ0����δ������        
        PFDL_Write(0x802,1,&address);
        
        PFDL_Close(); 
        
        bflg_buzz_one_tick = 1;
    }
}

/*********************************************************************************************************************************
��������:   ������յ�������������

��������:  ÿ�α���4���ֽڣ���IC����ID��
           
����λ��:   card_set_start_stop() ---------------------------------ok
**********************************************************************************************************************************/
uint8_t card_rx_data_deal(void)
{
    uint8_t  msb_byte = 0, lsb_byte = 0, i = 0, ic_bcc = 0;
    uint8_t  temp_flash_buffer[4] = {0};
    uint16_t flash_address = 0, group = 0;

    ic_bcc = IC_BCC(guc_card_rx_buffer);     //У��
              
    if((guc_card_rx_buffer[0] == 0x0f) && (guc_card_rx_buffer[14] == 0x03) && (guc_card_rx_buffer[13] == ic_bcc) )
    {                                                                      
        if((guc_com_rx_buffer[7] == 0x40) && (guc_com_rx_buffer[8] == 0x40))   //����ע��״̬,�������巢�͵�led1��led2ֵ
        {                                                                
            PFDL_Init();  
            
            PFDL_Read(0x801,1,&msb_byte);      //���ﵽ�ĵ�ַ���ֽ�
            PFDL_Read(0x802,1,&lsb_byte);      //���ﵽ�ĵ�ַ���ֽ�

            flash_address = ((flash_address | msb_byte)<<8) | lsb_byte; 
            
            if(flash_address > 0x18c)         //��ֹ������ֲ�ƷҪ�����99�� 99*4=396=0x18c��
            {   
                PFDL_Close();
                bflg_rx_data_deal_ok = 1; 
                return 0;
            }          
            else
            {
                group = flash_address / 4;
                
                for(i=0;i<group;i++)
                {
                    PFDL_Read(i*4,4,temp_flash_buffer);  //ѭ����ȡ�Ѵ������  
                    
                    if((guc_card_rx_buffer[9]  == temp_flash_buffer[0])&&    
                       (guc_card_rx_buffer[10] == temp_flash_buffer[1])&&
                       (guc_card_rx_buffer[11] == temp_flash_buffer[2])&&
                       (guc_card_rx_buffer[12] == temp_flash_buffer[3]))
                     {
                         PFDL_Close();                  
                         bflg_rx_data_deal_ok = 1; 
                         return 0;                       //���Ѵ����˳�
                     }
                }
                
                PFDL_Write(flash_address,4,&guc_card_rx_buffer[9]);  //δ����򱣴����ݵ�flash
                
                flash_address += 4;
                msb_byte = 0;
                lsb_byte = 0;
                msb_byte |= (uint8_t)(flash_address >> 8);
                lsb_byte |= (uint8_t)flash_address;
                
                PFDL_Erase(0x02);                    //д���ù��ĵ�ַ���Ȳ���������С��λ�ǿ�
                PFDL_Write(0x801,1,&msb_byte);       //��IC���к�������flash ���ֽ�
                PFDL_Write(0x802,1,&lsb_byte);       //���ֽ�
                PFDL_Close();
                
                bflg_rx_data_deal_ok = 1;           
                bflg_buzz_one_tick = 1;
            }

        }        
        else  // ����Ƿ�ע���
        {
            PFDL_Init();      
            
            PFDL_Read(0x801,1,&msb_byte);      //���ﵽ�ĵ�ַ���ֽ�
            PFDL_Read(0x802,1,&lsb_byte);      //���ﵽ�ĵ�ַ���ֽ� 
            flash_address = 0;
            flash_address = ((flash_address | msb_byte)<<8) | lsb_byte;       

            group = flash_address / 4;         //������ID

            if(group == 0)                     //��û�д洢��
            {            
                PFDL_Close(); 
                bflg_rx_data_deal_ok = 1;                
                return 0;
            }
            else
            {
                for(i=0;i<group;i++)
                {
                    PFDL_Read(i*4,4,temp_flash_buffer);   //������  
                    
                    if((guc_card_rx_buffer[9]  == temp_flash_buffer[0])&&
                       (guc_card_rx_buffer[10] == temp_flash_buffer[1])&&
                       (guc_card_rx_buffer[11] == temp_flash_buffer[2])&&
                       (guc_card_rx_buffer[12] == temp_flash_buffer[3]))
                     {
                
                        PFDL_Close();  
                         
                        guc_IC_access_state = 1;          //�ɿ�������bflg_com_tx_ok�����÷��Ͷ�μ���,��ֹ����
                        if(bflg_card_buzz_interval == 0)  //ÿ��� 2s ��
                        {
                            bflg_card_buzz_interval = 1;
                            guc_card_buzz_interval_delaytime = 0;

                            bflg_buzz_one_tick = 1;       //��������ʱע�͵�
                        }
                        
                        bflg_rx_data_deal_ok = 1;
                        return 0;
                     }
                }
                
                PFDL_Close();
                bflg_rx_data_deal_ok = 1;
            }
        }
   }
   else       //���յ����ݴ���
   {
        bflg_rx_data_deal_ok = 1;
   } 
   return 0;
}

/*********************************************************************************************************************************
��������: ģ�⴮�ڷ��ͺ���,ÿ1/9600us��һ���ж�

�¸ĵ�ģʽ:   ��IC��Ϊ��˫��   

����ʱ��:
        �ϵ�6s��/ÿ���6s��                1ms��      100ms��
                           --->0x20------>0x20------->�Զ��������
                           
����λ��: ϵͳ��ʱ��2���жϺ����У�--------ok
**********************************************************************************************************************************/
void card_send_data(uint8_t *txbuff,uint8_t length)
{  
    //---------------------------------------------------------------------------------------------
    //���͵����ֽ�                                                                      ִ��˳��_2
    if(bflg_card_tx_startbit == 1)             
    {
        guc_card_tx_temp = (uint8_t)((*(txbuff + guc_card_tx_byte_index) >> guc_card_tx_bit_index) & 0x01);
        if(guc_card_tx_temp == 1)
        {
            CARD_TX_PIN = 1;         
        }
        else
        {
            CARD_TX_PIN = 0;
        }

        guc_card_tx_bit_index++;                //���ÿ���ֽڵ�8λ�Ƿ�����
        if (guc_card_tx_bit_index > 7)
        {
            guc_card_tx_bit_index = 0;
            bflg_card_tx_startbit = 0;
            
            bflg_card_tx_stoptbit = 1;   
        }
    }
    
    //---------------------------------------------------------------------------------------------
    //������֡����                                                                      ִ��˳��_3
    else if(bflg_card_tx_stoptbit == 1)
    {
        CARD_TX_PIN = 1;                        //����ֹͣλ�ߵ�ƽ
        bflg_card_tx_stoptbit = 0;

        guc_card_tx_byte_index++;
        if(guc_card_tx_byte_index >= length)    //����֡��������˳�
        {
            bflg_card_tx_ok = 1;
            bflg_card_tx_busy = 0; 
            bflg_card_send_allow = 0;     
            guc_card_tx_byte_index = 0;  
            
            //--------------------------------------------------
            //������һ���Զ�����ָ��
            //����ʱ��1������ʱ��             2��bflg_card_send_allow = 1;         
                        
            guc_card_tx_cnt++;
            if(guc_card_tx_cnt == 1)       //1_��һ�η�����0x20
            {
                bflg_tx_20_delaytime = 1;  //��ʱ���͵ڶ���0x20 
                gss_card_tx_20_delaytimer = 0;
            }
            else if(guc_card_tx_cnt == 2)  //2_�ڶ��η�����0x20
            {
                bflg_tx_auto_delaytime = 1; //��ʱ�����Զ��������
                gss_card_tx_auto_delaytimer = 0;
            }
            else if(guc_card_tx_cnt == 3)  //3_�������Զ��������
            {
                guc_card_tx_cnt = 0;       //�����˲�����
            }
        }
    }
    
    //---------------------------------------------------------------------------------------------
    //������ʼλ                                                                       ִ��˳��_1
    else
    {
        CARD_TX_PIN = 0;                        //������ʼλ�͵�ƽ
        bflg_card_tx_busy = 1; 
        bflg_card_tx_startbit = 1;               

        TDR02 = BIT_RX_T;                      //�����趨��ʱ�ж�����
        TS0 |= _0004_TAU_CH2_START_TRG_ON;     //������ʱ�����ɽ�TCR����װ��TDR��ֵ�������¼���
    }
}

/*********************************************************************************************************************************
��������: �������һ��0x20����ʱ��ڶ��η���0x20

          
����λ��: 1ms��ʱ�ж� -----------------------ok
**********************************************************************************************************************************/
void card_tx_20_delaytime(void)
{
    if(bflg_tx_20_delaytime == 1)
    {
        gss_card_tx_20_delaytimer++;

        if(gss_card_tx_20_delaytimer >= 1)     //���͵�һ��0x20����ʱ1ms�ٷ��͵ڶ���20 
        {         
            gss_card_tx_20_delaytimer = 0;
            bflg_tx_20_delaytime = 0;
            
            bflg_card_send_allow = 1;
            card_tx_init();
            R_TAU0_Channel2_Start(); 
        }
    }
}

/*********************************************************************************************************************************
��������: ����������0x20����ʱ 100ms�����Զ��������

����λ��: 1ms��ʱ�ж��� --------------------ok
**********************************************************************************************************************************/
void card_tx_auto_delaytime(void)
{
    if(bflg_tx_auto_delaytime == 1)
    {
        gss_card_tx_auto_delaytimer++;
        if(gss_card_tx_auto_delaytimer >= 100) //��ʱ100ms�ٷ����Զ�����
        {
            gss_card_tx_auto_delaytimer = 0;
            bflg_tx_auto_delaytime = 0;

            bflg_card_send_allow = 1;
            card_tx_init();
            R_TAU0_Channel2_Start(); 
        }
    }
}

/**********************************************************************************************************************************************
��������: ����̶�ʱ�䷢�ͼ���IC��ָ��
          һ:�����ݽ���
          1����û�����ݽ��յ�����£�ÿ���6s���·���IC����ָ��

          ��:�����ݽ���:
          2�������ݽ���ʱ����0���еķ�����ص����ݣ������¼�ʱ�ط�
          3�����ط�ָ����е�һ��ʱ���������ݽ��գ�Ҳ��0���У����¼�ʱ�ط�
          
����λ��: 1s��ʱ�ж��� ----------------------------------------ok
***********************************************************************************************************************************************/
void card_tx_order_interval(void) 
{
    gss_card_tx_order_interval_delaytime++;
    if(gss_card_tx_order_interval_delaytime >= REPEAT_TX_TIME)  // 6s
    {
        gss_card_tx_order_interval_delaytime = 0;
        
        bflg_card_send_allow = 1;
        card_tx_init();
        R_TAU0_Channel2_Start(); 
    }
}

/**********************************************************************************************************************************************
��������: �������ݽ���ʱ��  ��0���й����ط�"IC����ָ��"������
          
����λ��:  ��ѭ����------ok
***********************************************************************************************************************************************/
void card_reset_repeat_tx_order(void)
{
    if(bflg_card_reset_repeat_tx_order == 1)
    {
        bflg_card_reset_repeat_tx_order = 0;
        
        //���¼�ʱ
        gss_card_tx_order_interval_delaytime = 0;
        
        //�ܵķ���
        bflg_tx_20_delaytime = 0;
        bflg_tx_auto_delaytime = 0;
        gss_card_tx_20_delaytimer = 0;
        gss_card_tx_auto_delaytimer = 0;
        
        //����ϸ��
        bflg_card_tx_busy = 0;
        bflg_card_tx_stoptbit = 0;
        bflg_card_tx_startbit = 0;
        guc_card_tx_byte_index = 0;
        guc_card_tx_bit_index = 0;
    }
}

/*********************************************************************************************************************************
��������: ���ճ�ʱ��־

��ע��    ���ģ��û�г�ʱ����ΪֻҪ�Ѵ�����ʼλbusy�ˣ���ʱ���᲻ͣ�ж�ȥ����һ�����������ݣ�
       ������ֹͣλ����ֹͣ���գ�

����λ��: 1ms��ʱ����---------------ok
**********************************************************************************************************************************/
void card_rx_end_delaytime(void) //ͨѶ���������ʱ������1ms��ʱ�����е���
{
    if (bflg_card_rx_busy == 1)  //�����ǰ����æ
    {
        gss_card_rx_end_delaytimer++;
        if (gss_card_rx_end_delaytimer >= 50)  //50ms
        {
            gss_card_rx_end_delaytimer = 0;
            
            bflg_card_rx_end = 1;
            bflg_card_rx_busy = 0;
            bflg_card_receive_allow = 0;
        }
    }
    else
    {        
        gss_card_rx_end_delaytimer = 0;
    }
}

/*********************************************************************************************************************************
��������: ���ճ�ʼ��

����λ��:     card_rx_data_deal() ---ok 
**********************************************************************************************************************************/
void card_rx_init(void)
{
    uint8_t i;
    
    for (i = 0; i < CARD_RXBUF_LEN; i++)
    {
        guc_card_rx_buffer[i] = 0;       //����ջ�����
    }

    bflg_card_rx_startbit = 0;
    bflg_card_rx_stopbit = 0;
    bflg_card_rx_busy = 0;
    
    guc_card_rx_bit_index = 0;
    guc_card_rx_byte_index = 0;
}

/*********************************************************************************************************************************
��������: ���ͳ�ʼ��

����λ��:     ���ͼ�����������һ��20������ڶ���20-------------ok
**********************************************************************************************************************************/
void card_tx_init(void)
{
    bflg_card_tx_startbit = 0;
    bflg_card_tx_stoptbit = 0;
    bflg_card_tx_busy = 0;
    
    guc_card_tx_bit_index = 0;
    guc_card_tx_byte_index = 0;
}

/*********************************************************************************************************************************
��������: ������־��Ӧ�Ŀ����ж�

����λ��:   ������ѭ��   ---OK
**********************************************************************************************************************************/
void card_set_start_stop(void)
{
    if(bflg_test_mode == 0)
    {
        //--------------------------------------------------------------------
        if((bflg_card_startbit_error ==1)||(bflg_card_stopbit_error ==1) ||  //���մ���
            (bflg_card_rx_end == 1))
        {
            bflg_card_startbit_error = 0;
            bflg_card_stopbit_error = 0;
            bflg_card_rx_end = 0;
            
            bflg_card_bus_moitor = 0;  //��ʼ���߼��

            R_TAU0_Channel2_Stop();    //�ض�ʱ��
        }            
        //--------------------------------------------------------------------
        if(bflg_card_rx_ok == 1)       //���ճɹ�
        {
            bflg_card_rx_ok = 0;
                        
            R_TAU0_Channel2_Stop();    //�ض�ʱ��
            card_rx_data_deal();       //ͨѶ�������ݴ������
        }
        //--------------------------------------------------------------------
        if(bflg_rx_data_deal_ok == 1)  //��������յ�����    
        {
            bflg_rx_data_deal_ok = 0;

            bflg_card_bus_moitor = 0; //��ʼ���߼��
        }
        //--------------------------------------------------------------------
        if(bflg_card_tx_ok == 1)      //�������
        {
            bflg_card_tx_ok = 0;
            R_TAU0_Channel2_Stop();
        }  

       //--------------------------------------------------------------------
       if(bflg_buzz_one_tick == 1)
       {
            bflg_buzz_one_tick = 0;
            
            bflg_key_buzz_delaytime = 1;  // ��������һ��
            gss_key_buzz_delaytimer = 0;               
            R_PCLBUZ0_Start();
       }
    }
}

/**********************************************************************************************************************************
��������: ������ÿ����⵽��ȷ�ź�,����೤ʱ����

����λ��:  1s -----------------------------------ok
***********************************************************************************************************************************/
void card_buzz_interval_delaytimer(void)
{   
    if(bflg_card_buzz_interval == 1)
    {
        guc_card_buzz_interval_delaytime++;
        if(guc_card_buzz_interval_delaytime >=2)   //�������ڶ������ϣ����2s��
        {                                          //�����1s�죬�״ηſ�������2��������ʱ�����д˺���
            guc_card_buzz_interval_delaytime = 0;
            bflg_card_buzz_interval = 0;
        }
    }
}

/**********************************************************************************************************************************
��������: �������������ߵĿ���ʱ�䣬������10ms���ⲿ�жϣ��ȴ������½���

��ע:       IC��ʾ������������һ֡������16ms���ң����67ms�����ٴη���

����λ��:  1ms ------------------------------ok
***********************************************************************************************************************************/
void card_bus_monitor(void)
{
    if(bflg_card_bus_moitor == 0)
    {
        if(CARD_RX_PIN == 1)                     //���߸ߵ�ƽ����ʱ
        {
            guc_card_bus_monitor_time++;
            if(guc_card_bus_monitor_time >= 10)   //10ms
            {
                guc_card_bus_monitor_time = 0;
                bflg_card_bus_moitor = 1;

                card_rx_init();           //��ʼ��             
                R_INTC5_Start();          //���ⲿ�ж�
            }
        }
        else
        {
            guc_card_bus_monitor_time = 0;           
        }
    }
}

/**********************************************************************************************************************************
��������: �Խ��յ�IC���ݽ���У��--���ȡ���ķ�ʽ����Ϊģ����˵��
          IC�����͵�У��������ȡ����


����λ��:  card_rx_data_deal()�� ------------------------------ok
***********************************************************************************************************************************/
uint8_t IC_BCC(uint8_t *SerBfr)
{
    uint8_t BCC = 0,i = 0;
    
    for (i = 0; i < (SerBfr[0] - 2); i++) 
    {
        BCC ^= SerBfr[i];
    }
    //SerBfr[SerBfr[0]-2] = ~BCC;    //��SerBfr[13]�� SerBfr[0]Ϊ�ֽ��ܳ�����������

    //BCC = SerBfr[SerBfr[0]-2];  

    BCC = (uint8_t)~BCC;
    return BCC;
}


/*******************************************END OF THE FILE****************************************************/
