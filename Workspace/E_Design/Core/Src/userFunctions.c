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
	uint16_t PKPos[20];
	uint16_t maxADC = 0;
	uint16_t minADC = 4096;
	uint16_t cnt = 0;
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

				for(uint32_t i = 0; i < ADC_MEAS-1; i++){
					HAL_ADC_Start(&hadc);
					HAL_ADC_PollForConversion(&hadc,500);
					adc_Values[i] = HAL_ADC_GetValue(&hadc);
					HAL_ADC_Stop(&hadc);
					if(adc_Values[i] > maxADC) maxADC = adc_Values[i];
					if(adc_Values[i] < minADC) minADC = adc_Values[i];
				}
				float maxVoltage = ((maxADC*3.3)/4096)*1000 * correctionFac;
				float minVoltage = ((minADC*3.3)/4096)*1000 * correctionFac;
				output = maxVoltage - minVoltage;
				updateMeasVal(output);
				break;
			case 'f'://Measure AC Voltage frequency

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
					indexTotal = indexTotal + (PKPos[i+1] - PKPos[i]);
				}
				float indexPeriod = indexTotal/40;
				float freqCorrFac = 0.0085;
				output = indexPeriod*freqCorrFac;
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
	HAL_GPIO_WritePin(D2_GPIO_Port,D2_Pin,menuState);//LED2 -> active if in menu display state
	HAL_GPIO_WritePin(D3_GPIO_Port,D3_Pin,measDispState);//LED3 -> active if in measurement display state
	HAL_GPIO_WritePin(D4_GPIO_Port,D4_Pin,outDispState);//LED4 -> active if in the output display state
	HAL_GPIO_WritePin(D5_GPIO_Port,D5_Pin, sigState);//LED5 -> active if the output is on
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

void initialiseLCD(void){
	//uint8_t BF = 0;
	uint8_t RS=0,RNW=0,DB4=0,DB5=0,DB6=0,DB7 = 0;
	//Setup First bit message (same for the first 3 submits)
	//RS	R/W	DB7	DB6	DB6	DB5	DB4
	//0		0	0	0	0	1	1
	DB5 = 1;
	DB4 = 1;
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
	HAL_Delay(4.2);//wait atleast 4.2 ms
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
	HAL_Delay(0.1);//wait atleast 100 us
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
	DB4 = 0;
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
	DB6 = 1;
	DB7 = 1;
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
	DB5 = 0;
	DB6 = 0;
	DB7 = 0;
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
	DB7 = 1;
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
	DB7 = 0;
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
	DB4 = 1;
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
	DB4 = 0;
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
	DB5 = 1;
	txLCD(RS,RNW,DB4,DB5,DB6,DB7);
}

void txLCD(uint8_t RS, uint8_t RNW, uint8_t DB4, uint8_t DB5, uint8_t DB6, uint8_t DB7){
	HAL_GPIO_WritePin(E_GPIO_Port,E_Pin, 1);
	HAL_GPIO_WritePin(RS_GPIO_Port,RS_Pin, RS);
	HAL_GPIO_WritePin(RNW_GPIO_Port,RNW_Pin, RNW);
	HAL_GPIO_WritePin(DB4_GPIO_Port,DB4_Pin, DB4);
	HAL_GPIO_WritePin(DB5_GPIO_Port,DB5_Pin, DB5);
	HAL_GPIO_WritePin(DB6_GPIO_Port,DB6_Pin, DB6);
	HAL_GPIO_WritePin(DB7_GPIO_Port,DB7_Pin, DB7);
	HAL_GPIO_WritePin(E_GPIO_Port,E_Pin,0);
	HAL_Delay(0.005);//wait at least 230ns
	HAL_GPIO_WritePin(E_GPIO_Port,E_Pin,1);
}

void displayLCD(uint8_t msg[2]){

	for(int i = 0;i<2;i++){
		uint8_t RS=0,RNW=0,DB4=0,DB5=0,DB6=0,DB7 = 0;
		switch(msg[i]){
		case '0':
			RS = 1;
			DB5=1;
			DB4 = 1;
			txLCD(RS,RNW,DB4,DB5,DB6,DB7);
			DB5=0;
			DB4=0;
			txLCD(RS,RNW,DB4,DB5,DB6,DB7);
			break;
		case '1':

			break;
		case '2':

			break;
		case '3':

			break;
		case '4':

			break;
		case '5':

			break;
		case '6':

			break;
		case '7':

			break;
		case '8':

			break;
		case '9':

			break;
		case 'A':

			break;
		case 'B':

			break;
		case 'C':

			break;
		case 'D':

			break;
		case 'E':

			break;
		case 'F':

			break;
		case 'G':

			break;
		case 'H':

			break;
		case 'I':

			break;
		case 'J':

			break;
		case 'K':

			break;
		case 'L':

			break;
		case 'M':

			break;
		case 'N':

			break;
		case 'O':

			break;
		case 'P':

			break;
		case 'Q':

			break;
		case 'R':

			break;
		case 'S':

			break;
		case 'T':

			break;
		case 'U':

			break;
		case 'V':

			break;
		case 'W':

			break;
		case 'X':

			break;
		case 'Y':

			break;
		case 'Z':

			break;
		}
	}
}

