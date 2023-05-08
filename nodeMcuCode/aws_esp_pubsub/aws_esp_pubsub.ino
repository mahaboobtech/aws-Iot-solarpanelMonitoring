#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error Platform not supported
#endif
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#define emptyString String()
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "LiquidCrystal.h"

#include "secrets.h"

Adafruit_INA219 ina219;
#define DHT_PIN_DATA 0
#define LCD_PIN_RS 16
#define LCD_PIN_E 15
#define LCD_PIN_DB4 2
#define LCD_PIN_DB5 14
#define LCD_PIN_DB6 12
#define LCD_PIN_DB7 13
#define LDR_PIN_SIG A0

LiquidCrystal lcd(LCD_PIN_RS, LCD_PIN_E, LCD_PIN_DB4, LCD_PIN_DB5, LCD_PIN_DB6, LCD_PIN_DB7);

#if !(ARDUINOJSON_VERSION_MAJOR == 6 and ARDUINOJSON_VERSION_MINOR >= 7)
#error "Install ArduinoJson v6.7.0-beta or higher"
#endif

const int MQTT_PORT = 8883;
const char *MQTT_SUB_TOPIC = "$aws/things/" THINGNAME "/shadow/update/delta";
const char *MQTT_SUB_TOPIC1 = "$aws/things/" THINGNAME "/shadow/get/accepted";
const char *MQTT_PUB_TOPIC = "$aws/things/" THINGNAME "/shadow/update";

#ifdef USE_SUMMER_TIME_DST
uint8_t DST = 1;
#else
uint8_t DST = 0;
#endif

WiFiClientSecure net;

#ifdef ESP8266
BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);
#endif

PubSubClient client(net);

time_t now;
time_t nowish = 1510592825;
#define DHTTYPE DHT11 // DHT 11

const int DHTPin = 5; // D1 Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// const int red = D2;
const int led = D10;

void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, DST * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void messageReceived(char *topic, byte *payload, unsigned int length)
{
  String msgIN = "";
  for (int i = 0; i < length; i++)
  {
    msgIN += (char)payload[i];
  }
  String msgString = msgIN;
  Serial.println("Recieved [" + String(topic) + "]: " + msgString);

  StaticJsonDocument<2000> doc;
  //  StaticJsonDocument<64> filter;

  DeserializationError error = deserializeJson(doc, msgString);

  // Test if parsing succeeds.
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  if (String(topic) == "$aws/things/esp_test8266/shadow/update/delta")
  {
    String ledstr = doc["state"]["led1"];
    // String greenstr = doc["state"]["green"];
    // Serial.println("Red: "+redstr);
    //    char ledstr = (char)payload[30];
    Serial.println("led: " + ledstr);
    if (ledstr == "1")
    {
      digitalWrite(led, HIGH);
      delay(2000);
      digitalWrite(led, LOW);
    }
    else
    {
      digitalWrite(led, LOW);
    }
  }
  if (String(topic) == "$aws/things/esp_test8266/shadow/get/accepted")
  {
    String ledstr = doc["state"]["desired"]["led"];
    // String greenstr = doc["state"]["desired"]["green"];
    Serial.println("Get led: " + ledstr);

    if (ledstr == "1")
    {
      digitalWrite(led, HIGH);
    }
    else
    {
      digitalWrite(led, LOW);
    }

    String str = "{ \"state\": { \"reported\": { \"led\":" + ledstr + "} } }";
    client.publish("$aws/things/esp_test8266/shadow/update", str.c_str());
  }
  Serial.println();
}

void pubSubErr(int8_t MQTTErr)
{
  if (MQTTErr == MQTT_CONNECTION_TIMEOUT)
    Serial.print("Connection tiemout");
  else if (MQTTErr == MQTT_CONNECTION_LOST)
    Serial.print("Connection lost");
  else if (MQTTErr == MQTT_CONNECT_FAILED)
    Serial.print("Connect failed");
  else if (MQTTErr == MQTT_DISCONNECTED)
    Serial.print("Disconnected");
  else if (MQTTErr == MQTT_CONNECTED)
    Serial.print("Connected");
  else if (MQTTErr == MQTT_CONNECT_BAD_PROTOCOL)
    Serial.print("Connect bad protocol");
  else if (MQTTErr == MQTT_CONNECT_BAD_CLIENT_ID)
    Serial.print("Connect bad Client-ID");
  else if (MQTTErr == MQTT_CONNECT_UNAVAILABLE)
    Serial.print("Connect unavailable");
  else if (MQTTErr == MQTT_CONNECT_BAD_CREDENTIALS)
    Serial.print("Connect bad credentials");
  else if (MQTTErr == MQTT_CONNECT_UNAUTHORIZED)
    Serial.print("Connect unauthorized");
}

