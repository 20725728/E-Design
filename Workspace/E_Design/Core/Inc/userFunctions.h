/*
 * userFunctions.h
 *
 *  Created on: 01 Mar 2022
 *      Author: 20725728
 */

#ifndef INC_USERFUNCTIONS_H_
#define INC_USERFUNCTIONS_H_
#define DB4_Pin GPIO_PIN_12
#define DB4_GPIO_Port GPIOA
#define DB5_Pin GPIO_PIN_5
#define DB5_GPIO_Port GPIOC
#define DB6_Pin GPIO_PIN_6
#define DB6_GPIO_Port GPIOC
#define DB7_Pin GPIO_PIN_8
#define DB7_GPIO_Port GPIOC
#define RS_Pin GPIO_PIN_1
#define RS_GPIO_Port GPIOB
#define RNW_Pin GPIO_PIN_2
#define RNW_GPIO_Port GPIOB
#define E_Pin GPIO_PIN_11
#define E_GPIO_Port GPIOA
#define D5_Pin GPIO_PIN_13
#define D5_GPIO_Port GPIOB
#define D4_Pin GPIO_PIN_14
#define D4_GPIO_Port GPIOB
#define D3_Pin GPIO_PIN_15
#define D3_GPIO_Port GPIOB
#define D2_Pin GPIO_PIN_12
#define D2_GPIO_Port GPIOB
#define S5_Pin GPIO_PIN_8
#define S5_GPIO_Port GPIOA
#define S5_EXTI_IRQn EXTI9_5_IRQn
#define S4_Pin GPIO_PIN_9
#define S4_GPIO_Port GPIOA
#define S4_EXTI_IRQn EXTI9_5_IRQn
#define S3_Pin GPIO_PIN_10
#define S3_GPIO_Port GPIOA
#define S3_EXTI_IRQn EXTI15_10_IRQn
#define S2_Pin GPIO_PIN_7
#define S2_GPIO_Port GPIOA
#define S2_EXTI_IRQn EXTI9_5_IRQn
#define S1_Pin GPIO_PIN_6
#define S1_GPIO_Port GPIOA
#define S1_EXTI_IRQn EXTI9_5_IRQn


#include "stm32f3xx_hal.h"

void rxMessage(uint8_t msg[11], uint8_t length);
void displayLCD(uint8_t msg[2]);
void setMode(void);
void setParam(void);
void rqStatus(UART_HandleTypeDef huart2);
void rqMeas(UART_HandleTypeDef huart2, ADC_HandleTypeDef hadc);
void measure(ADC_HandleTypeDef hadc);
void setLEDs(void);
void updateMeasVal(uint16_t value);
void updateMenuState(void);
void setMeasMode(void);
void initialiseLCD(void);
void txLCD(uint8_t RS, uint8_t RNW, uint8_t DB4, uint8_t DB5, uint8_t DB6, uint8_t DB7);



#endif /* INC_USERFUNCTIONS_H_ */
