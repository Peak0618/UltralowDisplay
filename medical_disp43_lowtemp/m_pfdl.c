//------------------------------------------------------------------------------------
//包含的头文件
#include "r_cg_macrodriver.h"
#include "pfdl.h"
#include "pfdl_types.h"
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

#include "m_pfdl.h"

#include "h_type_define.h"
#include "m_card.h"


//------------------------------------------------------------------------------------
//定义变量
pfdl_descriptor_t my_pfdl_descriptor_t;         
pfdl_status_t     my_pfdl_status_t;           
pfdl_request_t    my_pfdl_request_t;          

//------------------------------------------------------------------------------------
//函数声明
void ERROR_Handler(void);
void PFDL_Init(void);

void PFDL_Write(unsigned int Flash_index,unsigned int Data_Count,unsigned char* buffer);	//写函数

//void PFDL_Write(unsigned int Flash_index,unsigned int Data_Count);

unsigned char PFDL_BlankCheck(unsigned int Flash_index,unsigned int Length);   
void PFDL_Erase(unsigned int Flash_index);    
unsigned char PFDL_Iverify(unsigned int Flash_index,unsigned int Length);

void PFDL_Read(unsigned int Flash_index,unsigned int Data_Count,unsigned char *buffer);        //Read函数

//void PFDL_Read(unsigned int Flash_index,unsigned int Data_Count);



//------------------------------------------------------------------------------------
//定义变量

unsigned char Execute_status; 
/*
uint8_t W_DataFla_buff[100] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                11,12,13,14,15,16,17,18,19,20,
                                21,22,23,24,25,26,27,28,29,30,
                                31,32,33,34,35,36,37,38,39,40,
                                41,42,43,44,45,46,47,48,49,50,
                                51,52,53,54,55,56,57,58,59,60,
                                61,62,63,64,65,66,67,68,69,70,
                                71,72,73,74,75,76,77,78,79,80,
                                81,82,83,84,85,86,87,88,89,90,
                                91,92,93,94,95,96,97,98,99,100
};
*/

uint8_t W_DataFla_buff[10] = {0xad,0x00,0x04,0x00,0x00,
                              0x00,0x00,0x00,0x00,0x00,};

uint8_t R_DataFla_buff[35];

unsigned int dat=0;

uint16_t flash_address;   // 4k:0-0xfff=4096 bits  =1024 byte  地址范围是0~1024


//------------------------------------------------------------------------------------
//具体函数

void ERROR_Handler(void)
{
	//WDTE = 0xAC;   
}	

/*
void PFDL_Init(void)
{
	my_pfdl_descriptor_t.fx_MHz_u08 = 7;   					//1MHz, must be integer     
	my_pfdl_descriptor_t.wide_voltage_mode_u08 = 0x01;  	//00: full-speed mode, 01 wide voltage mode   
	my_pfdl_status_t = PFDL_Open((__near pfdl_descriptor_t*) &my_pfdl_descriptor_t);  
	if (my_pfdl_status_t != PFDL_OK) ERROR_Handler();   
}
*/


///*
void PFDL_Init(void)
{
    pfdl_descriptor_t descriptor;
    
    descriptor.fx_MHz_u08            = 32;//FDL_FRQ;
    descriptor.wide_voltage_mode_u08 = 0x01;//FDL_VOL;
    PFDL_Open(&descriptor);                                                            /* Initialize RAM */
}

//*/



unsigned char PFDL_BlankCheck(unsigned int Flash_index,unsigned int Length)
{
	my_pfdl_request_t.index_u16 = Flash_index;        
	my_pfdl_request_t.bytecount_u16 = Length;   
	my_pfdl_request_t.command_enu = PFDL_CMD_BLANKCHECK_BYTES;  
	my_pfdl_status_t = PFDL_Execute(&my_pfdl_request_t);      	
	do 
	{
		my_pfdl_status_t = PFDL_Handler();
	}
	while(my_pfdl_status_t == PFDL_BUSY);     	
	return (my_pfdl_status_t);   
}

/*****************************************************************
功能描述: 擦除Flash数据 Flash_index-擦除的起始地址
                         
函数位置:       
******************************************************************/
void PFDL_Erase(unsigned int Flash_index)              //Erase函数
{
    my_pfdl_request_t.index_u16 = Flash_index;              //Erase地址      
    my_pfdl_request_t.command_enu = PFDL_CMD_ERASE_BLOCK;   //Erase命令
    my_pfdl_status_t = PFDL_Execute(&my_pfdl_request_t);    //Erase执行函数    	   	
    do 
    {
    	my_pfdl_status_t = PFDL_Handler();
    }
    while(my_pfdl_status_t == PFDL_BUSY);                   //等待	Erase完毕
    if (my_pfdl_status_t != PFDL_OK) 
    {
    	ERROR_Handler();      //如果Erase失败、调用错处理函数（复位）	
    }
	
}

