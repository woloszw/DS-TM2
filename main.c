#include "frdm_bsp.h"
#include "SharpSens.h"
#include "UART.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tracks.h"

#define ENTER	0xa

float volt_coeff = ((float)(((float)2.91) / 4095) );			// Wspólczynnik korekcji wyniku, w stosunku do napiecia referencyjnego przetwornika
uint16_t sensOne, sensTwo, sensThree, sensFour;
float ADC_temp, ADC_temp2;
uint16_t counter = 1;

void ADC0_IRQHandler()
{	
	if (counter == 0)
	{
		
		//ADC0->SC1[0] &= ~(ADC_SC1_ADCH(6)); 
		
		sensOne = ADC0->R[0];
		ADC_temp =(float) sensOne;
	}
	else if (counter == 1)
	{
		
		//ADC0->SC1[0] &= ~(ADC_SC1_ADCH(2));

		sensTwo = ADC0->R[0];
		ADC_temp2 =(float) sensTwo;
	}
	
		//Uzywam countera by moc zczytywac raz z jednego raz z drugiego sensora
	counter += 1;
	counter = counter % 2;
	
	
	NVIC_EnableIRQ(ADC0_IRQn);
	if(counter ==0)
		ADC0->SC1[0] |= ADC_SC1_ADCH(0x02); 
	else if (counter == 1)
		ADC0->SC1[0] |= ADC_SC1_ADCH(0x06); 
}


int main (void)
{
	char rx_buf[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	char rx_buf2[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	char counter_string[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20}; 
	
	
	char calib_error[] = "Calibration error";
	char calib_correct[] = "Calibration succeeded";
		
	uint32_t ADC_calibration = ADC_Init();
	UART0_Init();

	tracks_init();
	
	
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
	
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK;
	
	while(1)
		
	
	
{		
			sprintf(counter_string,"Counter=%d", counter);
			//Printing counter value
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			UART0->D = ENTER;
			for(uint32_t i=0;counter_string[i]!=0;i++)
			{
				while(!(UART0->S1 & UART0_S1_TDRE_MASK));
				UART0->D = counter_string[i];
			}
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			UART0->D = ENTER;
	
	
	//DATA FROM 1ST SENSOR
			ADC_temp = ADC_temp * volt_coeff;
			sprintf(rx_buf,"V1=%f", ADC_temp);
		
			for(uint32_t i=0;rx_buf[i]!=0;i++)
			{
				while(!(UART0->S1 & UART0_S1_TDRE_MASK));
				UART0->D = rx_buf[i];
			}
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			UART0->D = ENTER;
			
			//DATA FROM 2ND SENSOR
			
			ADC_temp2 = ADC_temp2 * volt_coeff;
			sprintf(rx_buf2,"V2=%f", ADC_temp2);
		
			for(uint32_t i=0;rx_buf2[i]!=0;i++)
			{
				while(!(UART0->S1 & UART0_S1_TDRE_MASK));
				UART0->D = rx_buf2[i];
			}
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			UART0->D = ENTER;
			
			DELAY(500);	
	}
}




