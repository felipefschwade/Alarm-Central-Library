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
			void AlarmCentral::setLedPins(int greenLed, int redLed);
		private:
			RCSwitch _mySwitch;
			int _PIRSensors;
			int _greenLed;
			int _redLed;

	};

#endif