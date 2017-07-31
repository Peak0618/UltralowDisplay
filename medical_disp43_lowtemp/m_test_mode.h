#ifndef _M_TEST_MODE_H_
#define _M_TEST_MODE_H_

#include "h_type_define.h"

//外部变量
extern flag_type flag_test_mode;

        #define bflg_test_mode            flag_test_mode.bits.bit0
        #define bflg_test_mode_tx_allow   flag_test_mode.bits.bit1  //bflg_card_receive_allow
        #define bflg_test_mode_rx_allow   flag_test_mode.bits.bit2



//----------------------------------------------------------------------
extern uint8_t   guc_test_mode_error_code;    //不同的错误代码

extern uint8_t   guc_test_com_buf;      //接收的数据  串口1  485
extern uint8_t   guc_test_usb_buf;      //            串口0
extern uint8_t   guc_test_card_buf;     //            模拟
extern uint8_t   guc_test_wifi_buf;     //            串口2

extern uint8_t const card_data ;



//外部函数

extern void test_mode_com_rx_int(uint8_t rx_data);  // 测试模式485口的接收
extern void test_mode_com_tx_int(void);             // 测试模式485口的发送
extern void test_mode_com_rx_data_deal(void);
extern void test_mode_deal(void);                      //测试模式下的通讯处理
extern void test_mode_error_code_deal(void);
extern void test_mode_error_code_set(void);
extern void test_mode_error_code_view(void);



#endif
