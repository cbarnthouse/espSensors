#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "softSerial.h"
String host_id = "temperature_1"; // Change this to whatever you want!
String voltage = "0";

#define timetosleep 100
#define timetowake 500 * 60 * 1 // Every 10 minutes

static TimerEvent_t sleep;
static TimerEvent_t wakeup;
uint8_t lowpower = 1;
bool done = false;

#define RF_FREQUENCY                                915000000
#define TX_OUTPUT_POWER                             14
#define LORA_BANDWIDTH                              0
#define LORA_SPREADING_FACTOR                       8
#define LORA_CODINGRATE                             4
#define LORA_PREAMBLE_LENGTH                        8
#define LORA_SYMBOL_TIMEOUT                         0
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30

char txPacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );

typedef enum
{
  LOWPOWER,
  ReadVoltage,
  TX
} States_t;

States_t state;
bool sleepMode = false;
int16_t rssi, rxSize;

void setup() {

  
  Serial.begin(115200);
  delay(1000);
  rssi = 0;
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;

  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
    true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

}

void loop()
{

    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    delay(1000);

    float t = 10; // Fahrenheit = true

    Radio.IrqProcess();

    // InfluxDB line protocol format
    String reading = "/home/soil/temp:" + String(t);

    Serial.print(reading);

    Radio.Send((uint8_t *)reading.c_str(), reading.length() );
    Serial.print("Sent over Lora radio");
    delay(500);
     
    pinMode(Vext, OUTPUT);  
    digitalWrite(Vext, HIGH);
    delay(500);

    done = true;

   // enterDeepSleep(60000);
  }

void enterDeepSleep(unsigned long duration) {
  // Enter deep sleep mode
  // Set the duration for the sleep
  Radio.Sleep();

 // LowPowerClass::sleep(duration);
}



void OnTxDone( void )
{
  Serial.print("TX done!");
}

void OnTxTimeout( void )
{
  Serial.print("TX Timeout......");
}
