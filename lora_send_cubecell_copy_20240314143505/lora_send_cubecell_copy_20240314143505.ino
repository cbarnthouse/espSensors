#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "softSerial.h"

//Define parameters for Lowe power based on heltec example
#define timetillsleep 3000
#define timetillwakeup 300000
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=1;
uint16_t bootCount;

//Lora details
#define RF_FREQUENCY            915000000
#define TX_OUTPUT_POWER         14
#define LORA_BANDWIDTH          0
#define LORA_SPREADING_FACTOR   8
#define LORA_CODINGRATE         4
#define LORA_PREAMBLE_LENGTH    8
#define LORA_FIX_LENGTH_PAYLOAD_ON  false
#define LORA_IQ_INVERSION_ON    false
#define BUFFER_SIZE             30


char txPacket[BUFFER_SIZE];

// Define RadioEvents structure
static RadioEvents_t RadioEvents;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Radio.IrqProcess();
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

  TimerInit( &sleep, onSleep );
  TimerInit( &wakeUp, onWakeUp );
  bootCount=0;
  onSleep();

}

void loop() {
  if(lowpower){
    lowPowerHandler();
  }

  //Obtain values we want to send over lora to MQTT
  //This section should be updated to provide actual values from sensors. 
  float t = 10; // Fahrenheit = true


  // Send Sensor Reading
  String reading = "/home/soil/temp:" + String(t);
  Radio.Send((uint8_t *)reading.c_str(), reading.length() );
  Serial.println("Sent over LoRa radio");
  delay(1500);

//Send Battery level
  uint16_t voltage = getBatteryVoltage();
  Serial.println(voltage);
  int p = map(voltage, 3500, 4200, 0, 100); // Mapping the ADC values to percentage

  Serial.println(p);
  String voltage_p = "/home/soil/battery:" + String(p);
  Radio.Send((uint8_t *)voltage_p.c_str(), voltage_p.length() );

//Count loop. This may help understand how long battery will last in different configurations. 
  bootCount++;
  String boots = String(bootCount);
  Serial.println(boots);
  delay(2500);

}

void onSleep()
{
  Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n",timetillwakeup);
  lowpower=1;
  //timetillwakeup ms later wake up;
  Radio.Sleep( );
  TimerSetValue( &wakeUp, timetillwakeup );
  TimerStart( &wakeUp );
}

void onWakeUp()
{
  Serial.printf("Woke up, %d ms later into lowpower mode.\r\n",timetillsleep);
  lowpower=0;
  //timetillsleep ms later into lowpower mode;
  TimerSetValue( &sleep, timetillsleep );
  TimerStart( &sleep );
}