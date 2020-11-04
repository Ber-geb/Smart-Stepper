#include <Arduino_LSM9DS1.h>
//#include <MadgwickAHRS.h>  -- uncomment to use this filter
#include "./senior_design_library.h"

// Madgwick
//Madgwick filter;
// sensor's sample rate is fixed at 119 Hz:
//const float sensorRate = 119;

//float roll, pitch, heading;

//long previousMillis = 0;  // last timechecked, in ms
//unsigned long micros_per_reading, micros_previous;
//String oldimuReading = "0";

const int fslpSenseLine = A0;
const int fslpDriveLine1 = 8;
const int fslpDriveLine2 = A1;
const int fslpBotR0 = 9;

int pressure, position; // pressure and position of fslp

int fsrPin_1 = 2; // the FSR and 10K pulldown are connected to a2 & a3
int fsrPin_2 = 3;
int fsrReading_1; // the analog reading from the FSR resistor divider
int fsrReading_2;



void setup() {
  Serial.begin(115200);    // initialize serial communication


  pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected

  // begin initialization
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // start the filter to run at the sample rate:
  //filter.begin(119);

  //delay(10000);

  Serial.print("IMU sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();

  //  micros_per_reading = 1000000 / 119;
  //  micros_previous = micros();
  delay(250);
}

void loop() {
  pressure = fslpGetPressure();

  if (pressure == 0)
  {
    // There is no detectable pressure, so measuring
    // the position does not make sense.
    position = 0;
  }
  else
  {
    position = fslpGetPosition();  // Raw reading, from 0 to 1023.
  }

//  char report[80];
//  sprintf(report, "pressure: %5d   position: %5d\n",
//          pressure, position);
//  Serial.print(report);

  fsrReading_1 = analogRead(fsrPin_1);
  fsrReading_2 = analogRead(fsrPin_2);

  sendSensorData();

  delay(20);
  
}
