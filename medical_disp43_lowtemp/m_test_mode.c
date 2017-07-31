//---------------------------------------------------------
//包含的头文件
#include "r_cg_macrodriver.h"
#include "r_cg_intc.h"
#include "r_cg_serial.h"
#include "r_cg_timer.h"
#include "m_test_mode.h"
#include "h_com.h"
#include "h_main.h"
#include "m_card.h"

//------------------------------------------------------------------------------
//led显示编码宏定义
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
#define  data_E_dp       0XF9    // 显示 E 和 点；
#define  data_line       0X40    // 只显示 g 代表的那条横线 

//函数声明
void test_mode_com_rx_int(uint8_t rx_data);  // 测试模式485口的接收
void test_mode_com_tx_int(void);             // 测试模式485口的发送
void test_mode_com_rx_data_deal(void);
void test_mode_deal(void);                   //测试模式下的通讯处理
void test_mode_error_code_deal(void);
void test_mode_error_code_set(void);
void test_mode_error_code_view(void);


//定义的变量
flag_type flag_test_mode;

uint8_t   guc_test_mode_error_code;    //不同的错误代码数值
 
uint8_t   guc_test_com_buf;      //             串口1      发  A1  收 1A
uint8_t   guc_test_usb_buf;      //             串口0          A2     2A
uint8_t   guc_test_wifi_buf;     //             串口2          A3     3A
uint8_t   guc_test_card_buf;     //             模拟           A4     4A

uint8_t   guc_test_com_error_sum;      //连续接收错误数据的次数，达到10则为通讯错误
uint8_t   guc_test_usb_error_sum;
uint8_t   guc_test_wifi_error_sum;
uint8_t   guc_test_card_error_sum;

uint8_t const card_data = 0xA4;




//具体函数
//--------------------------------------------------------------------------
/***************************************************************************
函数功能: 测试模式485口的接收, 总的通讯测试节奏按此口的来；

函数位置: 系统串口1接收中断-----ok
***************************************************************************/
void test_mode_com_rx_int(uint8_t rx_data)  
{
    guc_test_com_buf = rx_data;
    
    bflg_com_rx_ok = 1;                       //置接收成功标志
    
    //bflg_com_rx_delaytime = 1;              //置接收延时标志
    //test_mode_guc_com_rx_delaytimer = 50;   //接收延时计时器赋值   
}


/***************************************************************************
函数功能:  测试模式485口的发送，发送中断中

函数位置: 系统串口1发送中断中-----ok
***************************************************************************/
void test_mode_com_tx_int(void)  
{
    if (bflg_com_tx_busy == 1)
    {         
        //TXD1 = 0xA1;            //发送数据   直接在 test_mode_com_rx_data_deal()中处理了

        bflg_com_tx_busy = 0;     //清发送忙标志
        bflg_com_tx_ok = 1;

        bflg_com_rx_delaytime = 1;             //置com通讯接收延时标志
        gss_com_rx_delaytimer = 5;             //com通讯接收延时计时器赋值

        bflg_com_tx_delaytime = 1;             
        gss_com_tx_delaytimer = 100;         
    }
}

/****************************************************************************
函数功能: 485接收数据后的处理

函数位置: test_mode_deal   --------------ok
*****************************************************************************/
void test_mode_com_rx_data_deal(void)    
{
    if (guc_test_com_buf == 0x1A)  
    {
        bflg_com_tx_delaytime = 1;                 //置发送延时标志
        gss_com_tx_delaytimer = 10;                //发送延时计时器赋值   
    }
}

