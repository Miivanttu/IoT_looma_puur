#include <Arduino.h>
#include <Servo.h>
#include <ittiot.h>

//configuration
#define BUTTON_TOPIC "ESP33/ESP99/servo"
#define NODERED_TOPIC "ESP33/ESP99/servored"
#define RELAY_TOPIC "ESP33/ESP99/relay"
#define FOOD_TOPIC "ESP99/foodlevel"
#define WATER_TOPIC "ESP99/waterlevel"
#define WIFI_NAME ""
#define WIFI_PASSWORD ""
#define MQTT_SERVER "193.40.245.72"
#define MQTT_PORT "1883"
#define MQTT_USER "M2"
#define MQTT_PASS "test"

//hardware
#define SERVO_PIN D3
#define RELAY_PIN D1

Servo myservo;

//state variables
unsigned long lastServoCycle = 0;
unsigned long lastRelayCycle = 0;
bool isServoOpen = false;
bool isRelayOn = false;
bool buttonActive = false;
bool nodeRedActive = false;
bool relayManualControl = false;
bool relayPendingReset = false;
unsigned long relayResetTime = 0;
int foodLevel = 100;
int waterLevel = 100;
const int DISPENSE_AMOUNT = 10;
const unsigned long SERVO_INTERVAL = 5000;
const unsigned long SERVO_OPEN_DURATION = 1000;
const unsigned long RELAY_INTERVAL = 10000;
const unsigned long RELAY_DURATION = 2000;
const unsigned long RELAY_RESET_DELAY = 1000;

//function for publishing food and water levels to MQTT
void publishLevel(const char* topic, int level) {
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", level);
  iot.publishMsg(topic, buf);
  Serial.printf("Published %s level: %d%%\n", topic, level);
}

//function for changing food level value
void dispenseFood() {
  if (foodLevel >= DISPENSE_AMOUNT) {
    foodLevel -= DISPENSE_AMOUNT;
    publishLevel(FOOD_TOPIC, foodLevel);
  } else {
    foodLevel = 0;
    publishLevel(FOOD_TOPIC, foodLevel);
    Serial.println("Food empty!");
  }
}

//function for changing water level value
void dispenseWater() {
  if (waterLevel >= DISPENSE_AMOUNT) {
    waterLevel -= DISPENSE_AMOUNT;
    publishLevel(WATER_TOPIC, waterLevel);
  } else {
    waterLevel = 0;
    publishLevel(WATER_TOPIC, waterLevel);
    Serial.println("Water empty!");
  }
}

//function for reseting food and water level values
void resetLevel(const char* topic) {
  if (strcmp(topic, FOOD_TOPIC) == 0) {
    foodLevel = 100;
    publishLevel(FOOD_TOPIC, foodLevel);
    Serial.println("Food level reset to 100%");
  } else if (strcmp(topic, WATER_TOPIC) == 0) {
    waterLevel = 100;
    publishLevel(WATER_TOPIC, waterLevel);
    Serial.println("Water level reset to 100%");
  }
}

//function for opening and closing servo
void controlServo(bool open) {
  if (open) {
    myservo.write(180);
    isServoOpen = true;
    dispenseFood();
    Serial.println("Servo opened - food dispensed");
  } else {
    myservo.write(0);
    isServoOpen = false;
    Serial.println("Servo closed");
  }
}

//function for turning relay on and off
void controlRelay(bool on) {
  if (on) {
    digitalWrite(RELAY_PIN, LOW);
    isRelayOn = true;
    dispenseWater();
    Serial.println("Relay activated - water dispensed");
  } else {
    digitalWrite(RELAY_PIN, HIGH);
    isRelayOn = false;
    Serial.println("Relay deactivated");
  }
}

