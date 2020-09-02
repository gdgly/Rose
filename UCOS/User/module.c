/****************************************Copyright (c)****************************************************
**                                     Tomorn  Co.,LTD.
**                                     
**                                   http://www.tomorn.cn
**                                       wujikun
**--------------File Info---------------------------------------------------------------------------------
** File Name:               charger.c
** Last modified Date:      2019.09.18
** Last Version:            1.0
** Description:             
*********************************************************************************************************/
#include <module.h>
#include <insulation.h>
#include <adc.h>

static void	Module_RECData_Pro(void);  
static void ModuleSet(uint8_t CMD, uint16_t V, uint16_t I,uint8_t ModuleType);
static void ModuleGet(uint8_t CMD,uint32_t Voltage,uint32_t Current,uint8_t err);

static void AnalyseMT30(void);
static void AnalyseMT31(void);
static void AnalyseMT32(void);
static void AnalyseMT33(void);
static void AnalyseMT34(void);
static void AnalyseMT35(void);
static void AnalyseMT46(void);

ModuMsg ModuleMsg;
MotorMsg MonitorMsg;

uint32_t ModuleVoltage;
uint32_t ModuleCurrent[8];                                                       //����ģ�����

void ModuleMain (void)
{   
	ModuleMsg.Cnt = 0;
	MonitorMsg.Cnt = 0;
	ModuleMsg.StartFlag = 0;                                                       // ������ʶ
	ChargerMsg.PreCharge= 0;
	ModuleMsg.NOcount = 1;
  while (1) 
 {	 
	  OSTimeDlyHMSM(0,0,0,10);
	  ModuleMsg.Cnt++;
	  MonitorMsg.Cnt++;
		Module_RECData_Pro();

/***************************************************************************************************
	                              ���ģ��˵����ݴ���
**************************************************************************************************/	
//        ����ģ����ͳһ�ĳ���	
//	if ((ModuleMsg.Cnt%200)==0)	                                                   //����
//	{	 	 
//		ModuleSet(4,0,0,0);	 
//	}
		 
	if ((ModuleMsg.Cnt%25)==0)	 
	{
		   if(ChargerMsg.ChargeStage==2)                                               // ��Ե���״̬
		   {		
				   if(BMSMessage.MAXVoltage<=ChargerMsg.MAXVoltage)                        //�綨��Ե����ѹ
							 {
									ChargerMsg.InsuVoltage=BMSMessage.MAXVoltage;
									if(ChargerMsg.InsuVoltage<=2000)
										ChargerMsg.InsuVoltage=2000;						
							 }
							 else
							 {
									ChargerMsg.InsuVoltage=ChargerMsg.MAXVoltage;			 
							 }
				       ChargerMsg.InsuCurrent=ModuleMsg.NOcount*200;                        //�綨��Ե������
				 
							if(	ModuleMsg.StartFlag == 0)
							 {
  								ModuleSet(2,ChargerMsg.InsuVoltage,ModuleMsg.NOcount*200,ModuleMsg.ModuleType);	     // ������Ե���
								 	OSTimeDlyHMSM(0,0,0,20);
								 	ModuleMsg.StartFlag=1;                                               // ������ʶ
							 } 
             else{
                  ModuleSet(1,ChargerMsg.InsuVoltage,ModuleMsg.NOcount*200,ModuleMsg.ModuleType);	
						      }							 
							 if((ChargerMsg.InsuVoltage-ModuleMsg.OutVoltage)<=100)
							 {
								   if(InsulationFlag==0)
										 {
											 DC_SWITCH_ON();						
							         ChargerMsg.DCSwitchFlag=1;
											 InsulationFlag=1;
											}  
										 
							 }
                if(InsulationFlag==2)
										 {
											  ModuleSet(3, 0, 0,0);
											  OSTimeDlyHMSM(0,0,1,0);
											 	DC_SWITCH_OFF();						
							          ChargerMsg.DCSwitchFlag=0;
											  InsulationFlag=3;
										 }							 
		   }
		
		 if(ChargerMsg.ChargeStage==7)                                                              // ���״̬
		  {
				if((BMSMessage.ChargeSuspendTime<10*60)&&(BMSMessage.BatteryChgAlow	=0x10))             //�����ͣ10����
				{
						if(ChargerMsg.PreCharge==0)             //Ԥ������                                        
						{
								 if(BMSMessage.RequestVoltage<=BMSMessage.BatteryVoltage)                       //�綨�����ѹ
								 {
										 BMSMessage.RequestVoltage=BMSMessage.BatteryVoltage-50;
								 }
								 if(	ModuleMsg.StartFlag == 0)
								 {
										ModuleSet(2,BMSMessage.BatteryVoltage-50,ModuleMsg.NOcount*200,ModuleMsg.ModuleType);	// ����Ԥ��
										OSTimeDlyHMSM(0,0,0,20); 
										ModuleMsg.StartFlag=1;                                            // ������ʶ
										AnalyseMT31();                                                    // �������	
								 }	            				 
								 if((BMSMessage.BatteryVoltage-ModuleMsg.OutVoltage)<=100)
								 {

											DC_SWITCH_ON();                                              //����ֱ���Ӵ���
											OSTimeDlyHMSM(0,0,0,20);								 
											ChargerMsg.DCSwitchFlag=1;
											ChargerMsg.PreCharge=1;	                                     // Ԥ���ʶ										 
								 }
						}
					 else{                                    //ģ���·�ָ��Զ������л�����
						 
								ModuleSet(1, BMSMessage.RequestVoltage, BMSMessage.RequestCurrent*10/ModuleMsg.NOcount,ModuleMsg.ModuleType);         //�������
						    //�����л��������  todu
						}
				}					 
				else if(BMSMessage.ChargeSuspendTime>10*60)                                            //�����ͣ����10����
				{
					ChargerMsg.ChargeStage=8;
				}
			 	
	    }
		
			if((ChargerMsg.ChargeStage!=0)&&(ChargerMsg.ChargeStage!=2)&&(ChargerMsg.ChargeStage!=7))
		  {
				 if(ModuleMsg.StartFlag==1)
				 { 
					 ModuleSet(3, 0, 0,0);		
         }					 
					if(ChargerMsg.DCSwitchFlag==0)
					{
            ModuleMsg.StartFlag=0;						
					}					
		  } 
		}	
 		
/***************************************************************************************************
	                             ��ض˵����ݴ���
**************************************************************************************************/
	
	 if(ChargerMsg.StartCompleteflag==1)
		 {
		   AnalyseMT33();
			 ChargerMsg.StartCompleteflag=0;
		 }
		
 }

} 


