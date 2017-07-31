/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized. This 
* software is owned by Renesas Electronics Corporation and is protected under 
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING 
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT 
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
* AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR 
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE 
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software 
* and to discontinue the availability of this software.  By using this software, 
* you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2010, 2011 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_pfdl.c
* Version      : CodeGenerator for RL78/G13 V1.03.01 [11 Oct 2011]
* Device(s)    : R5F100LE
* Tool-Chain   : CA78K0R
* Description  : This file implements main function.
* Creation Date: 2012/07/11
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
/* Start user code for include. Do not edit comment generated here */

#include "r_cg_macrodriver.h"
#include "pfdl.h"
#include "pfdl_types.h"
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"


/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
/*peak 注释掉原来的
extern uint8_t  g_write_value;
extern uint16_t g_write_address;
extern uint8_t  g_read_value;
*/
//peak 从原来的改的
 uint8_t  g_write_value = 0x77;
 uint16_t g_write_address = 0x00 ;
 uint8_t  g_read_value;

/***********************************************************************************************************************
* Function Name: R_FDL_Init
* Description  : This function initializes RAM used by flash data library.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_FDL_Init(void)
{
    pfdl_descriptor_t descriptor;
    
    /* ---- Setting for PFDL_Open ---- */
    descriptor.fx_MHz_u08            = 8;//FDL_FRQ;
    descriptor.wide_voltage_mode_u08 = FDL_VOL;
    PFDL_Open(&descriptor);                                                            /* Initialize RAM */
}

/***********************************************************************************************************************
* Function Name: R_FDL_BlankCheck
* Description  : This function initializes RAM used by flash data library.
* Arguments    : None
* Return Value : PFDL_OK - 
                     Success initialize RAM
                 PFDL_IDLE -
				     Idling statement
			     PFDL_ERR_MARGIN - 
				     Blank check error
***********************************************************************************************************************/
uint8_t R_FDL_BlankCheck(void)
{
    pfdl_request_t requester;
    pfdl_status_t  ret;
    
    /* ---- Setting for blank check ---- */
    requester.command_enu   = PFDL_CMD_BLANKCHECK_BYTES;
    requester.index_u16     = g_write_address;
    requester.bytecount_u16 = WRITE_SIZE;
    ret                     = PFDL_Execute(&requester);                                /* Blank check */
    
    /* ---- Waiting for command finish ---- */
    while(ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();                                                          /* Status check process */
    }
    
    return ret;
}

/***********************************************************************************************************************
* Function Name: R_FDL_Erase
* Description  : This function erases block data.
* Arguments    : None
* Return Value : PFDL_OK - 
                     Success erases data
                 PFDL_NG -
                     Failure erases data
***********************************************************************************************************************/
uint8_t R_FDL_Erase(void)
{
    pfdl_request_t requester;
    pfdl_status_t  ret;
    
    /* ---- Setting for erase ---- */
    requester.command_enu = PFDL_CMD_ERASE_BLOCK;
    requester.index_u16   = TARGET_BLOCK;    
    ret                   = PFDL_Execute(&requester);                                  /* Erase block data */
    
    /* ---- Waiting for command finish ---- */
    while(ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();                                                          /* Status check process */
    }
    
    return ret;
}

/***********************************************************************************************************************
* Function Name: R_FDL_Verify
* Description  : This function does verify.
* Arguments    : None
* Return Value : PFDL_OK - 
                     Success verify
                 PFDL_NG -
                     Failure verify
***********************************************************************************************************************/
uint8_t R_FDL_Verify(void)
{
    pfdl_request_t requester;
    pfdl_status_t  ret;
    
	/* ---- Setting for verify ---- */
    requester.command_enu   = PFDL_CMD_IVERIFY_BYTES;
	requester.index_u16     = g_write_address;
	requester.bytecount_u16 = WRITE_SIZE;    
    ret                     = PFDL_Execute(&requester);                                /* Execute internal verify */
    
    /* ---- Waiting for command finish ---- */
    while(ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();                                                          /* Status check process */
    }
    
    return ret;
}

/***********************************************************************************************************************
* Function Name: R_FDL_Read
* Description  : This function read from flash data.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_FDL_Read(void)
{
    pfdl_request_t requester;
        
    /* ---- Setting for read ---- */
    requester.command_enu   = PFDL_CMD_READ_BYTES;
    requester.index_u16     = g_write_address;
    requester.bytecount_u16 = WRITE_SIZE;
    requester.data_pu08     = &g_read_value;    
    PFDL_Execute(&requester);                                                          /* Execute read data */    
}

