/********************************************************************************************************************************
overview:
           0)此IC卡为半双工 ；
           1)模拟串口收发
           2)IC注册检测   
           3)数据闪存     
           
定时器2:      1)用作接收脚RX;
           2)用作发送脚TX;
外部中断5：   用于检测串口的起始位    

通讯流程：
         1、显示板发送激活指令 ------打卡板发送回应一串数据--06 02 00 00 fb 03(只在第一次激活时回应，
                                     之后的防止打卡板掉线的重复发送激活指令打卡板不再回应 )
         2、当有IC卡靠近时            ------打卡板发送IC卡的序列号;
                                     一帧连续发送时这15个字节发送15.6ms 
                                     两个帧之间的间隔是66.8ms
串口格式：1位停止位、8位数据、1位停止位、无校验、9600                                     
*********************************************************************************************************************************/


//包含的头文件
//----------------------------------------------------------------
#include "r_cg_macrodriver.h"
#include "r_cg_port.h"
#include "r_cg_intc.h"
#include "r_cg_timer.h"


#include "h_type_define.h"
#include "m_card.h"
#include "m_pfdl.h"
#include "h_com.h"
#include "pfdl.h"

#include "r_cg_pclbuz.h"
#include "h_main.h"
#include "m_test_mode.h"
//--------------------------------------------------------------------
//函数声明
void card_interrupt_check(void);
void card_receive_data(uint8_t length);
void card_flash_init(void);
void card_erase_all_data(void);
uint8_t card_rx_data_deal(void);
void card_send_data(uint8_t *txbuff,uint8_t length);
void card_tx_20_delaytime(void);
void card_tx_auto_delaytime(void);
void card_tx_order_interval(void);
void card_reset_repeat_tx_order(void);
void card_rx_end_delaytime(void);  
void card_rx_init(void);
void card_tx_init(void);
void card_set_start_stop(void);  
void card_buzz_interval_delaytimer(void);
void card_bus_monitor(void);
uint8_t IC_BCC(uint8_t *SerBfr);


//-------------------------------------------------------------
//宏定义
#define   CARD_RX_PIN      P1.6    //模拟串口的输入引脚
#define   CARD_TX_PIN      P1.7    //模拟串口的输出引脚

#define   CARD_RXBUF_LEN   15
#define   CARD_TXBUF_LEN   10

//9600  -- 此IC卡最小可使用的波特率为9600
#define   BIT_RX_T        833    //实际计算 = 833.33.. =1/9600 us           9600波特率1bit的时长对应定时器的计数值   
#define   BIT_RX_T_1_2    416    //实际计算 = 416.66.. =(  1/9600)/2 us     9600波特率1/2bit的时长对应定时器的计数值




//按键值_新版 --但都未用
#define   KEY_NONE            0x00
#define   KEY_SET             0x20
#define   KEY_UP              0x08
#define   KEY_DOWN            0x10
#define   KEY_BUZ             0x40 
#define   KEY_SET_AND_UP      0x28 
#define   KEY_SET_AND_DOWN    0x30 
#define   KEY_SET_AND_BUZ     0x60 
#define   KEY_UP_AND_DOWN     0x18 
#define   KEY_UP_AND_BUZ      0x48 
#define   KEY_DOWN_AND_BUZ    0x50 


//---------------------------------------------------------------
//定义的变量 
flag_type card_com,card_com_2;


uint8_t   guc_card_rx_byte;                   //接收的单个字节
uint8_t   guc_card_rx_bit_index;              //字节的位长度接收索引
uint8_t   guc_card_rx_byte_index;             //数据帧字节长度接收索引
uint8_t   guc_card_rx_buffer[CARD_RXBUF_LEN]= {0}; //接收的数据缓存
uint8_t   gss_card_rx_end_delaytimer;         //设定的超时时间

uint8_t   guc_card_tx_byte;                    //发送的单个字节
uint8_t   guc_card_tx_bit_index;               //字节的位长度发送索引
uint8_t   guc_card_tx_byte_index;              //数据帧字节长度发送索引
uint8_t   guc_card_tx_temp;                    //发送临时变量

uint8_t   gss_card_tx_20_delaytimer;           //发送20的延时计数器
uint8_t   gss_card_tx_auto_delaytimer;         //发送完20后多长时间发送自动命令
int8_t    gss_card_tx_order_interval_delaytime;  //重新发送一遍激活指令的延时，防止中途断电

