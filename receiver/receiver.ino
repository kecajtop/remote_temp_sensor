/**
 * @file   receiver.ino
 * @Author kecajtop (kecajtop@gmail.com)
 * @date   March, 2016
 * @brief  Brief description of file.
 * - WHAT IT DOES: Receives data from another transceiver with
   temperature and displays received values on LCD
 - SEE the comments after "//" on each line below
 - CONNECTIONS: nRF24L01 Modules See:
 http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
   1 - GND
   2 - VCC 3.3V !!! NOT 5V
   3 - CE to Arduino pin A2
   4 - CSN to Arduino pin A3
   5 - SCK to Arduino pin 13
   6 - MOSI to Arduino pin 11
   7 - MISO to Arduino pin 12
   8 - BEEP to Arduino pin 5
   9 - SDA  to Arduino pin A4
   10 - SCL  to Arduino pin A5
   11 - SQ to Arduino pin 3
   12 - DS to Arduino pin 4
   13 - KEYPAD to Arduino A0 
   
 * Detailed description of file.
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/wdt.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include "pitches.h"
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <RTClib.h> 
#include <Wire.h>
#include <OneWire.h>

#define CONFIG_VERSION "_TIME_"
#define MINVAL 1
#define MAXVAL 1000
/*-----( Declare Constants and Pin Numbers )-----*/
#define CE_PIN   A2
#define CSN_PIN  A3
//#define BACK_LIGHT 10
#define BEEP 5
#define KEYPAD A0
#define SQ 3
#define DS 4
#define TIME_OUT 2000
//#define LCD_RS 8
//#define LCD_EN 9
//#define LCD_D4 4
//#define LCD_D5 5
//#define LCD_D6 6
//#define LCD_D7 7

#define Int_beep 0

#define tMIN_min 05
#define tMIN_max 99
//#define tMAX_min 99
#define tMAX_max 105

#define PROGRESSPIXELS_PER_CHAR	6

byte tMAX_min;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

const byte firmwareMajor = 0;
const byte firmwareMinor = 54;
  
// The time model
int days = 0;
int hours = 0;
int minutes = 0;
int years = 0;
int months= 0;
bool alarm_f=0;
byte dive;
byte dive1=0;
byte dive2=0;
int timer_alarm=0;
byte buttonPressed=5;
bool timeout_flag=0;
bool signal_lost_f=0;
byte high_low=0;
byte high_low_int=0;
int Tc_100;
int Tc_100_int;
byte display_state=0;
byte display_state1=0;
byte time=0;
int timer2=0;
byte addr[8];

bool mainMenu_flag=0;
bool second_f=0;
int currentMenuItem = 1;
byte lastState = 0;
byte state = 0;
bool once_disp=0;
int button_timer=0;
bool mainmenu_message=0;

// NOTE: the "LL" at the end of the constant is "LongLong" type
const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe

//LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 
RTC_DS1307 rtc;

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

OneWire ds(DS);

int startuptune[] = {
  262, 196, 262, 196};
int mailtune[] = {
  196, 196, 262};

