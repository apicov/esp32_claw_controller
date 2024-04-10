#include <Arduino.h>

const int pwmResolution = 8;  // PWM resolution (bits) - 8 bits for duty cycle
const int pwmFrequency = 40000;  // PWM frequency in Hz

//pwm channels for crane motor
const int pwmChannelCrane1 = 0;  // PWM channel (0-15) on ESP32
const int pwmChannelCrane2 = 1;  // PWM channel (0-15) on ESP32
//pwm pins of crane motor
const int pwmCrane1Pin = 18;
const int pwmCrane2Pin = 19;


void clawUp(int speed);
void clawDown(int speed);


//top switch pin
const int topSwitchPin = 35;
const int bottomSwitchPin  = 34;

void setup() {
  Serial.begin(9600);
  Serial.print("lalalalalalalaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

  pinMode(topSwitchPin, INPUT);
  pinMode(bottomSwitchPin, INPUT);

  // Configure PWM settings
  ledcSetup(pwmChannelCrane1, pwmFrequency, pwmResolution);
  // Attach the PWM channel to a GPIO pin
  ledcAttachPin(pwmCrane1Pin, pwmChannelCrane1);

  ledcSetup(pwmChannelCrane2, pwmFrequency, pwmResolution);
  ledcAttachPin(pwmCrane2Pin, pwmChannelCrane2);

  // Set initial duty cycle (0-255)
  int dutyCycle = 0;  
  ledcWrite(pwmChannelCrane1, dutyCycle);
  ledcWrite(pwmChannelCrane2, dutyCycle);
}

void loop() {

clawUp(255);
delay(3000);
clawDown(255);
delay(3000);

}



void clawUp(int speed){
  int pinState;

  ledcWrite(pwmChannelCrane1, speed);
  ledcWrite(pwmChannelCrane2, 0);

  do {
    pinState = digitalRead(topSwitchPin);
    Serial.println(pinState);
    delay(20);
  } while (pinState == 1);

  ledcWrite(pwmChannelCrane1, 0);
}


void clawDown(int speed){
  int pinState;

  ledcWrite(pwmChannelCrane2, speed);
  ledcWrite(pwmChannelCrane1, 0);

  do {
    pinState = digitalRead(bottomSwitchPin);
    Serial.println(pinState);
    delay(20);
  } while (pinState == 1);

  ledcWrite(pwmChannelCrane2, 0);
}