void iot_received(String topic, String msg) {
  Serial.printf("Received: %s - %s\n", topic.c_str(), msg.c_str());

  //is button pressed topic
  if (topic == BUTTON_TOPIC) {
    if (msg == "1") {          //close servo when button is pressed
      buttonActive = true;
      controlServo(false);
      Serial.println("Button pressed - servo stopped");
    } else if (msg == "0") {   //keep opening and closing servo until button is pressed 
      buttonActive = false;
      Serial.println("Button released - servo cycle will resume");
    }
  } 

  //is there a signal from Node-REd topic
  else if (topic == NODERED_TOPIC) {
    if (msg == "1" && !isServoOpen && !buttonActive) { //open and close servo when user sends signal through Node-RED
      nodeRedActive = true;
      controlServo(true);
      lastServoCycle = millis();
    } else if (msg == "0" && nodeRedActive) {
      controlServo(false);
      nodeRedActive = false;
    }
  }

  //water giving topic with relay
  else if (topic == RELAY_TOPIC) {
    if (msg == "1") {   //enough time has passed, give water
      relayManualControl = true;
      controlRelay(true);
      relayResetTime = millis();
      relayPendingReset = true;
      Serial.println("Manual Relay → ON (refill)");
    } else if (msg == "0") {
      relayManualControl = false;
      controlRelay(false);
    }
  }

  //read food and water levels from MQTT
  else if (topic == FOOD_TOPIC || topic == WATER_TOPIC) {
    if (msg == "reset") {
      resetLevel(topic.c_str());
    } else {
      int newLevel = msg.toInt();
      if (newLevel >= 0 && newLevel <= 100) {
        if (topic == FOOD_TOPIC) {
          foodLevel = newLevel;
        } else {
          waterLevel = newLevel;
          if (newLevel == 100) {
            relayManualControl = false;
          }
        }
        publishLevel(topic.c_str(), newLevel);
      }
    }
  }
}

void iot_connected() {
  Serial.println("MQTT connected");
  iot.subscribe(BUTTON_TOPIC);
  iot.subscribe(NODERED_TOPIC);
  iot.subscribe(RELAY_TOPIC);
  iot.subscribe(FOOD_TOPIC);
  iot.subscribe(WATER_TOPIC);
  publishLevel(FOOD_TOPIC, foodLevel);
  publishLevel(WATER_TOPIC, waterLevel);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting food and water dispenser...");

  iot.setConfig("wname", WIFI_NAME);
  iot.setConfig("wpass", WIFI_PASSWORD);
  iot.setConfig("msrv", MQTT_SERVER);
  iot.setConfig("mport", MQTT_PORT);
  iot.setConfig("muser", MQTT_USER);
  iot.setConfig("mpass", MQTT_PASS);
  iot.setup();

  myservo.attach(SERVO_PIN);
  myservo.write(0);
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
}

void loop() {
  iot.handle();
  unsigned long currentTime = millis();

  //open and close servo when button is not pressed and there is no Node-RED signal to open servo
  if (!buttonActive && !nodeRedActive) {
    if (!isServoOpen && (currentTime - lastServoCycle >= SERVO_INTERVAL)) {
      controlServo(true);
      lastServoCycle = currentTime;
    }
    if (isServoOpen && (currentTime - lastServoCycle >= SERVO_OPEN_DURATION)) {
      controlServo(false);
    }
  }
  
  //close servo after Node-RED signal timeout
  if (nodeRedActive && (currentTime - lastServoCycle >= 1000)) {
    controlServo(false);
    nodeRedActive = false;
  }
  
  //close relay after relay reset delay
  if (relayPendingReset && currentTime - relayResetTime >= RELAY_RESET_DELAY) {
    controlRelay(false);
    relayPendingReset = false;
    iot.publishMsg(RELAY_TOPIC, "0");
    Serial.println("Manual Relay → OFF (auto reset)");
  }

  //open and close when water giving is pending
  if (!relayManualControl) {
    if (!isRelayOn && (currentTime - lastRelayCycle >= RELAY_INTERVAL)) {
      controlRelay(true);
      lastRelayCycle = currentTime;
    }
    if (isRelayOn && (currentTime - lastRelayCycle >= RELAY_DURATION)) {
      controlRelay(false);
    }
  }
}
