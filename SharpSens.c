#include "SharpSens.h"
float volt_coeff = ((float)(((float)2.91) / 4095) );			// Wsp�lczynnik korekcji wyniku, w stosunku do napiecia referencyjnego przetwornika

uint8_t ADC_Init(void)
{
	uint16_t calib_temp;
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;          // Dolaczenie sygnalu zegara do ADC0
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK;					// Dolaczenie syganlu zegara do portu A
	
	PORTA->PCR[9] &=~ (PORT_PCR_MUX(0));				//PTA9 - wejscie analogowe, kanal 2
	PORTB->PCR[0] &=~ (PORT_PCR_MUX(0));				//PTB0 - wejscie analogowe, kanal 6
	PORTA->PCR[8] &=~ (PORT_PCR_MUX(0));				//PTA8 - wejscie analogowe, kanal 3
	PORTB->PCR[2] &=~ (PORT_PCR_MUX(0));				//PTB0 - wejscie analogowe, kanal 4
	
	

	ADC0->CFG1 = ADC_CFG1_ADICLK(ADICLK_BUS_2) | ADC_CFG1_ADIV(ADIV_4) | ADC_CFG1_ADLSMP_MASK;	// Zegar ADCK r�wny 2.62MHz (2621440Hz)
	ADC0->CFG2 = ADC_CFG2_ADHSC_MASK;																														// Wlacz wspomaganie zegara o duzej czestotliwosci
	ADC0->SC3  = ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3);																						// Wlacz usrednianie na 32
	
	ADC0->SC3 |= ADC_SC3_CAL_MASK;					// Rozpoczecie kalibracji
	while(ADC0->SC3 & ADC_SC3_CAL_MASK);		// Czekaj na koniec kalibracji
	
	if(ADC0->SC3 & ADC_SC3_CALF_MASK)
{
	  ADC0->SC3 |= ADC_SC3_CALF_MASK;
	  return(1);								// Wr�c, jesli blad kalibracji
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
	calib_temp |= 0x8000;                       // Ustaw najbardziej zanaczacy bit na 1
	ADC0->PG = ADC_PG_PG(calib_temp);           // Zapisz w  "plus-side gain calibration register"
	//ADC0->OFS = 0;														// Klaibracja przesuniecia zera (z pomiaru swojego punktu odniesienia - masy)
	ADC0->SC1[0] = ADC_SC1_ADCH(31);
	ADC0->CFG2 |= ADC_CFG2_ADHSC_MASK;
	ADC0->CFG1 = ADC_CFG1_ADICLK(ADICLK_BUS_2) | ADC_CFG1_ADIV(ADIV_1) | ADC_CFG1_ADLSMP_MASK | ADC_CFG1_MODE(MODE_12);	// Zegar ADCK r�wny 10.49MHz (10485760Hz), rozdzielczosc 12 bit�w, dlugi czas pr�bkowania
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK;
	ADC0->SC1[0] = ADC_SC1_ADCH(2);
	ADC0->SC3 |= ADC_SC3_ADCO_MASK;						//Przetwarzanie ciagle
	//ADC0->SC2 |= ADC_SC2_ADTRG_MASK;						// Wlaczenie wyzwalania sprzetowego
	//SIM->SOPT7 |= SIM_SOPT7_ADC0ALTTRGEN_MASK | SIM_SOPT7_ADC0TRGSEL(4);		// Wyzwalanie ADC0 przez PIT0
	
	NVIC_ClearPendingIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC0_IRQn);
	
	return(0);
}

float GetSensorData(uint16_t channel)
{
	ADC0->SC1[0] = ADC_SC1_ADCH(channel);																					// Zmiana kanalu na ADC_SE0
	while(((ADC0->SC1[0] & ADC_SC1_COCO_MASK)>>ADC_SC1_COCO_SHIFT) == 0);		// Czekaj na odczyt
	uint16_t adc_value = ADC0->R[0];	// Odczyt danej i skasowanie flagi COCO
	float value = adc_value * volt_coeff;
	return value;
}
