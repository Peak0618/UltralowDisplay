//---------------------------------------------------------
//������ͷ�ļ�
#include "r_cg_macrodriver.h"
#include "r_cg_intc.h"
#include "r_cg_serial.h"
#include "r_cg_timer.h"
#include "m_test_mode.h"
#include "h_com.h"
#include "h_main.h"
#include "m_card.h"

//------------------------------------------------------------------------------
//led��ʾ����궨��
//---------------------
//|        a          |
//|       ----        |
//|    f | g  | b     |
//|       ----        |
//|    e |    | c     |
//|       ---- .      |
//|        d    dp    |
//---------------------
//----------------------------------------------------------
//    dp   |  g   |  f   |  e   |  d   |  c   |  b   |  a
//   bit7  | bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0
//----------------------------------------------------------
#define   data_all_off   0x00
#define   data_dot       0x80
#define   data_hyphen    0x40

#define   data_0         0x3F
#define   data_1         0x06
#define   data_2         0x5B
#define   data_3         0x4F
#define   data_4         0x66
#define   data_5         0x6D
#define   data_6         0x7D
#define   data_7         0x07
#define   data_8         0x7F
#define   data_9         0x6F

#define   data_A         0x77
#define   data_b         0x7C
#define   data_C         0x39
#define   data_c         0x58
#define   data_d         0x5E
#define   data_E         0x79
#define   data_F         0x71
#define   data_G         0x3D
#define   data_H         0x76
#define   data_i         0x04
#define   data_J         0x0E
#define   data_L         0x38
#define   data_N         0x37
#define   data_n         0x54
#define   data_o         0x5C
#define   data_P         0x73
#define   data_q         0x67
#define   data_r         0x50
#define   data_S         0x6D
#define   data_T         0x31
#define   data_t         0x78
#define   data_U         0x3E
#define   data_u         0x1C
#define   data_y         0x6E

//peak add
#define  data_E_dp       0XF9    // ��ʾ E �� �㣻
#define  data_line       0X40    // ֻ��ʾ g ������������� 

//��������
void test_mode_com_rx_int(uint8_t rx_data);  // ����ģʽ485�ڵĽ���
void test_mode_com_tx_int(void);             // ����ģʽ485�ڵķ���
void test_mode_com_rx_data_deal(void);
void test_mode_deal(void);                   //����ģʽ�µ�ͨѶ����
void test_mode_error_code_deal(void);
void test_mode_error_code_set(void);
void test_mode_error_code_view(void);


//����ı���
flag_type flag_test_mode;

uint8_t   guc_test_mode_error_code;    //��ͬ�Ĵ��������ֵ
 
uint8_t   guc_test_com_buf;      //             ����1      ��  A1  �� 1A
uint8_t   guc_test_usb_buf;      //             ����0          A2     2A
uint8_t   guc_test_wifi_buf;     //             ����2          A3     3A
uint8_t   guc_test_card_buf;     //             ģ��           A4     4A

uint8_t   guc_test_com_error_sum;      //�������մ������ݵĴ������ﵽ10��ΪͨѶ����
uint8_t   guc_test_usb_error_sum;
uint8_t   guc_test_wifi_error_sum;
uint8_t   guc_test_card_error_sum;

uint8_t const card_data = 0xA4;




//���庯��
//--------------------------------------------------------------------------
/***************************************************************************
��������: ����ģʽ485�ڵĽ���, �ܵ�ͨѶ���Խ��ఴ�˿ڵ�����

����λ��: ϵͳ����1�����ж�-----ok
***************************************************************************/
void test_mode_com_rx_int(uint8_t rx_data)  
{
    guc_test_com_buf = rx_data;
    
    bflg_com_rx_ok = 1;                       //�ý��ճɹ���־
    
    //bflg_com_rx_delaytime = 1;              //�ý�����ʱ��־
    //test_mode_guc_com_rx_delaytimer = 50;   //������ʱ��ʱ����ֵ   
}


