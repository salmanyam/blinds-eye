/*
 * microsd.c
 *
 * Created: 24-11-11 পুর্বাহ্ন 02.12.29
 *  Author: Salman
 */ 


#define F_CPU 8000000UL		//freq 8 MHz

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "SPI_routines.h"
#include "SD_routines.h"
#include "FAT32.h"
#include "lcd.h"

#include <string.h>
#include <stdlib.h>

#define US_PORT PORTD
#define	US_PIN	PIND
#define US_DDR 	DDRD
#define US_POS	PD6


#define US_ERROR 0xffff
#define	US_NO_OBSTACLE 0xfffe

uint16_t getPulseWidth()
{
	uint32_t i,result;

	//Wait for the rising edge
	for(i=0;i<600000;i++)
	{
		if(!(US_PIN & (1<<US_POS))) continue; else break;
	}

	if(i==600000)
		return 0xffff;	//Indicates time out
	
	//High Edge Found

	//Setup Timer1
	TCCR1A=0X00;
	TCCR1B=(1<<CS11);	//Prescaler = Fcpu/8
	TCNT1=0x00;			//Init counter

	//Now wait for the falling edge
	for(i=0;i<600000;i++)
	{
		if(US_PIN & (1<<US_POS))
		{
			if(TCNT1 > 60000) break; else continue;
		}
		else
			break;
	}

	if(i==600000)
		return 0xffff;	//Indicates time out

	//Falling edge found

	result=TCNT1;

	//Stop Timer
	TCCR1B=0x00;

	if(result > 34800)
		return 0xfffe;	//No obstacle
	else
		return result;
}

void Wait()
{
	uint8_t i;
	for(i=0;i<10;i++)
		_delay_loop_2(0);
}

void port_init(void)
{
	PORTB = 0xEF;
	DDRB  = 0xBF; //MISO line i/p, rest o/p
	PORTC = 0x00;
	DDRC  = 0x00;
	PORTD = 0x00;
	DDRD  = 0xFE;
}


//call this routine to initialize all peripherals
void init_devices(void)
{
	 cli();  //all interrupts disabled
	 port_init();
	 spi_init();
	 LCDInit(LS_NONE);

	 MCUCR = 0x00;
	 GICR  = 0x00;
	 TIMSK = 0x00; //timer interrupt sources
	 //all peripherals are now initialized
}

int main()
{
	unsigned char memory,error,FAT32_active,c;
	unsigned char *name;
	int i,len=0,s;
	uint16_t r;
	
	char str[2];
	
	_delay_ms(1000);
	cli();
	init_devices();
	memory=SD_init();
	
	if(memory != 0)
	{
		LCDClear();
		LCDWriteString("Card not Found");
		_delay_ms(1000);
	}
	else
	{
		LCDClear();
		LCDWriteString("Mem Card Found!");
		_delay_ms(1000);
		
		SPI_HIGH_SPEED;	//SCK - 4 MHz
		_delay_ms(1);   //some delay
	
		FAT32_active = 1;
		error = getBootSectorData (); //read boot sector and keep necessary data in global variables
		if(error) 	
		{
			FAT32_active = 0;
			LCDClear();
			LCDWriteString("Fat not found!!");
			_delay_ms(1000);
	  
		}
		else
		{
		/*	
			name = "hellloo.wav";
			readFile(0,name);
			free(name);
			
			name = "hellloo.wav";
			readFile(0,name);
			free(name);
			
			name = "hellloo.wav";
			readFile(0,name);
			free(name);
			
			name = "3333333.wav";
			readFile(0,name);
			free(name);
			
			name = "4444444.wav";
			readFile(0,name);
			free(name);
			
			name = "5555555.wav";
			readFile(0,name);
			free(name);
			
			*/
		
			while(1)
			{
				
				//Set Ultra Sonic Port as out
				US_DDR|=(1<<US_POS);

				_delay_us(10);

				//Give the US pin a 15us High Pulse
				US_PORT|=(1<<US_POS);	//High

				_delay_us(15);

				US_PORT&=(~(1<<US_POS));//Low

				_delay_us(20);

				//Now make the pin input
				US_DDR&=(~(1<<US_POS));
	
				//Measure the width of pulse
				r=getPulseWidth();

				//Handle Errors
				if(r==US_ERROR)
				{
					LCDWriteStringXY(0,0,"Error !");
				}
			/*	else if(r==US_NO_OBSTACLE)
				{
					LCDWriteStringXY(0,0,"Clear !");
				}*/
				else
				{
		
					int d;

					d=(r/58);	//Convert to cm
					s = d%100;
					
					
					if(d >= 2 && d <=300)
					{
						LCDClear();
						LCDWriteIntXY(0,0,d,4);
						LCDWriteString(" cm");	
					}
					
					else if(d > 300)
					{
						LCDClear();
						
						LCDWriteString("Clear!!!");	
						_delay_ms(2000);
					}
					
					/*
					if()readFile(0,"10.wav");
					else if()readFile(0,"20.wav");
					else if()readFile(0,"30.wav");
					else if()readFile(0,"40.wav");
					else if()readFile(0,"50.wav");
					//Wait();
					*/
					DDRD |= ~(1<<PD5);
					if(PIND &= 00100000)
					{
						if(d >=3 && d < 100)
						{
							readFile(0,strcat(itoa(d,str,10),".wav"));
							readFile(0,"cm.wav");					
						}
						else if(d > 100 && d < 200)
						{	
							readFile(0,"100.wav");
							readFile(0,strcat(itoa(s,str,10),".wav"));
							readFile(0,"cm.wav");							
						}
						else if(d > 200 && d < 300)
						{	
							readFile(0,"200.wav");
							readFile(0,strcat(itoa(s,str,10),".wav"));
							readFile(0,"cm.wav");							
						}
						else if(d == 100)
						{
							readFile(0,"100.wav");
							readFile(0,"cm.wav");
						}
						else if(d == 200)
						{
							readFile(0,"200.wav");
							readFile(0,"cm.wav");
						}
						else if(d == 300)
						{
							readFile(0,"300.wav");
							readFile(0,"cm.wav");
						}
					}
					else _delay_ms(500);
					
					DDRD |= (1<<PD5);
					
					
				}

		
			}
		}
		
		
	}
	while(1)
	{
		LCDClear();
		LCDWriteString("Running!!!");
		_delay_ms(100);
	}
	
	return 0;
}
