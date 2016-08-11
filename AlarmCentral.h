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
			void functioname();
		private:
			RCSwitch _mySwitch;
	};

#endif