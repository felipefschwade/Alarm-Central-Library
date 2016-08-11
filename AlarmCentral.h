/*
	AlarmCentral.h
	written by Felipe Faé Schwade
	2016, August 8.
	MIT license

*/

#ifndef AlarmCentral_h
#define AlarmCentral_h

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
			unsigned long _currentMillis;
			unsigned long _previousMillis;
			void AlarmCentral::ledBlink(int _led, int s_peed_milis);
			void AlarmCentral::turnOn(int _pin);
			void AlarmCentral::turnOff(int _pin);
			void AlarmCentral::sirenBeep(int _times);

	};

#endif