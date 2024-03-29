/**
 * @file   transmiter.ino
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
   3 - CE to Arduino pin 9
   4 - CSN to Arduino pin 10
   5 - SCK to Arduino pin 13
   6 - MOSI to Arduino pin 11
   7 - MISO to Arduino pin 12
   8 - RELAY to Arduino pin A0
   9 - DS to Arduino pin 2

*/

/*-----( Import needed libraries )-----*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <OneWire.h>
#include <avr/wdt.h>

// DS18S20 Temperature chip i/o


/*-----( Declare Constants and Pin Numbers )-----*/

/** 
* int
* choose the input pin (for a pushbutton)
*/

#define CE_PIN   9
#define CSN_PIN 10
#define DS 2
#define RELAY A0

OneWire ds(DS);

// NOTE: the "LL" at the end of the constant is "LongLong" type
const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe

/*-----( Declare objects )-----*/
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
/*-----( Declare Variables )-----*/

void setup()   /****** SETUP: RUNS ONCE ******/
{
  Serial.begin(9600);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  delay(1000);
  digitalWrite(RELAY, HIGH);
  radio.begin();
  radio.openWritingPipe(pipe);
  wdt_enable(WDTO_4S);
  Serial.println("Board start");
}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  wdt_reset();
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  

  ds.reset_search();
  if ( !ds.search(addr)) {
    Serial.print("No more addresses.\n");
    ds.reset_search();
    return;
  }

  Serial.print("R=");
  for ( i = 0; i < 8; i++) {
    Serial.print(addr[i], HEX);
    Serial.print(" ");
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
    Serial.print("CRC is not valid!\n");
    return;
  }

  if ( addr[0] == 0x10) {
    Serial.print("Device is a DS18S20 family device.\n");
  }
  else if ( addr[0] == 0x28) {
    Serial.print("Device is a DS18B20 family device.\n");
  }
  else {
    Serial.print("Device family is not recognized: 0x");
    Serial.println(addr[0], HEX);
    return;
  }
  //delay(5000);
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("P=");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print( OneWire::crc8( data, 8), HEX);
  Serial.println();
  radio.stopListening();

  //radio.write( data, sizeof(data) );
  radio.write( data, sizeof(data) );
  radio.startListening();

}//--(end main loop )---

/*-----( Declare User-written Functions )-----*/

//NONE
//*********( THE END )***********
