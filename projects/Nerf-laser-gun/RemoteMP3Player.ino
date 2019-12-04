/*
 * Sound player for laser gun.
 * 
 * Receives weapon request for sound from nRF24L01+ radio and play selected sound on DFPlayer Mini module.
 * Module 1 is for all gun sounds, module 2 is for playing Star trek theme song only. The 2 module audio
 * output are mixed so can sounds can be played concurrently. On the SD card, file 001 is the theme song, and
 * weapon sounds from file 002...max.
 */
  /*
 INTERNAL_PULLUP:
 +---->pin 3
 |
 \
 |
 +---->0V
*/

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "Button.h"
#define WEAPON_SELECT 8

RF24 radio(10,9);   //RX, TX
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };              // Radio pipe addresses for the 2 nodes to communicate.

ButtonCB btn1(WEAPON_SELECT, Button::INTERNAL_PULLUP);      //weapon selection mode
SoftwareSerial Serial1(4, 5); // RX, TX
SoftwareSerial Serial2(6, 7); // RX, TX
DFRobotDFPlayerMini myDFPlayer1;       //general purpose sound module
DFRobotDFPlayerMini myDFPlayer2;       //Star Trek theme song only
int weapon;
const int MIN_WEAPON = 2;
const int MAX_WEAPON = 31;


void setup()
{
  Serial.begin(57600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  printf_begin();
  Serial.println("");
  Serial.println(F("Laser Gun Sound - receives radio request from Laser Gun to play soundfx."));

  Serial2.listen();
  if (!myDFPlayer2.begin(Serial2,true,false)) {  //Use softwareSerial to communicate with mp3. true ACK, false no RESET
    Serial.println(F("Unable to begin player 2:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  else {
    myDFPlayer2.volume(0x20);      //Set volume value. From 0 to 30
    myDFPlayer2.playFolder(1,25);  //Play sound test on player2
  }
  Serial1.listen();
  if (!myDFPlayer1.begin(Serial1,true,false)) {  //Use softwareSerial to communicate with mp3. true ACK, false no RESET
    Serial.println(F("Unable to begin player 1:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  else {
    myDFPlayer1.volume(0x20);     //Set volume value. From 0 to 30
    myDFPlayer1.playFolder(1,4);  //Play sound test on player1
  }
  
  Serial.println(F("DFPlayers ready."));
  
  radio.begin();
  radio.setDataRate(RF24_2MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads to test the call-response speed
  radio.setRetries(0,15);                 // Smallest time between retries, max no. of retries
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();
  if (radio.isChipConnected())
    Serial.println(F("Radio ready."));
  else
    Serial.println(F("Radio Chip not connected"));
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging

  btn1.setPressHandler(onPressHoldFB);    //define button callback
  btn1.setHoldHandler(onPressHoldFB);
  btn1.setHoldRepeats(true);
  btn1.setHoldThreshold(500);
  weapon=MIN_WEAPON;              //default weapon
}

void onPressHoldFB(const Button&) {
    weapon++;                     //cycle thru all weapons from 2...max
    if (weapon > MAX_WEAPON)
      weapon=MIN_WEAPON;          //start from beginning if > max
    Serial1.listen();
    myDFPlayer1.playFolder(1,weapon);  //Play all other sounds on player1
    Serial.print("weapon selected: ");
    Serial.println(weapon);
}

void loop()
{
   btn1.process();

  if (radio.available() ) {
    byte recvbuf;
    radio.read( &recvbuf, sizeof(recvbuf) );
    Serial.print("Received: ");
    Serial.println(recvbuf);
    if (recvbuf == 1) {           //001.mp3 = Star Trek theme song
      Serial2.listen();
      myDFPlayer2.playFolder(1,recvbuf);  //Play theme song on player2
      /* debug
      if (myDFPlayer2.available()) {
        printDetail(myDFPlayer2.readType(), myDFPlayer2.read()); //Print the detail message from DFPlayer to handle different errors and states.
      }
      */
    }
    else if (recvbuf > 1) {
      Serial1.listen();
      myDFPlayer1.playFolder(1,recvbuf);  //Play all other sounds on player1
      /* debug
      if (myDFPlayer1.available()) {
        printDetail(myDFPlayer1.readType(), myDFPlayer1.read()); //Print the detail message from DFPlayer to handle different errors and states.
      }
      */
    }
  }
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  
}
