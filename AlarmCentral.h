/*
	AlarmCentral.h
	written by Felipe Faé Schwade
	2016, August 8.
	MIT license

*/

#ifndef AlarmCentral_h
#define AlarmCentral_h
	//A flag used to return undefined received signal
	#define UNDEFINED -1
	#include "Arduino.h"
	class AlarmCentral {
		public:
			AlarmCentral(RCSwitch mySwitch);
			void setPIRSensors(int *sensors);
			void setLedPins(int greenLed, int redLed);
			void AlarmCentral::setSirenPin
		private:
			RCSwitch _mySwitch;
			int _PIRSensors;
			int _greenLed;
			int _redLed;
			int _sirenPin;
			//The new control variable for a better code understanding
			long int _new_control;
			unsigned long _currentMillis;
			unsigned long _previousMillis;
			int _state;
			//The possible status enum
			enum _Status {
			    ALARM_OFF,
			    ALARM_ON,
			    ALARM_STARTED,
			    NEW_CONTROL_ADDING
			  };
			  
			//All the possible received signals
			enum _receivedSignal {
			    CONTROL_SIGNAL,
			    SENSOR_SIGNAL,
			    NEW_CONTROL_BUTTON_PRESSED
			  };

			//Define the file containing all control codes
			File _myFile;

			//TODO Auto control codes generation
			//Here you put the quantity of controls that you want in you 
			long int _controls*;
			void AlarmCentral::ledBlink(int _led, int s_peed_milis);
			void AlarmCentral::turnOn(int _pin);
			void AlarmCentral::turnOff(int _pin);
			void AlarmCentral::sirenBeep(int _times);

	};

#endif