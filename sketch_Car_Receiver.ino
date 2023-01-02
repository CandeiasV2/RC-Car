/*
 * RC Receiver Using nRF24L01 Module
 * Website used to setup the nRF24L01: www.howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/ 
 * 
 * Created by Tiago, www.github.com/CandeiasV2
 */
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);                   // CE and CSN pins of the nRF24L01
const byte address[6] = "00001";

// For L298N control pins
#define LeftMotorForward 6          // PWM pin 6 is wired to the IN3 port of the L298N Module
#define LeftMotorBackward 3         // PWN pin 3 is wired to the IN4 port of the L298N Module
#define RightMotorForward 9         // PWN pin 9 is wired to the IN1 port of the L298N Module
#define RightMotorBackward 5        // PWN pin 5 is wired to the IN2 port of the L298N Module
#define FrontLedPin 10

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

void setup() {
  // Setting up the nRF24L01 
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  // Setting up the ports direction
  pinMode(RightMotorForward, OUTPUT);
  pinMode(LeftMotorForward, OUTPUT);
  pinMode(LeftMotorBackward, OUTPUT);
  pinMode(RightMotorBackward, OUTPUT);
  pinMode(FrontLedPin, OUTPUT);
  // Initializing initial setings
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(RightMotorForward, LOW);
  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorBackward, LOW);
  analogWrite(FrontLedPin, LOW);  
}

void loop() {
  if (radio.available()) {
    // If the radio is available, read the data from the transmitter
    radio.read(&data, sizeof(Data_Package));

    // Computing the Car movement
    if (data.FR == 1) { 
      // If forwards, turn the backwards signal off
      digitalWrite(LeftMotorBackward, LOW);
      digitalWrite(RightMotorBackward, LOW);
      if (data.RL == 1) { 
        // If to turn to the right, set the appropriate wheel movement based on the speed and turning angle
        analogWrite(LeftMotorForward, (255*data.speed)/100);
        analogWrite(RightMotorForward, (((255*data.speed)/100)*(100-data.angle))/100);
      } else { 
        // If to turn to the left, set the appropriate wheel movement based on the speed and turning angle
        analogWrite(LeftMotorForward, (((255*data.speed)/100)*(100-data.angle))/100);
        analogWrite(RightMotorForward, (255*data.speed)/100);
      }
    } else {  
      // If backwards, turn the forwards signal off
      digitalWrite(LeftMotorForward, LOW);
      digitalWrite(RightMotorForward, LOW);
      if (data.RL == 1) { 
        // If to turn to the right, set the appropriate wheel movement based on the speed and turning angle
        analogWrite(LeftMotorBackward, (255*data.speed)/100);
        analogWrite(RightMotorBackward, (((255*data.speed)/100)*(100-data.angle))/100);
      } else { 
        // If to turn to the left, set the appropriate wheel movement based on the speed and turning angle
        analogWrite(LeftMotorBackward, (((255*data.speed)/100)*(100-data.angle))/100);
        analogWrite(RightMotorBackward, (255*data.speed)/100);
      }
    }

    // Output the appropriate light level based on the light state
    if (data.light == 0) {
      // Off
      analogWrite(FrontLedPin, LOW);   
    } else if (data.light == 1) {
      // 25% Brightness
      analogWrite(FrontLedPin, 64); 
    } else if (data.light == 2) {
      // 50% Brightness
      analogWrite(FrontLedPin, 128);
    } else if (data.light == 3) {
      // 100% Brightness
      analogWrite(FrontLedPin, 255);
    }    
  }
}
