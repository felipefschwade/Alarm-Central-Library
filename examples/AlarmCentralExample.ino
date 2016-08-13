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
#define SDCARD 4
#define NEW_CONTROL_BUTTON 5 
#define SIREN 15 // Analog 1
#define RED_LED 16 // Analog 2
#define GREEN_LED 17 // Analog 3

//Defining all global scope variables
//The state control variable



void setup() {
  Serial.begin(9600);
  Serial.println("INICIADO!");
  initiatePins();
  //If the card isn't located the software will get into sleep mode.
  if (!SD.begin(SDCARD)) {
    SDReadFailed();
  }
  loadData();
  state = ALARM_OFF;
}

void loop() {
  int signalReceived = receivedSignal();
  switch (state) {
      case ALARM_OFF:
          if (signalReceived == CONTROL_SIGNAL) {
              setAlarmOn();
              break;
            } else if (signalReceived == NEW_CONTROL_BUTTON_PRESSED) {
              setNewControllAddingState();
              break;
            }
            ledBlink(GREEN_LED, 700);
      break;
      case ALARM_ON:
          if (signalReceived == CONTROL_SIGNAL) {
              setAlarmOff();
              break;
          } else if (signalReceived == SENSOR_SIGNAL) {
              startAlarm();
              break;
          }
          ledBlink(RED_LED, 700);
      break;
      case ALARM_STARTED:
          if (signalReceived == CONTROL_SIGNAL) {
                setAlarmOff();
                break;
              }
           ledBlink(RED_LED, 200);
      break;
      //Reset your arduino after adding a new control.
      case NEW_CONTROL_ADDING:
        addNewControl(signalReceived);
        break;
    } 
  }

void setAlarmOn() {
    turnOff(GREEN_LED);
    Serial.println("Alarm On");
    state = ALARM_ON;
    mySwitch.resetAvailable();
    //Setting a delay to avoid turning the alarm off accidentally
    delay(300);
    sirenBeep(1);
    Serial.println(state);
    turnOff(GREEN_LED);  
}

void setNewControllAddingState() {
      state = NEW_CONTROL_ADDING;
      Serial.println("New Control Adding");
      for (int i=0; i <= 2; i++) {
        Serial.println(i);
        turnOn(GREEN_LED);
        turnOn(RED_LED);
        delay(300);
        turnOff(GREEN_LED);
        turnOff(RED_LED);
        delay(200);
      }
}
/** 
*
*
*
**/
void setAlarmOff() {
    Serial.println("Alarm Off");
    turnOff(SIREN); 
    //Delay to avoid an accidental alarm activitation while the control button is pressed
    delay(300);
    mySwitch.resetAvailable();
    state = ALARM_OFF;
    turnOff(RED_LED);
    sirenBeep(2);
    turnOff(RED_LED);
}

void startAlarm() {
    state = ALARM_STARTED;  
    Serial.println(state);
    Serial.println("Alarm STARTED");
    turnOn(SIREN);
}


//Insert a new control into the SDCard
void addNewControl(int signalReceived) {
  boolean flag = 0; //Set a flag that I'll be used to detect a user interation
        if (mySwitch.available()) { //Search for a RF433Mhz signal
         turnOn(GREEN_LED);
         new_control = mySwitch.getReceivedValue(); //put the received signal code into a new variable
         Serial.println(new_control); //Print the code (For debugging only)
         myFile = SD.open("codes.txt", FILE_WRITE); //Open the codes file to write the new code into the file
        // if the file opened okay, write to it
        if (myFile) {
          //Debuging messages
          Serial.print("Writing the new code into the codes.txt..."); 
          myFile.println(new_control);
          myFile.close(); //Close the readed file
          Serial.println("Control Code save with success."); //More debuging Message
          loadData(); //Reload the data into the Arduino RAM
          //Make a loop to indicate using led blink that the control were successfull saved
          for (int i=0; i <= 10; i++) {
            //Proposital delay for avoid a accindetal Alarm Set while adding a control
            turnOn(GREEN_LED);
            delay(100);
            turnOff(GREEN_LED);
            delay(100);
            flag = 1; //Mark the flag as true
           }
        } else {
          //Lock the file again if something went wrong
          SDOpenFileFailed(); 
         }
       } else if (signalReceived == NEW_CONTROL_BUTTON_PRESSED) { //Read the NEW_CONTROL_BUTTON STATE
          //Delay for the user don't accidetaly get again into this state
          delay(1000);
          flag = 1; //Mark the flag as tru
       }
      if (flag == 1) {
        mySwitch.resetAvailable(); //Reset the old receiver RF433Mhz code 
        Serial.println("Alarm Off"); //Debuggin Message
        state = ALARM_OFF; //Set the next State
      }  
}
