#include "frdm_bsp.h"
#include "SharpSens.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tracks.h"
#include "MKL05Z4.h"
#include "uart0.h"


#define ENTER	0xa
#define BuffSize  16 

// Channels of ADC0 corresponding to particular GPIO (see SharpSens.c)
#define Sens1Channel 2
#define Sens2Channel 7
#define Sens3Channel 3
#define Sens4Channel 6
int ChannelArr[] = {Sens1Channel, Sens2Channel, Sens3Channel, Sens4Channel};

#define LEFT_TRACK_PWM_CHANNEL 5
#define RIGHT_TRACK_PWM_CHANNEL 0
#define RIGHT_TRACK_DIR 7
#define LEFT_TRACK_DIR 10


char rx_buf[BuffSize];
char angle_buf[BuffSize];


volatile  int angle = 1;
volatile  int spin_status = 0;
volatile uint8_t track_stop = 1;

char temp;
uint8_t rx_buf_pos=0;
uint8_t rx_ready=1;
uint8_t too_long=0;

uint16_t count_left = 0;
uint16_t count_right = 0;
uint16_t count_forward = 0;
uint16_t count_backward = 0;

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
	int i = 0;
	while(i < (sizeof(ChannelArr)/sizeof ChannelArr[0]))
	{		
			float ADC_temp = GetSensorData(ChannelArr[i]);
			sprintf(rx_buf,"%f", ADC_temp);
			mPrint(rx_buf);
			UART0->D = ';';
			angle = (angle)%90 + 90*i;
			sprintf(angle_buf,"%i", angle);
			mPrint(angle_buf);
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));
			UART0->D = ENTER;			
		i++;
	}
		while(!(UART0->S1 & UART0_S1_TDRE_MASK));
}

void SysTick_Handler(void)  
	{
	if(left_go_flag ==1)
		{
		if (count_left  == 10)
			{
				left_go_flag = 0;
				count_left = 0;
				TPM0->CONTROLS[RIGHT_TRACK_PWM_CHANNEL].CnV = 100;
				TPM0->CONTROLS[LEFT_TRACK_PWM_CHANNEL].CnV = 100;	
			}
		else 
			count_left++;
		}
	
	if(right_go_flag)
		{
		if (count_right  == 10)
			{
				right_go_flag = 0;
				count_right = 0;
				TPM0->CONTROLS[RIGHT_TRACK_PWM_CHANNEL].CnV = 100;
				TPM0->CONTROLS[LEFT_TRACK_PWM_CHANNEL].CnV = 100;	
			}
		else 
			count_right++;
		}
	
	if(forward_go_flag)
		{
		if (count_forward  == 10)
			{
				forward_go_flag = 0;
				count_forward = 0;
				TPM0->CONTROLS[RIGHT_TRACK_PWM_CHANNEL].CnV = 100; // 68 best speed
				TPM0->CONTROLS[LEFT_TRACK_PWM_CHANNEL].CnV = 100;	
			}
		else 
			count_forward++;	
		}
	
	if(backward_go_flag)
		{
		if (count_backward  == 10)
			{
				backward_go_flag = 0;
				count_backward = 0;
				TPM0->CONTROLS[RIGHT_TRACK_PWM_CHANNEL].CnV = 100; // 68 best speed
				TPM0->CONTROLS[LEFT_TRACK_PWM_CHANNEL].CnV = 100;	
			}
		else 
			count_backward++;			
		}
	
	if (!track_stop)		
	{		
		track_stop = 1;
		TPM0->CONTROLS[RIGHT_TRACK_PWM_CHANNEL].CnV = 100; // 68 best speed
		TPM0->CONTROLS[LEFT_TRACK_PWM_CHANNEL].CnV = 100;		// 68 best speed	
	
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
	angle += 1;
	}
	NVIC_EnableIRQ(SysTick_IRQn);
}

void UART0_IRQHandler()
{
	if(UART0->S1 & UART0_S1_RDRF_MASK)
	{
		temp=UART0->D;	// Odczyt wartosci z bufora odbiornika i skasowanie flagi RDRF
		if(!rx_ready)
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
				rx_ready=1;
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
	
	SysTick_Config(SystemCoreClock/39 );

	while(1)
	{
		if(rx_ready) //Is the data ready?
		{
			if(too_long)
			{
				mPrint("Too long\n");
				too_long=0;
			}
			else
			{
				if(strcmp(rx_buf,"run")==0)	// Zaswiec czerwona diode LED 
					spin_status = 1;
				else if (strcmp(rx_buf,"left")==0)
					go_left();
				else if (strcmp(rx_buf,"right")==0)
					go_right();				
				else if (strcmp(rx_buf,"forward")==0)
					go_forward();
				else if (strcmp(rx_buf,"backward")==0)
					go_backward();
			}
			rx_buf_pos=0;
			rx_ready=0;
		}
		
		if (spin_status == 1) 
		{
			if(track_stop == 1) 
			{
				SysTick_Config(SystemCoreClock/39);
				PTB->PSOR|=(1<<RIGHT_TRACK_DIR);
				PTB->PCOR|=(1<<LEFT_TRACK_DIR);
				TPM0->CONTROLS[RIGHT_TRACK_PWM_CHANNEL].CnV = 65;
				TPM0->CONTROLS[LEFT_TRACK_PWM_CHANNEL].CnV = 65;
				track_stop = 0;
			}
		}
	}
}
