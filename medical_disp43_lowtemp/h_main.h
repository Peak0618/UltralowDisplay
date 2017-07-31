#ifndef _MAIN_H_
#define _MAIN_H_
//------------------------------------------------------------------------------
//外部函数声明//
extern void system_init(void);     //系统初始化程序

extern void main_loop(void);       //主循环程序

extern void timer_int(void);       //1ms定时中断程序

extern void iic_int(void);         //iic模拟定时中端程序

//------------------------------------------------------------------------------
//外部变量声明//
extern flag_type flg_time;
          
          #define   bflg_1ms_reach      flg_time.bits.bit0  //1ms计时到标志
          #define   bflg_5ms_reach      flg_time.bits.bit1  //5ms计时到标准
          #define   bflg_10ms_reach     flg_time.bits.bit2  //10ms计时到标准
          
          #define   bflg_led_key        flg_time.bits.bit2
          #define   bflg_iic_wait       flg_time.bits.bit3
          
          #define   bflg_key_buzz_delaytime       flg_time.bits.bit4
          
          #define   bflg_led_allon_delaytime      flg_time.bits.bit5

//------------------------------------------------------------------------------
extern int16_t gss_led_allon_delaytimer;
extern uint8_t guc_lamp_val;  //报警灯值

//peak 
extern int16_t   gss_key_buzz_delaytimer;


//外部宏定义
#define   COM_RX_MODE    (P12.0 = 0)    //接收模式
#define   COM_TX_MODE    (P12.0 = 1)    //发送模式


//------------------------------------------------------------------------------
#endif
