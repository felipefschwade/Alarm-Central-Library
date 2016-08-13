/*
	AlarmCentral.cpp
	written by Felipe Faé Schwade
	2016, August 8.
	MIT license

*/

#include "Arduino.h"
#include "AlarmCentral.h"

/**
*	Class Consctructor
*	Receive the RCSwitch Object as a Parameter	
*/
AlarmCentral::AlarmCentral(RCSwitch mySwitch) {
	_mySwitch = mySwitch;
	//Enable Receive on interrupt 0, by default it is on Pin2
	_mySwitch.enableReceive(0);
	//The timer to use on Blinks.
	_previousMillis = 0;
}

/**
	Set the pins that will be used as PIR sensors Receiver
	you might put all the sensors in a serie and use it in only
	one Arduino PIN. But if you want a better feedback from the central
	you can set one pin for each sensor.
*/
void AlarmCentral::setPIRSensors(int sensors[]) {
	for (int i = 0; i < sizeof(sensors); ++i) {
		_PIRSensors[i] = sensors[i];
	}
}

/**
	Set the 2 pins for the Output leds;
*/
void AlarmCentral::setLedPins(int greenLed, int redLed) {
	_greenLed = greenLed;
	_redLed = redLed;
}

/**
	Set the Siren Pin output
*/
void AlarmCentral::setSirenPin(int sirenPin) {
	_sirenPin = sirenPin;
}
/**
	Define the new_controll Button input pin
*/
void setNewControlButtonPin(int newControlButtonPin)

/**
	Create all the structure needed to make the AlarmCentral works
*/
void AlarmCentral::begin() {
	for (int i = 0; i < sizeof(_PIRSensors); ++i) {
		pinMode(_PIRSensors[i], INPUT);
	}
	pinMode(_greenLed, OUTPUT);
	pinMode(_redLed, OUTPUT);
	pinMode(_sirenPin, OUTPUT);
}
/**
	<----------------------------------- Private Functions ------------------------------------->
*/

/**
	Searche for any king of signal and return it,
	if no one signal defineted on _receivedSignals var
	it retunrs a UNDEFINED
*/
int AlarmCentral::getReceivedSignal() {
       if (_mySwitch.available()) {
              Serial.println(_mySwitch.getReceivedValue()); // Debug Only
              Serial.println(); // Debug Only
              for (int i=0; i < 21; i++) {
                  Serial.println(i); //Debug only
                  Serial.println(_controls[i]); //Debug only
                  if (_controls[i] == _mySwitch.getReceivedValue()) {
                    Serial.println("Control Signal");
                    _mySwitch.resetAvailable();
                    //Delay to slow down the RFsignal reading
                    delay(500);
                    return CONTROL_SIGNAL; 
                  }
              }
              _mySwitch.resetAvailable();
          }
        if (digitalRead(NEW_CONTROL_BUTTON) == 0) {
            return NEW_CONTROL_BUTTON_PRESSED;
        }
       if (digitalRead(SENSOR_PIR1) == 0) {
             return SENSOR_SIGNAL; 
        }
      return UNDEFINED;
}

/**
	Make a led blink after a Ms definited time
*/
void AlarmCentral::ledBlink(int led, int speed_milis) {

   int state = digitalRead(led);
   const long interval = speed_milis; 
   _currentMillis = millis();
    if (_currentMillis - _previousMillis >= interval) {
      _previousMillis = _currentMillis;
      //Invert the LED state, it always will make a blink
      digitalWrite(led, !state);
    }
}
/**
	Put a pin on HIGH logic state
*/
void AlarmCentral::turnOn(int pin) {
	digitalWrite(pin, HIGH);
}
/**
	Put a pin on LOW logic state
*/
void AlarmCentral::turnOff(int pin) {
	digitalWrite(pin, LOW);
}
/**
	For the Siren used in Production, a 300ms delay
	is the time for a exactly one Beep, and the delay is used to
	lock up the processor and avoid any accidental UI
*/
void AlarmCentral::sirenBeep(int times) {
	turnOn(_sirenPin);
	delay(times * 300);
	turnOff(_sirenPin);  
}
/**
	Make the code get locked and light up a LED
	to indicate the user some problem with the SD
	Card file reading
*/
void AlarmCentral::SDOpenFileFailed() {
  // if the file didn't open, print an error and stay:
    Serial.println("Error opening codes.txt, please review your SD Card");
    turnOn(GREEN_LED);
    delay(999999999999999);
}
/**
	Make the code get locked and light up a LED
	to indicate the user some problem with the SD
	Card reading
*/
void AlarmCentral::SDReadFailed() {
    Serial.println("Initialization Failed! Please verify your SD Card and try Again");
    digitalWrite(RED_LED, HIGH);
    delay(9999999999999999);
}

/**
	Load all the data from de SD card and put it into the Arduino RAM
	for a better perfomance
*/
void loadData() {
  _myFile = SD.open("codes.txt", FILE_WRITE);
  // Open the file for reading:
  _myFile = SD.open("codes.txt");
  if (_myFile) {
    // read from the file until there's nothing else in it:
    int i = 0;
    while (_myFile.available()) {
     Serial.println("Lendo o arquivo"); //Debug Message
     _controls[i] = _myFile.parseInt();
     Serial.println(_controls[i]); //Debug Message
     i++;
    }
    // close the file:
    _myFile.close();
  } else {
    SDOpenFileFailed();
  }
}