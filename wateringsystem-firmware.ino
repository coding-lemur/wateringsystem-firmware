#include <pgmspace.h>

#include <ESP8266WiFi.h>

#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <BME280I2C.h>

#include "Helper.h"
#include "Watering.h"
#include "SensorValues.h"

#define PUMP_ACTIVATE_PIN D5
#define WATER_SENSOR_ACTIVATE_PIN D6
#define WATER_SENSOR_PIN A0

#define TOPIC_SENSORS_EVENT "wateringsystem/sensors"
#define TOPIC_WATERING_EVENT "wateringsystem/watering"

#define TOPIC_ACTION_SUBSCRIPTION "wateringsystem/actions/#"
#define TOPIC_WATERING_ACTION "wateringsystem/actions/watering"
#define TOPIC_MEASURING_ACTION "wateringsystem/actions/measuring"
#define TOPIC_DEEPSLEEP_ACTION "wateringsystem/actions/deepsleep"

const char *ssid = "mywifi";
const char *password = "123456";

const char *mqtt_server = "mqtt.eclipse.org";
const char *mqtt_user = "mceddy";
const char *mqtt_password = "123456";

const unsigned long waitAfterMeasureMillis = 900000; // 15 minutes

DynamicJsonBuffer jsonBuffer;

Watering watering(PUMP_ACTIVATE_PIN);

WiFiClient espClient;
PubSubClient mqttClient(espClient);
BME280I2C bme;

SensorValues values;
boolean measuringInProgress = false;

unsigned long lastMeasureMillis;

void setup()
{
  pinMode(WATER_SENSOR_ACTIVATE_PIN, OUTPUT);
  pinMode(WATER_SENSOR_PIN, INPUT);

  watering.setup();
  watering.setCallback(wateringFinishedCallback);

  Serial.begin(57600);
  Wire.begin();

  setupWifi();
  setupMqttClient();
  setupBME280();
}

void loop()
{
  if (!mqttClient.connected())
  {
    reconnectMqttClient();
  }

  bool isTimeForMeasure = (millis() - lastMeasureMillis) > waitAfterMeasureMillis;

  if (!measuringInProgress && (values.isEmpty() || isTimeForMeasure))
  {
    doMeasure();
  }

  mqttClient.loop();
  watering.loop();
}

void doMeasure()
{
  measuringInProgress = true;

  values = getSensorValues();

  debugOutputValues(values);
  publishSensorValues(values);

  lastMeasureMillis = millis();
  measuringInProgress = false;
}

void setupWifi()
{
  Serial.print("Connecting to ");
  Serial.println(ssid);

  showInfo("connecting to wifi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);
}

void setupMqttClient()
{
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
}

void setupBME280()
{
  while (!bme.begin())
  {
    showInfo("BME280", "Could not find BME280 sensor!");
    delay(1000);
  }

  switch (bme.chipModel())
  {
  case BME280::ChipModel_BME280:
    Serial.println("Found BME280 sensor! Success.");
    break;
  case BME280::ChipModel_BMP280:
    Serial.println("Found BMP280 sensor! No Humidity available.");
    break;
  default:
    Serial.println("Found UNKNOWN sensor! Error!");
  }
}

SensorValues getSensorValues()
{
  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);
  bme.read(pres, temp, hum, tempUnit, presUnit);

  int soilMoisture = getWaterValue();

  return SensorValues(temp, hum, pres, soilMoisture);
}

int getWaterValue()
{
  // enable water sensor
  digitalWrite(WATER_SENSOR_ACTIVATE_PIN, HIGH);

  delay(1000);

  // read value
  int value = analogRead(WATER_SENSOR_PIN);

  // disable water sensor
  digitalWrite(WATER_SENSOR_ACTIVATE_PIN, LOW);

  return value;
}

void debugOutputValues(SensorValues values)
{
  Serial.print("temperature: ");
  Serial.print(values.getTemperature());
  Serial.println("C");

  Serial.print("humidity: ");
  Serial.print(values.getHumidity());
  Serial.println("%");

  Serial.print("pressure: ");
  Serial.print(values.getPressure());
  Serial.println("hPa");

  Serial.print("water: ");
  Serial.println(values.getSoilMoisture());
}

void publishSensorValues(SensorValues values)
{
  JsonObject &json = jsonBuffer.createObject();

  json["Temperature"] = values.getTemperature();
  json["Humidity"] = values.getHumidity();
  json["SoilMoisture"] = values.getSoilMoisture();
  json["Pressure"] = values.getPressure();

  char buffer[90];
  json.printTo(buffer);

  mqttClient.publish(TOPIC_SENSORS_EVENT, buffer);
}

void reconnectMqttClient()
{
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttClient.connect("watering_system", mqtt_user, mqtt_password))
    {
      Serial.println("connected");

      mqttClient.subscribe(TOPIC_ACTION_SUBSCRIPTION);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void showInfo(String line1)
{
  showInfo(line1, "");
}

void showInfo(String line1, String line2)
{
  Serial.println(line1);

  if (line2.length() > 0)
  {
    Serial.println(line2);
  }
}

void wateringFinishedCallback(char *action)
{
  mqttClient.publish(TOPIC_WATERING_EVENT, action);
}

// handle message arrived
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  String payloadString = Helper::byteArrayToString(payload, length);
  String topicString = String(topic);

  Serial.print("Message arrived [");
  Serial.print(topicString);
  Serial.println("]");

  Serial.print("payload: ");
  Serial.println(payloadString);

  if (topicString == TOPIC_WATERING_ACTION)
  {
    int wateringMilliseconds = payloadString.toInt();
    Serial.print("watering value as int: ");
    Serial.println(wateringMilliseconds);

    // value in rage (between 1 second and 3 minutes)?
    if ((wateringMilliseconds >= 1000) && (wateringMilliseconds <= 180000))
    {
      showInfo("MQTT action", "watering " + String(wateringMilliseconds) + " millis");

      watering.startPump(wateringMilliseconds);
    }
  }
  else if (topicString == TOPIC_MEASURING_ACTION)
  {
    showInfo("MQTT action", "measuring...");

    doMeasure();
  }
  else if (topicString == TOPIC_DEEPSLEEP_ACTION)
  {
    if (payloadString.length() < 1)
    {
      // no payload
      return;
    }

    // convert payload (seconds to µ)
    uint64_t sleepTimeMicroSeconds = payloadString.toInt() * 1e6;

    ESP.deepSleep(sleepTimeMicroSeconds);
  }
}
