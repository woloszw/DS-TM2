#include "SharpSens.h"
float volt_coeff = ((float)(((float)2.91) / 4095) );	//Correction coefficient

uint8_t ADC_Init(void)
{
	uint16_t calib_temp;
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;      
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK;
	
	PORTA->PCR[9] &=~ (PORT_PCR_MUX(0));				//PTA9 - analog input, channel 2
	PORTB->PCR[0] &=~ (PORT_PCR_MUX(0));				//PTB0 - analog input, channel 6
	PORTA->PCR[8] &=~ (PORT_PCR_MUX(0));				//PTA8 - analog input, channel 3
	PORTA->PCR[7] &=~ (PORT_PCR_MUX(0));				//PTA7 - analog input, channel 7
	

	ADC0->CFG1 = ADC_CFG1_ADICLK(ADICLK_BUS_2) | ADC_CFG1_ADIV(ADIV_4) | ADC_CFG1_ADLSMP_MASK;	
	ADC0->CFG2 = ADC_CFG2_ADHSC_MASK;																														
	ADC0->SC3  = ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3);																						
	ADC0->SC3 |= ADC_SC3_CAL_MASK;					
	while(ADC0->SC3 & ADC_SC3_CAL_MASK);		
	
	if(ADC0->SC3 & ADC_SC3_CALF_MASK)
{
	  ADC0->SC3 |= ADC_SC3_CALF_MASK;
	  return(1);								
	}
	
	calib_temp = 0x00;
	calib_temp += ADC0->CLP0;
	calib_temp += ADC0->CLP1;
	calib_temp += ADC0->CLP2;
	calib_temp += ADC0->CLP3;
	calib_temp += ADC0->CLP4;
	calib_temp += ADC0->CLPS;
	calib_temp += ADC0->CLPD;
	calib_temp /= 2;
	calib_temp |= 0x8000;                      
	ADC0->PG = ADC_PG_PG(calib_temp);           
	ADC0->SC1[0] = ADC_SC1_ADCH(31);
	ADC0->CFG2 |= ADC_CFG2_ADHSC_MASK;
	ADC0->CFG1 = ADC_CFG1_ADICLK(ADICLK_BUS_2) | ADC_CFG1_ADIV(ADIV_1) | ADC_CFG1_ADLSMP_MASK | ADC_CFG1_MODE(MODE_12);	// Zegar ADCK równy 10.49MHz (10485760Hz), rozdzielczosc 12 bitów, dlugi czas próbkowania
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK;
	ADC0->SC1[0] = ADC_SC1_ADCH(2);
	ADC0->SC3 |= ADC_SC3_ADCO_MASK;						
	NVIC_ClearPendingIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC0_IRQn);
	
	return(0);
}

float GetSensorData(uint16_t channel)
{
	ADC0->SC1[0] = ADC_SC1_ADCH(channel);																					
	while(((ADC0->SC1[0] & ADC_SC1_COCO_MASK)>>ADC_SC1_COCO_SHIFT) == 0);		
	uint16_t adc_value = ADC0->R[0];
	float value = adc_value * volt_coeff;
	return value;
}
