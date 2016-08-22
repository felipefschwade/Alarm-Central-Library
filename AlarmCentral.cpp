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
  if (!SD.begin(SD_PIN)) {
      SDReadFailed();
   }
  _myFile = SD.open("codes.txt", FILE_WRITE);
  _myFile.close();
}

/**
	Set the pins that will be used as PIR sensors Receiver
	you might put all the sensors in a serie and use it in only
	one Arduino PIN. But if you want a better feedback from the central
	you can set one pin for each sensor.
*/
void AlarmCentral::addPIRSensor(int sensor) {
      _PIRSensors[_PIRqty - 1] = sensor;
      _PIRqty++;
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
void AlarmCentral::setNewControlButtonPin(int newControlButtonPin) {
	_newControlButton = newControlButtonPin;
}

/**
	Create all the structure needed to make the AlarmCentral works
*/
void AlarmCentral::begin() {
	for (int i = 0; i < _PIRqty; ++i) {
    Serial.println(_PIRSensors[i]);
    verifyPin(_PIRSensors[i]);
		pinMode(_PIRSensors[i], INPUT);
	}
	pinMode(_greenLed, OUTPUT);
  verifyPin(_greenLed);
	pinMode(_redLed, OUTPUT);
  verifyPin(_redLed);
	pinMode(_sirenPin, OUTPUT);
  verifyPin(_sirenPin);
	pinMode(_newControlButton, INPUT_PULLUP); //Using the arduino internal pullUp, use a own 10K resistor if you had some trouble
	verifyPin(_newControlButton);
  loadData();
	_state = ALARM_OFF;
}
/**
	Searche for any king of signal and return it,
	if no one signal defineted on _receivedSignals var
	it retunrs a UNDEFINED
*/
int AlarmCentral::getReceivedSignal() {
	   //Search for a RF433Mhz singal
       if (_mySwitch.available()) {
              Serial.println(_mySwitch.getReceivedValue()); // Debug Only
              Serial.println(); // Debug Only
              Serial.println(sizeof(_controls)); //Debug Only
              for (int i=0; i < _controlsqty; i++) {
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
        if (digitalRead(_newControlButton) == 0) {
            return NEW_CONTROL_BUTTON_PRESSED;
        }
        for (int i = 0; i < _PIRqty; ++i) {
        	if (digitalRead(_PIRSensors[i]) == 0) {
	            return SENSOR_SIGNAL; 
	        }
        }    	
      return UNDEFINED;
}
/**
	The alarm works as a state machine, it 
	do it's actions according with the received
	signal and it current state
*/
void AlarmCentral::treatReceivedSignal(int receivedSignal) {
  switch (_state) {
      case ALARM_OFF:
          if (receivedSignal == CONTROL_SIGNAL) {
              setAlarmOn();
              break;
            } else if (receivedSignal == NEW_CONTROL_BUTTON_PRESSED) {
              setNewControlAddingState();
              break;
            }
            ledBlink(_greenLed, 700);
      break;
      case ALARM_ON:
          if (receivedSignal == CONTROL_SIGNAL) {
              setAlarmOff();
              break;
          } else if (receivedSignal == SENSOR_SIGNAL) {
              startAlarm();
              break;
          }
          ledBlink(_redLed, 700);
      break;
      case ALARM_STARTED:
          if (receivedSignal == CONTROL_SIGNAL) {
                setAlarmOff();
                break;
              }
           ledBlink(_redLed, 200);
      break;
      //Reset your arduino after adding a new control.
      case NEW_CONTROL_ADDING:
        addNewControl(receivedSignal);
        break;
    } 
}
/**
	<----------------------------------- Private Functions ------------------------------------->
*/

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
    turnOn(_greenLed);
    //Lock de processor into a infinite loop
    while(1) {
    }
}
/**
	Make the code get locked and light up a LED
	to indicate the user some problem with the SD
	Card reading
*/
void AlarmCentral::SDReadFailed() {
    Serial.println("Initialization Failed! Please verify your SD Card and try Again");
    turnOn(_redLed);
    //Lock de processor into a infinite loop
    while(1) {
    }
}

/**
	Load all the data from de SD card and put it into the Arduino RAM
	for a better perfomance
*/
void AlarmCentral::loadData() {
  // Open the file for create a codes.txt
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
    _controlsqty = i;
    Serial.println(_controlsqty);// Debug
    // close the file:
    _myFile.close();
  } else {
    SDOpenFileFailed();
  }
}

void AlarmCentral::verifyPin(int pin) {
  //Validate if the defined pins aren't any pin of the SPI protocol
    if ( pin == SD_PIN ||
       pin == SD_MOSI ||
       pin == SD_MISO ||
       pin == SD_CLK) {
      /**
      Create a infinite loop to make the user reset the Board and replace de pins,
      In this loop the Arduino Led and RX will blink fast.
      */
      pinMode(13, OUTPUT);
      while(1) {
        Serial.println("Invalid PIN, the pins: 13,11,12 and 4 Are for the SDCard Only, please switch the pins");
        digitalWrite(13, HIGH);
        delay(200);
        digitalWrite(13, LOW);
      }
    }
}

/*
--------------------------------------- ALARM STATE TRANSITION FUNCTIONS -------------
*/
/**
	Set the state of the central Alarm and do
	a noise and blink signals for the user for
	HM comunication
*/
void AlarmCentral::setAlarmOn() {
    turnOff(_greenLed);
    Serial.println("Alarm On"); //Debugging Message
    _state = ALARM_ON;
    _mySwitch.resetAvailable(); //Reset the RF433Mhz last code
    //Setting a delay to avoid turning the alarm off accidentally
    delay(300);
    sirenBeep(1);
    Serial.println(_state); 
}
/**
	Set the state of the central Alarm and do
	a noise and blink signals for the user for
	HM comunication
*/
void AlarmCentral::setAlarmOff() {
    Serial.println("Alarm Off"); // Debuggin Message
    //Always turn on the siren off
    turnOff(_sirenPin); 
    //Delay to avoid an accidental alarm activitation while the control button is pressed
    delay(300);
    _mySwitch.resetAvailable();
    _state = ALARM_OFF;
    turnOff(_redLed);
    sirenBeep(2);

}
/**
	Set the state of the central Alarm and do
	activate the siren to make noise
*/
void AlarmCentral::startAlarm() {
    _state = ALARM_STARTED;  
    Serial.println(_state);
    Serial.println("Alarm STARTED");
    turnOn(_sirenPin);
}
/**
	Insert a new control into the codes.txt file
	and reload all the data into the arduino RAM
*/
void AlarmCentral::addNewControl(int _receivedSignal) {
  boolean flag = 0; //Set a flag that I'll be used to detect a user interation
        if (_mySwitch.available()) { //Search for a RF433Mhz signal
         turnOn(_greenLed);
         _new_control = _mySwitch.getReceivedValue(); //put the received signal code into a new variable
         Serial.println(_new_control); //Print the code (For debugging only)
         _myFile = SD.open("codes.txt", FILE_WRITE); //Open the codes file to write the new code into the file
        // if the file opened okay, write to it
        if (_myFile) {
          //Debuging messages
          Serial.print("Writing the new code into the codes.txt..."); 
          _myFile.println(_new_control);
          _myFile.close(); //Close the readed file
          Serial.println("Control Code save with success."); //More debuging Message
          loadData(); //Reload the data into the Arduino RAM
          //Make a loop to indicate using led blink that the control were successfull saved
          for (int i=0; i <= 5; i++) {
            //Proposital delay for avoid a accindetal Alarm Set while adding a control
            turnOn(_greenLed);
            delay(100);
            turnOff(_greenLed);
            delay(100);
            flag = 1; //Mark the flag as true
           }
        }
       } else if (_receivedSignal == NEW_CONTROL_BUTTON_PRESSED) { //Read the NEW_CONTROL_BUTTON STATE
          //Delay for the user don't accidetaly get again into this state
          delay(1000);
          flag = 1; //Mark the flag as tru
       }
      if (flag == 1) {
        _mySwitch.resetAvailable(); //Reset the old receiver RF433Mhz code 
        Serial.println("Alarm Off"); //Debuggin Message
        _state = ALARM_OFF; //Set the next State
      }  
}
/**
	Do some blinks for indicate the state transition
*/
void AlarmCentral::setNewControlAddingState() {
      _state = NEW_CONTROL_ADDING;
      Serial.println("New Control Adding");
      for (int i=0; i <= 2; i++) {
        Serial.println(i);
        turnOn(_greenLed);
        turnOn(_redLed);
        delay(300);
        turnOff(_greenLed);
        turnOff(_redLed);
        delay(200);
      }
}