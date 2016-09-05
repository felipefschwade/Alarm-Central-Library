/*
 * SD.h and SPI.h for the SDcard
 * SD card attached to SPI bus in an Arduino UNO R3 as follows:
 * If you are using another Arduino, please switch the constants at
 * AlarmaCentral.h
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
#include <AlarmCentral.h>

//Defining the RF433Mhz object
RCSwitch mySwitch = RCSwitch();
AlarmCentral myCentral = AlarmCentral(mySwitch);

//Pins that will be used in the example
#define SENSOR_PIR1 14 // Analog 0
#define NEW_CONTROL_BUTTON 5 // Digital 5
#define SIREN 15 // Analog 1
#define RED_LED 16 // Analog 2
#define GREEN_LED 17 // Analog 3
void setup() {
  Serial.begin(9600);
  //For the led Pins, the first parameter is the greenLed, and the second parameter is the redLed (You might use another leds as you want)
  myCentral.setLedPins(GREEN_LED, RED_LED);
  //The PIR Sensors must be passed inside of array, with the quantity and the pins that I'll be used (Also you can put all of them in Serie and use just one pin)
  myCentral.addPIRSensor(SENSOR_PIR1);
  //Define the Siren Pin
  myCentral.setSirenPin(SIREN);
  //Define the newcontrol button
  myCentral.setNewControlButtonPin(NEW_CONTROL_BUTTON);
  //Initiate all pins and sensor as inputs or outputs
  myCentral.begin();
}

void loop() {
  //Get a possible signal from the Arduino inputs
  int receivedSignal = myCentral.getReceivedSignal();
  //Place the signal into the "Main" function of central, that I'll treat it according to it current state
  myCentral.treatReceivedSignal(receivedSignal);
}

