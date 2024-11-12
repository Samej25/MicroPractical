/*  -----------   on-chip ADC Programming ----------------- 
The analog voltage is applied to analog pin AN0(PORTA - RA0)
The resultant digital data in Voltage Form is put on LCD  
---------------------------------------------------------*/
/* Interface Details
 * Analog Trimpot       - RA0 - AN0
 * LCD Data (D0 to D7)  - PORTD (RD0 to RD7)
 * LCD RS               - RE0 
 * LCD RW               - RE1
 * LCD EN               - RE2
---------------------------------------------------------*/

#include <xc.h>

//Configuration bit setting//
#pragma config OSC = HS //Oscillator Selection
#pragma config WDT = OFF //Disable Watchdog timer
#pragma config LVP = OFF //Disable Low Voltage Programming
#pragma config PBADEN = OFF //Disable PORTB Analog inputs


//Declarations for LCD Connection
#define LCD_DATA    PORTD               //LCD data port
#define en          PORTEbits.RE2      // enable signal
#define rw          PORTEbits.RE1      // read/write signal
#define rs          PORTEbits.RE0     // register select signal

//Function Prototypes
void ADC_Init(void);					//Function to initialize the ADC
unsigned int Get_ADC_Result(void);		//Function to Get ADC result after conversion
void Start_Conversion(void);			//Function to Start of Conversion
void msdelay (unsigned int time);		//Function to generate delay
void init_LCD(void);					//Function to initialise the LCD		
void LCD_command(unsigned char cmd);	//Function to pass command to the LCD
void LCD_data(unsigned char data);		//Function to write character to the LCD
void LCD_write_string(char *str);//Function to write string to the LCD

//Start of main program
void main()
{ 
 char msg1[] = "On-chip ADC Prog. ";
 char msg2[] = "ADC VOLT:";
 unsigned char i, Thousands,Hundreds,Tens,Ones;
 unsigned int adc_val;
 unsigned long Voltage;
 ADCON1 = 0x0F;        //Configuring the PORTE pins as digital I/O 
 TRISD = 0x00;         //Configuring PORTD as output
 TRISE = 0x00;         //Configuring PORTE as output
	
 ADC_Init();				// Init ADC peripheral
 init_LCD();				// Init LCD Module
 LCD_write_string(msg1);	// Display Welcome Message
 LCD_command (0xC0);			// Goto second line, 0th place of LCD
 LCD_write_string(msg2);	// Display Message "ADC O/P"

 while(1)
 {
  	Start_Conversion();			//Trigger conversion
  	adc_val= Get_ADC_Result();//Get the ADC output by polling GO bit
    
	Voltage = (long) adc_val*0x01E9; //Convert Binary result into Voltage 
  	adc_val = Voltage /0x0064;		//e.g. ADC OP 0x03FF (1023) means 5V (5000)
									// Therefore Formulae used
									// ADC VOLT = (ADC Result * 489)/100
	LCD_command (0xC9);			//Goto 9th place on second line of LCD
  	
	i = adc_val/1000 ;		//Get the thousands place 
  	Thousands = i + 0x30;		// Convert it to ASCII
  	LCD_data (Thousands);	// Display thousands place
  	LCD_data ('.');	// Display decimal point
	i = (adc_val%1000)/100;	//Get the Hundreds place
  	Hundreds = i + 0x30;		// Convert it to ASCII
  	LCD_data (Hundreds);		//Display Hundreds place
  
	i = ((adc_val%1000)%100)/10; //Get the Tens place
  	Tens = i + 0x30;			// Convert it to ASCII
  	LCD_data (Tens);			//Display Tens place
  	
	i = adc_val%10 ;			//Get the Ones place
  	Ones = i + 30; 			// Convert it to ASCII
  	LCD_data (i + 0x30);		//Display Ones place
 
  	msdelay(300);		//Delay between conversions. It is a library function,refer delays.h file in MCC18 installation directory
 }
}


//Function Definitions
void ADC_Init()
{
 ADCON0=0b00000000;	//A/D Module is OFF and Channel 1 is selected
 ADCON1=0b00001110;	// Reference as VDD & VSS, AN0 set as analog pins
 ADCON2=0b10001110; // Result is right Justified
					//Acquisition Time 2TAD
					//ADC Clk FOSC/64
 ADCON0bits.ADON=1; //Turn ON ADC module
}

void Start_Conversion()
{
 ADCON0bits.GO=1;
}

//If you do not wish to use adc conversion interrupt you can use this
//to do conversion manually. It assumes conversion format is right adjusted
unsigned int Get_ADC_Result()
{
 unsigned int ADC_Result=0;
 while(ADCON0bits.GO);
 ADC_Result=ADRESL;
 ADC_Result|=((unsigned int)ADRESH) << 8;
 return ADC_Result;
} 

void msdelay (unsigned int time) //Function to generate delay
{
unsigned int i, j;
  for (i = 0; i < time; i++)
	for (j = 0; j < 710; j++);//Calibrated for a 1 ms delay in MPLAB
}


void init_LCD(void)		// Function to initialise the LCD
{
    LCD_command(0x38);      // initialization of 16X2 LCD in 8bit mode
    msdelay(15);
    LCD_command(0x01);      // clear LCD
    msdelay(15);
    LCD_command(0x0C);      // cursor off
    msdelay(15);
    LCD_command(0x80);      // go to first line and 0th position
    msdelay(15);
}

void LCD_command(unsigned char cmd) //Function to pass command to the LCD
{
    LCD_DATA = cmd;		//Send data on LCD data bus
    rs = 0;				//RS = 0 since command to LCD
    rw = 0;				//RW = 0 since writing to LCD
    en = 1;				//Generate High to low pulse on EN
    msdelay(15);
    en = 0;
}

void LCD_data(unsigned char data)//Function to write data to the LCD
{
    LCD_DATA = data;	//Send data on LCD data bus
    rs = 1;				//RS = 1 since data to LCD
    rw = 0;				//RW = 0 since writing to LCD
    en = 1;				//Generate High to low pulse on EN
  	msdelay(15);
    en = 0;
}
//Function to write string to LCD
void LCD_write_string(char *str)   
{
    int i = 0;
    while (str[i] != 0)
    {
        LCD_data(str[i]);      // sending data on LCD byte by byte
        msdelay(15);
        i++;
    }
}
