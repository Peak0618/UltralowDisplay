#ifndef _M_CARD_H_
#define _M_CARD_H_

//外部变量
extern uint8_t const  guc_card_tx_buffer[];          //发送的命令
extern uint8_t        guc_card_rx_buffer[];          //接收的数据缓存

extern uint8_t const  guc_card_20_order;                    //自动侦测是串口通讯的指令
extern uint8_t        guc_IC_access_state;                  //开锁状态                  
extern int8_t         gss_card_tx_order_interval_delaytime; //重新发送IC激活计时
extern uint8_t        guc_erasure_IC_all_data_order;        //接收主板发送的数据，根据主板指令是否要注销IC卡；
extern uint8_t        guc_card_tx_cnt;                      //发送那一套指令的计数


//外部宏定义
#define   REPEAT_TX_TIME               6                 //重新发送IC激活指令延时时间



//外部宏定义
extern flag_type card_com;
        #define     bflg_rx_valid_level             card_com.bits.bit0   // 有效电平
        #define     bflg_card_rx_startbit           card_com.bits.bit1   // 串口的起始位
        #define     bflg_card_rx_stopbit            card_com.bits.bit2   // 字节的结束位  
        #define     bflg_card_rx_ok                 card_com.bits.bit3   // 数据帧接收完
        #define     bflg_card_rx_busy               card_com.bits.bit4   // 接收忙标志
        #define     bflg_card_rx_end                card_com.bits.bit5   // 因超时结束标志

        #define     bflg_card_tx_startbit           card_com.bits.bit6   // 串口的起始位
        #define     bflg_card_tx_stoptbit           card_com.bits.bit7   // 串口的结束位
        #define     bflg_card_tx_ok                 card_com.bits.bit8   // 发送完成
        #define     bflg_card_tx_busy               card_com.bits.bit9   // 发送忙


extern flag_type card_com_2;
        #define     bflg_tx_20_delaytime               card_com_2.bits.bit0  // 发送两个 0x20之间的间隔
        #define     bflg_tx_auto_delaytime             card_com_2.bits.bit1  // 延时时间发送自动命令
        #define     bflg_card_receive_allow            card_com_2.bits.bit2  // 允许开始接收

        #define     bflg_rx_data_deal_ok               card_com_2.bits.bit3  // 处理完接收的数据了
        #define     bflg_buzz_one_tick                 card_com_2.bits.bit4  // 蜂鸣器响一声
        #define     bflg_card_buzz_interval            card_com_2.bits.bit5  // 蜂鸣器响的时间间距
        #define     bflg_card_send_allow               card_com_2.bits.bit6  // 允许发送
        #define     bflg_card_reset_repeat_tx_order    card_com_2.bits.bit7  // 在IC卡激活指令重发期间有数据进行接收，则停止重发操作，重新计时
        #define     bflg_card_bus_moitor               card_com_2.bits.bit8  // 总线空闲是否达到要求的10ms
        #define     bflg_card_startbit_error           card_com_2.bits.bit9  // 起始位错误
        #define     bflg_card_stopbit_error            card_com_2.bits.bit10 // 停止位错误


//-------------------------------------------------------------------------------------------------------------
//外部函数
extern void card_interrupt_check(void);
extern void card_receive_data(uint8_t length);
extern void card_flash_init(void);
extern void card_erase_all_data(void);
extern void card_send_data(uint8_t *txbuff,uint8_t length);
extern void card_tx_20_delaytime(void);
extern void card_tx_auto_delaytime(void);
extern void card_tx_order_interval(void);
extern void card_reset_repeat_tx_order(void);

extern void card_rx_end_delaytime(void);  
extern void card_set_start_stop(void);  
extern void card_buzz_interval_delaytimer(void);
extern void card_bus_monitor(void);




#endif
/*******************************************END OF THE FILE****************************************************/

