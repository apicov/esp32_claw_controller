/**
 * @file main.cpp
 * @brief Main source file for ESP32 Claw Controller. Handles WiFi, MQTT, and motor control logic.
 */
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "private_data.h"

/**
 * @brief Built-in LED pin number.
 */
int LED_BUILTIN = 2;

/**
 * @brief PWM resolution in bits.
 */
const int pwmResolution = 8;  // PWM resolution (bits) - 8 bits for duty cycle
/**
 * @brief PWM frequency in Hz.
 */
const int pwmFrequency = 40000;  // PWM frequency in Hz

/**
 * @brief PWM channel for crane motor 1.
 */
const int pwmChannelCrane1 = 0;  // PWM channel (0-15) on ESP32
/**
 * @brief PWM channel for crane motor 2.
 */
const int pwmChannelCrane2 = 1;  // PWM channel (0-15) on ESP32
/**
 * @brief PWM pin for crane motor 1.
 */
const int pwmCrane1Pin = 18;
/**
 * @brief PWM pin for crane motor 2.
 */
const int pwmCrane2Pin = 19;

/**
 * @brief PWM channel for claw.
 */
const int pwmChannelClaw = 2;  // PWM channel (0-15) on ESP32
/**
 * @brief Output pin for claw.
 */
const int pwmClawPin = 23; 

/**
 * @brief Top switch pin.
 */
const int topSwitchPin = 35;
/**
 * @brief Bottom switch pin.
 */
const int bottomSwitchPin  = 34;

/**
 * @brief Maximum number of command parameters.
 */
const int MAX_PARAMETERS = 2;

/**
 * @brief Moves the claw up at the specified speed until the top switch is triggered.
 * @param speed PWM duty cycle (0-255).
 */
void clawUp(int speed);
/**
 * @brief Moves the claw down at the specified speed until the bottom switch is triggered.
 * @param speed PWM duty cycle (0-255).
 */
void clawDown(int speed);
/**
 * @brief Executes the grab sequence: lowers claw, closes, then lifts.
 * @param speed PWM duty cycle for movement (0-255).
 * @param graspStrength PWM duty cycle for claw grip (0-255).
 */
void grabSequence(int speed, int graspStrength);
/**
 * @brief Closes the claw with the specified strength.
 * @param graspStrength PWM duty cycle for claw grip (0-255).
 */
void closeClaw(int graspStrength);
/**
 * @brief Opens the claw (sets grip to 0).
 */
void openClaw();


//////
/**
 * @brief MQTT client name.
 */
const char* MQTT_CLIENT_NAME= "ESP32_CLAW";
WiFiClient espClient;
PubSubClient client(espClient);

/**
 * @brief Connects to the MQTT broker.
 */
void mqtt_connect();
/**
 * @brief Callback for MQTT subscription messages.
 * @param topic Topic name.
 * @param payload Message payload.
 * @param length Payload length.
 */
void mqtt_subscriber_callback(char* topic, byte* payload, unsigned int length);
/**
 * @brief Initializes WiFi connection.
 */
void initWiFi();
/**
 * @brief Parses a command string into command and parameters.
 * @param data Input string.
 * @param command Output: command name.
 * @param parameters Output: array of parameters.
 * @param paramCount Output: number of parameters found.
 */
void parseCommandString(const String &data, String &command, String parameters[], int &paramCount);

/**
 * @brief Arduino setup function. Initializes serial, pins, PWM, WiFi, and MQTT.
 */
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

  //claw
  ledcSetup(pwmChannelClaw, pwmFrequency, pwmResolution);
  ledcAttachPin(pwmClawPin, pwmChannelClaw);
  
  // Set initial duty cycle (0-255)
  int dutyCycle = 0;  
  ledcWrite(pwmChannelCrane1, dutyCycle);
  ledcWrite(pwmChannelCrane2, dutyCycle);
  ledcWrite(pwmChannelClaw, 0);


  initWiFi();
  Serial.println("Connected to the WiFi network");
  // Set the MQTT server to the client
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(mqtt_subscriber_callback);
  mqtt_connect();

  client.subscribe("claw/ctl");

}

/**
 * @brief Arduino main loop. Handles WiFi/MQTT reconnection and client loop.
 */
