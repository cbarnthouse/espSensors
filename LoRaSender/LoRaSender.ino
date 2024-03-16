#include <SPI.h>
#include <LoRa.h>

// Define LoRa packet structure

typedef struct {
  char sensorId[20];
  float value;
} SensorData;

// LoRa parameters
const int lora_cs = 18;   // LoRa radio chip select
const int lora_rst = 14;  // LoRa radio reset
const int lora_irq = 26;  // change for your board; must be a hardware interrupt pin
const long lora_frequency = 915E6;


// Initialize LoRa
void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.println("Setup Serial");

  LoRa.setPins(lora_cs, lora_rst, lora_irq);

  if (!LoRa.begin(lora_frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
  Serial.println("LoRa initialization successful.");
}

void loop() {
  // Create and populate SensorData struct
  SensorData data;
  strcpy(data.sensorId, "sensor_1");
  data.value = 25.5; // Sample sensor value

  // Send LoRa packet
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&data, sizeof(SensorData));
  LoRa.endPacket();
  
  Serial.println("Packet sent");
  delay(5000); // Send every 5 seconds
}