/*
 * multitool.h
 *
 *  Created on: Feb 17, 2022
 *      Author: 22662790
 */

#ifndef INC_MULTITOOL_H_
#define INC_MULTITOOL_H_

#include "stm32f3xx_hal.h"

UART_HandleTypeDef* huartp;
ADC_HandleTypeDef* hadcpt;
ADC_HandleTypeDef* hadcpv;

void multitool_init(void);
void multitool_startup(void);

void handle_command(void);

void multitool_loop(void);

void write_stdnum(uint32_t stdno);

void display_lcd(uint8_t rs, uint8_t b);
void set_measure_mode(uint8_t c1, uint8_t c2);
void set_parameter(uint8_t param, uint32_t val);
void request_status(uint8_t out_on);
void request_measurement(uint8_t param);
double process_measurement(void);

uint32_t iabs(int32_t v);
uint32_t measure_contig(uint32_t starti, uint32_t tol, uint32_t cm);

void adc_callback(ADC_HandleTypeDef* hadcp);

enum mode {DCV, ACV, DCI, ACI, TC};
typedef enum mode MODE;

enum param {TYPE, AMPL, OFFS, FREQ, DUTY, TEMP};
typedef enum param PARAM;

enum otype {DC, SIN, PUL};
typedef enum otype OTYPE;

enum state {MENU, MEAS, OUT};
typedef enum state STATE;

#endif /* INC_MULTITOOL_H_ */