const byte heart[] PROGMEM= {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

const byte heart_off[] PROGMEM= {
  0b00000,
  0b01010,
  0b10101,
  0b10001,
  0b10001,
  0b01010,
  0b00100,
  0b00000
};

const byte custchar[][8] PROGMEM= 
{
  {
    B11111,
    B11111,
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  }
  , {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111
  }
  , {
    B11111,
    B11111,
    B11111,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111
  }
  , {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00110,
    B00110,
    B00110
  }
  , {
    B00000,
    B00000,
    B00000,
    B01110,
    B01110,
    B01110,
    B00000,
    B00000
  }
  , {
    B11111,
    B11111,
    B11011,
    B11011,
    B11111,
    B11111,
    B00000,
    B00000
  }
  , {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  }
  , {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
  }
};



const byte smiley[] PROGMEM= 
{
  0b00000000,
  0b00010001,
  0b00000000,
  0b00000000,
  0b00010001,
  0b00001110,
  0b00000000,
  0b00000000,
};

const byte prog_[][8] PROGMEM= 
{
  {
  0b00011111,
  0b00010001,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00010001,
  0b00011111,
  0b00000000,	
  //0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, // 0. 0/5 full progress block
  },
//byte prog_1[8] = 
{
  0b00011111,
  0b00010001,
  0b00010000,
  0b00010000,
  0b00010000,
  0b00010001,
  0b00011111,
  0b00000000,
  	//0x00, 0x1F, 0x10, 0x10, 0x10, 0x10, 0x1F, 0x00, // 1. 1/5 full progress block
}, 
//byte prog_2[8] = 
{  
  0b00011111,
  0b00011001,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011001,
  0b00011111,
  0b00000000,
  
  	//0x00, 0x1F, 0x18, 0x18, 0x18, 0x18, 0x1F, 0x00, // 2. 2/5 full progress block
}, 
  //byte prog_3[8] = 
{
  0b00011111,
  0b00011101,
  0b00011100,
  0b00011100,
  0b00011100,
  0b00011101,
  0b00011111,
  0b00000000,  
 	//0x00, 0x1F, 0x1C, 0x1C, 0x1C, 0x1C, 0x1F, 0x00, // 3. 3/5 full progress block
}, 
  //byte prog_4[8] = 
{
  0b00011111,
  0b00011111,
  0b00011110,
  0b00011110,
  0b00011110,
  0b00011110,
  0b00011111,
  0b00000000,
  	//0x00, 0x1F, 0x1E, 0x1E, 0x1E, 0x1E, 0x1F, 0x00, // 4. 4/5 full progress block
}, 
  //byte prog_5[8] = 
{
  0b00011111,
  0b00011111,
  0b00011111,
  0b00011111,
  0b00011111,
  0b00011111,
  0b00011111,
  0b00000000,
  	//0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, // 5. 5/5 full progress block
},
// byte prog_6[8] = 
//{
//  	0x03, 0x07, 0x0F, 0x1F, 0x0F, 0x07, 0x03, 0x00, // 6. rewind arrow
//}; 
  //byte prog_6[8] = 
{
  0b00011011,
  0b00011011,
  0b00011011,
  0b00011011,
  0b00011011,
  0b00011011,
  0b00011011,
  0b00000000,
},
  //byte prog_7[8] = 
{
  	0x18, 0x1C, 0x1E, 0x1F, 0x1E, 0x1C, 0x18, 0x00  // 7. fast-forward arrow
}
};
const byte prog_1[][8] PROGMEM= 
{
  {
  0b00000001,//0
  0b00000011,
  0b00000011,
  0b00000111,
  0b00000111,
  0b00001111,
  0b00001111,
  0b00011111,  
  },
 //byte prog_1_2[8] =
 {
  0b00000000,//1
  0b00000000,
  0b00000000,
  0b00000000,
  0b00011111,
  0b00011111,
  0b00011111,
  0b00011111,
 },
  //byte prog_1_3[8] =
  {
  0b00011111,//2
  0b00011111,
  0b00011111,
  0b00011111,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  },
   //byte prog_1_4[8] =
   {
  0b00011111,//3
  0b00011110,
  0b00011110,
  0b00011100,
  0b00011100,
  0b00011000,
  0b00011000,
  0b00010000,
   },
    //byte prog_1_5[8] =
  {
  0b00011111,//4
  0b00001111,
  0b00001111,
  0b00000111,
  0b00000111,
  0b00000011,
  0b00000011,
  0b00000001,
  },
 //byte prog_1_6[8] =
 {
  0b00010000,//5
  0b00011000,
  0b00011000,
  0b00011100,
  0b00011100,
  0b00011110,
  0b00011110,
  0b00011111,
 },
  //byte prog_1_7[8] =
  {
  0b00000111,//6
  0b00000111,
  0b00000111,
  0b00000111,
  0b00000111,
  0b00000111,
  0b00000111,
  0b00000111,
  },
   //byte prog_1_8[8] =
   {
  0b00011100,//7
  0b00011100,
  0b00011100,
  0b00011100,
  0b00011100,
  0b00011100,
  0b00011100,
  0b00011100,
   },
    //byte prog_1_9[8] =
    {
  0b00001100,//8
  0b00001100,
  0b00001100,
  0b00001100,
  0b00001100,
  0b00001100,
  0b00001100,
  0b00001100
    }
};

const byte kCharTable_Revised[] PROGMEM=
{
 // '0',
  B11011,
  B11011,
  B00000,
  B11000,
  B11000,  
  B00000,
  B11000,
  B11000,  
 // '1',
  B11011,
  B11011,
  B00000,
  B00011, 
  B00011,   
  B00000,
  B00011, 
  B00011,   
 // '2',
  B11000, 
  B11000, 
  B00000,  
  B11000, 
  B11000, 
  B00000,  
  B11011,  
  B11011,    
 // '3',
  B00011, 
  B00011, 
  B00000,  
  B00011, 
  B00011, 
  B00000,  
  B11011,  
  B11011,      
 // '4',
  B00000,  
  B00000,  
  B00000,  
  B00000,  
  B00000,  
  B00000,  
  B11011,  
  B11011,        
 // '5',
  B11011,
  B11011,
  B00000,
  B00011, 
  B00011,   
  B00000,
  B11011, 
  B11011,   
  //'6',
  B11011,  
  B11011,        
  B00000,  
  B00000,  
  B00000,  
  B00000,  
  B00000,  
  B00000,  
  //'7',
  B11011,
  B11011,
  B00000,
  B11000,
  B11000,  
  B00000,
  B11011,
  B11011,  
  
  //NULL
};

const byte kCharIndices_Revised[] PROGMEM=
{
        0, 1, 
        2, 3, //0
        1, ' ', 
        3, 4,//1
        6, 5, 
        7, 4,//2
        6, 5, 
        4, 3, //3
        2, 3, 
        ' ', 1,//4
        7, 6, 
        4, 5,//5 
        0, 6, 
        7, 5,//6
        6, 1, 
        ' ', 0,//7
        7, 5, 
        2, 3,//8
        7, 5, 
        4, 3,//9

        0, 1, 
        0, 1,//A
        0, 1, 
        0, 6,//P
        
   NULL
};

const uint8_t napis_numb[] PROGMEM=//define 8 custom LCD chars
{
	000,255,005,//0
	255,254,255,
	255,254,255,
	004,255,003,
	 
	000,255,254,//1
	254,255,254,
	254,255,254,
	001,255,001,
	
	000,255,005,//2
	254,000,003,
	000,003,254,
	255,255,255, 

	000,255,005,//3
	254,254,255,
	254,002,255,
	004,255,003,
	
	254,000,255,//4
	000,003,255,
	002,002,255,
	254,254,255,  

	255,255,255,//5
	255,001,001,
	254,254,255,
	004,255,003,

	000,255,005,//6
	255,254,254,
	255,002,005,
	004,255,003,

	255,255,255,//7
	254,000,003,
	000,003,254,
	003,254,254,

	000,255,005,//8
	004,254,003,
	000,002,005,
	004,255,003,

	000,255,005,//9
	004,001,255,
	254,254,255,
	254,254,255,

	006,255,005,//R
	006,001,255,
	006,004,005,
	006,254,255,

	000,255,005,//A
	255,001,255,
	255,002,255,
	255,254,255,

	006,255,005,//D
	006,254,255,
	006,254,255,
	006,255,003,

	000,255,005,//M
	007,255,006,
	007,255,006,
	007,255,006


};

const byte bignums[] PROGMEM= //12,2,3
{
 
      7, 0, 7,   
      7, 1, 7,
  
      0, 7, 6,
      1, 7, 1 ,   
  
      0, 0, 7,
      7, 2, 2,
 
      0, 2, 7,   
      1, 1, 7,

      7, 1, 7,
      6, 6, 7,
      
      7, 2, 2,
      1, 1, 7,

      7, 2, 2,
      7, 1, 7,

      0, 0, 7,
      6, 7, 6,

      7, 2, 7,
      7, 1, 7,

      7, 2, 7,
      6, 6, 7,
 
      7, 0, 0,
      7, 1, 1,
  
      6, 6, 6,
      6, 6, 6,
  
};

#define CONFIG_START 1

struct StoreStruct 
{
  // The variables of your settings
  byte t_max_value;
  byte t_min_value;
  byte back_light;
  byte t_min_value_int;
  byte t_max_value_int;
  char version_of_program[9]; // it is the last variable of the struct
  // so when settings are saved, they will only be validated if
  // they are stored completely.
} settings = 
{
  // The default values 
  85,
  10,
  128,
  15,
  25,
  CONFIG_VERSION
};

void lcd_progress(uint16_t progress, uint16_t maxprogress, byte length1)
{
        byte i;
	uint16_t pixelprogress;
	byte c;

	// draw a progress bar displaying (progress / maxprogress)
	// starting from the current cursor position
	// with a total length of "length" characters
	// ***note, LCD chars 0-5 must be programmed as the bar characters
	// char 0 = empty ... char 5 = full

	// total pixel length of bargraph equals length*PROGRESSPIXELS_PER_CHAR;
	// pixel length of bar itself is
	pixelprogress = ((progress*(length1*PROGRESSPIXELS_PER_CHAR))/maxprogress);
	
	// print exactly "length" characters
	for(i=0; i<length1; i++)
	{
		// check if this is a full block, or partial or empty
		// (u16) cast is needed to avoid sign comparison warning
		if( ((i*(uint16_t)PROGRESSPIXELS_PER_CHAR)+5) > pixelprogress )
		{
			// this is a partial or empty block
			if( ((i*(uint16_t)PROGRESSPIXELS_PER_CHAR)) > pixelprogress )
			{
				// this is an empty block
				// use space character?
				c = 0;
			}
			else
			{
				// this is a partial block
				c = pixelprogress % PROGRESSPIXELS_PER_CHAR;
			}
		}
		else
		{
			// this is a full block
			c = 5;
		}
		
		// write character to display
		  lcd.write(c);
                  //Serial.print(c);
	}

}


void loadConfig() 
{
  Serial.print("Loading settings...");
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  if (//EEPROM.read(CONFIG_START + sizeof(settings) - 1) == settings.version_of_program[3] // this is '\0'
      EEPROM.read(CONFIG_START + sizeof(settings) - 1) == settings.version_of_program[8] &&
      EEPROM.read(CONFIG_START + sizeof(settings) - 2) == settings.version_of_program[7] &&
      EEPROM.read(CONFIG_START + sizeof(settings) - 3) == settings.version_of_program[6] &&
      EEPROM.read(CONFIG_START + sizeof(settings) - 4) == settings.version_of_program[5] &&
      EEPROM.read(CONFIG_START + sizeof(settings) - 5) == settings.version_of_program[4] &&
      EEPROM.read(CONFIG_START + sizeof(settings) - 6) == settings.version_of_program[3] &&
      EEPROM.read(CONFIG_START + sizeof(settings) - 7) == settings.version_of_program[2] &&
      EEPROM.read(CONFIG_START + sizeof(settings) - 8) == settings.version_of_program[1] &&
      EEPROM.read(CONFIG_START + sizeof(settings) - 9) == settings.version_of_program[0])
  { // reads settings from EEPROM
    for (unsigned int t=0; t<sizeof(settings); t++)
      *((byte*)&settings + t) = EEPROM.read(CONFIG_START + t);
        Serial.println("OK");
      
  } else {
    Serial.println("Memory empty...");
    // settings aren't valid! will overwrite with default settings
    saveConfig();
  }
}

void saveConfig() 
{
  Serial.print("Saving settings...");
  for (unsigned int t=0; t<sizeof(settings); t++)
  { // writes to EEPROM
    EEPROM.write(CONFIG_START + t, *((byte*)&settings + t));
    // and verifies the data
    if (EEPROM.read(CONFIG_START + t) != *((byte*)&settings + t))
    {
      // error writing to EEPROM
      Serial.println("Fail");
    }
  }
  Serial.println("OK");
}

void load_default()
{
  //Serial.println("load_default");
  _delay_ms(1000);
  if(buttonPressed==btnSELECT)
   {
     Serial.println("Button Select pressed");
     _delay_ms(2000);
     if(buttonPressed==btnSELECT)
     {
       Serial.println("Restoring default settings");
       lcd.clear();
       lcd.setCursor(0, 0);
       lcd.print(F("Restoring default"));
       lcd.setCursor(0, 1);
       lcd.print(F("settings......"));
     saveConfig();
     //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
     }
   }
}
void alarm()
{
  //back_light_level(settings.back_light+30);
  if (timer_alarm<1)
  {
  if(timer2>map(Tc_100/100,0,tMAX_max,50,5))
    {
      timer2=0;  
      lcd.noBacklight();
      tone(BEEP,1000,200);
      _delay_ms(250);
      lcd.backlight();
    }
  }
}

//void back_light_level(byte level)
//{
//  analogWrite(BACK_LIGHT,level);
//}

void loadchars() 
{
  //lcd.command(64);
  byte c[8];
  for (int j = 0; j < 8; j++)
  {
    for (int i = 0; i < 8; i++)
    {
      c[i]=pgm_read_byte(&custchar[j][i]);
      
    }
  lcd.createChar(j,c);
  }
  lcd.home();
}
void zestaw1()
{  
  //lcd.createChar(0, prog_0);
  //lcd.createChar(1, prog_1);
  //lcd.createChar(2, prog_2);
  //lcd.createChar(3, prog_3);
  //lcd.createChar(4, prog_4);
  //lcd.createChar(5, prog_5);
  //lcd.createChar(6, prog_6);
  //lcd.createChar(7, prog_7);
    byte c[8];
  for (int j = 0; j < 8; j++)
  {
    for (int i = 0; i < 8; i++)
    {
      c[i]=pgm_read_byte(&prog_[j][i]);
      
    }
  lcd.createChar(j,c);
  }
  lcd.home();
}

void printbigchar(byte digit, byte col, byte row, byte symbol = 0) {
  if (digit > 11) return;
  for (int i = 0; i < 2; i++) {
    lcd.setCursor(col*4, row + i);
    for (int j = 0; j < 3; j++) {
      lcd.write(pgm_read_byte(&bignums[(digit*6)+(i*3)+(j)]));
    }
  }
  if (symbol == 0) {
    lcd.setCursor(col*4 + 3, row);
    lcd.write(6);
    lcd.setCursor(col*4 + 3, row + 1);
    lcd.write(6);
  } 
  else if (symbol == 1) {
    lcd.setCursor(col*4 + 3, row);
    lcd.write(6);
    lcd.setCursor(col*4 + 3, row + 1);
    lcd.write(3);
  } 
  else if (symbol == 2) {
    lcd.setCursor(col*4 + 3, row);
    lcd.write(4);
    lcd.setCursor(col*4 + 3, row + 1);
    lcd.write(4);
  }
  lcd.setCursor(col*4 + 4, row);
}

void bigprint(float number, byte col,byte row)
{
  printbigchar(number / 10,col,row);
  printbigchar(int(number)%10,col+1,row,1);
  printbigchar(int(number*10)%10,col+2,row);
}

void bigprint1(float number, byte col,byte row)
{
  napis1(number / 10,col,row);
  napis1(int(number)%10,col+1,row+1);
  napis1(int(number*10)%10,col+2,row+2);
}

void bigclock(int hour, int minute, int second, int col)
{
  printbigchar(hour/10,col,0);
  if(second%2==0) printbigchar(hour%10,col+1,0,2);
  else printbigchar(hour%10,col+1,0);
  printbigchar(minute/10,col+2,0);
  printbigchar(minute%10,col+3,0);
  /*printbigchar(11,col+4,0);*/
  lcd.setCursor(16,0);
}

void begin_radio()
{
  Serial.println("Nrf24L01 Receiver Starting");
  radio.begin();
  radio.openReadingPipe(1,pipe);
  
  radio.startListening();
}

////void setup_pins()
//{
//  pinMode(BACK_LIGHT, OUTPUT);
 // digitalWrite(BACK_LIGHT, LOW);
//  pinMode(KEYPAD, OUTPUT);
 // pinMode(SQ, INPUT);
    
//}

void play_tune1()
{
    for (int note=0; note<4; ++note) {
    tone(BEEP,startuptune[note],75);
    _delay_ms(100);
    noTone(BEEP);  
 } 
}

void bigtemp(double temp,byte row)
{
  bigprint(temp,0,row);
  lcd.setCursor(12,0+row);
  lcd.print("  ");
  lcd.setCursor(12,1+row);
  lcd.print("   ");
  lcd.setCursor(14,0+row);
  lcd.write(5);
  printbigchar(10,4,row);
}

void bigtemp1(double temp,byte row)
{
  bigprint1(temp,1,row);
  lcd.setCursor(12,0+row);
  lcd.print("  ");
  lcd.setCursor(12,1+row);
  lcd.print("   ");
  lcd.setCursor(14,0+row);
  lcd.write(5);
  //printbigchar(10,4,row);
}

void set_timer1()

{
cli();//stop interrupts

//set timer0 interrupt at 2kHz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 124;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

sei();//allow interrupts
}

#if Int_beep==1
boolean toggle1 = 0;
#endif  

void read_radio();

ISR(TIMER0_COMPA_vect)

{
  cli();
  dive++;
  if (dive>=200){dive=0;
  ten_hz_task();
  read_radio();
  if (timeout_flag!=0){dive1++;}else{dive1=0;signal_lost_f=0;}
  if (mainMenu_flag==1){dive2++;}else{dive2=0;}
  if (dive2>=100){mainMenu_flag=0;}
 

#if Int_beep==1
  if (toggle1){
    digitalWrite(2,HIGH);
    toggle1 = 0;
  }
  else{
    digitalWrite(2,LOW);
    toggle1 = 1;
  }
#endif  
}
sei();
}
void ten_hz_task()
{
  time++;
  timer2++;
  //timer3++;
  if (time>10){time=0;one_sec();}
  buttonPressed = readButtons();
  if (buttonPressed!=btnSELECT)
  {
  //if (button_timer >200){fault_flag=1;} 
  //else 
  if (button_timer >10)
        {
        mainMenu_flag=!mainMenu_flag;
        if (mainMenu_flag){once_disp=1;}
        }
  else if(button_timer>1)
  {
  //if (fault_flag==1){fault_flag=0;}
  //swtoggle=!swtoggle;
  }
  }
  if (buttonPressed!=btnNONE)
  {
      timer_alarm=TIME_OUT;
  }
  if (buttonPressed==btnSELECT){button_timer++;state=3;}
  else {button_timer=0;}
  
  if(signal_lost_f==0)
  {
      if (Tc_100/100>settings.t_max_value){alarm_f=1;high_low=1;timer_alarm--;}
      else if (Tc_100/100<settings.t_min_value){alarm_f=1;high_low=2;timer_alarm--;}
      else {high_low=0;timer_alarm=0;}
      if (Tc_100_int/100>settings.t_max_value_int){high_low_int=1;}
      else if (Tc_100_int/100<settings.t_min_value_int){high_low_int=2;}
      else {high_low=0;}
  }
  else
  {
    timer_alarm--;
  }
  if (timer_alarm<=0){timer_alarm=0;}
}

void one_sec()
{
   
   display_state++;
   if(display_state>17){display_state=0;}
   

}
void signal_lost()
{
alarm();
}

 int ds_to_temp(int LowByte,int HighByte)
{int tc;
       //int HighByte, LowByte, 
       int TReading, SignBit, Whole, Fract;
        //LowByte = data[0];
        //HighByte = data[1];
        TReading = (HighByte << 8) + LowByte;
        SignBit = TReading & 0x8000;  // test most sig bit
        
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  tc = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25
 
return tc;} 

void read_radio()
{
  byte data[12];
  int HighByte, LowByte, TReading, SignBit, Whole, Fract;
  //int bottom=MINVAL, top=MAXVAL;
  //int buttonPressed = readButtons();
  

 if ( radio.available() )
  {
    timeout_flag=0;
    bool done = false;
    while (!done)
    {
      // Fetch the data payload
     done = radio.read( data, sizeof(data) );
        LowByte = data[0];
        HighByte = data[1];
        TReading = (HighByte << 8) + LowByte;
        SignBit = TReading & 0x8000;  // test most sig bit
        
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25
    }
 }
  else
  { 
    timeout_flag=1;   
  }

}

void setup_RTC()
{
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  rtc.writeSqwPinMode(SquareWave1HZ);
}

void setup()   /****** SETUP: RUNS ONCE ******/
{
  //delay(10000);
  wdt_disable();
  Serial.begin(9600);
  Serial.println("Baudrate setup to 9600");
  setup_RTC();
  lcd.begin(20, 4);
  begin_radio();
  //back_light_level(settings.back_light);
  play_tune1();
  loadchars();
  set_timer1(); // 10Hz routine
  //attachInterrupt(digitalPinToInterrupt(SQ), second_int, LOW);
  load_default();
  loadConfig();
  wdt_enable(WDTO_4S);
  ds_init();
  
}

void second_int()
{
  second_f=1;
}
/* ************************************************************************
Read the buttons on the lcd shield
************************************************************************ */
int readButtons()
{
    int keyIn  = 0;
    keyIn = analogRead(KEYPAD);      // read the value from the sensor
    // Buttons when read are centered at these values: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    if (keyIn > 1000) return btnNONE;       // 1st option for efficiency
    if (keyIn < 50)   return btnRIGHT; 
    if (keyIn < 195)  return btnUP;
    if (keyIn < 380)  return btnDOWN;
    if (keyIn < 555)  return btnLEFT;
    if (keyIn < 790)  return btnSELECT;  
    return btnNONE;  // when all others fail
}


byte t_max(byte temp,byte min_value) 
{
  tMAX_min=min_value+1;
    switch (state) 
    {
    case 4:
         temp++;
         break;
    case 5:
         temp--;
         break; 
    }
    if (temp<tMAX_min){temp=tMAX_min;}
    if (temp>tMAX_max){temp=tMAX_max;}
    _delay_ms(200);
    return temp;
}
 
 byte t_min(byte temp) 
{
    //tMAX_min=settings.t_max_value;
    switch (state) 
    {
    case 4:
        temp++;
        break;
    case 5:
        temp--;
        break;
  
    }
    if (temp<tMIN_min){temp=tMIN_min;}
    if (temp>tMIN_max){temp=tMIN_max;}
    _delay_ms(200);
    return temp;
}

 void set_backlight() 
{
    switch (state) 
    {
    case 4:
        settings.back_light=settings.back_light+10;
        break;
    case 5:
        settings.back_light=settings.back_light-10;
        break;
    }
    if (settings.back_light<10){settings.back_light=10;}
    if (settings.back_light>225){settings.back_light=225;}
    _delay_ms(200);
}

void set_day() 
{
    DateTime now = rtc.now();
    switch (state) 
    {
    case 3:
        rtc.adjust(DateTime(now.year(), now.month(), days, now.hour(), now.minute(), now.second()));
        break;
    case 4:
        days++;
        break;
    case 5:
        days--;
        break;
    }
    if (days<0){days=31;}
    if (days>31){days=0;}
    _delay_ms(200);
}

  void set_month() 
{
    DateTime now = rtc.now();
    switch (state) 
    {
    case 3:
        rtc.adjust(DateTime(now.year(), months, now.day(), now.hour(), now.minute(), now.second()));
        break;
    case 4:
        months++;
        break;
    case 5:
        months--;
        break;
    }
    if (months<0){months=12;}
    if (months>12){months=0;}
    _delay_ms(200);
  }
  void set_year()
 {
    DateTime now = rtc.now();
    switch (state) 
    {
    case 3:
        rtc.adjust(DateTime(years, now.month(), now.day(), now.hour(), now.minute(), now.second()));
        break;
    case 4:
        years++;
        break;
    case 5:
        years--;
        break;
    }
    if (years<2015){years=2100;}
    if (years>2100){years=2015;}
    _delay_ms(200);
  }
 void set_hours()
{
    DateTime now = rtc.now();
    switch (state) 
    {
    case 3:
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), hours, now.minute(), now.second()));
        break;
    case 4:
        hours++;
        break;
    case 5:
        hours--;
        break;
  
    }
    if (hours<0){hours=23;}
    if (hours>23){hours=0;}
    _delay_ms(200);
}

 void set_minutes() 
{
  
    DateTime now = rtc.now();
    switch (state) 
    {
    case 3:
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), minutes, now.second()));
        break;
    case 4:
        minutes++;
        break;
    case 5:
        minutes--;
        break;
   
  
    }
    if (minutes<0){minutes=59;}
    if (minutes>59){minutes=0;}
    _delay_ms(200);
}
// Print the current setting

