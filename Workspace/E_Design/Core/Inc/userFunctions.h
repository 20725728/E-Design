/*
 * userFunctions.h
 *
 *  Created on: 01 Mar 2022
 *      Author: 20725728
 */

#ifndef INC_USERFUNCTIONS_H_
#define INC_USERFUNCTIONS_H_



#include "stm32f3xx_hal.h"

void rxMessage(uint8_t msg[11], uint8_t length);
void displayLCD(void);
void setMode(void);
void setParam(void);
void rqStatus(UART_HandleTypeDef huart2);
void rqMeas(UART_HandleTypeDef huart2, ADC_HandleTypeDef hadc);
void measure(ADC_HandleTypeDef hadc);
void setLEDs(void);
void updateMeasVal(uint16_t value);
void updateMenuState(void);
void setMeasMode(void);



#endif /* INC_USERFUNCTIONS_H_ */