void connectToMqtt(bool nonBlocking = false)
{
  Serial.print("MQTT connecting ");
  while (!client.connected())
  {
    if (client.connect(THINGNAME))
    {
      Serial.println("connected!");
      if (!client.subscribe(MQTT_SUB_TOPIC))
        pubSubErr(client.state());
      if (!client.subscribe(MQTT_SUB_TOPIC1))
        pubSubErr(client.state());

      client.publish("$aws/things/esp_test8266/shadow/get", "hi"); // For getting last saved state
      Serial.println("the led is updated");
    }
    else
    {
      Serial.print("failed, reason -> ");
      pubSubErr(client.state());
      if (!nonBlocking)
      {
        Serial.println(" < try again in 5 seconds");
        delay(5000);
      }
      else
      {
        Serial.println(" <");
      }
    }
    if (nonBlocking)
      break;
  }
}

void connectToWiFi(String init_str)
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void checkWiFiThenMQTT(void)
{
  connectToWiFi("Checking WiFi");
  connectToMqtt();
}

void setup()
{
  Serial.begin(115200);
  lcd.begin(16, 2);

  pinMode(led, OUTPUT);
  // pinMode(green, OUTPUT);
  dht.begin();

  delay(5000);
  Serial.println();
  if (!ina219.begin())
  {
    Serial.println("Failed to find INA219 chip");
    // while (1)
     //{
       //Serial.println("failed to load");
   //}
  }
  Serial.println("Measuring voltage and current with INA219 ...");

  WiFi.begin(ssid, pass);
  connectToWiFi(String("Attempting to connect to SSID: ") + String(ssid));

  NTPConnect();

#ifdef ESP32
  net.setCACert(cacert);
  net.setCertificate(client_cert);
  net.setPrivateKey(privkey);
#else
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);
#endif

  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(messageReceived);

  connectToMqtt();
}

void loop()
{
  now = time(nullptr);
  if (!client.connected())
  {
    checkWiFiThenMQTT();
  }
  else
  {
    client.loop();
    float h = dht.readHumidity();
    float temp = dht.readTemperature();
    //if (isnan(h) || isnan(temp))
    //{
    //  Serial.println("Failed to read from DHT sensor!");
    //  return;
    //}
    float shuntvoltage = 0;
    float busvoltage = 0;
    float current_mA = 0;
    float loadvoltage = 0;
    float power_mW = 0;

    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getPower_mW();
    loadvoltage = busvoltage + (shuntvoltage / 1000);

    Serial.print("Bus Voltage:   ");
    Serial.print(busvoltage);
    Serial.println(" V");
    Serial.print("Shunt Voltage: ");
    Serial.print(shuntvoltage);
    Serial.println(" mV");
    Serial.print("Load Voltage:  ");
    Serial.print(loadvoltage);
    Serial.println(" V");
    Serial.print("Current:       ");
    Serial.print(current_mA);
    Serial.println(" mA");
    Serial.print("Power:         ");
    Serial.print(power_mW);
    Serial.println(" mW");
    Serial.println("");

    delay(1000);

    lcd.setCursor(0, 0);
    int loadvoltagei = loadvoltage;
    // sets the cursor at row 0 column 0
    lcd.print(String("voltage :") + String(loadvoltagei)); // prints 16x2 LCD MODULE
    lcd.setCursor(2, 1);
    // sets the cursor at row 1 column 2
    int current_mAi = current_mA;
    lcd.print(String("current :") + String(current_mAi)); // prints HELLO WORLD
    int sensorValue = analogRead(A0);   // read the input on analog pin 0

    float ldrVoltage = sensorValue * (5.0 / 1023.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V)

    // Serial.println(ldrVoltage);   // print out the value you read
    Serial.println("running");
    float templove = 56.002;
    float hlove = 76.0021;
    float loadvoltagelove = 18;
    float current_mAlove = 1;
    float power_mWlove = loadvoltagelove * current_mAlove;
    float ldrVoltageLove = 0.9;
    String str = "{\"temp\" : " + String(templove) + " , \"humidity\": " + String(hlove) +",\"ldr\":" +String(ldrVoltageLove) +",\"voltage\":" +String(loadvoltagelove)+",\"current\":" +String(current_mAlove)+",\"power\":" +String(power_mWlove)+"}";
    client.publish("esp8266/pub", str.c_str());
    delay(5000);
  }
}
