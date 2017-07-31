#ifndef _USB_C_
#define _USB_C_
//------------------------------------------------------------------------------
//�����ļ�����
#include "r_cg_macrodriver.h"

#include "h_type_define.h"

#include "m_usb.h"
#include "r_cg_serial.h"

//------------------------------------------------------------------------------
//��������
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
uint8_t func_timer_4302_deal(uint8_t luc_timer_tmp); //ʱ��ת�����ݴ�����
uint8_t func_4302_timer_deal(uint8_t luc_4302_tmp);  //����ת��ʱ�䴦����




//------------------------------------------------------------------------------
//��־����
flag_type flg_com_usb,flg_com_usb_1;


//------------------------------------------------------------------------------
//��������
uint8_t guc_com_tx_buffer_usb[48];     //ͨѶ���ͻ�����
uint8_t guc_com_rx_buffer_usb[7];      //ͨѶ���ջ�����
uint8_t guc_usb_buffer[31];            //��ʾ��������л�ȡ�ķ��͸�usb�������

int16_t gss_com_tx_delaytimer_usb;     //ͨѶ������ʱ��ʱ��
int16_t gss_com_rx_delaytimer_usb;     //ͨѶ������ʱ��ʱ��

int16_t gss_com_rx_end_delaytimer_usb; //ͨѶ�������־
int16_t gss_com_fault_delaytimer_usb;  //ͨѶ������ʱ��ʱ��

uint8_t guc_com_tx_point_usb;          //ͨѶ��������
uint8_t guc_com_rx_point_usb;          //ͨѶ��������


//usb��״̬
uint8_t guc_Udisk_state;                //U�̵�״̬_ֻʹ�����2λ
uint8_t guc_usb_state;                  //usb���ӵ�״̬�Ƿ��й���


/***********************************************************************************************************************************************
�������: �� USB ���ӽ���ͨѶ��ʹ�ô���0��USB������ͨѶ��

ʵ��ʹ��: u������USB���ӵ�USB�ӿڣ��ļ����Զ��洢��U���У�
***********************************************************************************************************************************************/

//------------------------------------------------------------------------------
//��������
/***********************************************************************************************************************************************
��������:������0�ĳ�ʼ����ʹһ��ʼ���������----�ӻ�ģʽ

����λ��:  ��ʼ���� system_init() �У�----------OK
***********************************************************************************************************************************************/
void com_init_usb(void)
{
    bflg_com_rx_delaytime_usb = 1;
    gss_com_rx_delaytimer_usb = 5;
    
    bflg_com_tx_delaytime_usb = 0;
    gss_com_tx_delaytimer_usb = 0;
}

/***********************************************************************************************************************************************
��������:�� ����ǰ��ʼ�����ڽ��մ��������������ǰ��������buffer��գ�

����λ��:   ��ѭ����------------------------------ok
***********************************************************************************************************************************************/
void com_rx_init_usb(void)   //ͨѶ���ճ�ʼ����������ѭ�������е���
{
    uint8_t i;
    
    for (i = 0; i < 7; i++)  //13
    {
        guc_com_rx_buffer_usb[i] = 0;  //����ջ�����
    }
    
    bflg_com_rx_busy_usb = 0;
    guc_com_rx_point_usb = 0;
}

