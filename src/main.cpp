#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "private_data.h"

int LED_BUILTIN = 2;

const int pwmResolution = 8;  // PWM resolution (bits) - 8 bits for duty cycle
const int pwmFrequency = 40000;  // PWM frequency in Hz

//pwm channels for crane motor
const int pwmChannelCrane1 = 0;  // PWM channel (0-15) on ESP32
const int pwmChannelCrane2 = 1;  // PWM channel (0-15) on ESP32
//pwm pins of crane motor
const int pwmCrane1Pin = 18;
const int pwmCrane2Pin = 19;

//top and bottom switches pins
const int topSwitchPin = 35;
const int bottomSwitchPin  = 34;


void clawUp(int speed);
void clawDown(int speed);
void grabSequence(int speed, int graspStrength);


//////
const char* MQTT_CLIENT_NAME= "ESP32_CLAW";
WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_connect();
void mqtt_subscriber_callback(char* topic, byte* payload, unsigned int length);
void initWiFi();


void setup() {
  Serial.begin(9600);
  Serial.print("start");

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


  initWiFi();
  Serial.println("Connected to the WiFi network");
  // Set the MQTT server to the client
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(mqtt_subscriber_callback);
  mqtt_connect();

  client.subscribe("claw_ctl/cmd");

}

void loop() {

  //connect to network in case there is no connection
    if (WiFi.status() != WL_CONNECTED){
      initWiFi();
      Serial.print("wifi connected.");
    }
    
    if (!client.connected()) {
      mqtt_connect();
      client.subscribe("claw_ctl/cmd");
    }

  client.loop();
  

  /*clawUp(255);
  delay(3000);

  clawDown(255);
  delay(3000);
*/
}



void clawUp(int speed){
  int pinState;

  ledcWrite(pwmChannelCrane1, speed);
  ledcWrite(pwmChannelCrane2, 0);

  do {
    pinState = digitalRead(topSwitchPin);
    Serial.println(pinState);
    delay(10);
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

void grabSequence(int speed, int graspStrength){
  // move claw to the bottom
  clawDown(speed);
  delay(2000);

  // close claw

  // lift claw
  clawUp(speed); 
}



void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print('.');
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
  Serial.println(WiFi.localIP());
}


void mqtt_connect() {
  // Connect to MQTT Broker
  while (!client.connected()) {
      Serial.println("Connecting to MQTT Broker...");
      if (client.connect("MQTT_CLIENT_NAME")) {
          Serial.println("Connected to MQTT Broker");
      } else {
          Serial.print("Failed with state ");
          Serial.print(client.state());
          delay(5000);
      }
  }
}


void mqtt_subscriber_callback(char* topic, byte* payload, unsigned int length){
  Serial.print("message received: ");

  // convert payload to string
  byte message[20];
  memcpy(message, payload, length);
  // add null character at the end
  message[length] = '\0';
  // convert to string
  String cmd = (char*)message;
  Serial.println(cmd);
  
  if ( cmd == "grab_seq"){
      grabSequence(128,255);
    }

}