void loop() {

  //connect to network in case there is no connection
    if (WiFi.status() != WL_CONNECTED){
      initWiFi();
      Serial.print("wifi connected.");
    }
    
    if (!client.connected()) {
      mqtt_connect();
      client.subscribe("claw/ctl");
    }

  client.loop();
  

  /*clawUp(255);
  delay(3000);

  clawDown(255);
  delay(3000);
*/
}

/**
 * @brief Callback for MQTT messages. Parses and executes commands.
 * @param topic Topic name.
 * @param payload Message payload.
 * @param length Payload length.
 */
void mqtt_subscriber_callback(char* topic, byte* payload, unsigned int length){
  Serial.print("message received: ");

  // convert payload to string
  byte message[20];
  memcpy(message, payload, length);
  // add null character at the end
  message[length] = '\0';
  // convert to string
  String data = (char*)message;
  Serial.println(data);

  String command;
  String parameters[MAX_PARAMETERS]; // Array to store parameters
  int paramCount = 0;

  // Call the function to parse the command string
  parseCommandString(data, command, parameters, paramCount);
  
  if ( command == "grab_seq"){
    int speed = parameters[0].toInt();
    int grip = parameters[1].toInt();
    grabSequence(speed, grip);
    }
  else if (command == "open" ) {
    openClaw();
  }
  else if ( command == "close"){
    int grip = parameters[0].toInt();
    closeClaw(grip);
    }
  else if ( command == "up"){
    int speed = parameters[0].toInt();
    clawUp(speed);
    }
  else if ( command == "down"){
    int speed = parameters[0].toInt();
    clawDown(speed);
    }

  // publishes task done message
  client.publish("claw/status", "done");

}

/**
 * @brief Closes the claw with the specified strength.
 * @param graspStrength PWM duty cycle for claw grip (0-255).
 */
void closeClaw(int graspStrength){
  ledcWrite(pwmChannelClaw, graspStrength);
}

/**
 * @brief Opens the claw (sets grip to 0).
 */
void openClaw(){
  closeClaw(0);
}

/**
 * @brief Moves the claw up at the specified speed until the top switch is triggered.
 * @param speed PWM duty cycle (0-255).
 */
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

/**
 * @brief Moves the claw down at the specified speed until the bottom switch is triggered.
 * @param speed PWM duty cycle (0-255).
 */
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

/**
 * @brief Executes the grab sequence: lowers claw, closes, then lifts.
 * @param speed PWM duty cycle for movement (0-255).
 * @param graspStrength PWM duty cycle for claw grip (0-255).
 */
void grabSequence(int speed, int graspStrength){
  // move claw to the bottom
  clawDown(speed);
  delay(1000);

  // close claw
  closeClaw(graspStrength);
  delay(300);

  // lift claw
  clawUp(speed);

  //wait some seconds and then open claw again
  //delay(3000);
  //openClaw(); 
}

/**
 * @brief Initializes WiFi connection and waits until connected.
 */
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

/**
 * @brief Connects to the MQTT broker, retrying until successful.
 */
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

/**
 * @brief Parses a command string into command and parameters.
 * @param data Input string.
 * @param command Output: command name.
 * @param parameters Output: array of parameters.
 * @param paramCount Output: number of parameters found.
 */
void parseCommandString(const String &data, String &command, String parameters[], int &paramCount) {
  // Reset the parameters
  paramCount = 0;

  // Find the position of the first space
  int firstSpaceIndex = data.indexOf(' ');

  if (firstSpaceIndex != -1) {
    // Extract the command
    command = data.substring(0, firstSpaceIndex);

    // Extract parameters
    int startIndex = firstSpaceIndex + 1; // Start after the first space
    while (startIndex < data.length() && paramCount < MAX_PARAMETERS) {
      int spaceIndex = data.indexOf(' ', startIndex);
      if (spaceIndex == -1) {  // No more spaces, take the rest of the string
        spaceIndex = data.length();
      }
      parameters[paramCount++] = data.substring(startIndex, spaceIndex);
      startIndex = spaceIndex + 1; // Move past the last space
    }
  } else {
    // No spaces found, entire string is the command
    command = data;
  }
}