/***********************************************************************************************************************************************
��������:��  ���ڽ������ݳɹ�ʱ���Խ��յ����ݽ��д���
             �˴���� [2]�� 0x0A����У����ȷ����ɷ�������

����λ��:   ��ѭ����( bflg_com_rx_ok_usb==1)------------------------------ok
***********************************************************************************************************************************************/
void com_rx_data_deal_usb(void)   //ͨѶ�������ݴ����������ѭ�������е���
{
    uint8_t i,cksum;
    uint16_t lus_tmp;
    word_type com_crc;
    //------------------------------------------------------

    if(guc_com_rx_buffer_usb[2] == 0x05)         //������������������,��usb�巢��
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
            bflg_com_tx_delaytime_usb = 1;       //�÷�����ʱ��־
            gss_com_tx_delaytimer_usb = 10;      //������ʱ��ʱ����ֵ  //peak ������ɺ���ʱ10ms ��ʼ����
            
            bflg_com_rx_delaytime_usb = 0;       //�巢����ʱ��־
            gss_com_rx_delaytimer_usb = 0;       //������ʱ��ʱ������
            
            bflg_com_fault_usb = 0;              //��ͨѶ���ϱ�־
            gss_com_fault_delaytimer_usb = 0;    //��ͨѶ���ϼ�ʱ��
            */
        }
    }
    else if(guc_com_rx_buffer_usb[2] == 0x07)          //�����U��״̬�㱨�������ذ巢��
    {
        cksum = checksum((guc_com_rx_buffer_usb+2), 4);  
        if(cksum == guc_com_rx_buffer_usb[6])
        {
            guc_Udisk_state = guc_com_rx_buffer_usb[4]; //U��״̬     =0����/1æ/2���/ʧ��
            guc_usb_state   = guc_com_rx_buffer_usb[5]; //usb��״̬=0����/1~ff����
        }
    }
    else if((guc_com_rx_buffer_usb[2] & 0x80) == 0x80)            //�����usb���Ӧ��
    {
        cksum = checksum((guc_com_rx_buffer_usb+2), 3);  
        if(cksum == guc_com_rx_buffer_usb[5])
        {
           /* 
            if(guc_com_rx_buffer_usb[2] = 0x81)  //����֡Ӧ��
            {
                if(guc_com_rx_buffer_usb[4] == 0x00)      //��ȷ
                if(guc_com_rx_buffer_usb[4] == 0xff)      //����
            }
            if(guc_com_rx_buffer_usb[2] = 0x82)  //�¼�֡Ӧ��
            if(guc_com_rx_buffer_usb[2] = 0x83)  //ʱ��֡Ӧ��
            if(guc_com_rx_buffer_usb[2] = 0x84)  //����֡Ӧ��
            if(guc_com_rx_buffer_usb[2] = 0x86)  //����֡Ӧ��
            */

        }
    } 

    bflg_com_rx_delaytime_usb = 1;        //peak ���������ʱһ��ʱ���ȴ����� ??
    gss_com_rx_delaytimer_usb = 50;       
}

/***********************************************************************************************************************************************
��������:��ѡ����ʲô����:1����Ӧ����֡(����/ʱ��/����)  2���¼�����  3����������
           ����Ҫ���͵ĲŽ��з��ͣ�---���͵ķ���λ��
           
����λ��:  ��ѭ����------------------------------------------ok
***********************************************************************************************************************************************/
void com_tx_type_rank_usb(void)

