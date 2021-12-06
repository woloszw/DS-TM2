#include "frdm_bsp.h"
#include "SharpSens.h"
#include "UART.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ENTER	0xa

float volt_coeff = ((float)(((float)2.91) / 4095) );			// Wspólczynnik korekcji wyniku, w stosunku do napiecia referencyjnego przetwornika
uint16_t ADC_temp;
float wynik;

void ADC0_IRQHandler()
{	
	ADC_temp = ADC0->R[0];	// Odczyt danej i skasowanie flagi COCO

	wynik = ADC_temp;			// Wyslij nowa dana do petli glównej

	NVIC_EnableIRQ(ADC0_IRQn);
}

int main (void)
{
	uint8_t temp=0;
	char rx_buf[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	char calib_error[] = "Calibration error";
	char calib_correct[] = "Calibration succeeded";
		
	uint32_t ADC_calibration = ADC_Init();
	UART0_Init();

	if (ADC_calibration) //Testing whether the calibration has worked properly
	{
		for(uint32_t i=0;calib_error[i]!=0;i++)
				{
					while(!(UART0->S1 & UART0_S1_TDRE_MASK));
					UART0->D = calib_error[i];
				}
		while(!(UART0->S1 & UART0_S1_TDRE_MASK));
		UART0->D = ENTER;
		while(1);
	}
	else
	{
		for(uint32_t i=0;calib_correct[i]!=0;i++)
				{
					while(!(UART0->S1 & UART0_S1_TDRE_MASK));
					UART0->D = calib_correct[i];
				}
		while(!(UART0->S1 & UART0_S1_TDRE_MASK));
		UART0->D = ENTER;
	}
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(0);
	
	while(1)
	{
			//wynik = wynik * volt_coeff;
			sprintf(rx_buf,"N=%d%c", wynik);
			for(uint32_t i=0;rx_buf[i]!=0;i++)
			{
				while(!(UART0->S1 & UART0_S1_TDRE_MASK));
				UART0->D = rx_buf[i];
			}
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			UART0->D = ENTER;
			DELAY(500)	
	}
}

