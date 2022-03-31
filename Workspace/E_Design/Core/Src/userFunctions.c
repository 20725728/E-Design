/*
 * userFunctions.c
 *
 *  Created on: 01 Mar 2022
 *      Author: 20725728
 */

#include "userFunctions.h"

#define ADC_MEAS 8192

uint8_t rxMsg[11];
uint8_t measMode[2] = "DV";
uint8_t param[1] = "o";
uint8_t measVal[4] = "0000";
uint8_t sigType[1] = "d";
uint8_t sigState = 0;
uint16_t ampl = 0;
uint16_t freq = 0;
uint8_t offset = 0;
uint8_t temp = 0;

uint8_t statusMsg[11] = "@,XX,X,X,!\n";
uint8_t measMsg[13] = "@,m,X,XXXX,!\n";

uint8_t menuState = 0;
uint8_t measDispState = 0;
uint8_t outDispState = 0;
uint16_t adc_Values[ADC_MEAS];

void rxMessage(uint8_t msg[11], uint8_t length){
	rxMsg[0] = msg[11];
	for(int i=0;i<length;i++){
		rxMsg[i+1] = msg[i];
	}
}

void rqStatus(UART_HandleTypeDef huart){
	if(rxMsg[4] == '1'){
		sigState = 1;
	}else{
		sigState = 0;
	}
	//sigState = (uint8_t)rxMsg[4];//second term is the output state, first term is a comma
	statusMsg[2] = measMode[0];
	statusMsg[3] = measMode[1];
	statusMsg[5] = sigType[0];
	statusMsg[7] = rxMsg[4];
	HAL_UART_Transmit(&huart,statusMsg,11,500);
}

void rqMeas(UART_HandleTypeDef huart, ADC_HandleTypeDef hadc){
	//HAL_UART_Receive(&huart,measParam,1,500);
	measure(hadc);
	measMsg[4] = rxMsg[4];
	measMsg[6] = measVal[0];
	measMsg[7] = measVal[1];
	measMsg[8] = measVal[2];
	measMsg[9] = measVal[3];
	HAL_UART_Transmit(&huart,measMsg,13,500);
}

void measure(ADC_HandleTypeDef hadc){
	uint8_t cnt = 0;
	float tot = 0;
	float correctionFac = 1.08;
	switch(measMode[0]){
	case 'D'://DC
		switch(measMode[1]){
		case 'V'://DC Voltage offset
			while(cnt < 10){
				float temp;
				float correctionFac = 1.08;
				HAL_ADC_Start(&hadc);
				HAL_ADC_PollForConversion(&hadc,500);
				float adcVal = HAL_ADC_GetValue(&hadc);
				HAL_ADC_Stop(&hadc);
				temp = ((adcVal*3.3)/4096)*1000 *correctionFac;//total volts measured in mV
				tot = tot + temp;
				cnt++;
			}

			uint16_t output = tot/cnt;
			updateMeasVal(output);
			cnt=0;
			tot = 0;
			break;
		case 'I'://DC current

			break;
		}
		break;
	case 'A'://AC
		switch(measMode[1]){
		case 'V'://AC Voltage
			switch(param[0]){
			case 'o'://Measure AC Voltage offset
				while(cnt < 10){
					float temp;
					HAL_ADC_Start(&hadc);
					HAL_ADC_PollForConversion(&hadc,500);
					float adcVal = HAL_ADC_GetValue(&hadc);
					HAL_ADC_Stop(&hadc);
					temp = ((adcVal*3.3)/4096)*1000 *correctionFac;//total volts measured in mV
					tot = tot + temp;
					cnt++;
				}
				uint16_t output = tot/cnt;
				updateMeasVal(output);
				cnt=0;
				tot = 0;
				break;
//AC Voltage amplitude and frequency updated after DEMO 2 using code from 22662790
			case 'a'://Measure AC Voltage amplitude
				uint16_t maxADC = 0;
				uint16_t minADC = 4096;
				for(uint32_t i = 0; i < ADC_MEAS-1; i++){
					HAL_ADC_Start(&hadc);
					HAL_ADC_PollForConversion(&hadc,500);
					adc_Values[i] = HAL_ADC_GetValue(&hadc);
					HAL_ADC_Stop(&hadc);
					if(adc_Values[i] > maxADC) maxADC = adc_Values[i];
					if(adc_Values[i] < minADC) min ADC = adc_Values[i];
				}
				float maxVoltage = ((maxADC*3.3)/4096)*1000 * correctionFac;
				float minVoltage = ((minADC*3.3)/4096)*1000 * correctionFac;
				uint16_t output = maxVoltage - minVoltage;
				updateMeasVal(output);
				break;
			case 'f'://Measure AC Voltage frequency
				uint16_t PKPos[20];
				uint16_t cnt = 0;
				for(uint32_t i = 0; i < ADC_MEAS-1; i++){
					HAL_ADC_Start(&hadc);
					HAL_ADC_PollForConversion(&hadc,500);
					adc_Values[i] = HAL_ADC_GetValue(&hadc);
					HAL_ADC_Stop(&hadc);
				}
				uint16_t i = 1;
				while(cnt < 20){
					if(i == ADC_MEAS) i = 1;
					if(adc_Values[i] > adc_Values[i-1] && adc_Values[i] > adc_Values[i+1]){
						PKPos[cnt] = i;
						cnt++;
						i++;
					}
				}
				uint16_t indexTotal = 0;
				for(uint16_t i =0; i<19;i++){
					indexTotal = cnt[i+1] - cnt[i];
				}
				float indexPeriod = indexTotal/40;
				float freqCorrFac = 0.0085;
				uint16_t output = indexPeriod*freqCorrFac;
				updateMeasVal(output);
				break;
			}
//End of code from 22662790
			break;
		case 'I'://AC current

			break;
		}
		break;
	case 'T'://Temperature

		break;
	}
}

void updateMeasVal(uint16_t value){
	measVal[0] = (((int)value)/1000)%10 + 48;
	measVal[1] = (((int)value)/100)%10 + 48;
	measVal[2] = (((int)value)/10)%10 + 48;
	measVal[3] = ((int)value)%10 + 48;
}

void setLEDs(void){
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,menuState);//LED1 -> active if in menu display state
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,measDispState);//LED2 -> active if in measurement display state
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,outDispState);//LED3 -> active if in the output display state
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13, sigState);//LED4 -> active if the output is on
}

void updateMenuState(void){
	if(menuState == 1){
		menuState = 0;
	}else{
		menuState = 1;
	}
}

void setMeasMode(void){
	measMode[0] = rxMsg[2];
	measMode[1] = rxMsg[3];

}
