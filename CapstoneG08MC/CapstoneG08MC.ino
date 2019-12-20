#include <ArduinoJson.h>
#include <ADC.h>
#include <vector>

// constant declarations
#define BAUD_RATE 115200

#define NUM_SAMPLE_MIN 256
#define NUM_SAMPLE_MAX 2048000

#define TOLERANCE 0.0075


#define NUM_SAMPLES 256
#define RESISTOR 3.3

ADC *adc = new ADC();

// pin declarations
const int voltPin = A9; // place holder
const int currPin = A3; // place holder
const int sleepPin = 4; // placeholder

// global variables
double voltReading = 0.0;
double currReading = 0.0;
long sampleNum = 0;

// global variables
double voltReadings[NUM_SAMPLES];
double currReadings[NUM_SAMPLES];

// commands
const char* START_COMMAND = "START";
const char* NO_COMMAND = "NO_COMMAND";

const char* cmd = "NO_COMMAND";
unsigned long time;

void setup() {
  // configure pins
  pinMode(voltPin, INPUT);
  pinMode(currPin, INPUT);
  pinMode(A10, INPUT);
  pinMode(A11, INPUT);
  pinMode(sleepPin, INPUT);

  // start serial
  Serial.begin(BAUD_RATE);
  Serial1.begin(BAUD_RATE);

  Serial.flush();
  Serial1.flush();

  // configure ADC0
  adc->setAveraging(0, ADC_0);
  adc->setResolution(13, ADC_0); 
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_0);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_0);
  adc->setReference(ADC_REFERENCE::REF_3V3, ADC_0);

  // configure ADC1
  adc->setAveraging(0, ADC_1); 
  adc->setResolution(16, ADC_1); 
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_1);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_1);

  //adc->startSynchronizedContinuous(voltPin, currPin);
  //adc->startSynchronizedContinuousDifferential(A10, A11, A12, A13);

  adc->startContinuous(voltPin, ADC_0);
  adc->startContinuousDifferential(A10, A11, ADC_0);

  delay(2000);
}

ADC::Sync_result reading;
double readingADC0;
double readingADC1;
void loop() {
  if(Serial1.available()){
    if(strcmp(cmd, NO_COMMAND) == 0) {
      handleReceivedMessage();
    } 
  }
  //while(!Serial.available()){
    
  //}
    
 
  if(strcmp(cmd, START_COMMAND) == 0) {
//      long isample = 0;
//      
//      time = millis();
//      readingADC0 = adc->analogRead(voltPin, ADC_0);
//      readingADC1 = adc->analogReadDifferential(A10,A11, ADC_1);
//      voltReading = (double)readingADC0*3.3/adc->getMaxValue(ADC_0);
//      currReading = RESISTOR*(double)readingADC1*3.3/adc->getMaxValue(ADC_1);
//
//      double lastReadingMin = (double)voltReading*(1.0-TOLERANCE);
//      double lastReadingMax = (double)voltReading*(1.0+TOLERANCE);
//
//      isample++;
//      while((isample != NUM_SAMPLE_MAX || isample > NUM_SAMPLE_MAX) && voltReading > lastReadingMin && voltReading < lastReadingMax) {
//        //for(int isample = 0; isample < NUM_SAMPLE_MIN; isample++) {
//
//          sendData();
//      //for(long isample = 0; isample < NUM_SAMPLES; isample++) {
//        // get analog pin readings
//        //reading = adc->analogSynchronizedRead(voltPin, currPin);
//      
//        //reading = adc->analogSynchronizedReadDifferential(A10, A11, A12, A13);
//        
//        //reading.result_adc0 = (double)reading.result_adc0;
//        //reading.result_adc1 = (double)reading.result_adc1;
//      
//        //voltReading = (double)reading.result_adc0*3.3/adc->getMaxValue(ADC_0);
//        //currReading = RESISTOR*(double)reading.result_adc1*3.3/adc->getMaxValue(ADC_1);
//          if(isample >= NUM_SAMPLE_MAX) {
//            break;
//          }
//          time = millis();
//          // get analog pin readings
//          readingADC0 = adc->analogRead(voltPin, ADC_0);
//          readingADC1 = adc->analogReadDifferential(A10,A11, ADC_1);
//          voltReading = (double)readingADC0*3.3/adc->getMaxValue(ADC_0);
//          currReading = RESISTOR*(double)readingADC1*3.3/adc->getMaxValue(ADC_1);
//
//          isample++;  
//      }

      for(long isample = 0; isample < NUM_SAMPLES; isample++) {
          time = millis();
          // get analog pin readings
          readingADC0 = adc->analogRead(voltPin, ADC_0);
          readingADC1 = adc->analogReadDifferential(A10,A11, ADC_1);
          //voltReading = (double)readingADC0*3.3/adc->getMaxValue(ADC_0);
          //currReading = (double)readingADC1*3.3/adc->getMaxValue(ADC_1)/(double)RESISTOR;
          voltReadings[isample] = (double)readingADC0*3.3/adc->getMaxValue(ADC_0);
          currReadings[isample] = (double)readingADC1*3.3/adc->getMaxValue(ADC_1)/(double)RESISTOR;

          //sendData();
      }
      sendReadings();
      // sleep
      while(true){
        delay(3600);
      }
   }
}

void handleReceivedMessage() {
   DynamicJsonDocument data(50);
   deserializeJson(data, Serial1);
   cmd = data["command"];
   sampleNum = data["sampleNum"];
}

void sendReadings() {
    DynamicJsonDocument dataBuffer(200);
    JsonObject data = dataBuffer.to<JsonObject>();

    for(long isample = 0; isample < NUM_SAMPLES; isample++) {
          data["sampleNum"] = isample;
          data["time"] = "2019-11-15 16:07";
          data["volt"] = (double)voltReadings[isample];
          data["curr"] = (double)currReadings[isample];

//          Serial1.print(isample);
//          Serial1.print(",");
//          Serial1.print(voltReading, DEC);
//          Serial1.print(",");
//          Serial1.print(currReading, DEC);
//          Serial1.print(",");
//          Serial1.println(time, DEC);

          serializeJson(data, Serial1);
          delay(300);
      
    }
  
}
void sendData() {
    DynamicJsonDocument dataBuffer(100);
    JsonObject data = dataBuffer.to<JsonObject>();
    data["sampleNum"] = sampleNum;
    data["time"] = "2019-11-15 16:07";
    data["volt"] = voltReading;
    data["curr"] = currReading;
    sampleNum++;
//
//    Serial.print(sampleNum);
//    Serial.print(",");
//    Serial.print(voltReading, DEC);
//    Serial.print(",");
//    Serial.print(currReading, DEC);
//    Serial.print(",");
//    Serial.println(time, DEC);
    
    Serial1.print(sampleNum);
    Serial1.print(",");
    Serial1.print(voltReading, DEC);
    Serial1.print(",");
    Serial1.print(currReading, DEC);
    Serial1.print(",");
    Serial1.println(time, DEC);
    delay(500);
    
   //serializeJson(data, Serial1);
   
   //serializeJson(data, Serial);
   //Serial.println();

}
