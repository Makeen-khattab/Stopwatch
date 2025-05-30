/* ===================================================================================================
   File Name: Main_File.c
   Author: Makeen Khattab
   Description: Mini Project 2
   Data: 12/29/2023
 ===================================================================================================*/

/*=========================================================================
=============================  Includes   =================================
===========================================================================*/
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
/*=========================================================================
=============================  Common Macros   ============================
===========================================================================*/
#define FIRST_FOUR_PINS_OF_PORTC_OUTPUT			   0x0F
#define FIRST_SIX_PINS_OF_PORTA_OUTPUT             0x3F
#define SEC2_ENABLE                                PA0
#define SEC1_ENABLE                                PA1
#define MIN2_ENABLE                                PA2
#define MIN1_ENABLE                                PA3
#define HOUR2_ENABLE                               PA4
#define HOUR1_ENABLE                               PA5
#define RESET_BUTTON						  	   PD2
#define PAUSE_BUTTON                               PD3
#define RESUME_BUTTON                              PB2
#define MAX_SEC2									9
#define MAX_SEC1									5
#define MAX_MIN2									9
#define MAX_MIN1									5
#define MAX_HOUR2									9
#define SEC_MAX_HOUR2                               4
#define MAX_HOUR1									2
#define START_OF_SEC2								0
#define START_OF_SEC1								0
#define START_OF_MIN2								0
#define START_OF_MIN1								0
#define START_OF_HOUR2								0
#define START_OF_HOUR1								0
#define RESUME_CLOCK								1
#define PAUSE_CLOCK									0
/*=========================================================================
===========================  Global Variables  ============================
===========================================================================*/
typedef struct
{
	unsigned char Sec2;/*Range	0--->9*/
	unsigned char Sec1;/*Range	0--->5*/
	unsigned char Min2;/*Range	0--->9*/
	unsigned char Min1;/*Range	0--->5*/
	unsigned char Hour2;/*Range	0--->9*/
	unsigned char Hour1;/*Range	0--->2*/

}Digital_Clock;

Digital_Clock My_Clock;
unsigned char State_Of_Clock;

/*=========================================================================
=========================  Function Prototype   ===========================
===========================================================================*/
void Digital_Clock_Init(Digital_Clock*);
void Gpio_Init(void);
void Int0_Init(void);
void Int1_Init(void);
void Int2_Init(void);
void Timer1_Ctc_Init(void);
void Increment_Digital_Clock(Digital_Clock*);
void Reset_Clock(void);
void Display_Sec2(void);
void Display_Sec1(void);
void Display_Min2(void);
void Display_Min1(void);
void Display_Hour2(void);
void Display_Hour1(void);
void Pause__Digital_Clock(void);
void Resume__Digital_Clock(void);
void(*Call_Back_Ptr_To_Int0)(void);
void(*Call_Back_Ptr_To_Int1)(void);
void(*Call_Back_Ptr_To_Int2)(void);
void(*Call_Back_Ptr_To_Timer1Comp)(Digital_Clock*);
/*=========================================================================
=============================  Main File   ================================
===========================================================================*/
int main(void)
{
/*=========================================================================
 ============================  Application initialization   ===============
===========================================================================*/
	SREG|=1<<7; /* Activating the Global Interrupt*/
    Digital_Clock*Ptr2_My_Clock= &My_Clock;
	State_Of_Clock=RESUME_CLOCK;
	Gpio_Init();
    Int0_Init();
    Int1_Init();
	Int2_Init();
	Timer1_Ctc_Init();
	Digital_Clock_Init(Ptr2_My_Clock);
	Call_Back_Ptr_To_Int0=&Reset_Clock;
	Call_Back_Ptr_To_Int1=&Pause__Digital_Clock;
	Call_Back_Ptr_To_Int2=&Resume__Digital_Clock;
	Call_Back_Ptr_To_Timer1Comp=&Increment_Digital_Clock;

/*=========================================================================
============================== Super Loop   ===============================
===========================================================================*/
while(1)
{
	Display_Sec2();
	_delay_ms(30);
	Display_Sec1();
	_delay_ms(30);
	Display_Min2();
	_delay_ms(30);
	Display_Min1();
	_delay_ms(30);
	Display_Hour2();
	_delay_ms(30);
	Display_Hour1();
	_delay_ms(30);

}

}
/*=========================================================================
=========================  Function Definition   ==========================
===========================================================================*/
void Digital_Clock_Init(Digital_Clock*Ptr2_My_Clock)
{
	Ptr2_My_Clock->Sec2=START_OF_SEC2;
	Ptr2_My_Clock->Sec1=START_OF_SEC1;
	Ptr2_My_Clock->Min2=START_OF_MIN2;
	Ptr2_My_Clock->Min1=START_OF_MIN1;
	Ptr2_My_Clock->Hour2=START_OF_HOUR2;
	Ptr2_My_Clock->Hour1=START_OF_HOUR1;
}
void Gpio_Init(void)
{
	DDRC|=FIRST_FOUR_PINS_OF_PORTC_OUTPUT;
	DDRA|=FIRST_SIX_PINS_OF_PORTA_OUTPUT;
}