void disp_clear()
{
  if (display_state!=display_state1){lcd.clear();}
  //Serial.print(display_state);Serial.print(display_state1);
  display_state1=display_state; 
}

void high_low_disp()
{
             //if (alarm_f==1)
           //{
             //Serial.print("alarm");
             if(high_low==1)
             {
             lcd.print("H ");
             //Serial.print("high");
             }
             else if(high_low==2)
             {
             lcd.print("L ");
             //Serial.print("low");
             }
           //}
           else
           {
             lcd.print("  ");
           }
           
}

void high_low_disp_int()
{
             //if (alarm_f==1)
           //{
             //Serial.print("alarm");
             if(high_low_int==1)
             {
             lcd.print("  ");
             //Serial.print("high");
             }
             else if(high_low_int==2)
             {
             lcd.print("R ");
             //Serial.print("low");
             }
           //}
           else
           {
             lcd.print("R ");
           }
           
}
void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  DateTime now = rtc.now();
  //if (second_f==1)
   if (dive1>=100)
    {
    dive1=0;
    signal_lost_f=1;
    signal_lost();
    }
  if (alarm_f==1){alarm();alarm_f=0;}
 
  
   
  //ds_read();
  //
  //Serial.println(timer_alarm);
  //back_light_level(settings.back_light);
  if (button_timer==3){lcd.clear();}
  if (button_timer>2){zestaw1();lcd.setCursor(0,1);lcd_progress(button_timer,10,20);once_disp=1;}
  else
  {
   if (mainMenu_flag==1)
   {
    //second_f=0;    
   
    //Tc_100_int=ds_read();
    
     mainMenu();
   }
   else 
   {
   if (mainmenu_message)
   {
      _delay_ms(250);
      _delay_ms(250);
      _delay_ms(250);
      _delay_ms(250);
    mainmenu_message=0;
    }

    //Serial.print(display_state);Serial.print(display_state1);
    hours=now.hour();
    minutes=now.minute();
    months=now.month();
    days=now.day();
    years=now.year(); 
    
   // if (signal_lost_f==0)
   // {  
      
      switch (display_state)
      { 
      case 0:
           disp_clear();
           Tc_100_int=ds_read();
           
           if(timer_alarm!=0)
           {
             zestaw1();
             lcd.setCursor(0,1);
             lcd_progress(timer_alarm/10,TIME_OUT/10,20);
           } 
           break;
      case 1:
           disp_clear();
           zestaw2();
           //printbigchar(11,4,0);
           //bigclock(now.hour(),now.minute(),now.second(),0);
           time_hh_mm_ss(now.hour(),now.minute(),now.second());
           break;
      case 2: 
           time_hh_mm_ss(now.hour(),now.minute(),now.second());
           break;
      case 3:
           time_hh_mm_ss(now.hour(),now.minute(),now.second()); 
           break;
      case 4:
           time_hh_mm_ss(now.hour(),now.minute(),now.second());
           //lcd.clear(); 
           break;
      case 5:
           disp_clear();
           disp_temp();
           lcd.setCursor(0, 2);
           lcd.print(F("EXT. MIN "));
           lcd.print(settings.t_min_value);
           lcd.print(F(" MAX "));
           lcd.print(settings.t_max_value);
           lcd.setCursor(0, 3);
           lcd.print(F("INT. MIN "));
           lcd.print(settings.t_min_value_int);
           lcd.print(F(" MAX "));
           lcd.print(settings.t_max_value_int); 
           break;
      case 6:
           disp_temp(); 
           break;
      case 7:
           disp_temp();
           break;
      case 8:
           disp_temp(); 
           break;
      case 9:
           disp_clear();
           //
           zestaw3();
           //napis1(int(number)%10,col+1,row+1);
           lcd.setCursor(0, 1);
           lcd.print(F("TIME "));
           lcd.print(now.hour());
           lcd.print(F(":"));
           lcd.print(now.minute());
           lcd.setCursor(0, 3);
           lcd.print(F("DATE "));
           lcd.print(now.year());
           lcd.print(F("/"));
           lcd.print(now.month());
           lcd.print(F("/"));
           lcd.print(now.day());
           break;
      case 10:
           //disp_temp();//napis1
           break;
      case 11:
           //disp_clear(); 
           //zestaw3();
           //for(byte a=1;a<10;a++){
           //disp_temp();  
           //napis1(a-1,a,a);
           //} 
           break;
      case 12: 
           disp_clear();
           loadchars();
           disp_temp1();  
           break;
      case 13:
           //disp_clear();
           disp_temp1(); 
           break;
      case 14:
           //disp_clear();
           disp_temp1(); 
           break;
      case 15:
           //disp_clear();
           disp_temp1();
           break;
      case 16:
           //disp_clear();
           
           break;
      default:
       //
      break;
      }
     
   //}
   //else
   //{  //loadchars();
    //  lcd.setCursor(0, 0);
    //  lcd.print(F("Out of range     "));
     // lcd.setCursor(0, 1);
     // lcd.print(F("Signal Lost......"));
      //bigtemp((double)Tc_100_int/100,2);
    //}  
  }
 }
 
 wdt_reset();
 //Serial.println("wdt_reset");
}