static void	Module_RECData_Pro(void)  
{	
/***************************************************************************************************************************************
*	����ѭ���Ĵ����ٶȸ������жϵĴ����ٶȣ������һ�������������ж�д�����ݣ�����ѭ���������ݣ�ÿ������һ֡���ݣ���Ѹ������ݶ�����
* �������ݻ����������ݳ��ȴ�С���ޣ����жϵ�дָ��ԶԶ�������Ķ�ָ���ٶȣ����ǻ���ֶ�������
***************************************************************************************************************************************/
  	if(prMsgCAN1==&MsgCAN1BUF[CAN1_MSG_NUMBER-1])    /* Ԥ��һ���洢��Ԫ���洢����ֹдָ��ָ�����һ���洢��Ԫʱ����ָ�����ʱ���    */
	  {
			MessageCAN1 = *prMsgCAN1;
		  memset(prMsgCAN1,0,sizeof(*prMsgCAN1));        /*������һ֡���ݣ��Ͷ����ð�����                                                 */			
			prMsgCAN1=MsgCAN1BUF;			                     /* ����ָ��ָ�� ����β��ָ��ʼ                                                 */
	  }
	  else                                             
		{
	     MessageCAN1 = *prMsgCAN1;
		   memset(prMsgCAN1,0,sizeof(*prMsgCAN1));       /*������һ֡���ݣ��Ͷ����ð�����                                                 */
		   prMsgCAN1 ++;                                 /*ָ��������������󣬷�ָֹ�����������ظ���if����                           */
	  }
			
    switch(MessageCAN1.CANID)
	 {
/***************************************************************************************************
	                              ���ģ��˵����ݴ���
**************************************************************************************************/	
						 case 0x02c8f000:               
										ModuleMsg.OutVoltage=(MessageCAN1.DATAA&0xffffff)/100;
										ModuleCurrent[0]=(MessageCAN1.DATAB&0xffff)/10;
								 break ;

						 case 0x02c8f001:               
										ModuleMsg.OutVoltage=(MessageCAN1.DATAA&0xffffff)/100;
										ModuleCurrent[1]=(MessageCAN1.DATAB&0xffff)/10;
								 break ;

						 case 0x02c8f002:               
										ModuleMsg.OutVoltage=(MessageCAN1.DATAA&0xffffff)/100;
										ModuleCurrent[2]=(MessageCAN1.DATAB&0xffff)/10;
								 break ;

						 case 0x02c8f003:               
										ModuleMsg.OutVoltage=(MessageCAN1.DATAA&0xffffff)/100;
										ModuleCurrent[3]=(MessageCAN1.DATAB&0xffff)/10;
								 break ;


//        ����ģ����ͳһ����������		 
//				if(ModuleMsg.ModuleType==0)	
//				{	
//						 case 0x1820a160 :               
//										ModuleMsg.OutVoltage=MessageCAN1.DATAA>>16;
//										ModuleCurrent[0]=MessageCAN1.DATAB&0xffff;
//								 break ;
//						 
//						 case 0x1820a161 :               
//										ModuleCurrent[1]=MessageCAN1.DATAB&0xffff;
//								 break ;
//						 
//						 case 0x1820a162 :               
//										ModuleCurrent[2]=MessageCAN1.DATAB&0xffff;
//								 break ;
//						 
//						 case 0x1820a163 :               
//										ModuleCurrent[3]=MessageCAN1.DATAB&0xffff;
//								 break ;
//						 
//						 case 0x1820a164 :               
//										ModuleCurrent[4]=MessageCAN1.DATAB&0xffff;
//								 break ;
//						 
//						 case 0x1820a165 :               
//										ModuleCurrent[5]=MessageCAN1.DATAB&0xffff;
//								 break ;

//						 case 0x1820a166 :               
//										ModuleCurrent[6]=MessageCAN1.DATAB&0xffff;
//								 break ;
//						 
//						 case 0x1820a167 :               
//										ModuleCurrent[7]=MessageCAN1.DATAB&0xffff;
//								 break ;
//					 }
										 
/***************************************************************************************************
	                             ��ض˵����ݴ���
**************************************************************************************************/		
//Aǹ������ݴ���
		 case 0x184600a0:   
			 if(Sys_PARA.ChargerNO==0){
        if((MessageCAN1.DATAA&0xff)==0x05)
		    {			
         AnalyseMT34();           
			   AnalyseMT35();		
		     }
			 }
		     break ;
		 
		 case 0x103000a0:                  //�������֡
			 if(Sys_PARA.ChargerNO==0){			 
			     AnalyseMT30();
			   }
		     break ;		
				 

		 case 0x103100a0:                  //�������֡
		     break ;		 
		 
		 case 0x103200a0:                  //ֹͣ���֡     
			 if(Sys_PARA.ChargerNO==0){	
			   AnalyseMT32();
			 }		 
		     break ;	

		 case 0x103300a0:                  //�������֡
			 if(Sys_PARA.ChargerNO==0){	
			   AnalyseMT33();
		     break ;		
					 }
			 
					 
//Bǹ������ݴ���					 
		 case 0x184601a0: 			  
			 if(Sys_PARA.ChargerNO==1){		 
        if((MessageCAN1.DATAA&0xff)==0x05)
		    {			
         AnalyseMT34();           
			   AnalyseMT35();		
		     }
			 }
		     break ;
		 
		 case 0x103001a0:                  //�������֡
			 	if(Sys_PARA.ChargerNO==1){
			   AnalyseMT30();
				}
		     break ;		 

		 case 0x103101a0:                  //�������֡
		     break ;		 
		 
		 case 0x103201a0:                  //ֹͣ���֡     
			   if(Sys_PARA.ChargerNO==1){
			   AnalyseMT32();
				 }
		     break ;	

		 case 0x103301a0:                  //�������֡
			 	 if(Sys_PARA.ChargerNO==1){
			   AnalyseMT33();
				 }
		     break ;	


	 	 default:
			   break ;		 				 
	 }
	 	 
	 memset(&MessageCAN1,0,sizeof(MessageCAN1));//���CAN�����ݣ� ��ֹ�ظ�����

}


