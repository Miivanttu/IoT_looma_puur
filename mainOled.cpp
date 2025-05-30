#include <Arduino.h>
#include <ittiot.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIFI_NAME     ""
#define WIFI_PASSWORD ""

#define MQTT_SERVER   "193.40.245.72"
#define MQTT_PORT     "1883"
#define MQTT_USER     "M2"
#define MQTT_PASS     "test"

#define DEVICE_ID     "ESP60"
#define MODULE_TOPIC  "ESP40/temphumid"
#define OLED_RESET    0

Adafruit_SSD1306 display(OLED_RESET);

float h = 0.0; //humidity
float t = 0.0; //temperature

void iot_received(String topic, String msg) {
  if (topic == MODULE_TOPIC) {
    int comma = msg.indexOf(',');
    if (comma > 0) {
      t = msg.substring(0, comma).toFloat();
      h = msg.substring(comma + 1).toFloat();
    }
  }
}

void iot_connected() {
  Serial.println("MQTT connected callback");
  iot.subscribe(MODULE_TOPIC);
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

  //display temp and hum
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