uint8_t   guc_IC_access_state;                 // 开锁状态                  

uint8_t   guc_card_buzz_interval_delaytime;
uint8_t   guc_erasure_IC_all_data_order;       //接收主板发送的数据，根据主板指令是否要注销IC卡；

uint8_t   guc_card_bus_monitor_time;           //检测总线空闲的时间

uint8_t   guc_card_tx_cnt;                     //发送那一套指令的计数

uint8_t const  guc_card_20_order = 0x20;       //连续发送两次 0x20，模块将确定通信波特率，并回应 0x06
                                               //若不先进行这一步操作，模块不响应任何主机发送的命令

uint8_t const  guc_card_tx_buffer[CARD_TXBUF_LEN] = {  0x0a,0x02,0x4e,0x04,0x05,
                                                       0x03,0x26,0x00,0x9d,0x03};   //发送自动检测的命令
                                                
                                                  
                                                  
                                               

//--------------------------------------------------------------------
//定义的数组
// 置1的位
const uint8_t SET_bit_table[8]=
{
    SET0,
    SET1,
    SET2,
    SET3,
    SET4,
    SET5,
    SET6,
    SET7
};
//--------------------------------------------------------------------
// 置0的位
const uint8_t CLR_bit_table[8]=
{
    CLR0,
    CLR1,
    CLR2,
    CLR3,
    CLR4,
    CLR5,
    CLR6,
    CLR7
};


/***********************************************************************************************************************************
功能描述: 检测到有下降沿中断时的处理,即可能是检测到串口的起始位


函数位置:中断5的系统处理函数中-------------------------------------ok
************************************************************************************************************************************/
void card_interrupt_check(void)
{                   
                              //只有起始位进外部中断才为正确
    TDR02 = BIT_RX_T_1_2;     //定时器中断周期为该波特率1/2位的时长
    R_TAU0_Channel2_Start();  //开定时器中断，并且TCR计数开始计时
    
    R_INTC5_Stop();           //关外部中断
    
    //card_rx_init();                //内部初始化内容太多， 添加会影响时序。。 
    bflg_card_receive_allow = 1;     //允许接收标志
}

/*********************************************************************************************************************************
功能描述: 对数据进行接收

实施措施：     检测到下降沿(起始位)进中断，然后关外部中断；以该波特率1/2位的时间进定时器中断，
          然后修改以该波特率的1位时间为定时器中断的周期；
            
函数位置: 系统定时器2中断-----------------------------------------------ok
**********************************************************************************************************************************/
void card_receive_data(uint8_t length)
{       
    bflg_rx_valid_level = CARD_RX_PIN;         //获取引脚电平
        
    //---------------------------------------------------------------------------------
    //单个字节的接收                                                       //执行顺序_2
    if (bflg_card_rx_startbit == 1)            // 开始接收一个字节的每一位     
    {   
        if (bflg_rx_valid_level == 0)          //bit=0
        {
            guc_card_rx_byte &= CLR_bit_table[guc_card_rx_bit_index++];
        }
        else                                   //bit=1
        {
            guc_card_rx_byte |= SET_bit_table[guc_card_rx_bit_index++];  
        }
        //------------------------------------------------
        if (guc_card_rx_bit_index >= 8)       // 第9个了
        {
            guc_card_rx_bit_index = 0;
            bflg_card_rx_startbit = 0;   
            bflg_card_rx_stopbit = 1;         //表示接收完一个字节
        }
    }   
    //---------------------------------------------------------------------------------
    //设定长度的数据帧接收                                                    //执行顺序_3
    else if (bflg_card_rx_stopbit == 1)      //接收帧数据                     
    {       
        if (bflg_rx_valid_level == 1)        //停止位正确
        {
            bflg_card_rx_stopbit = 0;
            bflg_card_rx_startbit = 0;
            guc_card_rx_buffer[guc_card_rx_byte_index++] = guc_card_rx_byte;
            
            if (guc_card_rx_byte_index >= length)   //15  
            {   
                bflg_card_rx_ok = 1;                //置通讯接收结束标志  
                bflg_card_receive_allow = 0;
                bflg_card_rx_busy = 0;              //清接收忙标志
            } 
            else
            {
                R_TAU0_Channel2_Stop();              //收完1byte，在此为了能快速响应
                R_INTC5_Start();  
            }
        }
        else                                        //停止位错误，不为高
        {
            bflg_card_stopbit_error = 1;
            bflg_card_receive_allow = 0;
            bflg_card_rx_busy = 0;  
        }
    }      
    //---------------------------------------------------------------------------------
    //起始位判断                                                         //执行顺序_1
    else                                                                     
    {
        if (bflg_rx_valid_level == 0)            //起始位正确
        {
            TDR02 = BIT_RX_T;                    //重新设定定时中断周期
            TS0 |= _0004_TAU_CH2_START_TRG_ON;   //重启定时器，可将TCR重新装载TDR的值并能重新计数
            
            bflg_card_rx_startbit = 1;           //起始位正确，开始接收
            bflg_card_rx_busy = 1;
        }
        else                                     //起始位错误
        {   
            bflg_card_startbit_error = 1;
            bflg_card_receive_allow = 0;
        }
    }
}

