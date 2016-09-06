/*
	AlarmCentral.cpp
	written by Felipe Faé Schwade
	2016, August 8.
	MIT license

*/

#include "Arduino.h"
#include "AlarmCentral.h"

/**
*	Construtor da classe
*	Constroi um objeto Alarm central.
* @param mySwtich O objeto RCSwtich responsável pela decodificação 433Mhz.
*/
AlarmCentral::AlarmCentral(RCSwitch mySwitch) {
	_mySwitch = mySwitch;
	//Ativa a recepção no interrupt 0, por padrão é o pino 2.
	_mySwitch.enableReceive(0);
	//Contador utilizado para o led blink.
	_previousMillis = 0;
}

/**
	Define os pinos que irão funcionar como sensores de IR, você
  pode adicionar varios sensores em um único pino se os ligar em série.
  @param sensor O pino de entrada que será um sensor
*/
void AlarmCentral::addPIRSensor(int sensor) {
      _PIRSensors[_PIRqty - 1] = sensor;
      _PIRqty++;
}

/**
	Define os dois pinos de saída para os leds de interface HM.
  @param greenLed,redLed Os pinos de saída do led verde e vermelho.
*/
void AlarmCentral::setLedPins(int greenLed, int redLed) {
	_greenLed = greenLed;
	_redLed = redLed;
}

/**
	Define o da sirene.
  @param sirenPin O pino de saída aonde será ligada a sirene
*/
void AlarmCentral::setSirenPin(int sirenPin) {
	_sirenPin = sirenPin;
}
/**
	Define o botão de adicionar novos controles
  @param newControlButtonPin O pino de entrada do botão de adicionar controles.
*/
void AlarmCentral::setNewControlButtonPin(int newControlButtonPin) {
	_newControlButton = newControlButtonPin;
}

