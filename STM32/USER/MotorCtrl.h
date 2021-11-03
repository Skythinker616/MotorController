#ifndef _MOTORCTRL_H_
#define _MOTORCTRL_H_

#include "main.h"
#include "PID.h"
#include "can.h"
#include "tim.h"

//各种电机编码值与角度的换算
#define MOTO_M3508_DGR2CODE(dgr) ((int32_t)(dgr*436.9263f)) //3591/187*8191/360
#define MOTO_M3508_CODE2DGR(code) ((float)(code/436.9263f))
#define MOTO_M2006_DGR2CODE(dgr) ((int32_t)(dgr*819.1f)) //36*8191/360
#define MOTO_M2006_CODE2DGR(code) ((float)(code/819.1f))
#define MOTO_M6020_DGR2CODE(dgr) ((int32_t)(dgr*22.7528f)) //8191/360
#define MOTO_M6020_CODE2DGR(code) ((float)(code/22.7528f))

#define FRAME_HEADER 0xAA

#define GET_MOTOR_HCAN(index) ((index)<4?(&hcan1):((index)<8?(&hcan2):NULL))
#define GET_MOTOR_HTIM(index) ((index)<4?(&htim1):((index)<7?(&htim8):NULL))
#define GET_MOTOR_TIM_CHANNEL(index) ((index)%4*0x04)

typedef enum {
	FrameCmd_SetMode,
	FrameCmd_SetTor,
	FrameCmd_SetSpd,
	FrameCmd_SetSingAng,
	FrameCmd_SetCasAng,
	FrameCmd_SetCasSpd,
	FrameCmd_SetPID,
	FrameCmd_SetWorkState,
	FrameCmd_StopAll,
	FramdCmd_SetType,
	FramdCmd_SetId,
	FrameCmd_SetTimConf,
	FrameCmd_SetPwmPsc,
	FrameCmd_SetPwmArr,
	FrameCmd_SetPwmCcr,
	FrameCmd_SetPwmWorkState
}FrameCmdCode;

typedef enum {
	Motor_UnknownType,
	Motor_3508,
	Motor_6020,
	Motor_2006
}MotorType;

typedef enum {
	Can1Motor1,
	Can1Motor2,
	Can1Motor3,
	Can1Motor4,
	Can2Motor1,
	Can2Motor2,
	Can2Motor3,
	Can2Motor4,
	MaxCanMotorNum
}CanMotorIndex;

typedef enum {
	PwmMotor1,
	PwmMotor2,
	PwmMotor3,
	PwmMotor4,
	PwmMotor5,
	PwmMotor6,
	PwmMotor7,
	MaxPwmMotorNum
}PwmMotorIndex;

typedef enum {
	Ctrl_Tor,
	Ctrl_Spd,
	Ctrl_SingAng,
	Ctrl_CasAng
}CanCtrlMode;

typedef __packed struct{
	float kp,ki,kd,maxInt,maxOut;
}PidParam;

typedef __packed struct{
	PidParam inner,outer;
}CasPidParam;

typedef struct {
	bool working;
	MotorType type;
	uint8_t id;
	CanCtrlMode mode;
	struct{
		uint32_t rxID,txID,txPos;
	}canInfo;
	
	PID singPID;
	CascadePID casPID;
	
	int16_t speed,angle;//当前获取到的速度角度
	int16_t lastAngle;//记录上一次得到的角度
	int16_t targetSpeed;//目标速度(rpm)
	int32_t targetAngle;//目标角度(编码器值)
	int32_t totalAngle;//累计转过的编码器值
	float targetTorque;//目标扭矩百分比
}CanMotor;

extern CanMotor canMotors[];

void MotorCtrl_Update(CanMotor *moto,int16_t angle,int16_t speed);
void MotorCtrl_ParseUsb(uint8_t *buf,uint32_t len);
void MotorCtrl_Exec(void);

#endif
