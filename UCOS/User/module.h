#ifndef __MODULE_H_
#define __MODULE_H_
#include "LPC177x_8x.h"
#include "lpc_types.h"
#include "system_LPC177x_8x.h"
#include <ucos_ii.h>
#include  <can.h>

typedef struct _MessageModule
{	
     uint8_t	 ModuleType;	
     uint8_t	 NOcount;
     uint8_t	 StartFlag;
     uint16_t  OutVoltage;                                         
     uint16_t  OutCurrent; 
     uint16_t  MAXVoltage;            // ��������ѹ
     uint16_t  MINVoltage;            // ��С�����ѹ	
	   uint16_t  MAXCurrent;	          // ����������
	   uint16_t  MINCurrent;	          // ��С�������
	   uint32_t  Cnt;	
		
}ModuMsg;


extern ModuMsg ModuleMsg;

typedef struct _MessageMonitor
{	

	   uint32_t  Cnt;	
		
}MotorMsg;
extern MotorMsg MonitorMsg;

static void	Module_RECData_Pro(void);  
static void ModuleSet(uint8_t CMD, uint16_t V, uint16_t I,uint8_t ModuleType);
static void ModuleGet(uint8_t CMD,uint32_t Voltage,uint32_t Current,uint8_t err);
extern void ModuleMain (void);

#endif