void disp_temp()
{
           lcd.setCursor(0, 0);
           if (signal_lost_f==1)
           {
             lcd.print(F("Signal Lost......"));
           }
           else
           {
           lcd.print(F("EXTERNAL TEMP "));
           high_low_disp();
           lcd.print(int(Tc_100/100),DEC);
           lcd.print(F("."));
           lcd.print(int(Tc_100)%10);
           }
           lcd.setCursor(0, 1);
           lcd.print(F("INTERNAL TEMP "));
           high_low_disp_int();
           lcd.print(int(Tc_100_int/100),DEC);
           lcd.print(F("."));
           lcd.print(int(Tc_100_int)%10);
           //bigtemp((double)Tc_100_int/100,2);
}
void disp_temp1()
{
  if (signal_lost_f==1)
           {
           lcd.setCursor(0, 1);
           lcd.print(F("Out of range        "));
           lcd.setCursor(0, 2);
           lcd.print(F("Signal Lost......   "));
           }
           else
           {
           //loadchars();
           bigtemp((double)Tc_100/100,1);
           } 
}

void mainMenu() 
{
  lcd.setCursor(0,0);
  if (buttonPressed == btnRIGHT) {
    //Right
    state = 5;
    selectMenu(currentMenuItem);
  } else if (buttonPressed == btnLEFT) {
    //left
    state = 4;
    selectMenu(currentMenuItem);
  }
  else
  {
  state=0;
  }
  if (buttonPressed == btnUP){
   //Down
    state = 1;
  }
  else if (buttonPressed == btnDOWN)
  {
    //up
    state = 2;
  }
  else
  {
   state=0;
  }
  if (buttonPressed == btnSELECT)
  {
    //Select
    state = 3;
    //selectMenu(currentMenuItem);
  }
  //If we are out of bounds on th menu then reset it.
  if (state!=0){dive2=0;}
 if (once_disp){displayMenu(currentMenuItem);once_disp=0;}
   //If we have changed Index, saves re-draws.
   if (state != lastState) {
      if (state == 1) {
         //If Up
          currentMenuItem = currentMenuItem - 1;
          if (currentMenuItem <0){currentMenuItem = 14;}
          displayMenu(currentMenuItem);
      } else if (state == 2) {
         //If Down
          currentMenuItem = currentMenuItem + 1;  
          if (currentMenuItem > 14) {currentMenuItem = 0;}
          displayMenu(currentMenuItem);
          } else if (state == 3) {
         //If Selected
         selectMenu(currentMenuItem); 
      }
      //Save the last State to compare.
      lastState = state;
   } 
   //Small delay
  _delay_ms(10);
}
 
 
//Display Menu Option based on Index.
void displayMenu(int x) {
     switch (x) {
      case 1:
        clearPrintTitle();
        lcd.print (F("-> Set Ext Min Temp."));
        break;
      case 2:
        clearPrintTitle();
        lcd.print (F("-> Set Ext Max Temp."));
        break;
      case 3:
        clearPrintTitle();
        lcd.print (F("-> Set Int Min Temp."));
        break;
      case 4:
        clearPrintTitle();
        lcd.print (F("-> Set Int Max Temp."));
        break;
      case 5:
        clearPrintTitle();
        lcd.print (F("-> Set Hour         "));
        break;
      case 6:
        clearPrintTitle();
        lcd.print (F("-> Set Minute       "));
        break;
      case 7:
        clearPrintTitle();
        lcd.print (F("-> Set Day          "));
        break;
      case 8:
        clearPrintTitle();
        lcd.print (F("-> Set Mounth       "));
        break;
      case 9:
        clearPrintTitle();
        lcd.print (F("-> Set Year         "));
        break;
      case 10:
        clearPrintTitle();
        lcd.print (F("-> Set Default     "));
        break;
      case 11:
        clearPrintTitle();
        lcd.print (F("-> Version          "));
        break;
      case 12:
        clearPrintTitle();
        lcd.print (F("-> Save             "));
        break;
      case 13:
        clearPrintTitle();
        lcd.print (F("-> Save&EXIT        "));
        break;
      case 14:
        clearPrintTitle();
        lcd.print (F("-> EXIT             "));
       break;  
  } 
}
 