/**************************************************************************************************************
   ģ��ĺ�������
**************************************************************************************************************/

void ModuleSet(uint8_t CMD, uint16_t V, uint16_t I,uint8_t ModuleType)
{

     uint8_t i;
	
	for(i=0;i<ModuleMsg.NOcount;i++)
	{
				switch(CMD)
				 {	
					 case 1:
						 CANID=0x02DB00F0|(i<<8);                     //���õ�ѹ����
							 
						 CAN_Data[0]=0x00;
						 CAN_Data[1]=(V*100>>16)&0xff;
						 CAN_Data[2]=(V*100>>8)&0xff;
						 CAN_Data[3]=(V*100)&0xff;
						 CAN_Data[4]=0x00;
						 CAN_Data[5]=(V*100>>16)&0xff;; 			
						 CAN_Data[6]=(V*100>>8)&0xff;
						 CAN_Data[7]=(V*100)&0xff; 							 					 
						 WriteCAN1(8,1, CANID,CAN_Data);	
							break;
						 
					 case 2:	 
						 CANID=0x02DA00F0|(i<<8);                      //���ٿ���
					 
						 CAN_Data[0]=0x00;
						 CAN_Data[1]=0x00;
						 CAN_Data[2]=0x00;
						 CAN_Data[3]=0x00;
						 CAN_Data[4]=0x00;
						 CAN_Data[5]=0x00;
						 CAN_Data[6]=0x00;
						 CAN_Data[7]=0x00;		 
						 WriteCAN1(8,1, CANID,CAN_Data);	
							break;
					 case 3:	 
						 CANID=0x02DA00F0|(i<<8);                     //�ػ�
						 CAN_Data[0]=0x01;
						 CAN_Data[1]=0x00;
						 CAN_Data[2]=0x00;
						 CAN_Data[3]=0x00;
						 CAN_Data[4]=0x00;
						 CAN_Data[5]=0x00;
						 CAN_Data[6]=0x00;
						 CAN_Data[7]=0x00;					 
						 WriteCAN1(8,1, CANID,CAN_Data);	

					 default:
							break;			 		 
       }
				 OSTimeDlyHMSM(0,0,0,5);
    }
	 	
}

