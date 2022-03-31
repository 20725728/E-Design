/*
 * multitool.c
 *
 *  Created on: Feb 17, 2022
 *      Author: 22662790
 */

#include "multitool.h"
#include <stdio.h>

#define BUF_SIZE 8192

uint8_t rx_buf[128];
uint8_t tx_buf[128];
uint8_t rx_idx = 0;
uint8_t tx_len;
uint8_t cmd_flag = 0;
uint8_t cmd_buf[128];

uint32_t last_state[16];
uint32_t last_exti[16];

uint32_t adc_idx = 0;
uint16_t adc_buf[BUF_SIZE];

MODE sys_mode = DCV;
STATE dis_state = MEAS;
OTYPE out_type = DC;
PARAM in_param = OFFS;
uint8_t out_state = 0;

char* MODE_STR = "DVAVDIAITC";
char* OUTT_STR = "dsp";
char* PARAM_STR = "taofdc";

void multitool_init(void) {

}

void multitool_startup(void) {
	write_stdnum(22662790);
	HAL_UART_Receive_IT(huartp, rx_buf, 1);
}

void multitool_loop(void) {
	for (int i = 6; i <= 10; i++) {
		last_state[i] = HAL_GPIO_ReadPin(GPIOA, (1 << i));
	}

	if (cmd_flag > 1) {
		handle_command();
		cmd_flag = 0;
	}

	if (adc_idx >= BUF_SIZE) {
		double val = process_measurement();
		tx_len = sprintf((char*)tx_buf, "@,m,%c,%04lu,!\n", PARAM_STR[in_param], (uint32_t)val);
		HAL_UART_Transmit(huartp, tx_buf, tx_len, 1000);
		adc_idx = 0;
	}

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, dis_state == MENU);
}

void handle_command(void) {
	if (cmd_buf[0] != '@') return;
	if (cmd_buf[cmd_flag-1] != '!') return;
	uint32_t value;
	switch (cmd_buf[2]) {
	case '#':
		display_lcd(cmd_buf[4], cmd_buf[6]);
		break;
	case '$':
		set_measure_mode(cmd_buf[4], cmd_buf[5]);
		break;
	case '^':
		value = 1000*cmd_buf[6]+100*cmd_buf[7]+10*cmd_buf[8]+cmd_buf[9];
		set_parameter(cmd_buf[4], value);
		break;
	case '*':
		if (cmd_buf[4] == 's') request_status(cmd_buf[6]);
		else if (cmd_buf[4] == 'm') request_measurement(cmd_buf[6]);
		break;
	default:
		break;
	}
}

void write_stdnum(uint32_t stdno) {
	tx_len = sprintf((char*)tx_buf, "@,%08lu,!\n", stdno);
	HAL_UART_Transmit(huartp, tx_buf, tx_len, 1000);
}

void display_lcd(uint8_t rs, uint8_t b) {
	// not implemented
	return;
}

void set_measure_mode(uint8_t c1, uint8_t c2) {
	     if (c1 == 'A' && c2 == 'V') sys_mode = ACV;
	else if (c1 == 'D' && c2 == 'V') sys_mode = DCV;
	else if (c1 == 'A' && c2 == 'I') sys_mode = ACI;
	else if (c1 == 'D' && c2 == 'I') sys_mode = DCI;
	else if (c1 == 'T' && c2 == 'C') sys_mode = TC;
}

void set_parameter(uint8_t param, uint32_t val) {
	// not implemented
	return;
}

void request_status(uint8_t out_on) {
	out_state = out_on == '1';
	tx_len = sprintf((char*)tx_buf, "@,%c%c,%c,%c,!\n",
			                        MODE_STR[2*sys_mode],
									MODE_STR[2*sys_mode+1],
									OUTT_STR[out_type],
									out_state ? '1' : '0');
	HAL_UART_Transmit(huartp, tx_buf, tx_len, 1000);
	return;
}

void request_measurement(uint8_t param) {
	if (param == 'a') in_param = AMPL;
	else if (param == 'f') in_param = FREQ;
	else if (param == 'o') in_param = OFFS;
	else if (sys_mode == TC) in_param = TEMP;

	adc_idx = 0;
	if (sys_mode == DCV || sys_mode == ACV) HAL_ADC_Start_IT(hadcpv);
	else if (sys_mode == DCI || sys_mode == ACI) {}
	else if (sys_mode == TC) {};
	return;
}