/*************************************************************************************************************************************************
功能描述:  瑞萨芯片内部数据闪存的刚开始使用时的设定

具体设计: 设定
          1) 根据最后一位 0xfff 中的值是否是0xad判断此flash是否初始化设置了
          2) 倒数第二位   0xffe 设定为保存当前已经到达的地址的低字节，未用是0
          3) 倒数第三位   0xffd 设定为保存当前已经到达的地址的高字节，未用是0
          地址范围: 0~0x0fff
          
注意:      1)地址是相对地址,范围是: 0x00 ~ 0x0fff = 4K
         2)块block的范围  :           0 ~ 3

块0: 存储IC序列号使用： 范围         0x000--0x3ff                               
块1: 存储IC序列号使用： 范围         0x400--0x7ff  因为一个IC卡序列号是4个字节，所以最大存储0x7ff/4 = 512张，现限制在99

块2: 存已存的字节数(IC卡序列号)：  范围              0x800--0xbff(因写入需先擦，但擦是按块的，所以用一个块存数量)  
                                 使用:用地址0x801存储已存字节数量的高字节；         
                                      用地址0x802存储已存字节数量的低字节(比如500用一个字节存储不下)
块3: 标记初始化：        范围      0xc00--0xfff  
                          仅使用:地址0xfff  --写固定标记值0xad       

函数位置:  初始化 system_init() 中-----------------------------------------ok
**************************************************************************************************************************************************/
void card_flash_init(void)
{   
    uint8_t read_data = 0,write_data = 0xad,address = 0;

    PFDL_Init(); 
    PFDL_Read(0xfff,1,&read_data);       //读取最后一位的内容

    if(read_data != 0xad)                //值不是标记已初始化，则初始化
    {
        PFDL_Erase(0x00);                //擦除所有的块
        PFDL_Erase(0x01);
        PFDL_Erase(0x02);
        PFDL_Erase(0x03);
       
        PFDL_Write(0xfff,1,&write_data);
        
        PFDL_Write(0x801,1,&address);    //当前地址使用地址为0，还未存数据
        
        PFDL_Write(0x802,1,&address);    
    }

    PFDL_Close(); 
}

/*********************************************************************************************************************************
功能描述:   根据主控板发送的指令来擦除

函数位置:   主函数-----------暂时位置---ok
**********************************************************************************************************************************/
void card_erase_all_data(void)
{
    uint8_t write_data = 0xad,address = 0;
    
    if(guc_erasure_IC_all_data_order == 1)
    {
        guc_erasure_IC_all_data_order = 0;
        
        PFDL_Init(); 
        
        PFDL_Erase(0x00);                 //擦除所有的块
        PFDL_Erase(0x01);
        PFDL_Erase(0x02);
        PFDL_Erase(0x03);
       
        PFDL_Write(0xfff,1,&write_data);       
        PFDL_Write(0x801,1,&address);    //当前地址使用地址为0，还未存数据        
        PFDL_Write(0x802,1,&address);
        
        PFDL_Close(); 
        
        bflg_buzz_one_tick = 1;
    }
}

