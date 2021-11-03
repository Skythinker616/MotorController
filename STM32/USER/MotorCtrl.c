#include "MotorCtrl.h"
#include "USER_CAN.h"
#include <string.h>

CanMotor canMotors[MaxCanMotorNum]={0};

void MotorCtrl_StartCalcAngle(CanMotor *moto)
{
	moto->totalAngle=0;
	moto->lastAngle=moto->angle;
	moto->targetAngle=0;
}

//计算电机累计转过的角度
void MotorCtrl_CalcAngle(CanMotor *moto)
{
	int32_t dAngle=0;
	if(moto->angle-moto->lastAngle<-4000)
		dAngle=moto->angle+(8191-moto->lastAngle);
	else if(moto->angle-moto->lastAngle>4000)
		dAngle=-moto->lastAngle-(8191-moto->angle);
	else
		dAngle=moto->angle-moto->lastAngle;
	//将角度增量加入计数器
	moto->totalAngle+=dAngle;
	//记录角度
	moto->lastAngle=moto->angle;
}

//更新电机信息
void MotorCtrl_Update(CanMotor *moto,int16_t angle,int16_t speed)
{
	moto->angle=angle;
	moto->speed=speed;
	MotorCtrl_CalcAngle(moto);
}

void MotorCtrl_UpdateCanInfo(CanMotor *moto)
{
	if(moto->type!=Motor_3508 && moto->type!=Motor_2006 && moto->type!=Motor_6020)
		return;
	if(moto->id<1)
		return;
	if(moto->type==Motor_6020 && moto->id>7)
		return;
	if(moto->id>8)
		return;
	
	switch(moto->type)
	{
		case Motor_3508:
		case Motor_2006:
			moto->canInfo.rxID=0x200+moto->id;
			moto->canInfo.txID=(moto->id<=4?0x200:0x1FF);
			moto->canInfo.txPos=(moto->id-1)%4;
		break;
		case Motor_6020:
			moto->canInfo.rxID=0x204+moto->id;
			moto->canInfo.txID=(moto->id<=4?0x1FF:0x2FF);
			moto->canInfo.txPos=(moto->id-1)%4;
		break;
		default:
		break;
	}
}

inline int16_t MotorCtrl_GetMaxOutputOfMotorType(MotorType type)
{
	if(type==Motor_3508)
		return 20000;
	if(type==Motor_6020)
		return 30000;
	if(type==Motor_2006)
		return 10000;
	return 0;
}

void MotorCtrl_Exec()
{
	int16_t can1TxBuf200[4]={0},can1TxBuf1FF[4]={0},can1TxBuf2FF[4]={0};
	int16_t can2TxBuf200[4]={0},can2TxBuf1FF[4]={0},can2TxBuf2FF[4]={0};
	
	for(uint8_t index=0;index<MaxCanMotorNum;index++)
	{
		if(canMotors[index].working)
		{
			//计算电机输出值
			int16_t output=0;
			switch(canMotors[index].mode)
			{
				case Ctrl_Tor:
					output=canMotors[index].targetTorque*0.01f*MotorCtrl_GetMaxOutputOfMotorType(canMotors[index].type);
				break;
				case Ctrl_Spd:
					PID_SingleCalc(&canMotors[index].singPID,canMotors[index].targetSpeed,canMotors[index].speed);
					output=canMotors[index].singPID.output;
				break;
				case Ctrl_SingAng:
					PID_SingleCalc(&canMotors[index].singPID,canMotors[index].targetAngle,canMotors[index].totalAngle);
					output=canMotors[index].singPID.output;
				break;
				case Ctrl_CasAng:
					PID_CascadeCalc(&canMotors[index].casPID,canMotors[index].targetAngle,canMotors[index].totalAngle,canMotors[index].speed);
					output=canMotors[index].casPID.output;
				break;
			}
			//将输出值写入can缓冲区
			if(GET_MOTOR_HCAN(index)==&hcan1)
			{
				switch(canMotors[index].canInfo.txID)
				{
					case 0x200: can1TxBuf200[canMotors[index].canInfo.txPos]=output; break;
					case 0x1FF: can1TxBuf1FF[canMotors[index].canInfo.txPos]=output; break;
					case 0x2FF: can1TxBuf2FF[canMotors[index].canInfo.txPos]=output; break;
				}
			}
			else
			{
				switch(canMotors[index].canInfo.txID)
				{
					case 0x200: can2TxBuf200[canMotors[index].canInfo.txPos]=output; break;
					case 0x1FF: can2TxBuf1FF[canMotors[index].canInfo.txPos]=output; break;
					case 0x2FF: can2TxBuf2FF[canMotors[index].canInfo.txPos]=output; break;
				}
			}
		}
	}
	//发送can命令帧
	USER_CAN_SetMotoCurrent(&hcan1,0x200,can1TxBuf200);
	USER_CAN_SetMotoCurrent(&hcan1,0x1FF,can1TxBuf1FF);
	USER_CAN_SetMotoCurrent(&hcan1,0x2FF,can1TxBuf2FF);
	USER_CAN_SetMotoCurrent(&hcan2,0x200,can2TxBuf200);
	USER_CAN_SetMotoCurrent(&hcan2,0x1FF,can2TxBuf1FF);
	USER_CAN_SetMotoCurrent(&hcan2,0x2FF,can2TxBuf2FF);
}

