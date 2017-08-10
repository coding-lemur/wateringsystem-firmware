#include <pgmspace.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <PubSubClient.h>
#include <RtcDS3231.h>
#include <ArduinoJson.h>

#include "DhtHelper.h"
#include "Helper.h"

#define DHT_PIN                     D4
#define PUMP_ACTIVATE_PIN           D5
#define WATER_SENSOR_ACTIVATE_PIN   D6
#define WATER_SENSOR_PIN            A0

#define DHT_TYPE                    DHT11
#define NTP_TIME_OFFSET             7200 // for timezone (in seconds) = 2h

const char* ssid          = "mywifi";
const char* password      = "123456";

const char* mqtt_server   = "mqtt.eclipse.org";
const char* mqtt_user     = "mceddy";
const char* mqtt_password = "123456";

RtcDS3231<TwoWire> Rtc(Wire);
DynamicJsonBuffer jsonBuffer;

DhtHelper dhtHelper(DHT_PIN, DHT_TYPE);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

uint32_t nextTotalSeconds = 0;

void setup() {
  pinMode(PUMP_ACTIVATE_PIN, OUTPUT);
  pinMode(WATER_SENSOR_ACTIVATE_PIN, OUTPUT);
  pinMode(WATER_SENSOR_PIN, INPUT);
  
  Serial.begin(115200);
  
  delay(100);
  
  Rtc.Begin();
  dhtHelper.begin();
  
  delay(100);

  setupWifi();
  setupMqttClient();
  setupNtpClient();

  adjustDateTime();
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
    nextTotalSeconds = totalSeconds + 0.5*60; // wait 5 minutes

    Serial.print("datetime: ");
    Serial.println(Helper::getFormatedDateTime(now));
    
    dhtHelper.refreshValues();
    float temperature = dhtHelper.getTemperature();
    float humidity    = dhtHelper.getHumidity();
    
    Serial.println("temperature: " + String(temperature) + "C");
    Serial.println("humidity: " + String(humidity) + "%");
  
    int soilMoisture = getWaterValue();
    Serial.println("water: " + String(soilMoisture));

    publishSensorValues(temperature, humidity, soilMoisture);
  }

  mqttClient.loop();
}

void setupWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
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

void adjustDateTime() {
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

void publishSensorValues(float temperature, float humidity, int soilMoisture) {
  JsonObject& json = jsonBuffer.createObject();
  
  json["Temperature"]  = temperature;
  json["Humidity"]     = humidity;
  json["SoilMoisture"] = soilMoisture;
  
  char buffer[64];  
  json.printTo(buffer);
  
  mqttClient.publish("wateringsystem/sensors", buffer);
}

void watering(int milliseconds) {
  digitalWrite(PUMP_ACTIVATE_PIN, HIGH);
  delay(milliseconds);
  digitalWrite(PUMP_ACTIVATE_PIN, LOW);
}

void reconnectMqttClient() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect
    if (mqttClient.connect("watering_system", mqtt_user, mqtt_password)) {
      Serial.println("connected");

      mqttClient.subscribe("wateringsystem/watering");
  } else {
    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" try again in 5 seconds");
    
    // Wait 5 seconds before retrying
    delay(5000);
  }
 }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println();
  String payloadString = Helper::byteArrayToString(payload, length);
  Serial.println(payloadString);
}
