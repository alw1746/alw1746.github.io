 /*
Sound effects for laser gun.

Wire an Arduino Pro Mini with 3 button switches and an nRF24L01+ radio transceiver. The switches permit:
 -trigger laser gun sound fx
 -play Star trek theme song
 -cycle thru a variety weapons
When a weapon is triggered, the weapon number is sent to a remote DFPlayer Mini which plays the corresponding
mp3/wav file. A laser beam is triggered as well when the laser gun is fired.

This setup is placed inside a hacked toy laser gun and the trigger used as switch.
*/
 
 /*
 INTERNAL_PULLUP:
 +---->pin 6
 |
 \
 |
 +---->0V
*/

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "Button.h"

#define WEAPON_SELECT 5
#define THEME 6
#define TRIGGER 7
#define LASER 8
#define CSPIN 9
#define CEPIN 10
#define MOSI 11
#define MISO 12
#define SCK 13

// Hardware configuration: Set up nRF24L01 radio on SPI bus
RF24 radio(CEPIN,CSPIN);

ButtonCB btn1(THEME, Button::INTERNAL_PULLUP);            //theme song
ButtonCB btn2(TRIGGER, Button::INTERNAL_PULLUP);          //gun trigger
Button btn3(WEAPON_SELECT, Button::INTERNAL_PULLUP);      //weapon selection mode

long startTimer;
bool shotFired=false;
int weapon;
bool cycleWeapons=false;
const int MIN_WEAPON = 2;
const int MAX_WEAPON = 31;

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };              // Radio pipe addresses for the 2 nodes to communicate.

void setup(){
  pinMode(LASER, OUTPUT);       //laser pin
  Serial.begin(57600);
  printf_begin();
  Serial.println("");
  Serial.println(F("Laser Gun - fires laser and send radio request to Laser Gun Sound for soundfx."));
  // Setup and configure rf radio

  radio.begin();                          //init nRF24L01+ radio
  radio.setDataRate(RF24_2MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads to test the call-response speed
  radio.setRetries(0,15);                 // Smallest time between retries, max no. of retries
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  if (radio.isChipConnected())
    Serial.println(F("Radio ready."));
  else
    Serial.println(F("Radio Chip not connected"));
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging

  btn2.setPressHandler(onPressHoldFB);    //define button callback
  btn2.setHoldHandler(onPressHoldFB);
  btn2.setHoldRepeats(true);
  btn2.setHoldThreshold(500);

  btn1.setPressHandler(onPressHoldTB);
  btn1.setHoldHandler(onPressHoldTB);
  btn1.setHoldRepeats(true);
  btn1.setHoldThreshold(500);
  
  btn3.setHoldThreshold(500);

  weapon=5;          //default weapon
  sfxRequest(4);     //laser charging sound on powerup.

}

void sfxRequest(byte soundfileNumber) {
  radio.stopListening(); 
  byte soundfile=soundfileNumber;    //select 00n.mp3
  if (!radio.write( &soundfile, sizeof(soundfile) )) {     //tell DFPlayer Mini to play this
    Serial.print(F("Radio write failed: "));
    Serial.println(soundfile);
  }
}

void onPressHoldFB(const Button&) {
  Serial.println("Fire weapon");
  if (btn3.isDown()) {            //weapons selection button down?
    weapon++;                     //cycle thru all weapons from 2...max
    if (weapon > MAX_WEAPON)
      weapon=MIN_WEAPON;          //start from beginning if > max
    sfxRequest(weapon);           //play sound fx
    Serial.print("weapon selected: ");
    Serial.println(weapon);
  }
  else {
    sfxRequest(weapon);
    shotFired=true;
    startTimer=millis();
    digitalWrite(LASER, HIGH);     //turn on laser beam
  }
}

void onPressHoldTB(const Button&) {
  Serial.println("play theme");
  sfxRequest(1);    //play theme as background music
}

void loop(void) {
  btn1.process();
  btn2.process();
  btn3.process();
  if (shotFired) {                          //if weapon is fired,
    if (millis() > (startTimer + 500)) {    //if beam time expired after 500ms,
      digitalWrite(LASER, LOW);             //disable laser beam
      shotFired=false;
    }
  }
}
