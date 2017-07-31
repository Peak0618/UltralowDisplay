#ifndef _M_USB_H_
#define _M_USB_H_

#include "r_cg_macrodriver.h" 

//------------------------------------------------------------------------------
//外部函数声明//
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

//外部变量
extern int16_t gss_com_tx_delaytimer_usb;     //通讯发送延时计时器
extern int16_t gss_com_rx_delaytimer_usb;     //通讯接收延时计时器

//------------------------------------------------------------------------------
//外部变量声明
extern flag_type flg_com_usb; 
          #define   bflg_com_tx_delaytime_usb    flg_com_usb.bits.bit0   //通讯发送延时标志
          #define   bflg_com_rx_delaytime_usb    flg_com_usb.bits.bit1   //通讯接收延时标志
          #define   bflg_com_allow_tx_usb        flg_com_usb.bits.bit2   //通讯允许发送标志
          #define   bflg_com_allow_rx_usb        flg_com_usb.bits.bit3   //通讯允许接收标志
          #define   bflg_com_tx_busy_usb         flg_com_usb.bits.bit4   //通讯发送忙标志
          #define   bflg_com_rx_busy_usb         flg_com_usb.bits.bit5   //通讯接收忙标志
          #define   bflg_com_tx_ok_usb           flg_com_usb.bits.bit6   //通讯发送成功标志
          #define   bflg_com_rx_ok_usb           flg_com_usb.bits.bit7   //通讯接收成功标志
          #define   bflg_com_rx_end_usb          flg_com_usb.bits.bit8   //通讯接收结束标志
          
          #define   bflg_com_fault_usb           flg_com_usb.bits.bit9   //通讯故障标志
          
          #define   bflg_read_write_data_usb     flg_com_usb.bits.bit10  //读写数据标志
          
          #define   bflg_write_led_usb           flg_com_usb.bits.bit11  //写数码管显示标志
          #define   bflg_read_key_usb            flg_com_usb.bits.bit12  //读按键标志
          #define   bflg_request_data_type       flg_com_usb.bits.bit13  //数据请求帧
          #define   bflg_request_time_type       flg_com_usb.bits.bit14  //时间请求帧
          #define   bflg_request_machine_type    flg_com_usb.bits.bit15  //机型请求帧

extern flag_type flg_com_usb_1; 
          #define   bflg_init_data_type          flg_com_usb_1.bits.bit0    //发送初始化数据帧
          #define   bflg_init_time_type          flg_com_usb_1.bits.bit1    //发送初始化时间帧
          #define   bflg_init_machine_type       flg_com_usb_1.bits.bit2    //发送初始化机型帧
          #define   bflg_init_event_type         flg_com_usb_1.bits.bit3    //发送初始化事件帧
          #define   bflg_init_control_type       flg_com_usb_1.bits.bit4    //发送初始化控制帧

          #define   bflg_data_type_being_tx      flg_com_usb_1.bits.bit5    //数据帧正在发送
          #define   bflg_time_type_being_tx      flg_com_usb_1.bits.bit6    //时间帧正在发送
          #define   bflg_machine_type_being_tx   flg_com_usb_1.bits.bit7    //机型帧正在发送
          #define   bflg_event_type_being_tx     flg_com_usb_1.bits.bit8    //事件帧正在发送
          #define   bflg_control_type_being_tx   flg_com_usb_1.bits.bit9    //控制帧正在发送
          #define   bflg_rx_data_error_usb       flg_com_usb_1.bits.bit10   //usb串口接收到错误数据时的处理

          

extern uint8_t guc_Udisk_state;         //U盘的状态_只使用最低2位
extern uint8_t guc_usb_state;           //usb板子的状态是否有故障

extern uint8_t guc_usb_buffer[31];      //显示板从主板中获取的发送给usb板的数据


#endif
/***************************************END OF THE FILE************************************************/
