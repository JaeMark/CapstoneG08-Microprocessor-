#include <ArduinoJson.h>
#include <vector>
#include <ADC.h>
#include <Snooze.h>

// constant declarations
#define BAUD_RATE 115200
#define MAX_NUM_SAMPLES 2048
#define DELAY_T 1000

#define DEFAULT_SLEEP_TIME 3600000
#define DEFAULT_TRANS_DELIMETER 8
#define DEFAULT_SMALL_TRANSMISSION_DELAY 500
#define DEFAULT_BIG_TRANSMISSION_DELAY 1000
#define DEFAULT_NUM_SAMPLES 2000

// commands
#define DEFAULT_COMMAND 0
#define START_COMMAND 1
#define SLEEP_COMMAND 2
#define WAKE_UP_COMMAND 3

// global variables
int cmd;

unsigned long sleep_time;

int trans_delim;
int small_delay_t;
int big_delay_t;

int sampleNum = 0;
double voltReadings[MAX_NUM_SAMPLES];
double currReadings[MAX_NUM_SAMPLES];
//unsigned long microsReading[MAX_NUM_SAMPLES];

// pin declarations
const int voltPin = A9;
const int currPin = A3;
const int sleepPin = 7;
const int switchPin = 8;

ADC *adc = new ADC();

void setup() {
  // configure pins
  pinMode(voltPin, INPUT);
  pinMode(currPin, INPUT);

  pinMode(sleepPin, OUTPUT);
  digitalWrite(sleepPin, LOW);
  pinMode(switchPin, OUTPUT);
  digitalWrite(switchPin, LOW);
  
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
  adc->setReference(ADC_REFERENCE::REF_3V3, ADC_1);

  adc->startSynchronizedContinuous(voltPin, currPin);

  // initialize global variables
  cmd = DEFAULT_COMMAND;
  sleep_time = DEFAULT_SLEEP_TIME;
  trans_delim = DEFAULT_TRANS_DELIMETER;
  small_delay_t = DEFAULT_SMALL_TRANSMISSION_DELAY;
  big_delay_t = DEFAULT_BIG_TRANSMISSION_DELAY;

  delay(DELAY_T);
}

ADC::Sync_result reading;
double readingADC0;
double readingADC1;

unsigned long timeStart = 0;
int isampleNum = 0;

IntervalTimer sampleTimer;
SnoozeTimer sleepTimer;
SnoozeBlock sleepConfig(sleepTimer);

DynamicJsonDocument received_data(100);

void loop() {
  switch (cmd) {
    case DEFAULT_COMMAND:
      {
        if (Serial1.available()) {
            deserializeJson(received_data, Serial1);
            cmd = received_data["command"];
        }
      }
      break;
    case START_COMMAND:
      {
        timeStart = millis();
  
        // switch to measuring mode
        pinMode(switchPin, INPUT);
        digitalWrite(switchPin, HIGH);
  
        delay(DELAY_T);
        
        // parse the rest of the received json object
        sampleNum = received_data["sampleNum"];
        trans_delim = received_data["delim"];
        small_delay_t = received_data["smallDelay"];
        big_delay_t = received_data["bigDelay"];
        
        startHandshake();
  
        int isampleCopy = 0;  // a copy of the number of samples processed
        
        // begin ADC reading at 25 microseconds per sample = 40kHz sampling frequency
        sampleTimer.begin(readADC, 25); 
        while (isampleCopy < sampleNum) {
          delay(DELAY_T);
  
          // check if all samples have been read
          noInterrupts();
          isampleCopy = isampleNum;
          interrupts();
        }
        sampleTimer.end();
        isampleNum = 0;
  
        delay(DELAY_T);
        
        // switch to charging mode
        pinMode(switchPin, OUTPUT);
        digitalWrite(switchPin, LOW);
        
        sendReadings();
        
        cmd = DEFAULT_COMMAND;
      }
      break;
    case SLEEP_COMMAND:
      {
        // parse the rest of the received json object
        sleep_time = received_data["sleepTime"];

        // put XBee to sleep
        pinMode(sleepPin, INPUT);
        digitalWrite(sleepPin, HIGH);

        // calculate sleep time
        unsigned long timeElapsed = millis() - timeStart;
        unsigned long wakeupTime = 0;
        if(timeElapsed < sleep_time) wakeupTime = sleep_time - timeElapsed;

        // put Teensy to sleep sleep
        sleepTimer.setTimer(wakeupTime);  // set sleep duration
        Snooze.sleep(sleepConfig);
  
        // wake up!
        pinMode(sleepPin, OUTPUT);
        digitalWrite(sleepPin, LOW);

        delay(DELAY_T);
  
        wakeUpHandshake();
        
        cmd = DEFAULT_COMMAND;
      }
      break;
  }
}

void readADC() {
    // get analog pin reading
    if(isampleNum < sampleNum) {
      reading = adc->readSynchronizedContinuous();
      voltReadings[isampleNum] = (double)reading.result_adc0 * 3.3 / adc->getMaxValue(ADC_0);
      currReadings[isampleNum] = (double)reading.result_adc1 * 3.3 / adc->getMaxValue(ADC_1);
      //microsReading[isampleNum] = micros();
    }
    isampleNum++;
    
}
void startHandshake() {
  // this method sends a handshake to let the workstation know that the Teensy is not in measuring mode
  DynamicJsonDocument dataBuffer(50);
  JsonObject data_to_send = dataBuffer.to<JsonObject>();

  data_to_send["command"] = START_COMMAND;
  data_to_send["sampleNum"] = sampleNum;
  serializeJson(data_to_send, Serial1);
  delay(small_delay_t);
}

void wakeUpHandshake() {
  // this method sends a handshake to let the workstation know that the Teensy and XBee are awake
  DynamicJsonDocument dataBuffer(50);
  JsonObject data_to_send = dataBuffer.to<JsonObject>();

  data_to_send["command"] = WAKE_UP_COMMAND;
  serializeJson(data_to_send, Serial1);
  delay(small_delay_t);
}

void sendReadings() {
  // this method sends the samples read to the workstation
  DynamicJsonDocument dataBuffer(200);
  JsonObject data = dataBuffer.to<JsonObject>();

  for (int isample = 1; isample < sampleNum + 1; isample++) {
    data["sampleNum"] = isample;
    data["volt"] = (double)voltReadings[isample - 1];
    data["curr"] = (double)currReadings[isample - 1];
    //data["micros"] = microsReading[isample-1];

    serializeJson(data, Serial1);
    if (isample % trans_delim == 0) {
      delay(big_delay_t);
    } else {
      delay(small_delay_t);
    }

  }
}
