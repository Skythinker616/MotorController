#ifndef _LED_H_
#define _LED_H_

#include "main.h"

#define SET_LED_RED(state) HAL_GPIO_WritePin(GPIOH,GPIO_PIN_12,(GPIO_PinState)(state))
#define SET_LED_GREEN(state) HAL_GPIO_WritePin(GPIOH,GPIO_PIN_11,(GPIO_PinState)(state))
#define SET_LED_BLUE(state) HAL_GPIO_WritePin(GPIOH,GPIO_PIN_10,(GPIO_PinState)(state))

void Led_Exec(void);

#endif
