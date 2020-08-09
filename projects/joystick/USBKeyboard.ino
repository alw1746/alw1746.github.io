/*
  Control analog joystick using the HID driver of the o/s ie no specific device driver required.
  Joystick simulates pressing WASD keys for gaming purposes, typically moving camera view.
  Joystick switch sends out special code. Onboard led lights up when switch is pressed as a
  visual confirmation. Handles simultaneous WD,WA,SA,SD presses for NE,NW,SW,SE movements.
  Board support: ATmega32U4(Leonardo,Micro), Digispark(ATTiny85). STM32 not working, unknown USB descriptor.
  For STM32, Tools -> USB Support: HID (keyboard and mouse).
  
  Note: game engines process keypresses(up/down) from the keyboard, not characters (keypress codes are not the same
  as character codes). When 2 keys such as W and D are pressed simultaneously, the game engine detects 2 keypresses and
  will move the camera in a NE direction. When W is released, the D keypress remains in effect with camera moving E.
 
  MIT License
  Copyright (c) 2020 Alex Wong
  
 */
#if defined(__AVR_ATmega32U4__) || defined(ARDUINO_ARCH_STM32)
  #include "Keyboard.h"
  #if defined(ARDUINO_AVR_MICRO)
    #define LED_BUILTIN 17     //RXLED
    #define JOYSWITCH 21
    #define ANALOGX A1
    #define ANALOGY A2
  #elif defined(ARDUINO_ARCH_STM32)
    #define LED_BUILTIN PC13
    #define JOYSWITCH PA2
    #define ANALOGX PA0
    #define ANALOGY PA1
  #endif
  #define UP 'w'
  #define DOWN 's'
  #define LEFT 'a'
  #define RIGHT 'd'
  #define SWCODE ' '       //goto drop zone
#elif defined(ARDUINO_AVR_DIGISPARK)
  #include "DigiKeyboard.h"
  #define LED_BUILTIN 1
  #define JOYSWITCH 0
  #define ANALOGX 0    //PB5
  #define ANALOGY 1    //PB2
  #define UP KEY_W
  #define DOWN KEY_S
  #define LEFT KEY_A
  #define RIGHT KEY_D
  #define SWCODE KEY_SPACE
  
  #include <SoftSerial_INT0.h>
  #define DEBUG_TX_RX_PIN 1
  //SoftSerial Serial(DEBUG_TX_RX_PIN, DEBUG_TX_RX_PIN);
#endif

//ADC values at various joystick positions: W1023 E0, N1023 S0, center X520 Y498
//Joystick orientation: board pins pointing to right.

//If analogX or Y ADC value is within(or exceed) threshold limits below, the joystick position is confirmed.
#define W_X  550
#define E_X  450
#define N_Y  550
#define S_Y  450
#define NE_X 100
#define NE_Y 900
#define NW_X 900
#define NW_Y 900
#define SE_X 100
#define SE_Y 100
#define SW_X 900
#define SW_Y 100
int x,y;
int pos=0,prevpos=0;

//If pullup, pin is at HIGH, changing to LOW on press.
//If pulldown, pin is at LOW, changing to HIGH on press.
//Does not support button HELD state. No false positives.
bool isClick(int button,int LEVEL_CHANGE) {
  static long lastChecked=0;
  long debounceDelay=50;

  if (digitalRead(button) == LEVEL_CHANGE) {   //if pin changed to new LEVEL,
    if (lastChecked == 0)
      lastChecked=millis();     //start time of change
    if (millis() - lastChecked >= debounceDelay) {     //debounce delay
      while (digitalRead(button) == LEVEL_CHANGE)      //loop till button released
        ;
      lastChecked=0;           //reset
      return true;             //button state changed
    }
  }
  return false;
}

//send key press to HID driver.
void sendKey(byte keychr) {
#if defined(__AVR_ATmega32U4__)  || defined(ARDUINO_ARCH_STM32)
  Keyboard.press(keychr);
#elif defined(ARDUINO_AVR_DIGISPARK)
  DigiKeyboard.sendKeyPress(keychr);
#endif
}

//send key char to HID driver.
void sendKeyRel(byte keychr) {
#if defined(__AVR_ATmega32U4__)  || defined(ARDUINO_ARCH_STM32)
  Keyboard.write(keychr);
#elif defined(ARDUINO_AVR_DIGISPARK)
  DigiKeyboard.sendKeyStroke(keychr);
#endif
}

