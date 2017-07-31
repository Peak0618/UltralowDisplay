#ifndef _MAIN_H_
#define _MAIN_H_
//------------------------------------------------------------------------------
//�ⲿ��������//
extern void system_init(void);     //ϵͳ��ʼ������

extern void main_loop(void);       //��ѭ������

extern void timer_int(void);       //1ms��ʱ�жϳ���

extern void iic_int(void);         //iicģ�ⶨʱ�ж˳���

//------------------------------------------------------------------------------
//�ⲿ��������//
extern flag_type flg_time;
          
          #define   bflg_1ms_reach      flg_time.bits.bit0  //1ms��ʱ����־
          #define   bflg_5ms_reach      flg_time.bits.bit1  //5ms��ʱ����׼
          #define   bflg_10ms_reach     flg_time.bits.bit2  //10ms��ʱ����׼
          
          #define   bflg_led_key        flg_time.bits.bit2
          #define   bflg_iic_wait       flg_time.bits.bit3
          
          #define   bflg_key_buzz_delaytime       flg_time.bits.bit4
          
          #define   bflg_led_allon_delaytime      flg_time.bits.bit5

//------------------------------------------------------------------------------
extern int16_t gss_led_allon_delaytimer;
extern uint8_t guc_lamp_val;  //������ֵ

//peak 
extern int16_t   gss_key_buzz_delaytimer;


//�ⲿ�궨��
#define   COM_RX_MODE    (P12.0 = 0)    //����ģʽ
#define   COM_TX_MODE    (P12.0 = 1)    //����ģʽ


//------------------------------------------------------------------------------
#endif
