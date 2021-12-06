#include "UART.h"

void UART0_Init(void)
{
	SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;							//UART0 dolaczony do zegara
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;							//Port B dolaczony do zegara
	SIM->SOPT2 |= SIM_SOPT2_UART0SRC(MCGFLLCLK);		//Zegar MCGFLLCLK=41943040Hz (CLOCK_SETUP=0)
	PORTB->PCR[1] = PORT_PCR_MUX(2);								//PTB1=TX_D
	PORTB->PCR[2] = PORT_PCR_MUX(2);								//PTB2=RX_D
	
	UART0->C2 &= ~(UART0_C2_TE_MASK | UART0_C2_RE_MASK );		//Blokada nadajnika i o dbiornika
	UART0->BDH = 0;
	UART0->BDL =91;		//Dla CLOCK_SETUP=0 BR=28800	BDL=91	:	CLOCK_SETUP=1	BR=230400		BDL=13
	UART0->C4 &=~ UART0_C4_OSR_MASK;
	UART0->C4 |= UART0_C4_OSR(15);	//Dla CLOCK_SETUP=0 BR=28800	OSR=15	:	CLOCK_SETUP=1	BR=230400		OSR=15
	UART0->C5 |= UART0_C5_BOTHEDGE_MASK;	//Pr�bkowanie odbiornika na obydwu zboczach zegara
	UART0->C2 |= UART0_C2_TE_MASK;		//Wlacz nadajnik
}

