/*
	AlarmCentral.h
	written by Felipe Faé Schwade
	2016, August 8.
	MIT license

*/
//Incluindo as bibliotecas de terceiros
#include <Arduino.h>
#include <RCSwitch.h>
#include <SD.h>
#include <SPI.h>
#ifndef AlarmCentral_h
#define AlarmCentral_h
	//Flag utilizada para marcar um estado indefindo
	#define UNDEFINED -1
	/**
		Pinos definidos para o Arduino UNO R3,
		se você utiliza outra placa mude os respectivos
		pinos.
	*/
	#define SD_PIN 4
	#define SD_MOSI 11
	#define SD_MISO 12
	#define SD_CLK 13 

	class AlarmCentral {
		public:
			AlarmCentral(RCSwitch mySwitch);
			void addPIRSensor(int sensor);
			void setLedPins(int greenLed, int redLed);
			void setSirenPin(int sirenPin);
			void setNewControlButtonPin(int newControlButtonPin);
			void begin();
			int getReceivedSignal();
			void treatReceivedSignal(int receivedSignal);
		private:
			RCSwitch _mySwitch;
			int _signalReceived;
			int _newControlButton;
			int _PIRSensors[20];
			int _greenLed;
			int _redLed;
			int _sirenPin;
			int _PIRqty = 1;
			int _controlsqty;
			long int _new_control;
			unsigned long _currentMillis;
			unsigned long _previousMillis;
			int _state;
			//Enum com os possiveis estados
			enum _Status {
			    ALARM_OFF,
			    ALARM_ON,
			    ALARM_STARTED,
			    NEW_CONTROL_ADDING
			  };
			  
			//Enum com todos os sinais possíveis
			enum _receivedSignal {
			    CONTROL_SIGNAL,
			    SENSOR_SIGNAL,
			    NEW_CONTROL_BUTTON_PRESSED
			  };
			//Definição do arquivo onde ficarão os codigos dos controles
			File _myFile;
			long int _controls[];
			void ledBlink(int _led, int s_peed_milis);
			void turnOn(int _pin);
			void turnOff(int _pin);
			void sirenBeep(int _times);
			void SDOpenFileFailed();
			void SDReadFailed();
			void loadData();
			void setAlarmOn();
			void setAlarmOff();
			void startAlarm();
			void addNewControl(int signalReceived);
			void setNewControlAddingState();
			void verifyPin(int pin);
	};

#endif