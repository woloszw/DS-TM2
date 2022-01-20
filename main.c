#include "frdm_bsp.h"
#include "SharpSens.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tracks.h"
#include "MKL05Z4.h"
#include "uart0.h"

#include "led.h"



#define ENTER	0xa


#define Sens1Channel 2
#define Sens2Channel 7
#define Sens3Channel 3
#define Sens4Channel 6
int ChannelArr[] = {Sens1Channel, Sens2Channel, Sens3Channel, Sens4Channel};

#define LEFT_TRACK_PWM_CHANEL 5
#define RIGHT_TRACK_PWM_CHANEL 0

volatile uint8_t track_stop = 1;

#define BuffSize  16
char rx_buf[BuffSize];
char angle_buf[BuffSize];

volatile  int angle = 3;
volatile  int spin_status = 1;

uint8_t rx_buf_pos=0;
char temp, buf;
uint8_t rx_FULL=1;
uint8_t too_long=0;

char LED_R_ON[]="LRON";
char LED_R_OFF[]="LROFF";
char Error[]="Zla komenda";
char Too_Long[]="Zbyt dlugi ciag";
char Correct[] = "Wysylanie danych";

void mPrint(char array[])
{
	while(!(UART0->S1 & UART0_S1_TDRE_MASK));
	for(uint32_t m=0;array[m]!=0;m++)
		{
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			UART0->D = array[m];
		}
	while(!(UART0->S1 & UART0_S1_TDRE_MASK));
}	

void PrintSensorData(int angle)
{
	//Ta funckja wyswietla dane potrzebne do wyswietlenia w Matlabie
	int i = 0;
	while(i < (sizeof(ChannelArr)/sizeof ChannelArr[0]))
	{		
			float ADC_temp = GetSensorData(ChannelArr[i]);
			sprintf(rx_buf,"%f", ADC_temp);
			for(uint32_t j=0;rx_buf[j]!=0;j++)
			{
				while(!(UART0->S1 & UART0_S1_TDRE_MASK));
				UART0->D = rx_buf[j];
			}
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			UART0->D = ';';
			
			angle = (angle)%90 + 90*i;
			sprintf(angle_buf,"%i", angle);
			
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			for(uint32_t m=0;angle_buf[m]!=0;m++)
			{
				while(!(UART0->S1 & UART0_S1_TDRE_MASK));
				UART0->D = angle_buf[m];
			}
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			UART0->D = ENTER;			
		i++;
	}
		//DO WYRZUCENIA POD KONIEC
		while(!(UART0->S1 & UART0_S1_TDRE_MASK));
		//UART0->D = ENTER;	
}

void SysTick_Handler(void)  {
  
	if (track_stop == 0)		
	{		
	track_stop = 1;
	TPM0->CONTROLS[RIGHT_TRACK_PWM_CHANEL].CnV = 100; // 70 best speed
	TPM0->CONTROLS[LEFT_TRACK_PWM_CHANEL].CnV = 100;		// 70 best speed	
	
		if(angle == 3)
		{
			mPrint("start");
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			UART0->D = ENTER;
		}
		PrintSensorData(angle);
	
	if (angle == 90)
	{
		spin_status = 0;
		angle = 0;
		mPrint("stop\n");
		}
	else
	angle += 3;
	}
	NVIC_EnableIRQ(SysTick_IRQn);
}

void UART0_IRQHandler()
{
	if(UART0->S1 & UART0_S1_RDRF_MASK)
	{
		temp=UART0->D;	// Odczyt wartosci z bufora odbiornika i skasowanie flagi RDRF
		if(!rx_FULL)
		{
			if(temp!=ENTER)
			{
				if(!too_long)	// Jesli za dlugi ciag, ignoruj reszte znaków
				{
					rx_buf[rx_buf_pos] = temp;	// Kompletuj komende
					rx_buf_pos++;
					if(rx_buf_pos==16)
						too_long=1;		// Za dlugi ciag
				}
			}
			else
			{
				if(!too_long)	// Jesli za dlugi ciag, porzuc tablice
					rx_buf[rx_buf_pos] = 0;
				rx_FULL=1;
			}
		}
	NVIC_EnableIRQ(UART0_IRQn);
	}
}


int main (void)
{ 
	uint32_t ADC_calibration = ADC_Init();
	UART0_Init();
	tracks_init();
		
	if (ADC_calibration) //Testing whether the calibration has worked properly
		mPrint("Calibration Error\n");
	else
		mPrint("Calibration succeeded\n");
	

	while(1)
	{
		if(rx_FULL)		// Czy dana gotowa?
		{
			if(too_long)
			{
				mPrint("Too long\n");
				too_long=0;
			}
			else
			{
				if(strcmp(rx_buf,"RUN")==0)	// Zaswiec czerwona diode LED 
				{

					spin_status = 1;
				}
//       TUTAJ SIE DA TEZ STRINGI ODPOWIEDZIALNE ZA JAZDE ROBOTEM
//				else if (rx_buf, "FORWARD")
//				{
//					if(strcmp (rx_buf,"STOP")==0)
//					{
//						mPrint("Red Led off\n");
//					}
//						else
//					{
//						mPrint("Nothing will happen\n");
//					}
//				}
				}
			rx_buf_pos=0;
			rx_FULL=0;	// Dana skonsumowana
		}
		
		if (spin_status == 1)
		{
			if(track_stop == 1) 
			{
				SysTick_Config(SystemCoreClock/40 );
				TPM0->CONTROLS[RIGHT_TRACK_PWM_CHANEL].CnV = 70; // 70 best speed
				TPM0->CONTROLS[LEFT_TRACK_PWM_CHANEL].CnV = 70;
				track_stop = 0;
			}
		}
	}
}
