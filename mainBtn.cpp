#include <Arduino.h>
#include <ittiot.h>
#include <Switch.h>

#define WIFI_NAME     ""
#define WIFI_PASSWORD ""

#define MQTT_SERVER   "193.40.245.72"
#define MQTT_PORT     "1883"
#define MQTT_USER     "M2"
#define MQTT_PASS     "test"

#define DEVICE_ID     "ESP60"
#define TOPIC_SERVO   "ESP99/servo"


#define BUTTON_PIN    D3

Switch button(BUTTON_PIN); //create button object

unsigned long lastStatusTime = 0;

void iot_connected() {
  Serial.println("Connected to MQTT");
  iot.log("ESP60 connected and ready.");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting and connecting to WiFi/MQTT...");

  //initialize button pin mode
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  iot.setConfig("wname", WIFI_NAME);
  iot.setConfig("wpass", WIFI_PASSWORD);
  iot.setConfig("msrv", MQTT_SERVER);
  iot.setConfig("mport", MQTT_PORT);
  iot.setConfig("muser", MQTT_USER);
  iot.setConfig("mpass", MQTT_PASS);

  iot.printConfig();
  iot.setup();
}

void loop() {
  iot.handle();
  
  //poll the button state
  button.poll();

  //if the button is pressed, send 1 to MQTT
  if (button.pushed()) {
    char msg[2];
    snprintf(msg, sizeof(msg), "1");
    iot.publishMsg(TOPIC_SERVO, msg);
    Serial.println("Button pressed, sent message: 1");
  }

  //if the button is released, send 0 to MQTT
  if (button.released()) {
    char msg[2];
    snprintf(msg, sizeof(msg), "0");
    iot.publishMsg(TOPIC_SERVO, msg);
    Serial.println("Button released, sent message: 0");
  }

  //message for debugging
  if (millis() - lastStatusTime >= 5000) {
    Serial.println("Status: Module is running...");
    lastStatusTime = millis();
  }

  delay(10); //debounce
}
