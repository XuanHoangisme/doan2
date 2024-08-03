#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include inbuilt defined Delay header file */
#include <string.h>
#include <stdio.h>

#define LCD_Data_Dir DDRC		/* Define LCD data port direction */
#define LCD_Command_Dir DDRD		/* Define LCD command port direction register */
#define LCD_Data_Port PORTC		/* Define LCD data port */
#define LCD_Command_Port PORTD		/* Define LCD data port */
#define RS PD6		/* Define Register Select (data/command reg.)pin */
#define RW PD5			/* Define Read/Write signal pin */
#define EN PD7			/* Define Enable signal pin */

#define degree_sysmbol 0xdf

void LCD_Command(unsigned char cmnd)
{
	LCD_Data_Port= cmnd;
	LCD_Command_Port &= ~(1<<RS);	/* RS=0 command reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 Write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(3);
}

void LCD_Char (unsigned char char_data)	/* LCD data write function */
{
	LCD_Data_Port= char_data;
	LCD_Command_Port |= (1<<RS);	/* RS=1 Data reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable Pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(1);
}

void LCD_Init ()			/* LCD Initialize function */
{
	LCD_Command_Dir = 0xFF;		/* Make LCD command port direction as o/p */
	LCD_Data_Dir = 0xFF;		/* Make LCD data port direction as o/p */
	_delay_ms(20);			/* LCD Power ON delay always >15ms */
	
	LCD_Command (0x38);		/* Initialization of 16X2 LCD in 8bit mode */
	LCD_Command (0x0C);		/* Display ON Cursor OFF */
	LCD_Command (0x06);		/* Auto Increment cursor */
	LCD_Command (0x01);		/* Clear display */
	LCD_Command (0x80);		/* Cursor at home position */
}

void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void ADC_Init(){
	DDRA = 0x00;	        /* Make ADC port as input */
	ADCSRA = 0x87;          /* Enable ADC, with freq/128  */
	ADMUX = 0x40;           /* Vref: Avcc, ADC channel: 0 */
}

int ADC_Read(char channel)
{
	ADMUX = 0x40 | (channel);   /* set input channel to read */
	ADCSRA |= (1<<ADSC);               /* Start ADC conversion */
	while (!(ADCSRA & (1<<ADIF)));     /* Wait until end of conversion by polling ADC interrupt flag */
	ADCSRA |= (1<<ADIF);               /* Clear interrupt flag */
	_delay_ms(1);                      /* Wait a little bit */
	return ADCW;                       /* Return ADC word */
}

void LCD_String_xy (char row, char pos, char *str)/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);		/* Call LCD string function */
}



int main()
{
	char Temperature[10];
	//int celsius;

	LCD_Init();                 /* initialize 16x2 LCD*/
	ADC_Init();                 /* initialize ADC*/
	
	while(1)
	{
		LCD_String_xy(1,0,"Temperature");
		
		int i = 0;
		float temps = 0;
		//bool kt = 0;
		do {
			
			//Serial.println(reading);
			float temp = (ADC_Read(1) * 5.0 / 10.240);
			
			//float temp = voltage * 100.0;
			if(temp != 0) {
				temps += temp;
				i += 1;
				_delay_ms(10);
			}
		} while (i < 100);
		
		temps = temps/i;
		
		//celsius = (5*ADC_Read(1)/10.23);
		//celsius = (celsius*1000);
		sprintf(Temperature,"%d%cC  ", (int)temps, degree_sysmbol);/* convert integer value to ASCII string */
		LCD_String_xy(2,0,Temperature);/* send string data for printing */
		//_delay_ms(500);
		memset(Temperature,0,10);
		
	}
}
