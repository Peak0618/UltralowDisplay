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
* Copyright (C) 2011, 2013 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_userdefine.h
* Version      : CodeGenerator for RL78/G13 V2.00.00.07 [22 Feb 2013]
* Device(s)    : R5F100AD
* Tool-Chain   : CA78K0R
* Description  : This file includes user definition.
* Creation Date: 2017/5/11
***********************************************************************************************************************/

#ifndef _USER_DEF_H
#define _USER_DEF_H

/***********************************************************************************************************************
User definitions
***********************************************************************************************************************/

/* Start user code for function. Do not edit comment generated here */
#define SW_ON                  (1)                                     /* Detect switch push                         */
#define SW_OFF                 (0)                                     /* Undetect switch push                       */
#define ON_SW_1                (0x01)                                  /* SW1 status is on                           */
#define ON_SW_2                (0x02)                                  /* SW2 status is on                           */
#define ON_SW_3                (0x04)                                  /* SW3 status is on                           */
#define OFF_SW_ALL             (0x00)                                  /* Switch status is all off                   */
#define FLASH_START_ADDRESS    (0xF1000)                               /* Start address of data flash                */
#define TARGET_BLOCK           (0)                                     /* Block number of target(0 ~ 3)              */
#define BLOCK_SIZE             (0x400)                                 /* Block size                                 */
#define WRITE_SIZE             (1)                                     /* Size of data to be written at once         */
#define MAX_VALUE              (0xFF)                                  /* Maximum value of writing                   */
#define MAX_ADDRESS            ((TARGET_BLOCK + 1) * BLOCK_SIZE - 1)   /* Maximum address of writing                 */
#define PFDL_NG                (1)                                     /* Failure to Data Flash                      */
#define FDL_FRQ                (32)                                    /* Setting frequency (MHz)                    */
#define FDL_VOL                (0x00)                                  /* Voltage mode                               */
#define LCD_SIZE               (8)                                     /* Maximum number can be displayed on the LCD */

/***********************************************************************************************************************
Exported global functions (to be accessed by other files)
***********************************************************************************************************************/
void    R_MAIN_INTCStart(void);
void    R_MAIN_INTCStop(void);
void    R_MAIN_ClearSwitchFlag(void);
uint8_t R_MAIN_GetSwitchStatus(void);
void    R_MAIN_IncrementValue(void);
uint8_t R_MAIN_DetectLongPush(void);
uint8_t R_MAIN_SwitchProcess(uint8_t sw_status);
void    R_MAIN_UpdateStringUpper(int8_t upper_string[LCD_SIZE + 1]);
void    R_MAIN_UpdateStringDowner(int8_t downer_string[LCD_SIZE + 1]);
void    R_FDL_Init(void);
uint8_t R_FDL_BlankCheck(void);
uint8_t R_FDL_Erase(void);
uint8_t R_FDL_Verify(void);
void    R_FDL_Read(void);
uint8_t R_FDL_Write(void);
void    R_FDL_ChangeAddress(void);
uint8_t R_FDL_ExecuteWrite(void);
uint8_t R_FDL_ClearDataFlash(void);

#if    TARGET_BLOCK > 3
#error target_block error!!
#endif
/* End user code. Do not edit comment generated here */
#endif