/*********************************************************************************************************************************
功能描述:   处理接收到读卡器的数据

具体内容:  每次保存4个字节，即IC卡的ID号
           
函数位置:   card_set_start_stop() ---------------------------------ok
**********************************************************************************************************************************/
uint8_t card_rx_data_deal(void)
{
    uint8_t  msb_byte = 0, lsb_byte = 0, i = 0, ic_bcc = 0;
    uint8_t  temp_flash_buffer[4] = {0};
    uint16_t flash_address = 0, group = 0;

    ic_bcc = IC_BCC(guc_card_rx_buffer);     //校验
              
    if((guc_card_rx_buffer[0] == 0x0f) && (guc_card_rx_buffer[14] == 0x03) && (guc_card_rx_buffer[13] == ic_bcc) )
    {                                                                      
        if((guc_com_rx_buffer[7] == 0x40) && (guc_com_rx_buffer[8] == 0x40))   //进入注册状态,根据主板发送的led1、led2值
        {                                                                
            PFDL_Init();  
            
            PFDL_Read(0x801,1,&msb_byte);      //读达到的地址高字节
            PFDL_Read(0x802,1,&lsb_byte);      //读达到的地址低字节

            flash_address = ((flash_address | msb_byte)<<8) | lsb_byte; 
            
            if(flash_address > 0x18c)         //防止溢出，现产品要求最多99张 99*4=396=0x18c；
            {   
                PFDL_Close();
                bflg_rx_data_deal_ok = 1; 
                return 0;
            }          
            else
            {
                group = flash_address / 4;
                
                for(i=0;i<group;i++)
                {
                    PFDL_Read(i*4,4,temp_flash_buffer);  //循环读取已存的数据  
                    
                    if((guc_card_rx_buffer[9]  == temp_flash_buffer[0])&&    
                       (guc_card_rx_buffer[10] == temp_flash_buffer[1])&&
                       (guc_card_rx_buffer[11] == temp_flash_buffer[2])&&
                       (guc_card_rx_buffer[12] == temp_flash_buffer[3]))
                     {
                         PFDL_Close();                  
                         bflg_rx_data_deal_ok = 1; 
                         return 0;                       //若已存则退出
                     }
                }
                
                PFDL_Write(flash_address,4,&guc_card_rx_buffer[9]);  //未存过则保存数据到flash
                
                flash_address += 4;
                msb_byte = 0;
                lsb_byte = 0;
                msb_byte |= (uint8_t)(flash_address >> 8);
                lsb_byte |= (uint8_t)flash_address;
                
                PFDL_Erase(0x02);                    //写已用过的地址需先擦，擦除最小单位是块
                PFDL_Write(0x801,1,&msb_byte);       //存IC序列号数量到flash 高字节
                PFDL_Write(0x802,1,&lsb_byte);       //低字节
                PFDL_Close();
                
                bflg_rx_data_deal_ok = 1;           
                bflg_buzz_one_tick = 1;
            }

        }        
        else  // 检测是否注册过
        {
            PFDL_Init();      
            
            PFDL_Read(0x801,1,&msb_byte);      //读达到的地址高字节
            PFDL_Read(0x802,1,&lsb_byte);      //读达到的地址低字节 
            flash_address = 0;
            flash_address = ((flash_address | msb_byte)<<8) | lsb_byte;       

            group = flash_address / 4;         //多少组ID

            if(group == 0)                     //还没有存储过
            {            
                PFDL_Close(); 
                bflg_rx_data_deal_ok = 1;                
                return 0;
            }
            else
            {
                for(i=0;i<group;i++)
                {
                    PFDL_Read(i*4,4,temp_flash_buffer);   //读数据  
                    
                    if((guc_card_rx_buffer[9]  == temp_flash_buffer[0])&&
                       (guc_card_rx_buffer[10] == temp_flash_buffer[1])&&
                       (guc_card_rx_buffer[11] == temp_flash_buffer[2])&&
                       (guc_card_rx_buffer[12] == temp_flash_buffer[3]))
                     {
                
                        PFDL_Close();  
                         
                        guc_IC_access_state = 1;          //可开锁，在bflg_com_tx_ok中设置发送多次计数,防止丢包
                        if(bflg_card_buzz_interval == 0)  //每间隔 2s 响
                        {
                            bflg_card_buzz_interval = 1;
                            guc_card_buzz_interval_delaytime = 0;

                            bflg_buzz_one_tick = 1;       //调试无声时注释掉
                        }
                        
                        bflg_rx_data_deal_ok = 1;
                        return 0;
                     }
                }
                
                PFDL_Close();
                bflg_rx_data_deal_ok = 1;
            }
        }
   }
   else       //接收的数据错误
   {
        bflg_rx_data_deal_ok = 1;
   } 
   return 0;
}