/****************************************************************************
函数功能:  总的通讯的处理

函数位置: 主循环中   --------------ok
*****************************************************************************/
void test_mode_deal(void)         //测试模式下的通讯处理
{
    //-----------------------------------------------------------------------
    if (bflg_com_allow_rx == 1)          //如果允许接收
    {
        bflg_com_allow_rx = 0;
        //com_rx_init();                 //通讯接收初始化程序
        COM_RX_MODE;
        R_UART1_Start();                 // 485
        
        R_UART0_Start();                 // usb
        
        //R_UART2_Start();               // wifi

        //card模拟串口 
         R_INTC5_Start();               //中断5开等待接收数据  
         //card_rx_init();              //去掉，否则会因为初始化0数码管跳动
         bflg_card_receive_allow = 1;   //模拟串口要独立出来
         bflg_test_mode_tx_allow = 0;
    }

    //-----------------------------------------------------------------------
    if (bflg_com_rx_ok == 1)            //如果接收成功
    {
        bflg_com_rx_ok = 0;
        R_UART1_Stop();                 //485
        
        R_UART0_Stop();                 //usb
            
        //R_UART2_Stop();               //wifi
 
        //card模拟         
        //R_TAU0_Channel2_Stop();
        //bflg_test_mode_rx_allow = 0;
        //bflg_test_mode_tx_allow = 0;   
        
        test_mode_com_rx_data_deal();  //通讯接收数据处理程序
        test_mode_error_code_set();
    }

    //-----------------------------------------------------------------------
    if (bflg_com_allow_tx == 1)       //如果允许发送
    {
        bflg_com_allow_tx = 0;
        R_UART1_Start();              //485
        COM_TX_MODE;
        bflg_com_tx_busy = 1;
        //com_tx_init();              //通讯发送初始化程序
        TXD1 = 0xA1;

        R_UART0_Start();              //usb串口0
        TXD0 = 0xA2;
        
        //R_UART2_Start();            //wifi串口2
        //TXD2 = 0xA3;
        
        //--card模拟 
        R_TAU0_Channel2_Start();
        //card_tx_init();             //初始化   
        bflg_test_mode_tx_allow = 1;
        bflg_card_receive_allow = 0;        
    }

    //-----------------------------------------------------------------------
    if (bflg_com_tx_ok == 1)         //如果发送成功
    {
        bflg_com_tx_ok = 0;
    
        R_UART1_Stop();             //485
        
        R_UART0_Stop();             //usb
        
        //R_UART2_Stop();           //wifi

        //模拟    
        //bflg_test_mode_tx_allow = 0;  //因为节拍不是不是自己的，所以存在数据会发送几个错误的     
        //bflg_test_mode_rx_allow = 0;
        //R_TAU0_Channel2_Stop();
    }
}

/****************************************************************************
函数功能: 1、 防止错误数据接收太多溢出，
          2、 接收正确则将接收的错误计数清0

函数位置: 主循环中-----------------ok
*****************************************************************************/
void test_mode_error_code_set(void)  
{
                                                 //     收到错误数据大于20次
    if (guc_test_com_buf != 0x1A)                //E01   和被测板485通讯故障
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
    if(guc_card_rx_buffer[0] != 0x4A )           //E02   与IC卡模拟串口通讯故障
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
    
   if (guc_test_usb_buf != 0x2A)                //E03   与USB板通讯故障
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
    /*if (guc_test_wifi_buf != 0x3A)             //E04  与wifi通讯故障
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
函数功能:  如果接收的数据达到设定的错误次数，则认为错误

函数位置:  主循环中---------------ok
*****************************************************************************/
void test_mode_error_code_deal(void)  //     显示错误代码标号
{
    if (guc_test_com_error_sum >= 10)
    {
        guc_test_mode_error_code = 0x01;          //E01   和被测板485通讯故障
    }
    else if (guc_test_card_error_sum >= 20)
    {
        guc_test_mode_error_code = 0x02;          //E02   与IC卡模拟串口通讯故障
    }
    
    
    else if (guc_test_usb_error_sum >= 10)
    {
        guc_test_mode_error_code = 0x03;          //E03   与USB板通讯故障
    }
    
    /*else if (guc_test_wifi_error_sum >= 10)
    {
        guc_test_mode_error_code = 0x04;          //E04  与wifi通讯故障
    }*/
    else
    {
        guc_test_mode_error_code = 0;
    }
}



/****************************************************************************
函数功能:   LED1 中显示错误代码

            E01--和被测板485通讯故障
            E02--与card模拟串口通讯故障
            E03--与USB板通讯故障
            E04--与wifi通讯故障
 
函数位置:   主循环中--------------ok
*****************************************************************************/
void test_mode_error_code_view(void)
{
    if(guc_test_mode_error_code == 0)          //没有错误,显示pass
    {
        guc_led_buffer[1] = data_P;   
        guc_led_buffer[2] = data_A;         
        guc_led_buffer[3] = data_S;     
        guc_led_buffer[4] = data_S;         
    }  
    else                                        //显示错误代码
    { 
        guc_led_buffer[1] = data_all_off; 
        guc_led_buffer[2] = data_E;    
        guc_led_buffer[3] = data_0;  

        if(guc_test_mode_error_code == 1)
        {
            guc_led_buffer[4] = data_1;           //E01   和被测板485通讯故障
        }
        else if(guc_test_mode_error_code == 2)
        {
            guc_led_buffer[4] = data_2;           //E02   与IC卡模拟串口通讯故障
        }
        
        else if(guc_test_mode_error_code == 3)
        {
            guc_led_buffer[4] = data_3;           //E03  与USB板通讯故障
        }
        /*
        else if(guc_test_mode_error_code == 4)
        {
            guc_led_buffer[4] = data_4;           //E04   与wifi通讯故障
        } */    
    }    
}



/*******************************************END OF THE FILE****************************************************/