void Int0_Init(void)
{
	DDRD&=~(1<<RESET_BUTTON);/* Setting the Direction of the Reset Button as an input*/
	PORTD|=(1<<RESET_BUTTON);/* Activating the internal Pull up resistor*/
	GICR|=1<<INT0;           /*Enabling Interrupt 0*/
	MCUCR=(MCUCR&(0xFC))| (0x02);/* Activating the Falling edge interrupt request*/


}
void Int1_Init(void)
{
	DDRD&=~(1<<PAUSE_BUTTON);/* Setting the Direction of the Pause Button as an input*/
	GICR|=1<<INT1;          /*Enabling Interrupt 1*/
	MCUCR=(MCUCR&(0xF3))| (0x0C);/* Activating the Raising edge interrupt request*/
}
void Int2_Init(void)
{
	DDRB&=~(1<<RESUME_BUTTON );/* Setting the Direction of the Resume Button as an input*/
	PORTB|=(1<<RESUME_BUTTON); /* Activating the internal Pull up resistor*/
	GICR|=1<<INT2;
	MCUCSR&=~(1<<ISC2);
}
void Timer1_Ctc_Init(void)
{
	TCCR1A&= (~(1<<COM1A1)) & (~(1<<COM1A0)); /*Disconnecting OC1A*/
	TCCR1A= (1<<FOC1A) | (1<<FOC1B); /* We set those pins in any mode except pwm*/

											/* Activating the CTC Mode with OCR1 as a compare register */
	/*TCCR1B =(1<<WGM12)|(1<<CS12)|(1<<CS10);*//*Putting a prescaler with 1024
								Because as we decrease the frequency
								we increase the period taken by each Count*/
	TCNT1=0;  /*Initial Value of the Counter*/
	OCR1A=488; /* Compare value to Have an Interrupt Each 1 Sec
				F_CPU=1MHZ
				Fclk = F_CPU/Prescaler*2 = 488.28125
				Tclk= 2.048 msec
				Number of counts = 1 / Tclk = 488.28125 Approx 488 */

	TIMSK|=1<<OCIE1A;/*Activating Timer1 CTC interrupt*/

}
void Increment_Digital_Clock(Digital_Clock*Ptr2_My_Clock)
{   if(State_Of_Clock==RESUME_CLOCK)
{
	if((Ptr2_My_Clock->Sec2) < MAX_SEC2)
	{
		(Ptr2_My_Clock->Sec2)++;
	}
	else if(((Ptr2_My_Clock->Sec2)== MAX_SEC2) && ((Ptr2_My_Clock->Sec1) <  MAX_SEC1 ))
	{
		(Ptr2_My_Clock->Sec2)=START_OF_SEC2;
		(Ptr2_My_Clock->Sec1)++;

	}
	else if(((Ptr2_My_Clock->Sec2)== MAX_SEC2) && ((Ptr2_My_Clock->Sec1) == MAX_SEC1 ) && ((Ptr2_My_Clock->Min2)< MAX_MIN2 ))
	{
		(Ptr2_My_Clock->Sec2)=START_OF_SEC2;
		(Ptr2_My_Clock->Sec1)=START_OF_SEC1;
		(Ptr2_My_Clock->Min2)++;

	}
	else if(((Ptr2_My_Clock->Sec2)== MAX_SEC2) && ((Ptr2_My_Clock->Sec1) == MAX_SEC1 ) && ((Ptr2_My_Clock->Min2)==MAX_MIN2 ) && ((Ptr2_My_Clock->Min1)<MAX_MIN1 ))
	{
		(Ptr2_My_Clock->Sec2)=START_OF_SEC2;
		(Ptr2_My_Clock->Sec1)=START_OF_SEC1;
		(Ptr2_My_Clock->Min2)=START_OF_MIN2;
		(Ptr2_My_Clock->Min1)++;

	}
	else if(((Ptr2_My_Clock->Sec2)== MAX_SEC2) && ((Ptr2_My_Clock->Sec1) == MAX_SEC1 ) && ((Ptr2_My_Clock->Min2)==MAX_MIN2 ) && ((Ptr2_My_Clock->Min1)==MAX_MIN1 )&& ((Ptr2_My_Clock->Hour2)<MAX_HOUR2 ))
	{
		(Ptr2_My_Clock->Sec2)=START_OF_SEC2;
		(Ptr2_My_Clock->Sec1)=START_OF_SEC1;
		(Ptr2_My_Clock->Min2)=START_OF_MIN2;
		(Ptr2_My_Clock->Min1)=START_OF_MIN1;
		(Ptr2_My_Clock->Hour2)++;

	}
	else if (((Ptr2_My_Clock->Sec2)== MAX_SEC2) && ((Ptr2_My_Clock->Sec1) == MAX_SEC1 ) && ((Ptr2_My_Clock->Min2)==MAX_MIN2 ) && ((Ptr2_My_Clock->Min1)==MAX_MIN1 ) && ((Ptr2_My_Clock->Hour2)==MAX_HOUR2 )&& ((Ptr2_My_Clock->Hour1)<MAX_HOUR1 ))
	{
		(Ptr2_My_Clock->Sec2)=START_OF_SEC2;
		(Ptr2_My_Clock->Sec1)=START_OF_SEC1;
		(Ptr2_My_Clock->Min2)=START_OF_MIN2;
		(Ptr2_My_Clock->Min1)=START_OF_MIN1;
		(Ptr2_My_Clock->Hour2)=START_OF_HOUR2;
		(Ptr2_My_Clock->Hour1)++;
	}
	else if(((Ptr2_My_Clock->Sec2)== MAX_SEC2) && ((Ptr2_My_Clock->Sec1) == MAX_SEC1 ) && ((Ptr2_My_Clock->Min2)==MAX_MIN2 ) && ((Ptr2_My_Clock->Min1)==MAX_MIN1 ) && ((Ptr2_My_Clock->Hour2)==SEC_MAX_HOUR2 )&& ((Ptr2_My_Clock->Hour1)==MAX_HOUR1 ))
	{
		(Ptr2_My_Clock->Sec2)=START_OF_SEC2;
		(Ptr2_My_Clock->Sec1)=START_OF_SEC1;
		(Ptr2_My_Clock->Min2)=START_OF_MIN2;
		(Ptr2_My_Clock->Min1)=START_OF_MIN1;
		(Ptr2_My_Clock->Hour1)=START_OF_HOUR1;
		(Ptr2_My_Clock->Hour2)=START_OF_HOUR2;

	}
	else
		;
}
else
	;

}
void Reset_Clock(void)
{
	    (My_Clock.Sec2)=START_OF_SEC2;
		(My_Clock.Sec1)=START_OF_SEC1;
		(My_Clock.Min2)=START_OF_MIN2;
		(My_Clock.Min1)=START_OF_MIN1;
		(My_Clock.Hour1)=START_OF_HOUR1;
		(My_Clock.Hour2)=START_OF_HOUR2;

}
void Display_Sec2(void)
{
	PORTA=(PORTA&(0x01))|(1<<SEC2_ENABLE);
	PORTC =((PORTC)&(0xF0)) | ((My_Clock.Sec2) & (0x0F));
}
void Display_Sec1(void)
{
	PORTA=(PORTA&(0xC2))|(1<<SEC1_ENABLE);
	PORTC =((PORTC)&(0xF0)) | ((My_Clock.Sec1) & (0x0F));
}
void Display_Min2(void)
{
	PORTA=(PORTA&(0xC4))|(1<<MIN2_ENABLE);
	PORTC =((PORTC)&(0xF0)) | ((My_Clock.Min2) & (0x0F));
}
void Display_Min1(void)
{
	PORTA=(PORTA&(0xC8))|(1<<MIN1_ENABLE);
	PORTC =((PORTC)&(0xF0)) | ((My_Clock.Min1) & (0x0F));
}
void Display_Hour2(void)
{
	PORTA=(PORTA&(0xD0))|(1<<HOUR2_ENABLE);
	PORTC =((PORTC)&(0xF0)) | ((My_Clock.Hour2) & (0x0F));
}
void Display_Hour1(void)
{
	PORTA=(PORTA&(0xE0))|(1<<HOUR1_ENABLE);
	PORTC =((PORTC)&(0xF0)) | ((My_Clock.Hour1) & (0x0F));
}
void Pause__Digital_Clock(void)
{
	State_Of_Clock = PAUSE_CLOCK;
}
void Resume__Digital_Clock(void)
{
	State_Of_Clock = RESUME_CLOCK;
}
ISR(INT0_vect)
{
	(*Call_Back_Ptr_To_Int0)();
}
ISR(INT1_vect)
{
	(*Call_Back_Ptr_To_Int1)();
}
ISR(INT2_vect)
{
	(*Call_Back_Ptr_To_Int2)();
}
ISR(TIMER1_COMPA_vect)
{
	(*Call_Back_Ptr_To_Timer1Comp)(&My_Clock);

}
