//Adding the external libraries that are needed:
/*
 * SD.h and SPI.h for the SDcard
 * SD card attached to SPI bus in an Arduino UNO R3 as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 ** For other boards it's necessary to change the AlarmCentral.h File
 * RCSwitch..h for the RF433Mhz Receptor
 ** RFReceiver is by default definited on pin 2
 * AlarmCentral.h for all the alarm control and actions
*/
#include <RCSwitch.h>
#include <SD.h>
#include <SPI.h>
#include <AlarmCentraL.h>

//Defining the RF433Mhz object
RCSwitch mySwitch = RCSwitch();

//Pins that will be used in the example
#define SENSOR_PIR1 14 // Analog 0
#define NEW_CONTROL_BUTTON 5 
#define SIREN 15 // Analog 1
#define RED_LED 16 // Analog 2
#define GREEN_LED 17 // Analog 3

void setup() {
  Serial.begin(9600);
}