inline float MotorCtrl_DecodeFloat(void *buf)
{
	float ret=0;
	memcpy(&ret,buf,4);
	return ret;
}

void MotorCtrl_ParseUsb(uint8_t *buf,uint32_t len)
{
	if(buf[0]!=FRAME_HEADER || len<2)
		return;
	
	uint32_t frameLen=len;
	
	switch(buf[1])
	{
		case FrameCmd_SetMode:
			if(len>=3+1)
			{
				canMotors[buf[2]].mode=(CanCtrlMode)buf[3];
				if(buf[3]==Ctrl_CasAng||buf[3]==Ctrl_SingAng)
					MotorCtrl_StartCalcAngle(&canMotors[buf[2]]);
				PID_Clear(&canMotors[buf[2]].singPID);
				PID_Clear(&canMotors[buf[2]].casPID.inner);
				PID_Clear(&canMotors[buf[2]].casPID.outer);
				canMotors[buf[2]].casPID.output=0;
				frameLen=4;
			}
		break;
		case FrameCmd_SetTor:
			if(len>=3+4)
			{
				canMotors[buf[2]].targetTorque=MotorCtrl_DecodeFloat(buf+3);
				frameLen=7;
			}
		break;
		case FrameCmd_SetSpd:
			if(len>=3+4)
			{
				canMotors[buf[2]].targetSpeed=MotorCtrl_DecodeFloat(buf+3);
				frameLen=7;
			}
		break;
		case FrameCmd_SetSingAng:
		case FrameCmd_SetCasAng:
			if(len>=3+4)
			{
				if(canMotors[buf[2]].type==Motor_3508)
					canMotors[buf[2]].targetAngle=MOTO_M3508_DGR2CODE(MotorCtrl_DecodeFloat(buf+3));
				else if(canMotors[buf[2]].type==Motor_6020)
					canMotors[buf[2]].targetAngle=MOTO_M6020_DGR2CODE(MotorCtrl_DecodeFloat(buf+3));
				else if(canMotors[buf[2]].type==Motor_2006)
					canMotors[buf[2]].targetAngle=MOTO_M2006_DGR2CODE(MotorCtrl_DecodeFloat(buf+3));
				frameLen=7;
			}
		break;
		case FrameCmd_SetCasSpd:
			if(len>=3+4)
			{
				PID_SetMaxOutput(&canMotors[buf[2]].casPID.outer,MotorCtrl_DecodeFloat(buf+3));
				frameLen=7;
			}
		break;
		case FrameCmd_SetPID:
			if(len>=3+4*5+8*5)
			{
				PID *singPid=&canMotors[buf[2]].singPID;
				PID *inner=&canMotors[buf[2]].casPID.inner,*outer=&canMotors[buf[2]].casPID.outer;
				singPid->kp=MotorCtrl_DecodeFloat(buf+3);
				singPid->ki=MotorCtrl_DecodeFloat(buf+3+4);
				singPid->kd=MotorCtrl_DecodeFloat(buf+3+4*2);
				singPid->maxIntegral=MotorCtrl_DecodeFloat(buf+3+4*3);
				singPid->maxOutput=MotorCtrl_DecodeFloat(buf+3+4*4);
				inner->kp=MotorCtrl_DecodeFloat(buf+3+4*5);
				inner->ki=MotorCtrl_DecodeFloat(buf+3+4*6);
				inner->kd=MotorCtrl_DecodeFloat(buf+3+4*7);
				inner->maxIntegral=MotorCtrl_DecodeFloat(buf+3+4*8);
				inner->maxOutput=MotorCtrl_DecodeFloat(buf+3+4*9);
				outer->kp=MotorCtrl_DecodeFloat(buf+3+4*10);
				outer->ki=MotorCtrl_DecodeFloat(buf+3+4*11);
				outer->kd=MotorCtrl_DecodeFloat(buf+3+4*12);
				outer->maxIntegral=MotorCtrl_DecodeFloat(buf+3+4*13);
				outer->maxOutput=MotorCtrl_DecodeFloat(buf+3+4*14);
				frameLen=3+4*5+8*5;
			}
		break;
		case FrameCmd_SetWorkState:
			if(len>=3+1)
			{
				canMotors[buf[2]].working=buf[3];
				frameLen=4;
			}
		break;
		case FrameCmd_StopAll:
			if(len>=2)
			{
				for(uint8_t i=0;i<MaxCanMotorNum;i++)
					canMotors[i].working=false;
				frameLen=2;
			}
		break;
		case FramdCmd_SetType:
			if(len>=3+1)
			{
				canMotors[buf[2]].type=(MotorType)buf[3];
				MotorCtrl_UpdateCanInfo(&canMotors[buf[2]]);
				frameLen=4;
			}
		break;
		case FramdCmd_SetId:
			if(len>=3+1)
			{
				canMotors[buf[2]].id=buf[3];
				MotorCtrl_UpdateCanInfo(&canMotors[buf[2]]);
				frameLen=4;
			}
		break;
		case FrameCmd_SetTimConf:
			if(len>=2+2*4)
			{
				uint16_t tim1Psc=buf[2]<<8|buf[3],tim1Arr=buf[4]<<8|buf[5];
				uint16_t tim8Psc=buf[6]<<8|buf[7],tim8Arr=buf[8]<<8|buf[9];
				__HAL_TIM_PRESCALER(&htim1,tim1Psc);
				__HAL_TIM_SetAutoreload(&htim1,tim1Arr);
				__HAL_TIM_PRESCALER(&htim8,tim8Psc);
				__HAL_TIM_SetAutoreload(&htim8,tim8Arr);
				frameLen=2+2*4;
			}
		break;
		case FrameCmd_SetPwmPsc:
			if(len>=3+2)
			{
				uint16_t psc=buf[3]<<8|buf[4];
				__HAL_TIM_PRESCALER(GET_MOTOR_HTIM(buf[2]),psc);
				frameLen=5;
			}
		break;
		case FrameCmd_SetPwmArr:
			if(len>=3+2)
			{
				uint16_t arr=buf[3]<<8|buf[4];
				__HAL_TIM_SetAutoreload(GET_MOTOR_HTIM(buf[2]),arr);
				frameLen=5;
			}
		break;
		case FrameCmd_SetPwmCcr:
			if(len>=3+2)
			{
				uint16_t ccr=buf[3]<<8|buf[4];
				__HAL_TIM_SetCompare(GET_MOTOR_HTIM(buf[2]),GET_MOTOR_TIM_CHANNEL(buf[2]),ccr);
				frameLen=5;
			}
		break;
		case FrameCmd_SetPwmWorkState:
			if(len>=3+1)
			{
				if(buf[3])
					HAL_TIM_PWM_Start(GET_MOTOR_HTIM(buf[2]),GET_MOTOR_TIM_CHANNEL(buf[2]));
				else
					HAL_TIM_PWM_Stop(GET_MOTOR_HTIM(buf[2]),GET_MOTOR_TIM_CHANNEL(buf[2]));
				frameLen=4;
			}
		break;
	}
	
	if(len>frameLen)
		MotorCtrl_ParseUsb(buf+frameLen,len-frameLen);
}
