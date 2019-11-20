#include <XBee.h>
#include <ArduinoJson.h>
#include <ADC.h>

// constant declarations
#define BAUD_RATE 9600

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
Rx64Response rx64 = Rx64Response();

ADC *adc = new ADC();

// pin declarations
const int voltPin = A9; // place holder
const int currPin = A3; // place holder
const int rxPin = 2; // place holder
const int txPin = 3; // placeholder
const int sleepPin = 4; // placeholder

// global variables
double voltReading = 0;
double currReading = 0;
long sampleNum = 0;

char* cmd;

uint8_t option = 0;
uint8_t data = 0;

void setup() {
  // configure pins
  pinMode(voltPin, INPUT);
  pinMode(currPin, INPUT);
  pinMode(sleepPin, INPUT);

  // start serial
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    // wait till initialization is complete
  }
  Serial.flush();
  xbee.begin(Serial);

  // configure ADC0
  adc->setAveraging(0, ADC_0);
  adc->setResolution(12, ADC_0); 
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_LOW_SPEED, ADC_0);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_LOW_SPEED, ADC_0);
  adc->setReference(ADC_REFERENCE::REF_3V3, ADC_0);

  // configure ADC1
  adc->setAveraging(0, ADC_1); 
  adc->setResolution(12, ADC_1); 
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_LOW_SPEED, ADC_1);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_LOW_SPEED, ADC_1);

  adc->startSynchronizedContinuous(voltPin, currPin);

  delay(100);

}

ADC::Sync_result reading;

void loop() {
  // get analog pin readings
  reading = adc->analogSynchronizedRead(voltPin, currPin);
  reading.result_adc0 = (double)reading.result_adc0;
  reading.result_adc1 = (double)reading.result_adc1;

  voltReading = (double)reading.result_adc0*3.3/adc->getMaxValue(ADC_0);
  currReading = (double)reading.result_adc1;
  
  sendData();
}

void handleCommand (uint8_t data) {
//  DeserializationError error = deserializeMsgPack(packet, data);

}

void sendData() {
    StaticJsonDocument<200> data;
    data["sampleNum"] = sampleNum;
    data["time"] = "2019-11-15 16:07";
    data["volt"] = voltReading;
    data["curr"] = currReading;
    sampleNum++;
    serializeJson(data, Serial);
    Serial.println();
   // serializeJsonPretty(data, Serial);
}

double readVoltPin() {
  double voltage = analogRead(voltPin); // fake data
  return voltage * (3.3 / 4095.0);;
}
double readCurrPin() {
  double current = 0.12234; // fake data
  return current;
}