void ModuleGet(uint8_t CMD,uint32_t Voltage,uint32_t Current,uint8_t err)
{
	  uint8_t i;
	  uint32_t Isum;
	for(i=0;i<ModuleMsg.NOcount;i++)
	{	
			switch (CMD)
			{
				case 0:                                                //�������ѹ�͵���
            CANID=0x02C800F0|(i<<8);
				
						 CAN_Data[0]=0x00;
						 CAN_Data[1]=0x00;
						 CAN_Data[2]=0x00;
						 CAN_Data[3]=0x00;
						 CAN_Data[4]=0x00;
						 CAN_Data[5]=0x00;
						 CAN_Data[6]=0x00; 
						 CAN_Data[7]=0x00;				
						 WriteCAN1(8,1, CANID,CAN_Data);			
						break;
						
				case 1:
            CANID=0x02C600F0|(i<<8);                           //�������ѹ
				
						 CAN_Data[0]=0x00;
						 CAN_Data[1]=0x00;
						 CAN_Data[2]=0x00;
						 CAN_Data[3]=0x00;
						 CAN_Data[4]=0x00;
						 CAN_Data[5]=0x00;
						 CAN_Data[6]=0x00; 
						 CAN_Data[7]=0x00;				
						 WriteCAN1(8,1, CANID,CAN_Data);						
							
						break;
				
				case 2:
            CANID=0x02C400F0|(i<<8);                           //��ģ�����״̬
				
						 CAN_Data[0]=0x00;
						 CAN_Data[1]=0x00;
						 CAN_Data[2]=0x00;
						 CAN_Data[3]=0x00;
						 CAN_Data[4]=0x00;
						 CAN_Data[5]=0x00;
						 CAN_Data[6]=0x00; 
						 CAN_Data[7]=0x00;				
						 WriteCAN1(8,1, CANID,CAN_Data);					
						break;		
									
				default:
						break;
						
									
			}
				for(i=0;i<ModuleMsg.NOcount;i++)
				{
					Isum= Isum + ModuleCurrent[i];					
				}
				 ModuleMsg.OutCurrent=Isum/10;		
				
			   OSTimeDlyHMSM(0,0,0,5);
		}
}

