#ifndef _M_USB_H_
#define _M_USB_H_

#include "r_cg_macrodriver.h" 

//------------------------------------------------------------------------------
//�ⲿ��������//
extern void com_init_usb(void);
extern void com_rx_init_usb(void);
extern void com_rx_data_deal_usb(void);
extern void com_tx_type_rank_usb(void)
;
extern void com_tx_init_usb(void);
extern void com_rx_delaytime_usb(void);
extern void com_tx_delaytime_usb(void);
extern void com_rx_end_delaytime_usb(void);
extern void com_fault_delaytime_usb(void);
extern void com_rx_int_usb(uint8_t rx_data) ;
extern void com_tx_int_usb(void);
extern uint8_t checksum (uint8_t *buffer, uint8_t size); 
extern void USB_RX_UART0_Start(void);
extern void USB_RX_UART0_Stop(void);
extern void USB_TX_UART0_Start(void);
extern void USB_TX_UART0_Stop(void);

//�ⲿ����
extern int16_t gss_com_tx_delaytimer_usb;     //ͨѶ������ʱ��ʱ��
extern int16_t gss_com_rx_delaytimer_usb;     //ͨѶ������ʱ��ʱ��

//------------------------------------------------------------------------------
//�ⲿ��������
extern flag_type flg_com_usb; 
          #define   bflg_com_tx_delaytime_usb    flg_com_usb.bits.bit0   //ͨѶ������ʱ��־
          #define   bflg_com_rx_delaytime_usb    flg_com_usb.bits.bit1   //ͨѶ������ʱ��־
          #define   bflg_com_allow_tx_usb        flg_com_usb.bits.bit2   //ͨѶ�����ͱ�־
          #define   bflg_com_allow_rx_usb        flg_com_usb.bits.bit3   //ͨѶ������ձ�־
          #define   bflg_com_tx_busy_usb         flg_com_usb.bits.bit4   //ͨѶ����æ��־
          #define   bflg_com_rx_busy_usb         flg_com_usb.bits.bit5   //ͨѶ����æ��־
          #define   bflg_com_tx_ok_usb           flg_com_usb.bits.bit6   //ͨѶ���ͳɹ���־
          #define   bflg_com_rx_ok_usb           flg_com_usb.bits.bit7   //ͨѶ���ճɹ���־
          #define   bflg_com_rx_end_usb          flg_com_usb.bits.bit8   //ͨѶ���ս�����־
          
          #define   bflg_com_fault_usb           flg_com_usb.bits.bit9   //ͨѶ���ϱ�־
          
          #define   bflg_read_write_data_usb     flg_com_usb.bits.bit10  //��д���ݱ�־
          
          #define   bflg_write_led_usb           flg_com_usb.bits.bit11  //д�������ʾ��־
          #define   bflg_read_key_usb            flg_com_usb.bits.bit12  //��������־
          #define   bflg_request_data_type       flg_com_usb.bits.bit13  //��������֡
          #define   bflg_request_time_type       flg_com_usb.bits.bit14  //ʱ������֡
          #define   bflg_request_machine_type    flg_com_usb.bits.bit15  //��������֡

extern flag_type flg_com_usb_1; 
          #define   bflg_init_data_type          flg_com_usb_1.bits.bit0    //���ͳ�ʼ������֡
          #define   bflg_init_time_type          flg_com_usb_1.bits.bit1    //���ͳ�ʼ��ʱ��֡
          #define   bflg_init_machine_type       flg_com_usb_1.bits.bit2    //���ͳ�ʼ������֡
          #define   bflg_init_event_type         flg_com_usb_1.bits.bit3    //���ͳ�ʼ���¼�֡
          #define   bflg_init_control_type       flg_com_usb_1.bits.bit4    //���ͳ�ʼ������֡

          #define   bflg_data_type_being_tx      flg_com_usb_1.bits.bit5    //����֡���ڷ���
          #define   bflg_time_type_being_tx      flg_com_usb_1.bits.bit6    //ʱ��֡���ڷ���
          #define   bflg_machine_type_being_tx   flg_com_usb_1.bits.bit7    //����֡���ڷ���
          #define   bflg_event_type_being_tx     flg_com_usb_1.bits.bit8    //�¼�֡���ڷ���
          #define   bflg_control_type_being_tx   flg_com_usb_1.bits.bit9    //����֡���ڷ���
          #define   bflg_rx_data_error_usb       flg_com_usb_1.bits.bit10   //usb���ڽ��յ���������ʱ�Ĵ���

          

extern uint8_t guc_Udisk_state;         //U�̵�״̬_ֻʹ�����2λ
extern uint8_t guc_usb_state;           //usb���ӵ�״̬�Ƿ��й���

extern uint8_t guc_usb_buffer[31];      //��ʾ��������л�ȡ�ķ��͸�usb�������


#endif
/***************************************END OF THE FILE************************************************/