uint32_t iabs(int32_t v) {
	if (v > 0) return (uint32_t) v;
	else return (uint32_t) (-v);
}

uint32_t measure_contig(uint32_t starti, uint32_t tol, uint32_t cm) {
	uint32_t i;
	for (i = starti; i < BUF_SIZE && iabs((int32_t)adc_buf[i]-cm) > tol+10; i++);
	uint32_t count;
	for (count = 1; i+count < BUF_SIZE && iabs((int32_t)adc_buf[i+count]-cm) <= tol; count++);
	return count;
}

double process_measurement(void) {
	uint32_t sum = 0;
	uint32_t amax = 0;
	uint32_t amin = 4096;

	for (uint32_t i = 0; i < BUF_SIZE; i++) {
		double x = (double)adc_buf[i];
		x = 55.72 + 0.8054*x + 0.000007137*x*x;
		adc_buf[i] = x < 0 ? 0 : (uint16_t)x;
		sum += adc_buf[i];
		if (adc_buf[i] > amax) amax = adc_buf[i];
		if (adc_buf[i] < amin) amin = adc_buf[i];
	}
	double cm = ((double)sum)/BUF_SIZE;
	if (sys_mode == DCV) return cm;
	if (sys_mode == DCI) return cm;
	if (sys_mode == TC) return cm;

	uint32_t tol = (amax-amin)/8;
	uint32_t starti = 0;
	while (adc_buf[starti+1] >= adc_buf[starti]) starti++;
	if (starti == 0) while (adc_buf[starti+1] <= adc_buf[starti]) starti++;
	while (measure_contig(starti, tol, (uint32_t)cm) > 20) tol /= 2;

	uint8_t cross_flag = iabs((int32_t)adc_buf[0]-(int32_t)cm) < tol;
	uint32_t isum = 0;
	uint32_t icnt = 0;
	double prevc = 0;
	double dsum = 0;
	uint32_t asum = 0;
	uint32_t acnt = 0;
	for (uint32_t i = 0; i < BUF_SIZE-1; i++) {
		if (cross_flag) {
			isum += i;
			icnt ++;
		}
		if (cross_flag && iabs((int32_t)adc_buf[i+1]-(int32_t)cm) > tol+10) {
			cross_flag = 0;
			if (prevc != 0) {
				dsum += ((double)isum)/icnt - prevc;
				asum += iabs((int32_t)adc_buf[(uint32_t)((((double)isum)/icnt + prevc)/2)]-(int32_t)cm);
				acnt ++;
			}
			prevc = ((double)isum)/icnt;
			isum = 0;
			icnt = 0;
		}
		else if (!cross_flag && iabs((int32_t)adc_buf[i+1]-(int32_t)cm) < tol) {
			cross_flag = 1;
		}
	}

	//double dm = ((double)asum)/acnt;
	double dm = ((double)amax - amin);

	double f = 0.5/(0.0000025*(dsum/(acnt)));
	f = 0.4589474*f;

	if (in_param == AMPL) return dm;
	if (in_param == OFFS) return cm;
	if (in_param == FREQ) return f;
	return 0;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (rx_buf[rx_idx] == '\n') {
		for (uint8_t i = 0; i < rx_idx; i++) cmd_buf[i] = rx_buf[i];
		cmd_flag = rx_idx;
		rx_idx = 0;
	} else {
		rx_idx++;
	}
	HAL_UART_Receive_IT(huart, rx_buf+rx_idx, 1);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == (1 << 8) && HAL_GetTick()-last_exti[8] > 100) {
		if (last_state[8]) dis_state = dis_state == MEAS ? MENU : MEAS;
		last_exti[8] = HAL_GetTick();
	}
}

void adc_callback(ADC_HandleTypeDef* hadcp) {
	adc_buf[adc_idx++] = (uint16_t)HAL_ADC_GetValue(hadcp);
	if (adc_idx >= BUF_SIZE) {
		HAL_ADC_Stop_IT(hadcp);
	}
}
