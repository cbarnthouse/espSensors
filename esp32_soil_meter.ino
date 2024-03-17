//Built for ESP3-WROOM Mini
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "fw-iot";
const char* password = "barnthouseiot";
const char* mqtt_server = "172.16.129.15";

const int soilMoisturePin = 34; // Analog pin connected to the soil moisture sensor
const int ledPin = 2; // Built-in LED pin
const int maxWifiAttempts = 5;
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

WiFiClient espClient;
PubSubClient client(espClient);

RTC_DATA_ATTR int bootCount = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < maxWifiAttempts) {
    delay(400);
    Serial.print(".");
    wifiAttempts++;
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-Soil";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // Turn on the built-in LED

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int soilMoistureValue = analogRead(soilMoisturePin);
  Serial.print(soilMoistureValue);
  Serial.print("C");
  Serial.print((float(analogRead(soilMoisturePin))/1023.0)*3.3); // read sensor
  Serial.println(" V");
  int percentage = map(soilMoistureValue, 1600, 3400, 100, 0); // Mapping the ADC values to percentage

  Serial.print("Soil Moisture: ");
  Serial.print(percentage);
  Serial.println("%");

  char msg[50];
  sprintf(msg, "%d", percentage);
  client.publish("home/soil/moisture/1", msg);


  delay(1000); // Wait for a short time before going to sleep

  // Increment boot count and print it
  bootCount++;
  char msgboot[50];
  sprintf(msgboot, "%d", bootCount);
  client.publish("home/esp32/soil/bootCount", msgboot);
  Serial.println("Boot count: " + String(bootCount));
  delay(1000);

  
  //Disconnect from mqtt and wifi
  client.disconnect();
  delay(500);
  WiFi.disconnect(true);
  // Enter deep sleep 
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Entering deep sleep");
  digitalWrite(ledPin, LOW); // Turn off the built-in LED
  esp_deep_sleep_start();
}
