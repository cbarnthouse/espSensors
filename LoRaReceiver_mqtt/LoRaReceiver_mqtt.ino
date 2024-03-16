#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define LORA_BAND 915E6  // US LoRa frequency band
#define LORA_SS_PIN 18    // LoRa radio SPI CS pin
#define LORA_RST_PIN 14   // LoRa radio reset pin
#define LORA_DIO0_PIN 26  // LoRa radio IRQ pin
#define MQTT_SERVER "iot-gw.barnthouse.net" // MQTT Server address
#define MQTT_PORT 1883 // MQTT Server port
#define WIFI_SSID "fw-iot" // Your WiFi SSID
#define WIFI_PASSWORD "barnthouseiot" // Your WiFi password
#define MQTT_TOPIC_PREFIX "home"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void onReceive(int packetSize) {
  if (packetSize == 0) return; // No packet received
  
  String receivedData = "";
  while (LoRa.available()) {
    receivedData += (char)LoRa.read();
  }

  // Convert received data to MQTT topic and value
  String topic = String(MQTT_TOPIC_PREFIX) + "/data";
  String value = receivedData;

  // Publish to MQTT server
  mqttClient.publish(topic.c_str(), value.c_str());
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("LoRa initialization failed!");
    while (1);
  }
  
  // Initialize WiFi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Initialize MQTT client
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

  // Set LoRa receive callback function
  LoRa.onReceive(onReceive);

  // Start LoRa receive mode
  LoRa.receive();
}

void loop() {
  if (!mqttClient.connected()) {
    // Reconnect to MQTT server
    if (mqttClient.connect("LoRaClient")) {
      Serial.println("Connected to MQTT server");
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying...");
      delay(5000);
      return;
    }
  }

  // Maintain MQTT connection
  mqttClient.loop();
}