//send key release to HID driver.
void freeKey(byte keychr) {
#if defined(__AVR_ATmega32U4__)  || defined(ARDUINO_ARCH_STM32)
  Keyboard.release(keychr);
#elif defined(ARDUINO_AVR_DIGISPARK)
  DigiKeyboard.sendKeyPress(0,0);
#endif
}

//send all key release to HID driver.
void freeKeyAll() {
#if defined(__AVR_ATmega32U4__)  || defined(ARDUINO_ARCH_STM32)
  Keyboard.releaseAll();
#elif defined(ARDUINO_AVR_DIGISPARK)
  DigiKeyboard.sendKeyPress(0,0);
#endif
}

//turn LED on/off. 
void setLed(int ledpin,int state) {
#if defined(__AVR_ATmega32U4__)
  digitalWrite(ledpin,!state);     //inverse of normal state
#else
  digitalWrite(ledpin,state);
#endif
}

void setup() {
  //Serial.begin(9600);
  pinMode(JOYSWITCH, INPUT_PULLUP);    //switch
  pinMode(LED_BUILTIN, OUTPUT);        //led
  setLed(LED_BUILTIN,0);
  #if defined(__AVR_ATmega32U4__) || defined(ARDUINO_AVR_DIGISPARK)
    ADCSRA =  bit (ADEN);
    ADCSRA |= bit (ADPS2);             //1mhz ADC clock
  #endif
  #if defined(__AVR_ATmega32U4__)
    Keyboard.begin();
  #endif
  analogRead(ANALOGX);  //always discard first analogread
  analogRead(ANALOGY);
}

void loop() {
  if (isClick(JOYSWITCH,LOW)) {  //with pullup, switch close=high->low
    setLed(LED_BUILTIN,1);
    sendKeyRel(SWCODE);
    setLed(LED_BUILTIN,0);
  }

  x = analogRead(ANALOGX);          //Read X  idle 522
  y = analogRead(ANALOGY);          //Read Y  idle 498
  //Serial.print("X: "); Serial.print(x); Serial.print(", Y: "); Serial.println(y);
  if ((x < W_X && x > E_X) && (y > S_Y && y < N_Y)) {     //compare ADC values against threshold limits.
    pos=0;     //center
  }
  else if ((x < W_X && x > E_X) && (y > N_Y)) {
    pos=1;     //N
  } else if ((x < E_X) && (y > S_Y && y < N_Y)) {
    pos=2;     //E
  } else if ((x < W_X && x > E_X) && (y < S_Y)) {
    pos=3;     //S
  } else if ((x > W_X) && (y > S_Y && y < N_Y)) {
    pos=4;     //W
  } else if (x < NE_X && y > NE_Y) {
    pos=5;     //NE
  }
  else if (x < SE_X && y < SE_Y) {
    pos=6;     //SE
  }
  else if (x > SW_X && y < SW_Y) {
    pos=7;     //SW
  }
  else if (x > NW_X && y > NW_Y) {
    pos=8;     //NW
  }

  //Serial.println(pos);
  if (pos != prevpos) {     //change direction if joystick position has changed
    freeKeyAll();
    switch (pos) {
      case 0:
        break;
      case 1:  //N
        sendKey(UP);
        break;
      case 2:  //E
        sendKey(RIGHT);
        break;
      case 3:  //S
        sendKey(DOWN);
        break;
      case 4:  //W
        sendKey(LEFT);
        break;
      case 5:  //NE
        sendKey(RIGHT);  //NB: digispark does not support multiple keypresses.
        sendKey(UP);     //so it will only send the last press ie UP.
        break;
      case 6:  //SE
        sendKey(RIGHT);
        sendKey(DOWN);
        break;
      case 7:  //SW
        sendKey(LEFT);
        sendKey(DOWN);
        break;
      case 8:  //NW
        sendKey(LEFT);
        sendKey(UP);
        break;
      default:
        break;
    }
    prevpos=pos;
  }

  #if defined(ARDUINO_AVR_DIGISPARK)
     //DigiKeyboard.delay(500);
     DigiKeyboard.update();
  #endif

}