//void ModuleSet(uint8_t CMD, uint16_t V, uint16_t I,uint8_t ModuleType)
//{
//if(ModuleMsg.ModuleType==0)
// {
//     uint8_t i;
//	
//	for(i=0;i<ModuleMsg.NOcount;i++)
//	{
//				switch(CMD)
//				 {	
//					 case 1:
//						 CANID=0x180160a1|(i<<8);                     //���õ�ѹ����
//					   if(V>5000)
//						 {
//						   CAN_Data[0]=0x75;
//						 }
//						 else
//						 {
//               CAN_Data[0]=0x65;
//						 }							 
//						 CAN_Data[1]=0x00;
//						 CAN_Data[2]=V&0xff;
//						 CAN_Data[3]=(V>>8)&0xff;
//						 CAN_Data[4]=I&0xff;
//						 CAN_Data[5]=(I>>8)&0xff;
//						 CAN_Data[6]=0x0c; 			
//						 CAN_Data[7]=0x17;
//						 WriteCAN1(8,1, CANID,CAN_Data);	
//							break;
//						 
//					 case 2:	 
//						 CANID=0x180160a1|(i<<8);                      //���ٿ���
//					 		if(V>5000)
//						 {
//						   CAN_Data[0]=0x11;
//						 }
//						 else
//						 {
//               CAN_Data[0]=0x01;
//						 }	
//						 CAN_Data[0]=0x01;
//						 CAN_Data[1]=0x00;
//						 CAN_Data[2]=V&0xff;
//						 CAN_Data[3]=(V>>8)&0xff;
//						 CAN_Data[4]=I&0xff;
//						 CAN_Data[5]=(I>>8)&0xff;
//						 CAN_Data[6]=0x6c;
//						 CAN_Data[7]=0x07;		 
//						 WriteCAN1(8,1, CANID,CAN_Data);	
//							break;
//					 case 3:	 
//						 CANID=0x180160a1|(i<<8) ;                     //�ػ�
//						 CAN_Data[0]=0x02;
//						 CAN_Data[1]=0x00;
//						 CAN_Data[2]=0x00;
//						 CAN_Data[3]=0x00;
//						 CAN_Data[4]=0x00;
//						 CAN_Data[5]=0x00;
//						 CAN_Data[6]=0x00;
//						 CAN_Data[7]=0x00;					 
//						 WriteCAN1(8,1, CANID,CAN_Data);	
//					 case 4:	 
//						 CANID=0x184060a1|(i<<8) ;                     //����
//						 CAN_Data[0]=0x00;
//						 CAN_Data[1]=0x00;
//						 CAN_Data[2]=0x00;
//						 CAN_Data[3]=0x00;
//						 CAN_Data[4]=0x00;
//						 CAN_Data[5]=0x00;
//						 CAN_Data[6]=0x00; 
//						 CAN_Data[7]=0x00;					 
//						 WriteCAN1(8,1, CANID,CAN_Data);	
//							break;
//					 default:
//							break;			 		 
//       }
//				 OSTimeDlyHMSM(0,0,0,2);
//    }
//	}
// 	
//}