//Print a basic header on Row 1.
void clearPrintTitle() 
{
  lcd.home();
  lcd.setCursor(0,0);
  lcd.print(F("..:Menu:..      "));
  lcd.setCursor(0,1); 
}
 
//Show the selection on Screen.
void selectMenu(int x) {
   switch (x) {
       case 1:
           clearPrintTitle();
           lcd.print (F("Ext Min Temp = "));
           lcd.setCursor(15,1);
           lcd.print(settings.t_min_value);
           lcd.print(F("      "));
           settings.t_min_value=t_min(settings.t_min_value);
           lcd.setCursor(15,1);
           lcd.print(settings.t_min_value);
           break;
       case 2:
           clearPrintTitle();
           lcd.print (F("Ext Max Temp = "));
           lcd.setCursor(15,1);
           lcd.print(settings.t_max_value);
           lcd.print(F("      "));
           settings.t_max_value=t_max(settings.t_max_value,settings.t_min_value);
           lcd.setCursor(15,1);
           lcd.print(settings.t_max_value);
           break;
       case 3:
           clearPrintTitle();
           lcd.print (F("Int Min Temp = "));
           lcd.setCursor(15,1);
           lcd.print(settings.t_min_value_int);
           lcd.print(F("      "));
           settings.t_min_value_int=t_min(settings.t_min_value_int);
           lcd.setCursor(15,1);
           lcd.print(settings.t_min_value_int);
           break;
       case 4:
           clearPrintTitle();
           lcd.print (F("Int Max Temp = "));
           lcd.setCursor(15,1);
           lcd.print(settings.t_max_value_int);
           lcd.print(F("      "));
           settings.t_max_value_int=t_max(settings.t_max_value_int,settings.t_min_value_int);
           lcd.setCursor(15,1);
           lcd.print(settings.t_max_value_int);
           break;
       case 5:
           clearPrintTitle();
           lcd.print (F("Hour = "));
           lcd.setCursor(7,1);
           set_hours();
           //_delay_ms(100);
           lcd.print(hours);
           lcd.print(F("     "));
           break;
       case 6:
           clearPrintTitle();
           lcd.print (F("Minute = "));
           lcd.setCursor(9,1);
           set_minutes();
           lcd.print(minutes);
           lcd.print(F("     "));
           break;
       case 7:
           clearPrintTitle();
           lcd.print (F("Day = "));
           set_day();
           lcd.setCursor(6,1);
           lcd.print(days);
           lcd.print(F("      "));
           break;
       case 8:
           clearPrintTitle();
           lcd.print (F("Month =  "));
           set_month();
           lcd.setCursor(11,1);
           lcd.print(months);
           lcd.print(F("       "));
           break;
       case 9:
           clearPrintTitle();
           lcd.print (F("Year = "));
           set_year();
           lcd.setCursor(7,1);
           lcd.print(years);
           lcd.print(F("       "));
           break;
       case 10:
           clearPrintTitle();
           lcd.print(F("Default settings "));
           lcd.setCursor(0,1);
           lcd.print(F("Restored         "));
           
           //saveDefault();
           mainMenu_flag=0;
           break;
      case 11:
           clearPrintTitle();
           lcd.print (F("Version "));
           lcd.print(firmwareMajor);
           lcd.print(F("."));
           lcd.print(firmwareMinor);
           lcd.print(F("    "));
           break;
      case 12:
           clearPrintTitle();
           lcd.print (F("Saved       "));
           saveConfig();
           break;
      case 13:   
           clearPrintTitle();
           lcd.print(F("Saved&Exiting  "));
           mainmenu_message=1;
           saveConfig();
           mainMenu_flag=0;
           break;
       case 14:
           clearPrintTitle();
           lcd.print (F("Exiting     "));
           mainmenu_message=1;
           mainMenu_flag=0;
           break;
    }
}

