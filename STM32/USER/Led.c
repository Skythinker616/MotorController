#include "Led.h"

void Led_Exec()
{
	static uint32_t lastChangeTime=0;
	if(HAL_GetTick()-lastChangeTime>500)
	{
		HAL_GPIO_TogglePin(GPIOH,GPIO_PIN_11);
		lastChangeTime=HAL_GetTick();
	}
}