//void ModuleGet(uint8_t CMD,uint32_t Voltage,uint32_t Current,uint8_t err)
//{
//	  uint8_t i;
//	  uint32_t Isum;
//	
//	switch (CMD)
//	{
//		case 0:
//				for(i=0;i<ModuleMsg.NOcount;i++)
//				{
//					Isum= Isum + ModuleCurrent[i];					
//				}
//				 ModuleMsg.OutCurrent=Isum/10;
//				break;
//				
//		case 1:
//		  	break;
//		
//		case 2:
//		  	break;		
//		
//		
//		
//		
//		
//		
//		
//		default:
//			  break;
//				
//							
//	}
//}


/**************************************************************************************************************
  ��صĺ�������
**************************************************************************************************************/
void AnalyseMT30(void)
{	

	    ChargerMsg.ChargeStage=1;
	 if(Sys_PARA.ChargerNO==0){	
			 CANID=0x1030a000;  
	  }else{
			 CANID=0x1030a001; 
		}	 
			 CAN_Data[0]=0x00;
			 CAN_Data[1]=0x00;
			 CAN_Data[2]=0x00;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;
			 CAN_Data[7]=0x00;		
			 WriteCAN1(8,1, CANID,CAN_Data);	
}

void AnalyseMT31(void)
{	
	 if(Sys_PARA.ChargerNO==0){	
			 CANID=0x1031a000;  
	  }else{
			 CANID=0x1031a001; 
		}		 	
			 CAN_Data[0]=0x01;
			 CAN_Data[1]=0x02;
			 CAN_Data[2]=0x00;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;	
			 CAN_Data[7]=0x00;		
			 WriteCAN1(8,1, CANID,CAN_Data);

			 CAN_Data[0]=0x02;
			 CAN_Data[1]=0x02;
			 CAN_Data[2]=0x00;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;		
			 CAN_Data[7]=0x00;			
			 WriteCAN1(8,1, CANID,CAN_Data);

			 CAN_Data[0]=0x03;
			 CAN_Data[1]=0x02;
			 CAN_Data[2]=0x00;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;		
			 CAN_Data[7]=0x00;					 
			 WriteCAN1(8,1, CANID,CAN_Data);

			 CAN_Data[0]=0x04;
			 CAN_Data[1]=0x02;
			 CAN_Data[2]=0x00;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;
			 CAN_Data[7]=0x00;					 
			 WriteCAN1(8,1, CANID,CAN_Data);

			 CAN_Data[0]=0x05;
			 CAN_Data[1]=0x02;
			 CAN_Data[2]=0x00;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;	
			 CAN_Data[7]=0x00;					 
			 WriteCAN1(8,1, CANID,CAN_Data);

			 CAN_Data[0]=0x06;
			 CAN_Data[1]=0x02;
			 CAN_Data[2]=0x00;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;	
			 CAN_Data[7]=0x00;					 
			 WriteCAN1(8,1, CANID,CAN_Data);

			 CAN_Data[0]=0x07;
			 CAN_Data[1]=0x02;
			 CAN_Data[2]=0x00;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;	
			 CAN_Data[7]=0x00;					 
			 WriteCAN1(8,1, CANID,CAN_Data);

}

void AnalyseMT32(void)
{	
	    ChargerMsg.ChargeStage=8;
	 if(Sys_PARA.ChargerNO==0){	
			 CANID=0x1032a000;  
	  }else{
			 CANID=0x1032a001; 
		}		                       
			 CAN_Data[0]=0x00;
			 CAN_Data[1]=0x00;
			 CAN_Data[2]=0x00;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;			
			 WriteCAN1(8,1, CANID,CAN_Data);	
}

