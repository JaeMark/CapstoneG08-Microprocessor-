#include <ArduinoJson.h>
#include <ADC.h>
#include <vector>

// constant declarations
#define BAUD_RATE 115200

//#define NUM_SAMPLE_MIN 256
//#define NUM_SAMPLE_MAX 2048000
//#define TOLERANCE 0.0075

#define SLEEP_TIME 3600000
#define TRANSMISSION_DELAY 260

#define NUM_SAMPLES 512
#define RESISTOR 10000

ADC *adc = new ADC();

// pin declarations
const int voltPin = A9; 
const int currPin = A3; 
const int currPos = A10; 
const int currNeg = A11; 
const int sleepPin = 7; 
const int switchPin = 8; 

// global variables
double voltReading = 0.0;
double currReading = 0.0;
long sampleNum = 0;

// global variables
double voltReadings[NUM_SAMPLES];
double currReadings[NUM_SAMPLES];
unsigned long timeMicros[NUM_SAMPLES];

// commands
const char* START_COMMAND = "START";
const char* NO_COMMAND = "NO_COMMAND";

const char* cmd = "NO_COMMAND";
unsigned long time;

void setup() {
  // configure pins
  pinMode(voltPin, INPUT);
  pinMode(currPin, INPUT);
  pinMode(currPos, INPUT);
  pinMode(currNeg, INPUT);
  pinMode(sleepPin, OUTPUT);
  pinMode(switchPin, INPUT);

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
  adc->setResolution(16, ADC_1); 
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_1);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_1);

  //adc->startSynchronizedContinuous(voltPin, currPin);
  //adc->startSynchronizedContinuousDifferential(A10, A11, A12, A13);

  adc->startContinuous(voltPin, ADC_0);
  adc->startContinuous(currPin, ADC_1);
  //adc->startContinuousDifferential(A10, A11, ADC_0);

  delay(1000);
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
  
  if(strcmp(cmd, START_COMMAND) == 0) {
      long timeStart = millis();
      pinMode(switchPin, INPUT);
      digitalWrite(switchPin, HIGH); 

      unsigned long timeMicro;
      for(long isample = 0; isample < NUM_SAMPLES; isample++) {
          timeMicro = micros();
          // get analog pin readings
          //reading = adc->readSynchronizedContinuous();
          readingADC0 = adc->analogRead(voltPin, ADC_0);
          readingADC1 = adc->analogRead(currPin, ADC_1);
          //voltReading = (double)readingADC0*3.3/adc->getMaxValue(ADC_0);
          //currReading = (double)readingADC1*3.3/adc->getMaxValue(ADC_1)/(double)RESISTOR;
          //voltReadings[isample] = (double)reading.result_adc0*3.3/adc->getMaxValue(ADC_0);
          //currReadings[isample] = (double)reading.result_adc1*3.3/adc->getMaxValue(ADC_1);
          voltReadings[isample] = (double)readingADC0*3.3/adc->getMaxValue(ADC_0);
          currReadings[isample] = (double)readingADC1*3.3/adc->getMaxValue(ADC_1);
          timeMicros[isample] = timeMicro;
      }
      sendReadings();
      
      // sleep
      pinMode(sleepPin, INPUT);
      digitalWrite(sleepPin, HIGH);
      pinMode(switchPin, OUTPUT);
      digitalWrite(switchPin, LOW); 
      
      long timeStop = millis();
      long sleepDur = SLEEP_TIME - (timeStop - timeStart);
      if(sleepDur < 0) {
        sleepDur = 0;
      }
      //delay(10000);
      delay(sleepDur);

      // wake up
      pinMode(sleepPin, OUTPUT);
      digitalWrite(sleepPin, LOW); 
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
          data["sampleNum"] = sampleNum++;
          //data["time"] = "2019-11-15 16:07";
          data["volt"] = (double)voltReadings[isample];
          data["curr"] = (double)currReadings[isample];
          data["micros"] = timeMicros[isample];

          //Serial.print(isample);
          //Serial.print(",");
          //Serial.print(voltReading, DEC);
          //Serial.print(",");
          //Serial.println(currReadings[isample], DEC);
          //Serial.print(",");
          //Serial.println(time, DEC);

          serializeJson(data, Serial1);
      //    if(isample+1 % 8 == 0) {
          delay(TRANSMISSION_DELAY);
        //  }
      
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

// MISC.
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