/*****************************************************************
功能描述: 写入Flash数据 Flash_index-写入的起始地址
                        Data_Count - 写入的数据长度

函数位置:       
******************************************************************/
void PFDL_Write(unsigned int Flash_index,unsigned int Data_Count,unsigned char* buffer)	//写函数
{
	my_pfdl_request_t.index_u16 = Flash_index;						//Write地址
	my_pfdl_request_t.data_pu08 = buffer;     				        //Write缓存首地址
	my_pfdl_request_t.bytecount_u16 = Data_Count;   				//Write数据长度
	my_pfdl_request_t.command_enu = PFDL_CMD_WRITE_BYTES;  			//Write命令
	my_pfdl_status_t = PFDL_Execute(&my_pfdl_request_t);      		//Write执行函数 
	do 
	{
		my_pfdl_status_t = PFDL_Handler();
	}
	while(my_pfdl_status_t == PFDL_BUSY);     						//等待Write完毕
	if (my_pfdl_status_t != PFDL_OK)  ERROR_Handler();     			//如果Write失败、调用错处理函数（复位）
}



/*
void PFDL_Write(unsigned int Flash_index,unsigned int Data_Count)	//写函数
{
	my_pfdl_request_t.index_u16 = Flash_index;						//Write地址
	my_pfdl_request_t.data_pu08 = &guc_card_rx_buffer[9];     				//Write缓存首地址
	my_pfdl_request_t.bytecount_u16 = Data_Count;   				//Write数据长度
	my_pfdl_request_t.command_enu = PFDL_CMD_WRITE_BYTES;  			//Write命令
	my_pfdl_status_t = PFDL_Execute(&my_pfdl_request_t);      		//Write执行函数 
	do 
	{
		my_pfdl_status_t = PFDL_Handler();
	}
	while(my_pfdl_status_t == PFDL_BUSY);     						//等待Write完毕
	if (my_pfdl_status_t != PFDL_OK)  ERROR_Handler();     			//如果Write失败、调用错处理函数（复位）
}

*/


unsigned char PFDL_Iverify(unsigned int Flash_index,unsigned int Length)//Iverify函数
{
	my_pfdl_request_t.index_u16 = Flash_index;        					//Iverify地址
	my_pfdl_request_t.bytecount_u16 = Length; 							//Iverify数据长度
	my_pfdl_request_t.command_enu = PFDL_CMD_IVERIFY_BYTES;  			//Iverify命令
	my_pfdl_status_t = PFDL_Execute(&my_pfdl_request_t);      			 //Iverify执行函数 
	do 
	{
		my_pfdl_status_t = PFDL_Handler();
	}
	while(my_pfdl_status_t == PFDL_BUSY);     							//等待Iverify完毕
	return (my_pfdl_status_t);  
}   


/*****************************************************************
功能描述: 读取Flash数据 Flash_index-读取的起始地址
                        Data_Count - 读取的数据长度

函数位置:       
******************************************************************/
void PFDL_Read(unsigned int Flash_index,unsigned int Data_Count,unsigned char *buffer)        //Read函数
{
    my_pfdl_request_t.index_u16 = Flash_index;                          //Read地址
    
    my_pfdl_request_t.data_pu08 = buffer;                               //Read缓存首地址-------------------------
    
    my_pfdl_request_t.bytecount_u16 = Data_Count;                       //Read数据长度
    my_pfdl_request_t.command_enu = PFDL_CMD_READ_BYTES;                //Read命令
    my_pfdl_status_t = PFDL_Execute(&my_pfdl_request_t);                //Read执行函数
    do 
    {
        my_pfdl_status_t = PFDL_Handler();
    }
    while(my_pfdl_status_t == PFDL_BUSY);                               //等待Read完毕
    if (my_pfdl_status_t != PFDL_OK)  ERROR_Handler();                  //如果Read失败、调用错处理函数（复位）
 
}

/*
void PFDL_Read(unsigned int Flash_index,unsigned int Data_Count)		//Read函数
    {
        my_pfdl_request_t.index_u16 = Flash_index;							//Read地址

        my_pfdl_request_t.data_pu08 = R_DataFla_buff;     					//Read缓存首地址-------------------------

        my_pfdl_request_t.bytecount_u16 = Data_Count;   					//Read数据长度
        my_pfdl_request_t.command_enu = PFDL_CMD_READ_BYTES;  				//Read命令
        my_pfdl_status_t = PFDL_Execute(&my_pfdl_request_t);      			//Read执行函数
        do 
        {
            my_pfdl_status_t = PFDL_Handler();
        }
        while(my_pfdl_status_t == PFDL_BUSY);     							//等待Read完毕
        if (my_pfdl_status_t != PFDL_OK)  ERROR_Handler();     				//如果Read失败、调用错处理函数（复位）

    }
//*/


/*****************************END OF THE FILE*************************************/
