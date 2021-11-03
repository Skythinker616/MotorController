#ifndef _USER_PID_H_
#define _USER_PID_H_

#include "main.h"

typedef struct _PID
{
	float kp,ki,kd;
	float error,lastError;//���ϴ����
	float integral,maxIntegral;//���֡������޷�
	float output,maxOutput;//���������޷�
}PID;

typedef struct _CascadePID
{
	PID inner;//�ڻ�
	PID outer;//�⻷
	float output;//�������������inner.output
}CascadePID;

void PID_Init(PID *pid,float p,float i,float d,float maxSum,float maxOut);
void PID_SingleCalc(PID *pid,float reference,float feedback);
void PID_CascadeCalc(CascadePID *pid,float angleRef,float angleFdb,float speedFdb);
void PID_Clear(PID *pid);
void PID_SetMaxOutput(PID *pid,float maxOut);

#endif
