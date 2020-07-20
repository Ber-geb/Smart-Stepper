#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>
#include <MadgwickAHRS.h>
#include "./senior_design_library.h"

// Madgwick
Madgwick filter;
// sensor's sample rate is fixed at 119 Hz:
const float sensorRate = 119;

float roll, pitch, heading;

// BLE Service
BLEService imuService("19b10000-e8f2-537e-4f6c-d104768a1214"); // Custom UUID

// BLE Characteristic
BLEStringCharacteristic imuCharacteristic("19b10010-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify, 50);

long previousMillis = 0;  // last timechecked, in ms
unsigned long micros_per_reading, micros_previous;
String oldimuReading = "0";

const int fslpSenseLine = A2;
const int fslpDriveLine1 = 8;
const int fslpDriveLine2 = A3;
const int fslpBotR0 = 9;


void setup() {
  Serial.begin(115200);    // initialize serial communication


  pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected

  // begin initialization
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  // start the filter to run at the sample rate:
  filter.begin(119);

  delay(10000);

  Serial.print("IMU sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();

  // Setup bluetooth
  BLE.setLocalName("ArduinoIMU");
  BLE.setAdvertisedService(imuService);
  imuService.addCharacteristic(imuCharacteristic);
  BLE.addService(imuService);
  imuCharacteristic.writeValue(oldimuReading); // set initial value for this characteristic

  // start advertising
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");

  micros_per_reading = 1000000 / 119;
  micros_previous = micros();
}

void loop() {
  long int pressure, position;

  // wait for a BLE central
  BLEDevice central = BLE.central();

  // if a BLE central is connected to the peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's BT address:
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);

    // while the central is connected:
    while (central.connected()) {
      //     unsigned long micros_now;
      //     micros_now = micros();
      //     if (micros_now - micros_previous >= micros_per_reading) {
      //       if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable() && IMU.magneticFieldAvailable()) { // XX

  
//      pressure = fslpGetPressure();
//
//      if (pressure == 0)
//      {
//        // There is no detectable pressure, so measuring
//        // the position does not make sense.
//        position = 0;
//      }
//      else
//      {
//        position = fslpGetPosition();  // Raw reading, from 0 to 1023.
//      }
//
//      char report[80];
//      sprintf(report, "pressure: %5lu   position: %5lu\n",
//              pressure, position);
      
      sendSensorData();
//      Serial.print(report);
      

      //         micros_previous = micros_previous + micros_per_reading;
      //       }
      //     }
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());

  }
}