void AnalyseMT33(void)
{	
	 if(Sys_PARA.ChargerNO==0){	
			 CANID=0x1033a000;  
	  }else{
			 CANID=0x1033a001; 
		}	                    
			 CAN_Data[0]=0x00;
			 CAN_Data[1]=0x02;
			 CAN_Data[2]=0x00;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;			
			 WriteCAN1(8,1, CANID,CAN_Data);	
}

void AnalyseMT34(void)
{	
	 if(Sys_PARA.ChargerNO==0){	
			 CANID=0x1834a000;  
	  }else{
			 CANID=0x1834a001; 
		}	  
    if(ADstatus == ADC4V)	
     {
			 CAN_Data[0]=0x04;
     }
		 else{
			 CAN_Data[0]=0x00;
		 }
			 CAN_Data[1]=0x00;
			 CAN_Data[2]=0x00;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;			
			 WriteCAN1(8,1, CANID,CAN_Data);	
}


void AnalyseMT35(void)
{	
	 if((MessageCAN1.DATAA&0xff)==0)
	 {
	    ChargerMsg.ChargeStage=1;
	 }
	 if(Sys_PARA.ChargerNO==0){	
			 CANID=0x1035a000;  
	  }else{
			 CANID=0x1035a001; 
		}	                      
			 CAN_Data[0]=0x01;
			 CAN_Data[1]=0x00;
			 CAN_Data[2]=ChargerMsg.ChargeVoltage&0xff;
			 CAN_Data[3]=(ChargerMsg.ChargeVoltage>>8)&0xff;
			 CAN_Data[4]=(4000-ChargerMsg.ChargeCurrent)&0xff;
			 CAN_Data[5]=((4000-ChargerMsg.ChargeCurrent)>>8)&0xff;
			 CAN_Data[6]=BMSMessage.SOC;			
	 		 CAN_Data[7]=BMSMessage.MINBatteryTemp;		
			 WriteCAN1(8,1, CANID,CAN_Data);	
	 
	     CAN_Data[0]=0x02;
			 CAN_Data[1]=BMSMessage.MAXBatteryTemp;
			 CAN_Data[2]=BMSMessage.MAXSingleVoltage&0xff;
			 CAN_Data[3]=(BMSMessage.MAXSingleVoltage>>8)&0xff;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=BMSMessage.RequestVoltage&0xff;			
	 		 CAN_Data[7]=(BMSMessage.RequestVoltage>>8)&0xff;	 	 
			 WriteCAN1(8,1, CANID,CAN_Data);	 
	 
	     CAN_Data[0]=0x03;
			 CAN_Data[1]=(4000-BMSMessage.RequestCurrent)&0xff;
			 CAN_Data[2]=((4000-BMSMessage.RequestCurrent)>>8)&0xff;
			 CAN_Data[3]=BMSMessage.RequestChargeMode;
			 CAN_Data[4]=BMSMessage.BCSdetVoltage&0xff;
			 CAN_Data[5]=(BMSMessage.BCSdetVoltage>>8)&0xff;
			 CAN_Data[6]=(4000-BMSMessage.BCSdetCurrent)&0xff;			
	 		 CAN_Data[7]=((4000-BMSMessage.BCSdetCurrent)>>8)&0xff;				 			 
			 WriteCAN1(8,1, CANID,CAN_Data);	
			 
	     CAN_Data[0]=0x04;
			 CAN_Data[1]=BMSMessage.RemainTime&0xff;
			 CAN_Data[2]=(BMSMessage.RemainTime>>8)&0xff;
			 CAN_Data[3]=0x00;
			 CAN_Data[4]=0x00;
			 CAN_Data[5]=0x00;
			 CAN_Data[6]=0x00;			
	 		 CAN_Data[7]=0x00;				 
			 WriteCAN1(8,1, CANID,CAN_Data);		  
	 
}