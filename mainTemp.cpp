#include <Arduino.h>
#include <ittiot.h>
#include <Ticker.h>
#include <DHT.h>

#define WIFI_NAME     ""
#define WIFI_PASSWORD ""

#define MQTT_SERVER   "193.40.245.72"
#define MQTT_PORT     "1883"
#define MQTT_USER     "M2"
#define MQTT_PASS     "test"

#define MODULE_TOPIC  "ESP40/temphumid"

#define DHTPIN  D3
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
Ticker timeTicker;
bool sendDataFlag = false;

void sendData() {
  sendDataFlag = true;
}

void iot_received(String topic, String msg) {
}

void iot_connected() {
  Serial.println("MQTT connected callback");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting DHT → MQTT");

  iot.setConfig("wname", WIFI_NAME);
  iot.setConfig("wpass", WIFI_PASSWORD);

  iot.setConfig("msrv",  MQTT_SERVER);
  iot.setConfig("mport", MQTT_PORT);
  iot.setConfig("muser", MQTT_USER);
  iot.setConfig("mpass", MQTT_PASS);

  iot.printConfig();  // dump json
  iot.setup();

  dht.begin();
  timeTicker.attach(1.0, sendData);  // set sendDataflag to true every 1 s
}

void loop() {
  iot.handle();

  if (sendDataFlag) {
    sendDataFlag = false;

    //Read hum and temp
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    Serial.printf("Temperature: %.2f C,   Humidity: %.2f %%\n", t, h);

    char buf[16];
    //float to string and publish data
    dtostrf(t, 5, 2, buf);
    iot.publishMsg("temp", buf);

    dtostrf(h, 5, 2, buf);
    iot.publishMsg("hum", buf);
  }
}
