#include "frdm_bsp.h"
#include "SharpSens.h"
#include "UART.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tracks.h"

#define ENTER	0xa

#define Sens1Channel 2
#define Sens2Channel 3
#define Sens3Channel 4
#define Sens4Channel 6

int ChannelArr[] = {Sens1Channel, Sens4Channel};

void ADC0_IRQHandler()
{	
	NVIC_EnableIRQ(ADC0_IRQn);
}

char rx_buf[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};

void PrintSensorData()
{
	int i = 0;
	while(i < (sizeof(ChannelArr)/sizeof ChannelArr[0]))
	{		
			float ADC_temp = GetSensorData(ChannelArr[i]);
			sprintf(rx_buf,"V=%f", ADC_temp);
			for(uint32_t j=0;rx_buf[j]!=0;j++)
			{
				while(!(UART0->S1 & UART0_S1_TDRE_MASK));
				UART0->D = rx_buf[j];
			}
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			UART0->D = ENTER;
			
		i++;
	}
		while(!(UART0->S1 & UART0_S1_TDRE_MASK));
		UART0->D = ENTER;	
}

int main (void)
{ 
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
	
	
	
	while(1)
{	
		PrintSensorData();
		DELAY(500);	
	}
}




