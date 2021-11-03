#ifndef _USER_CAN_H_
#define _USER_CAN_H_

#include "main.h"
#include "can.h"

/****接口函数声明****/
void USER_CAN_Init(void);
void USER_CAN_SetMotoCurrent(CAN_HandleTypeDef* hcan,
	uint32_t StdId, int16_t cur[4]);

#endif