/*********************************************************************************************************************************
功能描述: 模拟串口发送函数,每1/9600us进一次中断

新改的模式:   此IC卡为半双工   

发送时序:
        上电6s后/每间隔6s后                1ms后      100ms后
                           --->0x20------>0x20------->自动检测命令
                           
函数位置: 系统定时器2的中断函数中；--------ok
**********************************************************************************************************************************/
void card_send_data(uint8_t *txbuff,uint8_t length)
{  
    //---------------------------------------------------------------------------------------------
    //发送单个字节                                                                      执行顺序_2
    if(bflg_card_tx_startbit == 1)             
    {
        guc_card_tx_temp = (uint8_t)((*(txbuff + guc_card_tx_byte_index) >> guc_card_tx_bit_index) & 0x01);
        if(guc_card_tx_temp == 1)
        {
            CARD_TX_PIN = 1;         
        }
        else
        {
            CARD_TX_PIN = 0;
        }

        guc_card_tx_bit_index++;                //检测每个字节的8位是否发送完
        if (guc_card_tx_bit_index > 7)
        {
            guc_card_tx_bit_index = 0;
            bflg_card_tx_startbit = 0;
            
            bflg_card_tx_stoptbit = 1;   
        }
    }
    
    //---------------------------------------------------------------------------------------------
    //发送整帧数据                                                                      执行顺序_3
    else if(bflg_card_tx_stoptbit == 1)
    {
        CARD_TX_PIN = 1;                        //发送停止位高电平
        bflg_card_tx_stoptbit = 0;

        guc_card_tx_byte_index++;
        if(guc_card_tx_byte_index >= length)    //发送帧数据完成退出
        {
            bflg_card_tx_ok = 1;
            bflg_card_tx_busy = 0; 
            bflg_card_send_allow = 0;     
            guc_card_tx_byte_index = 0;  
            
            //--------------------------------------------------
            //设置那一套自动激活指令
            //发送时；1、开定时器             2、bflg_card_send_allow = 1;         
                        
            guc_card_tx_cnt++;
            if(guc_card_tx_cnt == 1)       //1_第一次发送完0x20
            {
                bflg_tx_20_delaytime = 1;  //延时后发送第二次0x20 
                gss_card_tx_20_delaytimer = 0;
            }
            else if(guc_card_tx_cnt == 2)  //2_第二次发送完0x20
            {
                bflg_tx_auto_delaytime = 1; //延时后发送自动检测命令
                gss_card_tx_auto_delaytimer = 0;
            }
            else if(guc_card_tx_cnt == 3)  //3_发送完自动检测命令
            {
                guc_card_tx_cnt = 0;       //结束此拨发送
            }
        }
    }
    
    //---------------------------------------------------------------------------------------------
    //发送起始位                                                                       执行顺序_1
    else
    {
        CARD_TX_PIN = 0;                        //发送起始位低电平
        bflg_card_tx_busy = 1; 
        bflg_card_tx_startbit = 1;               

        TDR02 = BIT_RX_T;                      //重新设定定时中断周期
        TS0 |= _0004_TAU_CH2_START_TRG_ON;     //重启定时器，可将TCR重新装载TDR的值并能重新计数
    }
}

/*********************************************************************************************************************************
功能描述: 发送完第一个0x20后，延时后第二次发送0x20

          
函数位置: 1ms定时中断 -----------------------ok
**********************************************************************************************************************************/
void card_tx_20_delaytime(void)
{
    if(bflg_tx_20_delaytime == 1)
    {
        gss_card_tx_20_delaytimer++;

        if(gss_card_tx_20_delaytimer >= 1)     //发送第一次0x20后，延时1ms再发送第二次20 
        {         
            gss_card_tx_20_delaytimer = 0;
            bflg_tx_20_delaytime = 0;
            
            bflg_card_send_allow = 1;
            card_tx_init();
            R_TAU0_Channel2_Start(); 
        }
    }
}