/**
	Cria a infra-estrutura responsável pelo funcionamento da central
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
	pinMode(_newControlButton, INPUT_PULLUP); //Embora esteja sendo utilizado o input pull up, coloque um resistor de 10KOhm se tiver problemas.
	verifyPin(_newControlButton);
   if (!SD.begin(SD_PIN)) {
      SDReadFailed();
   }
  _myFile = SD.open("codes.txt", FILE_WRITE);
  _myFile.close();
  loadData();
	_state = ALARM_OFF;
}
/**
	Procura pelos sinais possíveis e retorna
  qual foi o sinal recebido, se nenhum sinal for
  recebido, retorna UNDEFINED
*/
int AlarmCentral::getReceivedSignal() {
	   //Procura por sinais RF 433Mhz
       if (_mySwitch.available()) {
              Serial.println(_mySwitch.getReceivedValue()); // Mensagem de Debug
              Serial.println(); // Mensagem de Debug
              Serial.println(sizeof(_controls)); //Mensagem de Debug
              for (int i=0; i < _controlsqty; i++) {
                  Serial.println(i); //Mensagem de Debug
                  Serial.println(_controls[i]); //Mensagem de Debug
                  if (_controls[i] == _mySwitch.getReceivedValue()) {
                    Serial.println("Control Signal");
                    _mySwitch.resetAvailable();
                    //Delay proposital para reduzir a velocidade de leitura da RF
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
	Como a central funciona como uma máquina de estados,
  cada ação sera tomada de acordo com o estado atual e o
  sinal recebido.
  @Param receivedSignal Um sinal dentro do Enum definido no arquivo.h
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
      case NEW_CONTROL_ADDING:
        addNewControl(receivedSignal);
        break;
    } 
}
/**
	<----------------------------------- Private Functions ------------------------------------->
*/

/**
  Faz um led piscar após o tempo determinado em 
  milisegundos.
  @param led O pino do led escolhido
  @param speed_milis A velocidade que ele deverá piscar em Ms
*/
void AlarmCentral::ledBlink(int led, int speed_milis) {

   int state = digitalRead(led);
   const long interval = speed_milis; 
   _currentMillis = millis();
    if (_currentMillis - _previousMillis >= interval || _currentMillis - _previousMillis < 0) {
      _previousMillis = _currentMillis;
      //Invert the LED state, it always will make a blink
      digitalWrite(led, !state);
    }
}
/**
  Coloca um pino em nível lógico alto
  @param pin O pino de saída escolhido
*/
void AlarmCentral::turnOn(int pin) {
	digitalWrite(pin, HIGH);
}
/**
  Coloca um pino em nível lógico baixo
  @param pin O número do pino escolhido
*/
void AlarmCentral::turnOff(int pin) {
	digitalWrite(pin, LOW);
}
/**
  Para a sirene usada na prototipagem, cada "Beep" ocorria
  em um periodo de 300Ms, assim, foi extraida uma função que
  faz a sirene tocar X Beeps
  @param times O número de vezes que a sirene irá "Beepar"
*/
void AlarmCentral::sirenBeep(int times) {
	turnOn(_sirenPin);
	delay(times * 300);
	turnOff(_sirenPin);  
}
/**
  Liga o led verde e trava o processamento, indicando
  que ocorreu algum problema durante a leitura do arquivo.
*/
void AlarmCentral::SDOpenFileFailed() {
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
   //Trava o processamento em um loop infinito
    while(1) {
    }
}

/**
	Carrega os códigos dos controles no SD para
  a memória RAM do Arduino
*/
void AlarmCentral::loadData() {
  _myFile = SD.open("codes.txt");
  if (_myFile) {
    int i = 0;
    while (_myFile.available()) {
     Serial.println("Lendo o arquivo");
     _controls[i] = _myFile.parseInt();
     Serial.println(_controls[i]);
     i++;
    }
    _controlsqty = i;
    Serial.println(_controlsqty);
    _myFile.close();
  } else {
    SDOpenFileFailed();
  }
}

/**
  Verifica se o Pino escolhido não é nenhum
  pino do protocolo MISO ou o cartão SD
  @param pin O pino que seŕa testado
*/
void AlarmCentral::verifyPin(int pin) {
  //Valida os pinos
    if ( pin == SD_PIN ||
       pin == SD_MOSI ||
       pin == SD_MISO ||
       pin == SD_CLK) {
      /**
      Faz um loop infino com o LED do pino 13 piscando para indicar
      que algum pino está errado.
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
	Modifica o estado da central para
  Alarme Ligado, produz sons e muda o 
  led aceso para interface HM
*/
void AlarmCentral::setAlarmOn() {
    turnOff(_greenLed);
    Serial.println("Alarm On");
    _state = ALARM_ON;
    _mySwitch.resetAvailable(); //Reinicia o ultimo codigo recebido pelo receptro 433Mhz
    //Define um delay proposital para evitar o re-acionamento acidental da central
    delay(300);
    sirenBeep(1);
    Serial.println(_state); 
}
/**
	Modifica o estado da central para
  Alarme Desligado, produz sons e muda o 
  led aceso para interface HM
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
	Modifica o estado da central para alarme disparado
*/
void AlarmCentral::startAlarm() {
    _state = ALARM_STARTED;  
    Serial.println(_state);
    Serial.println("Alarm STARTED");
    turnOn(_sirenPin);
}
/**
	Insere um novo código no arquivo de controles
  @param receivedSignal Sinal utilizado para a central voltar ao modo desligado caso o botão de adicionar novo controle seja pressionado novamente
*/
void AlarmCentral::addNewControl(int _receivedSignal) {
  boolean flag = 0; //Define uma flag para detectar interação do usuário
        if (_mySwitch.available()) { //Procura por um sinal 433Mhz disponível
         turnOn(_greenLed);
         _new_control = _mySwitch.getReceivedValue();
         Serial.println(_new_control);
         _myFile = SD.open("codes.txt", FILE_WRITE);
        if (_myFile) {
          Serial.print("Writing the new code into the codes.txt..."); 
          _myFile.println(_new_control);
          _myFile.close();
          Serial.println("Control Code save with success.");
          loadData();
          for (int i=0; i <= 5; i++) {
            turnOn(_greenLed);
            delay(100);
            turnOff(_greenLed);
            delay(100);
            flag = 1; //Marca a flag como verdadeira
           }
        }
       } else if (_receivedSignal == NEW_CONTROL_BUTTON_PRESSED) {
          delay(1000);
          flag = 1;
       }
      if (flag == 1) {
        _mySwitch.resetAvailable();
        Serial.println("Alarm Off");
        _state = ALARM_OFF;
      }  
}
/**
	Pisca os leds para indicar a transicao de estados
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