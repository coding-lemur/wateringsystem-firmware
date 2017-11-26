#include <pgmspace.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <PubSubClient.h>
#include <RtcDS3231.h>
#include <ArduinoJson.h>

#include <SSD1306.h>

#include "DhtHelper.h"
#include "Helper.h"
#include "Watering.h"
#include "SensorValues.h"

#define DHT_PIN                     D4
#define PUMP_ACTIVATE_PIN           D5
#define WATER_SENSOR_ACTIVATE_PIN   D6
#define WATER_SENSOR_PIN            A0

#define DHT_TYPE                    DHT11
#define NTP_TIME_OFFSET             7200 // for timezone (in seconds) = 2h

#define TOPIC_SENSORS               "wateringsystem/sensors"
#define TOPIC_WATERING              "wateringsystem/watering"
#define TOPIC_MEASURING             "wateringsystem/measuring"

const char* ssid          = "mywifi";
const char* password      = "123456";

const char* mqtt_server   = "mqtt.eclipse.org";
const char* mqtt_user     = "mceddy";
const char* mqtt_password = "123456";

RtcDS3231<TwoWire> Rtc(Wire);
DynamicJsonBuffer jsonBuffer;

DhtHelper dhtHelper(DHT_PIN, DHT_TYPE);
Watering watering(PUMP_ACTIVATE_PIN);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

SSD1306 display(0x3C, SDA, SCL);

uint32_t nextTotalSeconds = 0;

void setup() {
  pinMode(WATER_SENSOR_ACTIVATE_PIN, OUTPUT);
  pinMode(WATER_SENSOR_PIN, INPUT);

  watering.setup();
  
  setupSerials();
  setupWifi();
  setupMqttClient();
  setupNtpClient();
  setupDisplay();

  adjustRTC();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMqttClient();
  }
  
  /*
  if (!Rtc.IsDateTimeValid()) 
  {
    // Common Cuases:
    //    1) the battery on the device is low or even missing and the power line was disconnected
    Serial.println("RTC lost confidence in the DateTime!");
  }
  */
  
  RtcDateTime now = Rtc.GetDateTime();
  uint32_t totalSeconds = now.TotalSeconds();
  
  if (totalSeconds >= nextTotalSeconds) {
    nextTotalSeconds = totalSeconds + 5 * 60; // wait 5 minutes
    
    doMeasure(now);
  }

  mqttClient.loop();
  watering.loop();
}

void doMeasure(RtcDateTime dateTime) {
  SensorValues values = getSensorValues();

  debugOutputValues(dateTime, values);
  displaySensorValues(dateTime, values);
  
  publishSensorValues(values);
}

void setupSerials() {
  Serial.begin(115200);
  
  delay(100);

  Wire.begin();
  Rtc.Begin();
  dhtHelper.begin();
  
  delay(100);
}

void setupWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected"); 
   
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);
}

void setupMqttClient() {
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
}

void setupNtpClient() {
  // set timezone
  timeClient.setTimeOffset(NTP_TIME_OFFSET);
}

void setupDisplay() {
  display.init();
  
  display.flipScreenVertically();
}

void adjustRTC() {
  if (!Rtc.IsDateTimeValid()) {
    Serial.println("adjust RTC");

    RtcDateTime now = getDateTimeFromNTP();
    Serial.print("NTP datetime: ");
    Serial.println(Helper::getFormatedDateTime(now));
    
    Rtc.SetDateTime(now);
  }
  
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
}

RtcDateTime getDateTimeFromNTP() {
  // get time from NTP
  timeClient.begin();
  timeClient.forceUpdate();
  timeClient.end();
  
  long timestamp = timeClient.getEpochTime();
  long timestampFrom2000 = timestamp - 946684800; // calc timestamp from 2000
  
  return RtcDateTime(timestampFrom2000);
}

SensorValues getSensorValues() {
  dhtHelper.refreshValues();
  int temperature = dhtHelper.getTemperature();
  int humidity    = dhtHelper.getHumidity();
  
  int soilMoisture = getWaterValue();

  return SensorValues(temperature, humidity, soilMoisture);
}

int getWaterValue() {
  // enable water sensor
  digitalWrite(WATER_SENSOR_ACTIVATE_PIN, HIGH);

  delay(1000);

  // read value
  int value = analogRead(WATER_SENSOR_PIN);

  // disable water sensor
  digitalWrite(WATER_SENSOR_ACTIVATE_PIN, LOW);

  return value;
}

void debugOutputValues(RtcDateTime dateTime, SensorValues values) {
  Serial.print("datetime: ");
  Serial.println(Helper::getFormatedDateTime(dateTime));
  
  Serial.print("temperature: ");
  Serial.print(values.getTemperature());
  Serial.println("C");
  
  Serial.print("humidity: ");
  Serial.print(values.getHumidity());
  Serial.println("%");
  
  Serial.print("water: ");
  Serial.println(values.getSoilMoisture());
}

void displaySensorValues(RtcDateTime dateTime, SensorValues values) {
  display.clear();

  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, Helper::getFormatedDateTime(dateTime));

  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 15, String(values.getTemperature()) + " °C");
  display.drawString(0, 31, String(values.getHumidity()) + "%");
  display.drawString(0, 47, String(values.getSoilMoisture()));
  
  display.display();
}

void publishSensorValues(SensorValues values) {
  JsonObject& json = jsonBuffer.createObject();
  
  json["Temperature"]  = values.getTemperature();
  json["Humidity"]     = values.getHumidity();
  json["SoilMoisture"] = values.getSoilMoisture();
  
  char buffer[64];  
  json.printTo(buffer);
  
  mqttClient.publish(TOPIC_SENSORS, buffer);
}

void reconnectMqttClient() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect
    if (mqttClient.connect("watering_system", mqtt_user, mqtt_password)) {
      Serial.println("connected");

      mqttClient.subscribe(TOPIC_WATERING);
  } else {
    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" try again in 5 seconds");
    
    // Wait 5 seconds before retrying
    delay(5000);
  }
 }
}

// handle message arrived
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String payloadString = Helper::byteArrayToString(payload, length);
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  Serial.print("payload: ");
  Serial.println(payloadString);

  if (topic == TOPIC_WATERING) {
    int wateringMilliseconds = payloadString.toInt();

    // value in rage (between 1 second and 3 minutes)?
    if ((wateringMilliseconds >= 1000) && (wateringMilliseconds <= 180000)) {
      watering.startPump(wateringMilliseconds);
    }
  }
  else if (topic == TOPIC_MEASURING) {
    RtcDateTime now = Rtc.GetDateTime();
    doMeasure(now);
  }
}