{
    //û����ʱ�ȴ����͵�/û������Ҫ���͵�/û������æ�ŷ��͵� ��ʱ�����Ƿ���Է����¸�һ����
    if((bflg_com_tx_delaytime_usb != 1)&& (bflg_com_allow_tx_usb != 1)&&(bflg_com_tx_busy_usb == 0)) 
    {
        if(bflg_request_data_type == 1)          //��������֡
        { 
            bflg_request_data_type = 0;
            bflg_init_data_type = 1;
            
            bflg_com_tx_delaytime_usb = 1;       //�÷�����ʱ��־
            gss_com_tx_delaytimer_usb = 10;      //������ʱ��ʱ����ֵ  //peak ������ɺ���ʱ10ms ��ʼ����
            /*
            bflg_com_rx_delaytime_usb = 0;       //�巢����ʱ��־
            gss_com_rx_delaytimer_usb = 0;       //������ʱ��ʱ������

            bflg_com_fault_usb = 0;              //��ͨѶ���ϱ�־
            gss_com_fault_delaytimer_usb = 0;    //��ͨѶ���ϼ�ʱ��
            */
        }
        else if((bflg_request_time_type == 1)||((guc_usb_buffer[0] & 0x20) == 0x20))  //ʱ������֡
        {   
            guc_usb_buffer[0] &= 0xdf;          //��5λ(�Ƿ���ʱ���޸�����)��0
            
            bflg_request_time_type = 0;         //usb��ʱ��������������޸�����ʱ�䷢ͬһ����      
            bflg_init_time_type = 1;          
            
            bflg_com_tx_delaytime_usb = 1;       
            gss_com_tx_delaytimer_usb = 10; 
        }
        else if(bflg_request_machine_type == 1)  //��������֡
        {
            bflg_request_machine_type = 0;
            bflg_init_machine_type = 1;

            bflg_com_tx_delaytime_usb = 1;      
            gss_com_tx_delaytimer_usb = 10; 
        }
        else if((guc_usb_buffer[0] & 0x80) == 0x80)     
        {
            guc_usb_buffer[0] &= 0x7f;               //��7λ(�Ƿ����¼�����)��0
            bflg_init_event_type = 1;

            bflg_com_tx_delaytime_usb = 1;      
            gss_com_tx_delaytimer_usb = 10; 
        }
        else if((guc_usb_buffer[0] & 0x40) == 0x40)  //���Ϳ���֡��������� --�����崫���
        {
            guc_usb_buffer[0] &= 0xbf;               //��6λ(�Ƿ��п���ָ��)��0
            bflg_init_control_type = 1;
            
            bflg_com_tx_delaytime_usb = 1;      
            gss_com_tx_delaytimer_usb = 10; 
        }
    }
}
/***********************************************************************************************************************************************
��������:�� ���ڷ���ǰ�ĳ�ʼ������Ҫ���͵����ݷŵ�����buffer�У�

����λ��:   ��ѭ���� (bflg_com_allow_tx_usb == 1)---------------------------ok
***********************************************************************************************************************************************/
void com_tx_init_usb(void)   //ͨѶ���ͳ�ʼ����������ѭ�������е���
{
    word_type com_crc;
    word_type lss_tmp;

    if(bflg_init_data_type == 1)          //�ظ���������֡ 17_bytes
    {
        bflg_init_data_type = 0;
        bflg_data_type_being_tx = 1;    
        
        guc_com_tx_buffer_usb[0] = 0x68;  //֡ͷ
        guc_com_tx_buffer_usb[1] = 0x96;  
        guc_com_tx_buffer_usb[2] = 0x01;  //֡����
        guc_com_tx_buffer_usb[3] = 0x0c;  //���س���
        
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
        
        guc_com_tx_buffer_usb[16] = checksum((guc_com_tx_buffer_usb+2), 14);   //�ۼӺ�У��
    }
    else if(bflg_init_time_type == 1)     //�ظ�ʱ������֡ 11_bytes
    {
        bflg_init_time_type = 0;
        bflg_time_type_being_tx = 1;  

        guc_com_tx_buffer_usb[0] = 0x68;  //֡ͷ
        guc_com_tx_buffer_usb[1] = 0x96;  
        guc_com_tx_buffer_usb[2] = 0x03;  //֡����
        guc_com_tx_buffer_usb[3] = 0x06;  //���س���

        guc_com_tx_buffer_usb[4] = func_timer_4302_deal(guc_usb_buffer[19]); //--- ��ȷ�� 17-->0x17
        guc_com_tx_buffer_usb[5] = func_timer_4302_deal(guc_usb_buffer[20]);
        guc_com_tx_buffer_usb[6] = func_timer_4302_deal(guc_usb_buffer[21]);
        guc_com_tx_buffer_usb[7] = func_timer_4302_deal(guc_usb_buffer[22]);
        guc_com_tx_buffer_usb[8] = func_timer_4302_deal(guc_usb_buffer[23]);
        guc_com_tx_buffer_usb[9] = func_timer_4302_deal(guc_usb_buffer[24]);

        guc_com_tx_buffer_usb[10] = checksum((guc_com_tx_buffer_usb+2), 8);   
    }
    else if(bflg_init_machine_type == 1)    //�ظ���������֡ 7_bytes
    {
        bflg_init_machine_type = 0;
        bflg_machine_type_being_tx = 1;    

        guc_com_tx_buffer_usb[0] = 0x68;    //֡ͷ
        guc_com_tx_buffer_usb[1] = 0x96;  
        guc_com_tx_buffer_usb[2] = 0x06;    //֡����
        guc_com_tx_buffer_usb[3] = 0x02;    //���س���

        guc_com_tx_buffer_usb[4] = guc_usb_buffer[29];
        guc_com_tx_buffer_usb[5] = guc_usb_buffer[30];

        guc_com_tx_buffer_usb[6] = checksum((guc_com_tx_buffer_usb+2), 4);   
    }
    else if(bflg_init_event_type == 1)      //�¼����� 11bytes
    {
        bflg_init_event_type = 0;
        bflg_event_type_being_tx = 1;    

        guc_com_tx_buffer_usb[0] = 0x68;    //֡ͷ
        guc_com_tx_buffer_usb[1] = 0x96;  
        guc_com_tx_buffer_usb[2] = 0x02;    //֡����
        guc_com_tx_buffer_usb[3] = 0x06;    //���س���

        guc_com_tx_buffer_usb[4] = guc_usb_buffer[13];
        guc_com_tx_buffer_usb[5] = guc_usb_buffer[14];
        guc_com_tx_buffer_usb[6] = guc_usb_buffer[15];
        guc_com_tx_buffer_usb[7] = guc_usb_buffer[16];
        guc_com_tx_buffer_usb[8] = guc_usb_buffer[17];
        guc_com_tx_buffer_usb[9] = guc_usb_buffer[18];

        guc_com_tx_buffer_usb[10] = checksum((guc_com_tx_buffer_usb+2), 8);
    }
    else if(bflg_init_control_type == 1)    //�������� 9�ֽ�
    {
        bflg_init_control_type = 0;
        bflg_control_type_being_tx = 1;   

        guc_com_tx_buffer_usb[0] = 0x68;    //֡ͷ
        guc_com_tx_buffer_usb[1] = 0x96;  
        guc_com_tx_buffer_usb[2] = 0x04;    //֡����
        guc_com_tx_buffer_usb[3] = 0x04;    //���س���

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
��������:�� �೤ʱ��֮�� �������

����λ��:   1ms��ʱ��------------------------------ok
***********************************************************************************************************************************************/
void com_rx_delaytime_usb(void)     //ͨѶ������ʱ������1ms��ʱ�����е���
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
��������:�� �೤ʱ��������� 

����λ��:   1ms��ʱ��------------------------------ok
***********************************************************************************************************************************************/
void com_tx_delaytime_usb(void)     //ͨѶ������ʱ������1ms��ʱ�����е���
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
��������:�����趨�Ľ���ʱ�䵱�У�û�н����꣬���˳����գ�  

����λ��:   1ms��ʱ��------------------------------ok
***********************************************************************************************************************************************/
void com_rx_end_delaytime_usb(void) //ͨѶ���������ʱ������1ms��ʱ�����е���
{
    if (bflg_com_rx_busy_usb == 1)  //�����ǰ����æ
    {
        gss_com_rx_end_delaytimer_usb++;
        if (gss_com_rx_end_delaytimer_usb >= 55)    //peak 50--->55  ���ڷ���48���ֽ�
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
��������:��  ������

����λ��:   1ms��ʱ��------------------------------δ��
***********************************************************************************************************************************************/
void com_fault_delaytime_usb(void)     //ͨѶ������ʱ������1s��ʱ�����е���
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
��������:����ϵͳ�жϽ����ж��н������ݵĽ���  

����λ��:  ϵͳ�Ĵ��ڽ����жϺ�����------------------------------ok
***********************************************************************************************************************************************/
void com_rx_int_usb(uint8_t rx_data)   //ͨѶ�����жϳ��� ���ڽ����ж��е���
{
    uint8_t luc_com_rx_byte;  //�����ֽ���ʱ������
    
    gss_com_rx_end_delaytimer_usb = 0; //���������ʱ��ʱ��
    //--------------------------------------------------
    luc_com_rx_byte = rx_data;
    
    if ((bflg_com_rx_busy_usb == 0) && (luc_com_rx_byte == 0x68))    //����޽���æ��֡ͷ��ȷ
    {
        bflg_com_rx_busy_usb = 1;
        
        guc_com_rx_buffer_usb[0] = luc_com_rx_byte;
        guc_com_rx_point_usb = 1;
    }
    else if ((bflg_com_rx_busy_usb == 1))         //�������æ
    {
        guc_com_rx_buffer_usb[guc_com_rx_point_usb++] = luc_com_rx_byte;

        if(guc_com_rx_buffer_usb[1] == 0x96)
        {
            if ((guc_com_rx_buffer_usb[2] == 0x05) && (guc_com_rx_point_usb >= 7))      //��������������ҽ������
            {
                guc_com_rx_point_usb = 0;         //������ֽ�����
                bflg_com_rx_busy_usb = 0;         //�����æ��־
                bflg_com_rx_ok_usb = 1;           //�ý��ճɹ���־
            }
            else if ((guc_com_rx_buffer_usb[2] == 0x07) && (guc_com_rx_point_usb >= 7))  //�����U��״̬�ҽ������
            {
                guc_com_rx_point_usb = 0;         
                bflg_com_rx_busy_usb = 0;        
                bflg_com_rx_ok_usb = 1;          
            }
            else if (((guc_com_rx_buffer_usb[2] & 0x80) == 0x80) && (guc_com_rx_point_usb >= 6))//�����Ӧ���ҽ������
            {
                guc_com_rx_point_usb = 0;        
                bflg_com_rx_busy_usb = 0;        
                bflg_com_rx_ok_usb = 1;          
            }
            else if(guc_com_rx_point_usb >= 7)   //����ֵԽ��
            {
                guc_com_rx_point_usb = 0; 
                bflg_rx_data_error_usb = 1;
            }
        }
        else                                     //��[1]��
        {
            guc_com_rx_point_usb = 0; 
            bflg_rx_data_error_usb = 1;
        }
    }
}

/***********************************************************************************************************************************************
��������:�� ��ϵͳ�ķ����ж��н������ݵķ��ͣ� 

����λ��:   ϵͳ�Ĵ��ڷ����жϺ�����------------------------------ok
***********************************************************************************************************************************************/
void com_tx_int_usb(void)    //ͨѶ�����жϳ���  ���ڷ����ж��е���
{
    if (bflg_com_tx_busy_usb == 1)
    {
        if(bflg_data_type_being_tx == 1)         //��������֡
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
        else if(bflg_time_type_being_tx == 1)    //����ʱ��֡
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
        else if(bflg_machine_type_being_tx == 1) //���ͻ���֡
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
        else if(bflg_event_type_being_tx == 1)   //���ͱ����¼�
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
        else if(bflg_control_type_being_tx == 1) //���Ϳ��Ʋ���
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
��������: ��usb�巢�͹�����U��״̬���д�����ٷ��͸�����(�������ݴ�����������)

����λ��:    -----------------------------δ��
***********************************************************************************************************************************************/
void usb_U_disk_state(void)
{
    
}


/***********************************************************************************************************************************************
��������:���ۼӺ�У��

����λ��:  ��.c�ļ��еĺ���ʹ�� ---------OK

 FF FF  0A 00 00 00 00 00 01 01 4D 01  5A  ��ѯָ�USB�巢����ʾ��Ĳ�ѯָ��
***********************************************************************************************************************************************/
uint8_t checksum (uint8_t *buffer, uint8_t size)    //�����ۼӺ�У��
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
��������: ����0���տ�ʼ����(Ϊ��ʹ������ڿ�������ȫ˫�������պͷ��͵Ŀ�ʼ�ͽ��շֿ�)

����λ��:    ------------------------------δ��
***********************************************************************************************************************/
void USB_RX_UART0_Start(void)
{
    //STIF0 = 0U;    /* clear INTST0 interrupt flag */  //�巢���жϱ�־
    //STMK0 = 0U;    /* enable INTST0 interrupt */      //ʹ�ܷ����ж�
    SRIF0 = 0U;    /* clear INTSR0 interrupt flag */  //������жϱ�־
    SRMK0 = 0U;    /* enable INTSR0 interrupt */      //ʹ�ܽ����ж�
    SREIF0 = 0U;   /* clear INTSRE0 interrupt flag */ //������жϱ�־
    SREMK0 = 0U;   /* enable INTSRE0 interrupt */     //ʹ�ܴ����ж�
    //SO0 |= _0001_SAU_CH0_DATA_OUTPUT_1;    /* output level normal */  //���������ʼΪ�ߵ�ƽ����Ϊ�͵�ƽΪ��ʼλ
    //SOE0 |= _0001_SAU_CH0_OUTPUT_ENABLE;    /* enable UART0 output */ //�������
    //SS0 |= _0002_SAU_CH1_START_TRG_ON | _0001_SAU_CH0_START_TRG_ON;    /* enable UART0 receive and transmit */
    SS0 |= _0002_SAU_CH1_START_TRG_ON;        //������ջ��ͣ�ż��λ���ͣ�����λ����
}

/***********************************************************************************************************************
��������: ����0����ֹͣ����

����λ��:    ------------------------------δ��
***********************************************************************************************************************/
void USB_RX_UART0_Stop(void)
{
    ST0 |= _0002_SAU_CH1_STOP_TRG_ON;    /* disable UART0 receive and transmit */
    //ST0 |= _0002_SAU_CH1_STOP_TRG_ON | _0001_SAU_CH0_STOP_TRG_ON;    /* disable UART0 receive and transmit */
    //SOE0 &= ~_0001_SAU_CH0_OUTPUT_ENABLE;    /* disable UART0 output */ //ʧ�����
    //STMK0 = 1U;    /* disable INTST0 interrupt */     //ֹͣ���
    //STIF0 = 0U;    /* clear INTST0 interrupt flag */  //�巢���жϱ�־
    SRMK0 = 1U;    /* disable INTSR0 interrupt */     //ʧ�ܽ����ж�
    SRIF0 = 0U;    /* clear INTSR0 interrupt flag */  //������жϱ�־
    SREMK0 = 1U;   /* disable INTSRE0 interrupt */    //ʧ�ܴ����ж�
    SREIF0 = 0U;   /* clear INTSRE0 interrupt flag */ //������жϱ�־
}


/***********************************************************************************************************************
��������: ����0���Ϳ�ʼ����

����λ��:    ------------------------------δ��
***********************************************************************************************************************/
void USB_TX_UART0_Start(void)
{
    STIF0 = 0U;    /* clear INTST0 interrupt flag */  //�巢���жϱ�־
    STMK0 = 0U;    /* enable INTST0 interrupt */      //ʹ�ܷ����ж�
    //SRIF0 = 0U;    /* clear INTSR0 interrupt flag */  //������жϱ�־
    //SRMK0 = 0U;    /* enable INTSR0 interrupt */      //ʹ�ܽ����ж�
    //SREIF0 = 0U;   /* clear INTSRE0 interrupt flag */ //������жϱ�־
    //SREMK0 = 0U;   /* enable INTSRE0 interrupt */     //ʹ�ܴ����ж�
    SO0 |= _0001_SAU_CH0_DATA_OUTPUT_1;    /* output level normal */  //���ڿ�ʼΪ�ߵ�ƽ����Ϊ�͵�ƽΪ��ʼλ
    SOE0 |= _0001_SAU_CH0_OUTPUT_ENABLE;    /* enable UART0 output */ //�������
    //SS0 |= _0002_SAU_CH1_START_TRG_ON | _0001_SAU_CH0_START_TRG_ON;    /* enable UART0 receive and transmit */
    SS0 |= _0001_SAU_CH0_START_TRG_ON;    /* enable UART0 receive and transmit */   
}

/***********************************************************************************************************************
��������: ����0����ֹͣ����

����λ��:    ------------------------------δ��
***********************************************************************************************************************/
void USB_TX_UART0_Stop(void)
{
    ST0 |=  _0001_SAU_CH0_STOP_TRG_ON;    /* disable UART0 receive and transmit */
    //ST0 |= _0002_SAU_CH1_STOP_TRG_ON | _0001_SAU_CH0_STOP_TRG_ON;    /* disable UART0 receive and transmit */
    SOE0 &= ~_0001_SAU_CH0_OUTPUT_ENABLE;    /* disable UART0 output */ //ֹͣ���
    STMK0 = 1U;    /* disable INTST0 interrupt */     //ʧ�ܷ����ж�
    STIF0 = 0U;    /* clear INTST0 interrupt flag */  //�巢���жϱ�־
    //SRMK0 = 1U;    /* disable INTSR0 interrupt */     //ʧ�ܽ����ж�
    //SRIF0 = 0U;    /* clear INTSR0 interrupt flag */  //������жϱ�־
    //SREMK0 = 1U;   /* disable INTSRE0 interrupt */    //ʧ�ܴ����ж�
    //SREIF0 = 0U;   /* clear INTSRE0 interrupt flag */ //������жϱ�־
}



//------------------------------------------------------------------------------
//�������

/*******************************************************************************
��������:�����Ӳ����Ϊԭ��ĺ���,��Ҫ���ڷ�������ʱʹ��

����λ��:  --------------------------------δ��
*******************************************************************************/



/*******************************************************************************
��������:��С��ģʽ�л�

����λ��:  --------------------------------δ��
*******************************************************************************/




/*******************************************************************************
��������:����RTC�ж�ȡ������ʱ��������ֲ������

����λ��:  --------------------------------δ��
*******************************************************************************/
uint8_t func_timer_4302_deal(uint8_t luc_timer_tmp) //ʱ��ת�����ݴ�����
{                                                             //peak ��Ҫд���ʱ��ת��Ϊ����
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
uint8_t func_4302_timer_deal(uint8_t luc_4302_tmp)  //����ת��ʱ�䴦����
{                                                             //peak����ȡ������ת��Ϊʱ��_BCD��
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