void ds_init()
{
byte i;
  ds.reset_search();
  if ( !ds.search(addr)) {
      Serial.print(F("No DS18B20 found.\n"));
      ds.reset_search();
      //return 0;
  }

  Serial.print("R=");
  for( i = 0; i < 8; i++) {
    Serial.print(addr[i], HEX);
    Serial.print(" ");
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n");
      //return 0;
  }

  if ( addr[0] == 0x10) {
      Serial.print("Device is a DS18S20 family device.\n");
  }
  else if ( addr[0] == 0x28) {
      Serial.print("Device is a DS18B20 family device.\n");
  }
  else {
      Serial.print("Device family is not recognized: 0x");
      Serial.println(addr[0],HEX);
      //return 0;
  }


}
int ds_read(void) {
   byte i;
  byte present = 0;
  byte data[12];

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end
  
  _delay_ms(750);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  //Serial.print("P=");
  //Serial.print(present,HEX);
  //Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    
   // return 0;
    
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
  //Serial.print(ds_to_temp(data[0],data[1]));
    return ds_to_temp(data[0],data[1]);
  //Serial.print(" CRC=");
  //Serial.print( OneWire::crc8( data, 8), HEX);
  //Serial.println();
  //return 0;
}

void zestaw2()
{  //lcd.createChar(0, prog_1_1);
  //lcd.createChar(1, prog_1_2);
  //lcd.createChar(2, prog_1_3);
  //lcd.createChar(3, prog_1_4);
  //lcd.createChar(4, prog_1_5);
  //lcd.createChar(5, prog_1_6);
  //lcd.createChar(6, prog_1_7);
  //lcd.createChar(7, prog_1_8);
  //lcd.createChar(7, prog_1_9);
  
  byte c[8];
  for (int j = 0; j < 8; j++)
  {
    for (int i = 0; i < 8; i++)
    {
      c[i]=pgm_read_byte(&prog_1[j][i]);
      
    }
  lcd.createChar(j,c);
  }
}

