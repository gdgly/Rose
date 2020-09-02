/****************************************Copyright (c)****************************************************
**                              ueee electronics Co.,LTD.
**                                     
**                                 http://www.ueee.com
**                                    wujikun
**--------------File Info---------------------------------------------------------------------------------
** File Name:               Main.c
** Last modified Date:      2017.05.18
** Last Version:            1.0
** Description:             The main function 
** 
**--------------------------------------------------------------------------------------------------------
** Created By:              wujikun 
** Created date:            2017.05.18
** Version:                 1.0
** Descriptions:            The original version ��ʼ�汾
**
*********************************************************************************************************/

#ifndef __MAIN_H 
#define __MAIN_H 

#include "lpc_types.h"


#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
  TASK PRIORITIES �������ȼ�
*********************************************************************************************************/		
#define  TASK_START_PRIO          0
#define  TASK_ADC_PRIO			   	  2
#define  TASK_BMS_PRIO			   	  5	
#define  TASK_Module_PRIO			   	8
#define  TASK_Insulation_PRIO			10	
	
/*********************************************************************************************************
  TASK STACK SIZES  �����ջ��С
*********************************************************************************************************/
#define  TASK_START_STK_SIZE      100
#define  TASK_ADC_STK_SIZE			  200
#define  TASK_BMS_STK_SIZE			  500
#define  TASK_Module_STK_SIZE			500
#define  TASK_Insulation_STK_SIZE			300

typedef struct
{	
	   uint8_t	 ChargerNO;             //ǹ�Ķ˿ں�
	   uint8_t	 PowerDivideModule;	    //���ʷ���ģʽ
     uint8_t	 ModuleNOcount;         //ģ������
     uint16_t  MAXVoltage;            // ��������ѹ
     uint16_t  MINVoltage;            // ��С�����ѹ	
	   uint16_t  MAXCurrent;	          // ����������
	   uint16_t  MINCurrent;	          // ��С�������
	   uint8_t   CS;
	
}SYS_PARA;


extern  SYS_PARA   Sys_PARA;

#ifdef __cplusplus
    }
#endif


#endif
/*********************************************************************************************************
  END FILE 
*********************************************************************************************************/