/*********************************************************************************************************************************
功能描述: 发送完两个0x20后，延时 100ms发送自动检测命令

函数位置: 1ms定时中断中 --------------------ok
**********************************************************************************************************************************/
void card_tx_auto_delaytime(void)
{
    if(bflg_tx_auto_delaytime == 1)
    {
        gss_card_tx_auto_delaytimer++;
        if(gss_card_tx_auto_delaytimer >= 100) //延时100ms再发送自动命令
        {
            gss_card_tx_auto_delaytimer = 0;
            bflg_tx_auto_delaytime = 0;

            bflg_card_send_allow = 1;
            card_tx_init();
            R_TAU0_Channel2_Start(); 
        }
    }
}

/**********************************************************************************************************************************************
功能描述: 间隔固定时间发送激活IC的指令
          一:无数据接收
          1、在没有数据接收的情况下，每间隔6s重新发送IC激活指令

          二:有数据接收:
          2、有数据接收时则清0所有的发送相关的内容，即重新计时重发
          3、在重发指令进行到一半时，若有数据接收，也清0所有，重新计时重发
          
函数位置: 1s定时中断中 ----------------------------------------ok
***********************************************************************************************************************************************/
void card_tx_order_interval(void) 
{
    gss_card_tx_order_interval_delaytime++;
    if(gss_card_tx_order_interval_delaytime >= REPEAT_TX_TIME)  // 6s
    {
        gss_card_tx_order_interval_delaytime = 0;
        
        bflg_card_send_allow = 1;
        card_tx_init();
        R_TAU0_Channel2_Start(); 
    }
}

/**********************************************************************************************************************************************
功能描述: 在有数据接收时，  清0所有关于重发"IC激活指令"的数据
          
函数位置:  主循环中------ok
***********************************************************************************************************************************************/
void card_reset_repeat_tx_order(void)
{
    if(bflg_card_reset_repeat_tx_order == 1)
    {
        bflg_card_reset_repeat_tx_order = 0;
        
        //重新计时
        gss_card_tx_order_interval_delaytime = 0;
        
        //总的发送
        bflg_tx_20_delaytime = 0;
        bflg_tx_auto_delaytime = 0;
        gss_card_tx_20_delaytimer = 0;
        gss_card_tx_auto_delaytimer = 0;
        
        //发送细节
        bflg_card_tx_busy = 0;
        bflg_card_tx_stoptbit = 0;
        bflg_card_tx_startbit = 0;
        guc_card_tx_byte_index = 0;
        guc_card_tx_bit_index = 0;
    }
}

/*********************************************************************************************************************************
功能描述: 接收超时标志

备注：    软件模拟没有超时，因为只要已触发起始位busy了，定时器会不停中断去接收一定数量的数据，
       除非是停止位错误停止接收；

函数位置: 1ms定时器中---------------ok
**********************************************************************************************************************************/
void card_rx_end_delaytime(void) //通讯接收完成延时程序，在1ms定时程序中调用
{
    if (bflg_card_rx_busy == 1)  //如果当前接收忙
    {
        gss_card_rx_end_delaytimer++;
        if (gss_card_rx_end_delaytimer >= 50)  //50ms
        {
            gss_card_rx_end_delaytimer = 0;
            
            bflg_card_rx_end = 1;
            bflg_card_rx_busy = 0;
            bflg_card_receive_allow = 0;
        }
    }
    else
    {        
        gss_card_rx_end_delaytimer = 0;
    }
}

/*********************************************************************************************************************************
功能描述: 接收初始化

函数位置:     card_rx_data_deal() ---ok 
**********************************************************************************************************************************/
void card_rx_init(void)
{
    uint8_t i;
    
    for (i = 0; i < CARD_RXBUF_LEN; i++)
    {
        guc_card_rx_buffer[i] = 0;       //清接收缓存器
    }

    bflg_card_rx_startbit = 0;
    bflg_card_rx_stopbit = 0;
    bflg_card_rx_busy = 0;
    
    guc_card_rx_bit_index = 0;
    guc_card_rx_byte_index = 0;
}

/*********************************************************************************************************************************
功能描述: 发送初始化

函数位置:     发送间隔到、发完第一个20、发完第二个20-------------ok
**********************************************************************************************************************************/
void card_tx_init(void)
{
    bflg_card_tx_startbit = 0;
    bflg_card_tx_stoptbit = 0;
    bflg_card_tx_busy = 0;
    
    guc_card_tx_bit_index = 0;
    guc_card_tx_byte_index = 0;
}

