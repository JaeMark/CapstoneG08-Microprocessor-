#include <ArduinoJson.h>
#include <ADC.h>
#include <vector>

// constant declarations
#define BAUD_RATE 115200
#define MAX_NUM_SAMPLES 2048

#define DEFAULT_SLEEP_TIME 3600000
#define DEFAULT_TRANS_DELIMETER 16
#define DEFAULT_SMALL_TRANSMISSION_DELAY 25
#define DEFAULT_BIG_TRANSMISSION_DELAY 100
#define DEFAULT_NUM_SAMPLES 512

#define RESISTOR 10000

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
unsigned long timeMicros[MAX_NUM_SAMPLES];

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
  adc->setResolution(13, ADC_1);
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_1);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_1);

  adc->startContinuous(voltPin, ADC_0);
  adc->startContinuous(currPin, ADC_1);

  // initialize global variables
  cmd = DEFAULT_COMMAND;
  sleep_time = DEFAULT_SLEEP_TIME;
  trans_delim = DEFAULT_TRANS_DELIMETER;
  small_delay_t = DEFAULT_SMALL_TRANSMISSION_DELAY;
  big_delay_t = DEFAULT_BIG_TRANSMISSION_DELAY;

  delay(1000);
}

ADC::Sync_result reading;
double readingADC0;
double readingADC1;

unsigned long timeStart;
unsigned long timeStop;

DynamicJsonDocument received_data(100);

void loop() {
  switch (cmd) {
    case DEFAULT_COMMAND:
      if (Serial1.available()) {
          deserializeJson(received_data, Serial1);
          cmd = received_data["command"];
      }
      break;
    case START_COMMAND:
      timeStart = millis();

      pinMode(switchPin, INPUT);
      digitalWrite(switchPin, HIGH);
      
      // parse the rest of the received json object
      sampleNum = received_data["sampleNum"];
      trans_delim = received_data["delim"];
      small_delay_t = received_data["smallDelay"];
      big_delay_t = received_data["bigDelay"];
      
      startHandshake();

      unsigned long timeMicro;
      for (int isample = 0; isample < sampleNum; isample++) {
        // get analog pin readings
        //reading = adc->readSynchronizedContinuous();
        readingADC0 = adc->analogRead(voltPin, ADC_0);
        readingADC1 = adc->analogRead(currPin, ADC_1);
        voltReadings[isample] = (double)readingADC0 * 3.3 / adc->getMaxValue(ADC_0);
        currReadings[isample] = (double)readingADC1 * 3.3 / adc->getMaxValue(ADC_1);

        timeMicro = micros();
        timeMicros[isample] = timeMicro;
      }
      sendReadings();
      cmd = DEFAULT_COMMAND;
      break;
    case SLEEP_COMMAND:
      // sleep
      pinMode(sleepPin, INPUT);
      digitalWrite(sleepPin, HIGH);
      pinMode(switchPin, OUTPUT);
      digitalWrite(switchPin, LOW);

      // parse the rest of the received json object
      sleep_time = received_data["sleepTime"];

      
      timeStop = millis();
      long sleepDur = sleep_time - (timeStop - timeStart);
      if (sleepDur < 0) {
        sleepDur = 0;
      }
      delay(sleepDur);

      // wake up
      pinMode(sleepPin, OUTPUT);
      digitalWrite(sleepPin, LOW);
      pinMode(switchPin, INPUT);
      digitalWrite(switchPin, HIGH);

      wakeUpHandshake();
      
      cmd = DEFAULT_COMMAND;
      break;
  }
}

void startHandshake() {
  DynamicJsonDocument dataBuffer(50);
  JsonObject data_to_send = dataBuffer.to<JsonObject>();

  data_to_send["command"] = START_COMMAND;
  data_to_send["sampleNum"] = sampleNum;
  serializeJson(data_to_send, Serial1);
  delay(small_delay_t);
}

void wakeUpHandshake() {
  DynamicJsonDocument dataBuffer(50);
  JsonObject data_to_send = dataBuffer.to<JsonObject>();

  data_to_send["command"] = WAKE_UP_COMMAND;
  serializeJson(data_to_send, Serial1);
  delay(small_delay_t);
}

void sendReadings() {
  DynamicJsonDocument dataBuffer(200);
  JsonObject data = dataBuffer.to<JsonObject>();

  for (int isample = 1; isample < sampleNum + 1; isample++) {
    data["volt"] = (double)voltReadings[isample - 1];
    data["curr"] = (double)currReadings[isample - 1];
    data["micros"] = (unsigned long)timeMicros[isample - 1];

    serializeJson(data, Serial1);
    if (isample % trans_delim == 0) {
      delay(big_delay_t);
    } else {
      delay(small_delay_t);
    }

  }
}
