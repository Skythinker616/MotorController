/*************自定义CAN通信************/

#include "USER_CAN.h"
#include "MotorCtrl.h"

uint8_t USER_CAN_SendData(CAN_HandleTypeDef* hcan,uint32_t StdId,uint8_t data[8]);

//can接收结束中断
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef header;
  uint8_t rx_data[8];
	
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &header, rx_data)!=HAL_OK)
		return;
	
	for(uint8_t index=0;index<MaxCanMotorNum;index++)
	{
		if(hcan==GET_MOTOR_HCAN(index) && canMotors[index].canInfo.rxID==header.StdId)
		{
			MotorCtrl_Update(&canMotors[index], (rx_data[0]<<8 | rx_data[1]), (rx_data[2]<<8 | rx_data[3]));
		}
	}
}

//can初始化，在main的while(1)前调用
void USER_CAN_Init()
{
	uint8_t errCnt=0;//错误计数
	//CAN1过滤器初始化
	CAN_FilterTypeDef Can1_Filter;
	Can1_Filter.FilterActivation = ENABLE;
  Can1_Filter.FilterMode = CAN_FILTERMODE_IDMASK;
  Can1_Filter.FilterScale = CAN_FILTERSCALE_32BIT;
  Can1_Filter.FilterIdHigh = 0x0000;
  Can1_Filter.FilterIdLow = 0x0000;
  Can1_Filter.FilterMaskIdHigh = 0x0000;
  Can1_Filter.FilterMaskIdLow = 0x0000;
  Can1_Filter.FilterBank = 0;
  Can1_Filter.FilterFIFOAssignment = CAN_RX_FIFO0;
	errCnt+=HAL_CAN_ConfigFilter(&hcan1, &Can1_Filter);
	//开启CAN1
	errCnt+=HAL_CAN_Start(&hcan1);
	errCnt+=HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
	//CAN2过滤器初始化
	CAN_FilterTypeDef Can2_Filter;
	Can2_Filter.FilterActivation = ENABLE;
  Can2_Filter.FilterMode = CAN_FILTERMODE_IDMASK;
  Can2_Filter.FilterScale = CAN_FILTERSCALE_32BIT;
  Can2_Filter.FilterIdHigh = 0x0000;
  Can2_Filter.FilterIdLow = 0x0000;
  Can2_Filter.FilterMaskIdHigh = 0x0000;
  Can2_Filter.FilterMaskIdLow = 0x0000;
  Can2_Filter.FilterFIFOAssignment = CAN_RX_FIFO0;
	Can2_Filter.SlaveStartFilterBank=14;
	Can2_Filter.FilterBank = 14;
	errCnt+=HAL_CAN_ConfigFilter(&hcan2, &Can2_Filter);
	//开启CAN2
	errCnt+=HAL_CAN_Start(&hcan2);
	errCnt+=HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
}

//CAN发送数据
uint8_t USER_CAN_SendData(CAN_HandleTypeDef* hcan,uint32_t StdId,uint8_t data[8])
{
	CAN_TxHeaderTypeDef tx_header;
	
	tx_header.StdId = StdId;
  tx_header.IDE   = CAN_ID_STD;
  tx_header.RTR   = CAN_RTR_DATA;
  tx_header.DLC   = 8;
	
	uint32_t txMail=0;
	uint8_t retVal=HAL_CAN_AddTxMessage(hcan, &tx_header, data, &txMail);
		
	return retVal;
}

//发送电机电流信息(0x1FF)
void USER_CAN_SetMotoCurrent(CAN_HandleTypeDef* hcan,
	uint32_t StdId, int16_t cur[4])
{
	uint8_t tx_data[8];
	tx_data[0] = (cur[0]>>8)&0xff;
  tx_data[1] =    (cur[0])&0xff;
  tx_data[2] = (cur[1]>>8)&0xff;
  tx_data[3] =    (cur[1])&0xff;
  tx_data[4] = (cur[2]>>8)&0xff;
  tx_data[5] =    (cur[2])&0xff;
  tx_data[6] = (cur[3]>>8)&0xff;
  tx_data[7] =    (cur[3])&0xff;
	USER_CAN_SendData(hcan, StdId, tx_data);
}
