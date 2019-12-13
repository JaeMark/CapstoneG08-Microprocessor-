#include <ArduinoJson.h>
#include <ADC.h>

// constant declarations
#define BAUD_RATE 9600

ADC *adc = new ADC();

// pin declarations
const int voltPin = A9; // place holder
const int currPin = A3; // place holder
const int sleepPin = 4; // placeholder

// global variables
double voltReading = 0;
double currReading = 0;
long sampleNum = 0;

// commands
const char* START_COMMAND = "START";
const char* NO_COMMAND = "NO_COMMAND";

const char* cmd = "NO_COMMAND";

void setup() {
  // configure pins
  pinMode(voltPin, INPUT);
  pinMode(currPin, INPUT);
  pinMode(sleepPin, INPUT);

  // start serial
  Serial.begin(BAUD_RATE);
  Serial1.begin(BAUD_RATE);

  Serial.flush();
  Serial1.flush();

  // configure ADC0
  adc->setAveraging(0, ADC_0);
  adc->setResolution(13, ADC_0); 
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_0);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_0);
  adc->setReference(ADC_REFERENCE::REF_3V3, ADC_0);

  // configure ADC1
  adc->setAveraging(0, ADC_1); 
  adc->setResolution(13, ADC_1); 
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_1);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_1);

  adc->startSynchronizedContinuous(voltPin, currPin);
  //adc->startSynchronizedContinuousDifferential(A10, A11, A12, A13);

  delay(2000);
}

ADC::Sync_result reading;

void loop() {
  if(Serial1.available()){
    if(strcmp(cmd, NO_COMMAND) == 0) {
      handleReceivedMessage();
    } 
  }
    
  if(strcmp(cmd, START_COMMAND) == 0) {
      if(strcmp(cmd, START_COMMAND) == 0) {
        while(true) {
        // get analog pin readings
        reading = adc->analogSynchronizedRead(voltPin, currPin);
      
        //reading = adc->analogSynchronizedReadDifferential(A10, A11, A12, A13);
       
        reading.result_adc0 = (double)reading.result_adc0;
        reading.result_adc1 = (double)reading.result_adc1;
      
        voltReading = (double)reading.result_adc0*3.3/adc->getMaxValue(ADC_0);
        currReading = (double)reading.result_adc1/adc->getMaxValue(ADC_1);
      
        sendData();
        }
      }
   }

   // sleep
}

void handleReceivedMessage() {
   DynamicJsonDocument data(100);
   deserializeJson(data, Serial1);
   cmd = data["command"];
   sampleNum = data["sampleNum"];
}

void sendData() {
    DynamicJsonDocument dataBuffer(100);
    JsonObject data = dataBuffer.to<JsonObject>();
    data["sampleNum"] = sampleNum;
    data["time"] = "2019-11-15 16:07";
    data["volt"] = voltReading;
    data["curr"] = currReading;
    sampleNum++;

   Serial1.write('<');
   serializeJson(data, Serial1);
   Serial1.write('>');
   
   //serializeJson(data, Serial);
   //Serial.println();

}
