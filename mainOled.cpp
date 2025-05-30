#include <Arduino.h>
#include <ittiot.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIFI_NAME     "C6"
#define WIFI_PASSWORD "Vanaema70"

#define MQTT_SERVER   "193.40.245.72"
#define MQTT_PORT     "1883"
#define MQTT_USER     "M2"
#define MQTT_PASS     "test"

#define DEVICE_ID     "ESP60"

#define MODULE_TOPIC  "ESP40/temphumid"

// #define TOPIC_WEIGHT  "ESP00/weight" (nupp)
// #define TOPIC_SERVO   "ESP99/servo" (servo/relee)

#define OLED_RESET    0

Adafruit_SSD1306 display(OLED_RESET);

float h = 0.0;
float t = 0.0;

void iot_received(String topic, String msg) {
  if (topic == MODULE_TOPIC) {
    int comma = msg.indexOf(',');
    if (comma > 0) {
      t = msg.substring(0, comma).toFloat();
      h = msg.substring(comma + 1).toFloat();
    }
  }
  
  // else if (topic == TOPIC_WEIGHT) {
  //   float weight = msg.toFloat();
  //   // process weight
  // }
  // else if (topic == TOPIC_SERVO) {
  //   int angle = msg.toInt();
  //   // process servo angle
  // }
}

void iot_connected() {
  Serial.println("MQTT connected callback");
  iot.subscribe(MODULE_TOPIC);
  
  // iot.subscribe(TOPIC_WEIGHT);
  // iot.subscribe(TOPIC_SERVO);
  
  iot.log("IoT OLED example subscribed");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting OLED → MQTT subscriptions");

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Booting...");
  display.display();

  iot.setConfig("wname", WIFI_NAME);
  iot.setConfig("wpass", WIFI_PASSWORD);

  iot.setConfig("msrv",  MQTT_SERVER);
  iot.setConfig("mport", MQTT_PORT);
  iot.setConfig("muser", MQTT_USER);
  iot.setConfig("mpass", MQTT_PASS);

  iot.printConfig();
  iot.setup();
}

void loop() {
  iot.handle();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Temp: "); display.print(t, 2); display.println(" C");
  display.setCursor(0,16);
  display.print("Hum:  "); display.print(h, 2); display.println(" %");
  display.display();

  delay(200);
}