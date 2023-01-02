/*
 * RC Transmitter Using nRF24L01 Module
 * Website used to setup the nRF24L01: www.howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/ 
 * 
 * Created by Tiago, www.github.com/CandeiasV2
 */
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define forwardLedPin 10              // define forward indicating LED pin
#define backwardLedPin 6              // define backward indicating LED pin
#define forwardReverseButtonPin 2     // define button pin to switch between forward and backward
#define lightButtonPin 9              // define button pin used for the brightness of the Car LEDs
#define yAxisPin 0                    // define forward pin of the joystick
#define xAxisPin 1                    // define turn pin of the joystick

RF24 radio(7, 8);                     // CE and CSN pins of the nRF24L01
const byte address[6] = "00001";

// Maximum size of a struct is 32 bytes - nRF24L01 buffer limit
//    speed: speed of the motors (0-100%)
//    angle: angle to turn the car (0-100%)
//    FR: 1 - Forward
//        0 - Reverse
//    RL: 1 - Right
//        0 - Left
//    light: 0 - Off
//           1 - 25% Brightness
//           2 - 50% Brightness
//           3 - 100% Brightness
struct Data_Package {
  int speed = -1;
  int angle = -1;
  int FR = -1;
  int RL = -1;
  int light = -1;
};

// Create a variable with the above structure
Data_Package data; 

// Initiate setup settings
int dirPrevState = 0;       // Direction previous state
int dirCurState = 0;        // Direction current state
int ledPrevState = 0;       // LED previous state
int ledCurState = 0;        // LED current state
int forwardLED = 1;         
int xAxisValue;

void setup() {
  // Setting up the nRF24L01 
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  // Setting up the ports direction
  pinMode(forwardReverseButtonPin, INPUT); // initialize the port to pull-up input
  pinMode(lightButtonPin, INPUT);
  pinMode(forwardLedPin, OUTPUT);
  pinMode(backwardLedPin, OUTPUT);
  // Initializing initial setings
  data.light = 0;
  data.FR = 1;
  digitalWrite(forwardLedPin, HIGH);
  digitalWrite(backwardLedPin, LOW);
}

void loop() {  
  // Calculating speed percentage from the joystick
  data.speed = map(analogRead(yAxisPin), 0, 1023, 0, 100);

  // Calculating angle percentage and turn direction from the joystick
  // Note: The joystick I used outputs a value of 502 when in 'neutral,  
  xAxisValue = analogRead(xAxisPin);
  if(xAxisValue > 504) {
    // Right direction
    data.angle = map(xAxisValue, 505, 1023, 0, 100);
    data.RL = 1;
  } else if (xAxisValue < 500) {
    // Left direction
    data.angle = map(xAxisValue, 0, 499, 100, 0);
    data.RL = 0;
  } else {
    // In neutral, no direction. 
    data.angle = 0;
    data.RL = 1;
  }

  // If the Forward/Reverse button is pressed, change to the opposite direction and switch the LED
  dirCurState = digitalRead(forwardReverseButtonPin);
  if((dirPrevState != 1) && (dirCurState == 1)){
     //Change LED and direction
    if(forwardLED == 1){
      digitalWrite(forwardLedPin, LOW);
      digitalWrite(backwardLedPin, HIGH);
      data.FR = 0;
      forwardLED = 0;
    } else {
      digitalWrite(forwardLedPin, HIGH);
      digitalWrite(backwardLedPin, LOW);
      data.FR = 1;
      forwardLED = 1;
    }
  }
  dirPrevState = dirCurState;

  // If the LED button is pressed, change to the next brightness level
  ledCurState = digitalRead(lightButtonPin);
  if((ledPrevState != 1) && (ledCurState == 1)){
     //Change LED light state
    if(data.light == 3){
      data.light = 0;
    } else {
      data.light++;
    }
  }
  ledPrevState = ledCurState;

  // Transmit the data over to the receiver 
  radio.write(&data, sizeof(Data_Package));
 
  // Small delay helps with the disconnecting issues  
  delay(70);
}