void zestaw3()
{
 byte c[8];
  for (int j = 0; j < 8; j++)
  {
    for (int i = 0; i < 8; i++)
    {
      c[i]=pgm_read_byte(&kCharTable_Revised[(j*8)+i]);
      
    }
  lcd.createChar(j,c);
  } 
}

void napis(uint8_t n,uint8_t p,uint8_t o)
{
uint8_t c;
uint8_t i,y,x;
for(y=0; y<4; y++)
     {
	 for (x=0;x<3;x++)
	 {
	 i=x+(3*y);
	 lcd.setCursor(x+3*(p-1)+o,y);
	 c=pgm_read_byte_near(&napis_numb[i+(n*12)]);
	 lcd.write(c);
	 }
	 }

}

void time_hh_mm_ss(uint8_t hh,uint8_t mm,uint8_t ss)
{
uint8_t h1,h2,m1,m2,s1,s2;
h1=hh/10;
h2=hh-(h1*10);
m1=mm/10;
m2=mm-(m1*10);
s1=ss/10;
s2=ss-(s1*10);
napis(h1,1,2);
napis(h2,3,0);
napis(m1,4,1);
napis(m2,5,2);
//napis(s1,5,2);
//napis(s2,6,2);
if (s2%2==0){
lcd.setCursor(9,1);
lcd.print("o");
lcd.setCursor(9,2);
lcd.print("o");
//lcd.setCursor(13,1);
//lcd.print("o");
//lcd.setCursor(13,2);
//lcd.print("o");
}
else
{
lcd.setCursor(9,1);
lcd.print(" ");
lcd.setCursor(9,2);
lcd.print(" ");
//lcd.setCursor(13,1);
//lcd.print(" ");
//lcd.setCursor(13,2);
//lcd.print(" ");
}
}

void napis1(uint8_t n,uint8_t p,uint8_t o)
{
uint8_t c;
uint8_t i,y,x;
for(y=0; y<2; y++)
     {
	 for (x=0;x<2;x++)
	 {
	 i=x+(2*y);
	 lcd.setCursor(x+2*(p-1)+o,y);
	 c=pgm_read_byte_near(&kCharIndices_Revised[i+(n*4)]);
         // Serial.print(c,DEC);
	 lcd.write(c);
	 }
         //Serial.println();	 
  }

}