/*********************************************************************************************************************************
功能描述: 各个标志对应的开关中断

函数位置:   主程序循环   ---OK
**********************************************************************************************************************************/
void card_set_start_stop(void)
{
    if(bflg_test_mode == 0)
    {
        //--------------------------------------------------------------------
        if((bflg_card_startbit_error ==1)||(bflg_card_stopbit_error ==1) ||  //接收错误
            (bflg_card_rx_end == 1))
        {
            bflg_card_startbit_error = 0;
            bflg_card_stopbit_error = 0;
            bflg_card_rx_end = 0;
            
            bflg_card_bus_moitor = 0;  //开始总线监控

            R_TAU0_Channel2_Stop();    //关定时器
        }            
        //--------------------------------------------------------------------
        if(bflg_card_rx_ok == 1)       //接收成功
        {
            bflg_card_rx_ok = 0;
                        
            R_TAU0_Channel2_Stop();    //关定时器
            card_rx_data_deal();       //通讯接收数据处理程序
        }
        //--------------------------------------------------------------------
        if(bflg_rx_data_deal_ok == 1)  //处理完接收的数据    
        {
            bflg_rx_data_deal_ok = 0;

            bflg_card_bus_moitor = 0; //开始总线监控
        }
        //--------------------------------------------------------------------
        if(bflg_card_tx_ok == 1)      //发送完成
        {
            bflg_card_tx_ok = 0;
            R_TAU0_Channel2_Stop();
        }  

       //--------------------------------------------------------------------
       if(bflg_buzz_one_tick == 1)
       {
            bflg_buzz_one_tick = 0;
            
            bflg_key_buzz_delaytime = 1;  // 蜂鸣器响一声
            gss_key_buzz_delaytimer = 0;               
            R_PCLBUZ0_Start();
       }
    }
}

/**********************************************************************************************************************************
功能描述: 蜂鸣器每隔检测到正确信号,间隔多长时间响

函数位置:  1s -----------------------------------ok
***********************************************************************************************************************************/
void card_buzz_interval_delaytimer(void)
{   
    if(bflg_card_buzz_interval == 1)
    {
        guc_card_buzz_interval_delaytime++;
        if(guc_card_buzz_interval_delaytime >=2)   //卡长放在读卡器上，间隔2s响
        {                                          //若间隔1s响，首次放卡可能响2声，因那时正运行此函数
            guc_card_buzz_interval_delaytime = 0;
            bflg_card_buzz_interval = 0;
        }
    }
}

/**********************************************************************************************************************************
功能描述: 检测接收引脚总线的空闲时间，若大于10ms则开外部中断，等待数据下降沿

备注:       IC卡示波器测量发送一帧数据在16ms左右，间隔67ms左右再次发送

函数位置:  1ms ------------------------------ok
***********************************************************************************************************************************/
void card_bus_monitor(void)
{
    if(bflg_card_bus_moitor == 0)
    {
        if(CARD_RX_PIN == 1)                     //总线高电平空闲时
        {
            guc_card_bus_monitor_time++;
            if(guc_card_bus_monitor_time >= 10)   //10ms
            {
                guc_card_bus_monitor_time = 0;
                bflg_card_bus_moitor = 1;

                card_rx_init();           //初始化             
                R_INTC5_Start();          //开外部中断
            }
        }
        else
        {
            guc_card_bus_monitor_time = 0;           
        }
    }
}

/**********************************************************************************************************************************
功能描述: 对接收的IC数据进行校验--异或取反的方式，因为模块中说明
          IC卡发送的校验就是异或取反；


函数位置:  card_rx_data_deal()中 ------------------------------ok
***********************************************************************************************************************************/
uint8_t IC_BCC(uint8_t *SerBfr)
{
    uint8_t BCC = 0,i = 0;
    
    for (i = 0; i < (SerBfr[0] - 2); i++) 
    {
        BCC ^= SerBfr[i];
    }
    //SerBfr[SerBfr[0]-2] = ~BCC;    //即SerBfr[13]， SerBfr[0]为字节总长，包括自身；

    //BCC = SerBfr[SerBfr[0]-2];  

    BCC = (uint8_t)~BCC;
    return BCC;
}


/*******************************************END OF THE FILE****************************************************/
