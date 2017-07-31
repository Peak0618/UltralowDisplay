#ifndef _M_TEST_MODE_H_
#define _M_TEST_MODE_H_

#include "h_type_define.h"

//�ⲿ����
extern flag_type flag_test_mode;

        #define bflg_test_mode            flag_test_mode.bits.bit0
        #define bflg_test_mode_tx_allow   flag_test_mode.bits.bit1  //bflg_card_receive_allow
        #define bflg_test_mode_rx_allow   flag_test_mode.bits.bit2



//----------------------------------------------------------------------
extern uint8_t   guc_test_mode_error_code;    //��ͬ�Ĵ������

extern uint8_t   guc_test_com_buf;      //���յ�����  ����1  485
extern uint8_t   guc_test_usb_buf;      //            ����0
extern uint8_t   guc_test_card_buf;     //            ģ��
extern uint8_t   guc_test_wifi_buf;     //            ����2

extern uint8_t const card_data ;



//�ⲿ����

extern void test_mode_com_rx_int(uint8_t rx_data);  // ����ģʽ485�ڵĽ���
extern void test_mode_com_tx_int(void);             // ����ģʽ485�ڵķ���
extern void test_mode_com_rx_data_deal(void);
extern void test_mode_deal(void);                      //����ģʽ�µ�ͨѶ����
extern void test_mode_error_code_deal(void);
extern void test_mode_error_code_set(void);
extern void test_mode_error_code_view(void);



#endif
