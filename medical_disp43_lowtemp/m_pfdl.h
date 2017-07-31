#ifndef _M_PFDL_H_
#define _M_PFDL_H_

//外部函数
extern void ERROR_Handler(void);
extern void PFDL_Init(void);

void PFDL_Write(unsigned int Flash_index,unsigned int Data_Count,unsigned char* buffer);	//写函数

//extern void PFDL_Write(unsigned int Flash_index,unsigned int Data_Count);

extern unsigned char PFDL_BlankCheck(unsigned int Flash_index,unsigned int Length);   
extern void PFDL_Erase(unsigned int Flash_index);    
extern unsigned char PFDL_Iverify(unsigned int Flash_index,unsigned int Length);

extern void PFDL_Read(unsigned int Flash_index,unsigned int Data_Count,unsigned char *buffer);        //Read函数

//extern void PFDL_Read(unsigned int Flash_index,unsigned int Data_Count);




//外部变量
extern uint16_t flash_address;


extern uint8_t R_DataFla_buff[];


extern uint8_t W_DataFla_buff[];




#endif 
/**********************************END OF THE FILE**************************************************/
