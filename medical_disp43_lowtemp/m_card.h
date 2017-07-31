#ifndef _M_CARD_H_
#define _M_CARD_H_

//�ⲿ����
extern uint8_t const  guc_card_tx_buffer[];          //���͵�����
extern uint8_t        guc_card_rx_buffer[];          //���յ����ݻ���

extern uint8_t const  guc_card_20_order;                    //�Զ�����Ǵ���ͨѶ��ָ��
extern uint8_t        guc_IC_access_state;                  //����״̬                  
extern int8_t         gss_card_tx_order_interval_delaytime; //���·���IC�����ʱ
extern uint8_t        guc_erasure_IC_all_data_order;        //�������巢�͵����ݣ���������ָ���Ƿ�Ҫע��IC����
extern uint8_t        guc_card_tx_cnt;                      //������һ��ָ��ļ���


//�ⲿ�궨��
#define   REPEAT_TX_TIME               6                 //���·���IC����ָ����ʱʱ��



//�ⲿ�궨��
extern flag_type card_com;
        #define     bflg_rx_valid_level             card_com.bits.bit0   // ��Ч��ƽ
        #define     bflg_card_rx_startbit           card_com.bits.bit1   // ���ڵ���ʼλ
        #define     bflg_card_rx_stopbit            card_com.bits.bit2   // �ֽڵĽ���λ  
        #define     bflg_card_rx_ok                 card_com.bits.bit3   // ����֡������
        #define     bflg_card_rx_busy               card_com.bits.bit4   // ����æ��־
        #define     bflg_card_rx_end                card_com.bits.bit5   // ��ʱ������־

        #define     bflg_card_tx_startbit           card_com.bits.bit6   // ���ڵ���ʼλ
        #define     bflg_card_tx_stoptbit           card_com.bits.bit7   // ���ڵĽ���λ
        #define     bflg_card_tx_ok                 card_com.bits.bit8   // �������
        #define     bflg_card_tx_busy               card_com.bits.bit9   // ����æ


extern flag_type card_com_2;
        #define     bflg_tx_20_delaytime               card_com_2.bits.bit0  // �������� 0x20֮��ļ��
        #define     bflg_tx_auto_delaytime             card_com_2.bits.bit1  // ��ʱʱ�䷢���Զ�����
        #define     bflg_card_receive_allow            card_com_2.bits.bit2  // ����ʼ����

        #define     bflg_rx_data_deal_ok               card_com_2.bits.bit3  // ��������յ�������
        #define     bflg_buzz_one_tick                 card_com_2.bits.bit4  // ��������һ��
        #define     bflg_card_buzz_interval            card_com_2.bits.bit5  // ���������ʱ����
        #define     bflg_card_send_allow               card_com_2.bits.bit6  // ������
        #define     bflg_card_reset_repeat_tx_order    card_com_2.bits.bit7  // ��IC������ָ���ط��ڼ������ݽ��н��գ���ֹͣ�ط����������¼�ʱ
        #define     bflg_card_bus_moitor               card_com_2.bits.bit8  // ���߿����Ƿ�ﵽҪ���10ms
        #define     bflg_card_startbit_error           card_com_2.bits.bit9  // ��ʼλ����
        #define     bflg_card_stopbit_error            card_com_2.bits.bit10 // ֹͣλ����


//-------------------------------------------------------------------------------------------------------------
//�ⲿ����
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

