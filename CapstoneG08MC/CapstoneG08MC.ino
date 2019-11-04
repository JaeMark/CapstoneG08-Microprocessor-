#include <XBee.h>
#include <ArduinoJson.h>

// constant declarations
#define BAUD_RATE 9600

// pin declarations
const int voltPin = 0; // place holder
const int currPin = 1; // place holder
const int rxPin = 2; // place holder
const int txPin = 3; // placeholder
const int sleepPin = 4; // placeholder

// global variables
float voltReading = 0;
float currReading = 0;
long sampleNum = 0;

XBee xbee = XBee();

void setup() {

  // start serial
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    // wait till initialization is complete
  }
  Serial.flush();
  xbee.begin(Serial);

  // configure pins
  pinMode(voltPin, INPUT);
  pinMode(currPin, INPUT);
  pinMode(sleepPin, INPUT);
}

void loop() {
  xbee.readPacket();
  if(xbee.getResponse().isAvailable()) {
    // read command from workstation
  }

  // send data
}

void readSensors() {
  voltReading = readVoltPin();
  currReading = readCurrPin();
}

float readVoltPin() {
  double voltage = 0;
  return voltage;
}
float readCurrPin() {
  double current = 0;
  return current;
}
