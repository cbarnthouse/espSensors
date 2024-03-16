#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "DHTesp.h"
#include "heltec.h"
#include <Wire.h>


#define BAND 868E6

String rssi = "RSSI --";
String packSize = "--";
String packet ;

char ssid[] = "YOUR_SSID";     //  your network SSID (name) 
char pass[] = "YOUR_PASSWORD";    // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

// the IP address of your InfluxDB host
byte host[] = {192, 168, 1, 46};

// the port that the InfluxDB UDP plugin is listening on
int port = 8089;

WiFiUDP udp;

void setup() {


  LoRa.setSpreadingFactor(8);
    // put in standby mode
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(4);
  LoRa.setSyncWord(0x12);   //0x34
  LoRa.setPreambleLength(8);

  delay(2000);

  adcAttachPin(37);
  analogSetClockDiv(255); // 1338mS

 
  // initialize serial port
  Serial.begin(115200);

  // attempt to connect using WPA2 encryption
  Serial.println("Attempting to connect to WPA network...");
  WiFi.disconnect();
  delay(10);
  status = WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println(WiFi.status());

  }
  
  // if the connection succeeded, print network info
  Serial.println("Connected to network");
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  
  Serial.print("IP Address: ");
  Serial.println(ip);

  
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("WiFi signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  digitalWrite(21,LOW);

  LoRa.receive();
}

void LoRaData(){
  String line;
  
  char str_array[packet.length() + 1];
  packet.toCharArray(str_array, packet.length());
  
  // send the packet
  Serial.println("Sending UDP packet...");
  Serial.println(packet);

  udp.beginPacket(host, port);
  udp.print(packet);
  udp.endPacket();

  digitalWrite(25,LOW);
  Serial.println("Done!");
}

void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  LoRaData();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) { cbk(packetSize);  }
  delay(50);

  if (WiFi.status() != WL_CONNECTED){
    delay(2000);
    setup();
    return;
  }
}