/***********************************************************************************************************************
* Function Name: R_FDL_Write
* Description  : This function write to flash data.
* Arguments    : None
* Return Value : PFDL_OK - 
                     Success write data
                 PFDL_NG -
                     Failure write data
***********************************************************************************************************************/
uint8_t R_FDL_Write(void)
{
    pfdl_request_t requester;
    pfdl_status_t  ret;
    
    /* ---- Setting for write ---- */
    requester.command_enu   = PFDL_CMD_WRITE_BYTES;
    requester.index_u16     = g_write_address;
    requester.bytecount_u16 = WRITE_SIZE;
    requester.data_pu08     = &g_write_value;    
    ret                     = PFDL_Execute(&requester);                                /* Execute write */
    
    /* ---- Waiting for command finish ---- */
    while(ret == PFDL_BUSY)
    {
        ret = PFDL_Handler();                                                          /* Status check process */
	}

    return ret;
}

/***********************************************************************************************************************
* Function Name: R_FDL_ChangeAddress
* Description  : This function changes Address for writing to the data flash.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_FDL_ChangeAddress(void)
{
    R_FDL_Init();                                                                      /* FDL initialization */

    /* ---- When g_write_address is over MAX_ADDRESS ---- */
    if(g_write_address >= MAX_ADDRESS)
    {
         g_write_address = TARGET_BLOCK * BLOCK_SIZE;                                  /* Return first write address */
    }
    /* ---- When g_write_address is less MAX_ADDRESS ---- */
    else
    {
        g_write_address += WRITE_SIZE;                                                 /* Increment write address */
    }

    R_FDL_Read();                                                                      /* Read data process */
    PFDL_Close();                                                                      /* Close FDL */
}

/***********************************************************************************************************************
* Function Name: R_FDL_ExecuteWrite
* Description  : This function execute series of write to flash data.
* Arguments    : None
* Return Value : PFDL_OK - 
                     Success write data
                 PFDL_NG -
                     Failure write data
***********************************************************************************************************************/
uint8_t R_FDL_ExecuteWrite(void)
{
    uint8_t ret;
    
    //P5.2 = 0;       //peak                                                           /* LED0 on */
    R_FDL_Init();                                                                      /* FDL initialization */
    ret  = R_FDL_BlankCheck();                                                         /* Blank check process */

    /* **** When blank check error **** */
    if(ret == PFDL_ERR_MARGIN)
    {
        ret = R_FDL_Erase();                                                           /* Erase data of target block */
    }
    /* **** When other than blank check error **** */
    else
    {
        /* Do nothing */
    }

    /* **** When blank check or success data erase is success **** */
    if(ret == PFDL_OK)
    {
        ret = R_FDL_Write();                                                           /* Write data process */
        /* ==== When write data is success ==== */
        if(ret == PFDL_OK)
        {
            ret = R_FDL_Verify();                                                      /* Internal verify process */
            /* ---- When internal verify process is success ---- */
            if(ret == PFDL_OK)
            {
                R_FDL_Read();                                                          /* Read data process */
				/* ---- When equal g_write_value and g_read_value ---- */
                if(g_write_value == g_read_value)
                {
                    /* Do nothing */
                }
                /* ---- When different g_write_value and g_read_value ---- */
                else
                {
                    ret = PFDL_NG;
                }
            }
            /* ---- When internal verify process is failure---- */
            else
            {
                ret = PFDL_NG;
            }
        }
        /* ==== When writing data is failure ==== */
        else
        {
            ret = PFDL_NG;
        }
    }
    /* **** When blank check or data erase is failure **** */
    else
    {
        ret = PFDL_NG;
    }

    PFDL_Close();                                                                      /* Close FDL */
    //P5.2 = 1;                                                                        /* LED0 off */
    return ret;
}

/***********************************************************************************************************************
* Function Name: R_FDL_ClearDataFlash
* Description  : This function clear data flash.
* Arguments    : None
* Return Value : PFDL_OK - 
                     Success clear data flash
                 PFDL_NG -
                     Failure clear data flash
***********************************************************************************************************************/
uint8_t R_FDL_ClearDataFlash(void)
{
    uint8_t        ret;
    pfdl_request_t requester;
    
    //P5.2 = 0;                                                                        /* LED0 on */    
    R_FDL_Init();                                                                      /* FDL initialization */
    ret  = R_FDL_Erase();                                                              /* Erase data of target block */

    /* ---- When erase data is success ---- */
    if(ret == PFDL_OK)
    {
        R_FDL_Read();                                                                  /* Read data process */
    }
    /* ---- When erase data is failure ----*/
    else
    {
        ret = PFDL_NG;
    }

    PFDL_Close();                                                                      /* Close FDL */
    //P5.2 = 1;                                                                        /* LED0 off */  
    return ret;
}


    
/* End user code. Do not edit comment generated here */