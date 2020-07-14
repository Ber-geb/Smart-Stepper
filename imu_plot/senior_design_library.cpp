#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h>
#include <MadgwickAHRS.h>
#include "senior_design_library.h"


extern Madgwick filter;
extern float roll, pitch, heading;

extern BLEService imuService;
extern BLEStringCharacteristic imuCharacteristic;


// Performs an ADC reading on the internal GND channel in order
// to clear any voltage that might be leftover on the ADC.
// Only works on AVR boards and silently fails on others.
void analogReset()
{
#if defined(ADMUX)
#if defined(ADCSRB) && defined(MUX5)
  // Code for the ATmega2560 and ATmega32U4
  ADCSRB |= (1 << MUX5);
#endif
  ADMUX = 0x1F;

  // Start the conversion and wait for it to finish.
  ADCSRA |= (1 << ADSC);
  loop_until_bit_is_clear(ADCSRA, ADSC);
#endif
}

// This function follows the steps described in the FSLP
// integration guide to measure the position of a force on the
// sensor.  The return value of this function is proportional to
// the physical distance from drive line 2, and it is between
// 0 and 1023.  This function does not give meaningful results
// if fslpGetPressure is returning 0.
int fslpGetPosition()
{
  // Step 1 - Clear the charge on the sensor.
  pinMode(fslpSenseLine, OUTPUT);
  digitalWrite(fslpSenseLine, LOW);

  pinMode(fslpDriveLine1, OUTPUT);
  digitalWrite(fslpDriveLine1, LOW);

  pinMode(fslpDriveLine2, OUTPUT);
  digitalWrite(fslpDriveLine2, LOW);

  pinMode(fslpBotR0, OUTPUT);
  digitalWrite(fslpBotR0, LOW);

  // Step 2 - Set up appropriate drive line voltages.
  digitalWrite(fslpDriveLine1, HIGH);
  pinMode(fslpBotR0, INPUT);
  pinMode(fslpSenseLine, INPUT);

  // Step 3 - Wait for the voltage to stabilize.
  delayMicroseconds(10);

  // Step 4 - Take the measurement.
  analogReset();
  return analogRead(fslpSenseLine);
}

// This function follows the steps described in the FSLP
// integration guide to measure the pressure on the sensor.
// The value returned is usually between 0 (no pressure)
// and 500 (very high pressure), but could be as high as
// 32736.
int fslpGetPressure()
{
  // Step 1 - Set up the appropriate drive line voltages.
  pinMode(fslpDriveLine1, OUTPUT);
  digitalWrite(fslpDriveLine1, HIGH);

  pinMode(fslpBotR0, OUTPUT);
  digitalWrite(fslpBotR0, LOW);

  pinMode(fslpSenseLine, INPUT);

  pinMode(fslpDriveLine2, INPUT);

  // Step 2 - Wait for the voltage to stabilize.
  delayMicroseconds(10);

  // Step 3 - Take two measurements.
  analogReset();
  int v1 = analogRead(fslpDriveLine2);
  analogReset();
  int v2 = analogRead(fslpSenseLine);

  // Step 4 - Calculate the pressure.
  // Detailed information about this formula can be found in the
  // FSLP Integration Guide.
  if (v1 == v2)
  {
    // Avoid dividing by zero, and return maximum reading.
    return 32 * 1023;
  }
  return 32 * v2 / (v1 - v2);
}

// send IMU data
void sendSensorData() {
 

  float ax, ay, az; // Acceleration
  float gx, gy, gz; // Gyroscope
  float mx, my, mz; // Magnometer
  if (IMU.accelerationAvailable()) {
    // read orientation x, y and z eulers
    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);
    //IMU.readMagneticField(mx, my, mz);
    //filter.update(gx, gy, gz, ax, ay, az, mx, my, mz); //for all 3'
    filter.updateIMU(gx, gy, gz, ax, ay, az);
    roll = filter.getRoll();
    pitch = filter.getPitch();
    heading = filter.getYaw();

    //  Serial.print("Orientation: ");
    //  Serial.print(heading);
    //  Serial.print(" ");
    //  Serial.print(pitch);
    //  Serial.print(" ");
    //  Serial.println(roll);

    // Send 3x eulers over bluetooth as 1x byte array
    String imuData = String(heading) + "|" + String(pitch) + "|" + String(roll);
    Serial.println(imuData);
    imuCharacteristic.writeValue(imuData);
  }
}