/***************************************************************************
��������:  ����ģʽ485�ڵķ��ͣ������ж���

����λ��: ϵͳ����1�����ж���-----ok
***************************************************************************/
void test_mode_com_tx_int(void)  
{
    if (bflg_com_tx_busy == 1)
    {         
        //TXD1 = 0xA1;            //��������   ֱ���� test_mode_com_rx_data_deal()�д�����

        bflg_com_tx_busy = 0;     //�巢��æ��־
        bflg_com_tx_ok = 1;

        bflg_com_rx_delaytime = 1;             //��comͨѶ������ʱ��־
        gss_com_rx_delaytimer = 5;             //comͨѶ������ʱ��ʱ����ֵ

        bflg_com_tx_delaytime = 1;             
        gss_com_tx_delaytimer = 100;         
    }
}

/****************************************************************************
��������: 485�������ݺ�Ĵ���

����λ��: test_mode_deal   --------------ok
*****************************************************************************/
void test_mode_com_rx_data_deal(void)    
{
    if (guc_test_com_buf == 0x1A)  
    {
        bflg_com_tx_delaytime = 1;                 //�÷�����ʱ��־
        gss_com_tx_delaytimer = 10;                //������ʱ��ʱ����ֵ   
    }
}

/****************************************************************************
��������:  �ܵ�ͨѶ�Ĵ���

����λ��: ��ѭ����   --------------ok
*****************************************************************************/
void test_mode_deal(void)         //����ģʽ�µ�ͨѶ����
{
    //-----------------------------------------------------------------------
    if (bflg_com_allow_rx == 1)          //����������
    {
        bflg_com_allow_rx = 0;
        //com_rx_init();                 //ͨѶ���ճ�ʼ������
        COM_RX_MODE;
        R_UART1_Start();                 // 485
        
        R_UART0_Start();                 // usb
        
        //R_UART2_Start();               // wifi

        //cardģ�⴮�� 
         R_INTC5_Start();               //�ж�5���ȴ���������  
         //card_rx_init();              //ȥ�����������Ϊ��ʼ��0���������
         bflg_card_receive_allow = 1;   //ģ�⴮��Ҫ��������
         bflg_test_mode_tx_allow = 0;
    }

    //-----------------------------------------------------------------------
    if (bflg_com_rx_ok == 1)            //������ճɹ�
    {
        bflg_com_rx_ok = 0;
        R_UART1_Stop();                 //485
        
        R_UART0_Stop();                 //usb
            
        //R_UART2_Stop();               //wifi
 
        //cardģ��         
        //R_TAU0_Channel2_Stop();
        //bflg_test_mode_rx_allow = 0;
        //bflg_test_mode_tx_allow = 0;   
        
        test_mode_com_rx_data_deal();  //ͨѶ�������ݴ������
        test_mode_error_code_set();
    }

    //-----------------------------------------------------------------------
    if (bflg_com_allow_tx == 1)       //���������
    {
        bflg_com_allow_tx = 0;
        R_UART1_Start();              //485
        COM_TX_MODE;
        bflg_com_tx_busy = 1;
        //com_tx_init();              //ͨѶ���ͳ�ʼ������
        TXD1 = 0xA1;

        R_UART0_Start();              //usb����0
        TXD0 = 0xA2;
        
        //R_UART2_Start();            //wifi����2
        //TXD2 = 0xA3;
        
        //--cardģ�� 
        R_TAU0_Channel2_Start();
        //card_tx_init();             //��ʼ��   
        bflg_test_mode_tx_allow = 1;
        bflg_card_receive_allow = 0;        
    }

    //-----------------------------------------------------------------------
    if (bflg_com_tx_ok == 1)         //������ͳɹ�
    {
        bflg_com_tx_ok = 0;
    
        R_UART1_Stop();             //485
        
        R_UART0_Stop();             //usb
        
        //R_UART2_Stop();           //wifi

        //ģ��    
        //bflg_test_mode_tx_allow = 0;  //��Ϊ���Ĳ��ǲ����Լ��ģ����Դ������ݻᷢ�ͼ��������     
        //bflg_test_mode_rx_allow = 0;
        //R_TAU0_Channel2_Stop();
    }
}

