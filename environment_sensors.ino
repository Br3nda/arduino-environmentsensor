#include <VirtualWire.h>
#include <dht.h>

dht DHT;

#define SOIL_PIN_1 7
#define SOIL_PIN_2 5
#define SENSOR_PIN 0
#define TIME_BETWEEN_READINGS 1000
#define LED_PIN 11
#define TRANSMIT_PIN 12
#define RECIEVE_PIN 2
#define TRANSMIT_EN_PIN 3
#define DHT11_PIN 11

const int plant_number = 1;

void setup() {

  //serial port comms, mostly for debugging
  Serial.begin(115200);

  //set up the plant sensor
  pinMode(SOIL_PIN_1, OUTPUT);
  pinMode(SOIL_PIN_2, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);

  // Initialise the IO and ISR
  vw_set_tx_pin(TRANSMIT_PIN);
  vw_set_rx_pin(RECIEVE_PIN);
  vw_set_ptt_pin(TRANSMIT_EN_PIN);
  vw_set_ptt_inverted(true); // Required for DR3100
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

  //average the two values
  debugMessage("Reading one = ", val1);
  debugMessage("Reading two = ", val2);

  int avg = (val1 + val2) / 2;
  debugMessage("Average reading = ", avg);

 
  sendDataViaRF("moisture", avg);
}

void debugMessage(String message, int value) {
  message += value;
  Serial.println(message);
}



byte count = 1;

void sendDataViaRF(String readingType, int reading) {
  debugMessage("Sending RF message, %s = ", reading);

  char msg[4] = {'p', plant_number, ':', reading};
  msg[6] = count;
  digitalWrite(LED_PIN, HIGH); // Flash a light to show transmitting
  vw_send((uint8_t *)msg, 7);
  vw_wait_tx(); // Wait until the whole message is gone
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  count = count + 1;
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
  // DISPLAY DATA
  Serial.print("Humidity: ");
  Serial.println(DHT.humidity, 1);
//  sendDataViaRF("humidity", DHT.humidity);
  
  Serial.print("Temp: ");
  Serial.println(DHT.temperature, 1);
//  sendDataViaRF("temperature", DHT.temperature);
}

void loop() {
//  readPlantMoisture();
  readTempHumidity();
  delay(TIME_BETWEEN_READINGS);
}


