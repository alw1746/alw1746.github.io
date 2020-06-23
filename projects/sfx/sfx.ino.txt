/*
 Play door sound on power up. Sound file must be created on a FAT32 SD card in a top-level dir:
   01\001.mp3
 A 3.3V Arduino is used as the dfplayer i/o lines are 3.3V logic level. But the player itself
 needs 5V. Speaker must be < 3W if no sound or crackling is heard, speaker is not usable. 
 Verify sound output through a headphone connected to DAC_1,DAC_2. Power supply must be
 capable of >= 80mA draw by the player's amp.
*/
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFPlayerMini.h>   //Sketch include lib -> install DFPlayerMini by Arsen Torbarina
#define busyPin 9 //dummy
#define TX 8     //send to player
#define RX 7     //recv from player

SoftwareSerial Serial2(RX, TX); // RX, TX ports
DFPlayerMini sfxplayer;       //sound module

void setup() {
  Serial.begin(9600);     //serial monitor
  Serial2.begin(9600);    //software serial1 port
  Serial.println(F("Sound Test."));

  sfxplayer.init(busyPin,Serial2,false);
  sfxplayer.setVolume(0x12);        //Set volume
  sfxplayer.playFileAndWait(1,1);   //Play sound
}

void loop() {
}
