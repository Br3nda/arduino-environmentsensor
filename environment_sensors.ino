#include <VirtualWire.h>
#include <dht.h>
#include <stdio.h>
#include <ArduinoJson.h>

dht DHT;

#define SOIL_PIN_1 7
#define SOIL_PIN_2 5
#define SENSOR_PIN 0
#define TIME_BETWEEN_READINGS 1000
#define LED_PIN 11
#define TRANSMIT_PIN 12
//#define RECIEVE_PIN 2
#define TRANSMIT_EN_PIN 3
#define DHT11_PIN 11


char SENSOR_NUMBER = 'A';

void setup() {

  //serial port comms, mostly for debugging
  Serial.begin(115200);

  //set up the plant sensor
  pinMode(SOIL_PIN_1, OUTPUT);
  pinMode(SOIL_PIN_2, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);

  // Initialise the IO and ISR
  vw_set_tx_pin(TRANSMIT_PIN);
//  vw_set_rx_pin(RECIEVE_PIN);
  vw_set_ptt_pin(TRANSMIT_EN_PIN);
//  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);       // Bits per sec
  pinMode(LED_PIN, OUTPUT);
  
}


void setSensorPolarity(boolean flip) {
  if (flip) {
    digitalWrite(SOIL_PIN_1, HIGH);
    digitalWrite(SOIL_PIN_2, LOW);
  }
  else {
    digitalWrite(SOIL_PIN_1, LOW);
    digitalWrite(SOIL_PIN_2, HIGH);
  }
}


void readPlantMoisture() {
  /**
  Read moisture (voltage change) across 2 pins
  Then read in reverse.
  Alternating polatity makes the metal in the pot last longer.
  */
  
  Serial.println("Reading moisture");
  
  //first read
  setSensorPolarity(true);
  delay(TIME_BETWEEN_READINGS);
  int val1 = analogRead(SENSOR_PIN);
  delay(TIME_BETWEEN_READINGS);


  //second read, opposite polarity
  setSensorPolarity(false);
  delay(TIME_BETWEEN_READINGS);
  // invert the reading
  int val2 = 1023 - analogRead(SENSOR_PIN);

  float avg = (val1 + val2) / 2;

  Serial.println("done");
  sendDataViaRF("moisture", avg);
}

/**
* Send data via the 433MHz transmitter
*/
void sendDataViaRF(String reading_type, float reading_value) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sensor"] = "coffee";
  root["type"] = reading_type;
  root["value"] = reading_value;

  char msg[256];
  root.printTo(msg, sizeof(msg));
  
  digitalWrite(LED_PIN, HIGH); // Flash a light to show transmitting
  
  Serial.println(msg);
  
  vw_send((uint8_t *)msg, strlen(msg));
  // Wait until the whole message is gone
  vw_wait_tx(); 

  delay(1000);

  digitalWrite(LED_PIN, LOW);

}

void readTempHumidity() {
  // READ DATA
  int chk = DHT.read11(DHT11_PIN);
  switch (chk)
  {
    case DHTLIB_OK:
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.println("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.println("Time out error,\t");
      break;
    default:
      Serial.println("Unknown error,\t");
      break;
  }
  
  sendDataViaRF("humidity", DHT.humidity);
  sendDataViaRF("temperature", DHT.temperature);
 
  delay(1000); // ensure we don't read too often
}

void loop() {
  readPlantMoisture();
  readTempHumidity();
}