/****************************************************************************
��������: 1�� ��ֹ�������ݽ���̫�������
          2�� ������ȷ�򽫽��յĴ��������0

����λ��: ��ѭ����-----------------ok
*****************************************************************************/
void test_mode_error_code_set(void)  
{
                                                 //     �յ��������ݴ���20��
    if (guc_test_com_buf != 0x1A)                //E01   �ͱ����485ͨѶ����
    {
        guc_test_com_error_sum++;
        if (guc_test_com_error_sum >= 20)
        {
              guc_test_com_error_sum = 20;
        }
    }
    else
    {
        guc_test_com_error_sum = 0;
    }
    
    //----------------------------------------------------------------------
    if(guc_card_rx_buffer[0] != 0x4A )           //E02   ��IC��ģ�⴮��ͨѶ����
    {
        guc_test_card_error_sum++;
        if (guc_test_card_error_sum >= 20)
        {
              guc_test_card_error_sum = 20;
        }
    }
    else
    {
        guc_test_card_error_sum = 0;
    }
    
    //----------------------------------------------------------------------
    
   if (guc_test_usb_buf != 0x2A)                //E03   ��USB��ͨѶ����
    {
        guc_test_usb_error_sum++;
        if (guc_test_usb_error_sum >= 20)
        {
              guc_test_usb_error_sum = 20;
        }
    }
    else
    {
        guc_test_usb_error_sum = 0;
    }
   
    //----------------------------------------------------------------------
    /*if (guc_test_wifi_buf != 0x3A)             //E04  ��wifiͨѶ����
    {
        guc_test_wifi_error_sum++;
        if (guc_test_wifi_error_sum >= 20)
        {
              guc_test_wifi_error_sum = 20;
        }
    }
    else
    {
        guc_test_wifi_error_sum = 0;
    }
    */
}

/****************************************************************************
��������:  ������յ����ݴﵽ�趨�Ĵ������������Ϊ����

����λ��:  ��ѭ����---------------ok
*****************************************************************************/
void test_mode_error_code_deal(void)  //     ��ʾ���������
{
    if (guc_test_com_error_sum >= 10)
    {
        guc_test_mode_error_code = 0x01;          //E01   �ͱ����485ͨѶ����
    }
    else if (guc_test_card_error_sum >= 20)
    {
        guc_test_mode_error_code = 0x02;          //E02   ��IC��ģ�⴮��ͨѶ����
    }
    
    
    else if (guc_test_usb_error_sum >= 10)
    {
        guc_test_mode_error_code = 0x03;          //E03   ��USB��ͨѶ����
    }
    
    /*else if (guc_test_wifi_error_sum >= 10)
    {
        guc_test_mode_error_code = 0x04;          //E04  ��wifiͨѶ����
    }*/
    else
    {
        guc_test_mode_error_code = 0;
    }
}



/****************************************************************************
��������:   LED1 ����ʾ�������

            E01--�ͱ����485ͨѶ����
            E02--��cardģ�⴮��ͨѶ����
            E03--��USB��ͨѶ����
            E04--��wifiͨѶ����
 
����λ��:   ��ѭ����--------------ok
*****************************************************************************/
void test_mode_error_code_view(void)
{
    if(guc_test_mode_error_code == 0)          //û�д���,��ʾpass
    {
        guc_led_buffer[1] = data_P;   
        guc_led_buffer[2] = data_A;         
        guc_led_buffer[3] = data_S;     
        guc_led_buffer[4] = data_S;         
    }  
    else                                        //��ʾ�������
    { 
        guc_led_buffer[1] = data_all_off; 
        guc_led_buffer[2] = data_E;    
        guc_led_buffer[3] = data_0;  

        if(guc_test_mode_error_code == 1)
        {
            guc_led_buffer[4] = data_1;           //E01   �ͱ����485ͨѶ����
        }
        else if(guc_test_mode_error_code == 2)
        {
            guc_led_buffer[4] = data_2;           //E02   ��IC��ģ�⴮��ͨѶ����
        }
        
        else if(guc_test_mode_error_code == 3)
        {
            guc_led_buffer[4] = data_3;           //E03  ��USB��ͨѶ����
        }
        /*
        else if(guc_test_mode_error_code == 4)
        {
            guc_led_buffer[4] = data_4;           //E04   ��wifiͨѶ����
        } */    
    }    
}



/*******************************************END OF THE FILE****************************************************/

