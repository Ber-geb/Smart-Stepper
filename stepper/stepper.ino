#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

#include <TensorFlowLite.h>
#include <tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h>
#include <tensorflow/lite/experimental/micro/micro_error_reporter.h>
#include <tensorflow/lite/experimental/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

#include "model.h" // implement our model

// --------------------------------------------------------------------------------------------------------------------- //
// BLE Service
BLEService imuService("917649A0-D98E-11E5-9EEC-0002A5D5C51B"); // Custom UUID
BLEService pressureService("ca4d93da-77d9-4c0b-89fc-4e18804f79c3"); // Custom UUID

// BLE Characteristic
// BLEStringCharacteristic imuCharacteristic("1A3AC130-31EE-758A-BC50-54A61958EF81", BLERead | BLENotify, 50);
BLECharacteristic imuAccCharacteristic("917649A1-D98E-11E5-9EEC-0002A5D5C51B", BLERead | BLENotify, 16 );
BLECharacteristic imuGyroCharacteristic("917649A2-D98E-11E5-9EEC-0002A5D5C51B", BLERead | BLENotify, 16 );
BLECharacteristic fslpCharacteristic("0487a2ba-9d48-4ab4-bde2-55b14a3b18bd", BLERead | BLENotify, 8 );
BLECharacteristic fsrCharacteristic("2d35254d-7963-47fb-aad3-5e4cc35f7d66", BLERead | BLENotify, 8 );


// More information about descriptors and CCCD here:
// https://www.oreilly.com/library/view/getting-started-with/9781491900550/ch04.html#gatt_cccd

BLEDescriptor imuAccDescriptor("2902", "block");
BLEDescriptor imuGyroDescriptor("2902", "block");
BLEDescriptor fslpDescriptor("2902", "block");
BLEDescriptor fsrDescriptor("2902", "block");

/**
  The union directive allows 3 variables to share the same memory location. Please see the
  tutorial covering this project for further discussion of the use of the union
  directive in C. https://www.hackster.io/gov/imu-to-you-ae53e1
*/
union
{
  float a[4];
  unsigned char bytes[16];
} accData;

union
{
  float g[4];
  unsigned char bytes[16];
} gyroData;

union
{
  int fslp[2];
  unsigned char bytes[8];
} fslpData;

union
{
  int fsr[2];
  unsigned char bytes[8];
} fsrData;

// --------------------------------------------------------------------------------------------------------------------- //

const float ACCELERATION_RMS_THRESHOLD = 2.0;  // RMS (root mean square) threshold of significant motion in G's -- might not need this..
const int NUM_CAPTURED_SAMPLES_PER_GESTURE = 119 * 2; // We have 12 features/gestures, 119 is the sampling rate
const int NUM_FEATURES_PER_SAMPLE = 8; // ax, ay, az, gx, gy, gz, abs_accel, abs_gyro, Pp, Px, P1, P2
const int TOTAL_SAMPLES = NUM_CAPTURED_SAMPLES_PER_GESTURE * NUM_FEATURES_PER_SAMPLE;

int capturedSamples = 0;

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::ops::micro::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 20 * 1024;
byte tensorArena[tensorArenaSize];

// pressure sensor global variables
const int fslpSenseLine = A0;
const int fslpDriveLine1 = 8;
const int fslpDriveLine2 = A1;
const int fslpBotR0 = 9;

int pressure, position; // pressure and position of fslp

int fsrPin_1 = 2; // the FSR and 10K pulldown are connected to a2 & a3
int fsrPin_2 = 3;
int fsrReading_1; // the analog reading from the FSR resistor divider
int fsrReading_2;

// array to map gesture index to a name
const char* GESTURES[] = {
  "normal walking/standing",
  "toe-walking/toe-standing"
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))


void setup() {
  Serial.begin(9600);
  while (!Serial);

  // initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  // print out the samples rates of the IMUs
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");

  Serial.println();

  // Setup bluetooth
  BLE.setLocalName("ArduinoIMU");
  BLE.setAdvertisedServiceUuid(imuService.uuid());  // add the service UUID
  BLE.setAdvertisedServiceUuid(pressureService.uuid());
  imuService.addCharacteristic(imuAccCharacteristic);
  imuAccCharacteristic.addDescriptor(imuAccDescriptor);
  imuService.addCharacteristic(imuGyroCharacteristic);
  imuGyroCharacteristic.addDescriptor(imuGyroDescriptor);
  pressureService.addCharacteristic(fslpCharacteristic);
  fslpCharacteristic.addDescriptor(fslpDescriptor);
  pressureService.addCharacteristic(fsrCharacteristic);
  fsrCharacteristic.addDescriptor(fsrDescriptor);
  BLE.addService(imuService);
  BLE.addService(pressureService);

  // All characteristics should be initialized to a starting value prior
  // using them.
  const unsigned char initializerAcc[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  const unsigned char initializerGyro[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  const unsigned char initializerFSLP[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  const unsigned char initializerFSR[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  imuAccCharacteristic.setValue( initializerAcc, 16);
  imuGyroCharacteristic.setValue( initializerGyro, 16 );
  fslpCharacteristic.setValue( initializerFSLP, 8 );
  fsrCharacteristic.setValue( initializerFSR, 8 );

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  // **if error occurs, might need to adjust tensor arena size
  TfLiteStatus allocate_status = tflInterpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed.");
    while (1);
  }

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
  
  // start advertising
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  float aX, aY, aZ, gX, gY, gZ;
  float abs_acc, abs_gyro;

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
      // collect the remaining samples
      while (capturedSamples < TOTAL_SAMPLES) {
        // You can grab the data from here to send via BLE onto the App. Accel/Gyro and pressure sensor values are being calculated and sampled here.

        // wait for both acceleration and gyroscope data to be available
        if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
          // read the acceleration and gyroscope data
          IMU.readAcceleration(aX, aY, aZ);
          IMU.readGyroscope(gX, gY, gZ);
          abs_acc = sqrt(aX * aX + aY * aY + aZ * aZ);
          abs_gyro = sqrt(gX * gX + gY * gY + gZ * gZ);

          // get fslp readings
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

          /**
            convert the raw gyro & accelerometer data to degrees/second and assign them to the elements of
            the float array in the union representing the gyroscope & accelerometer data.
          */

          gyroData.g[0] = gX;
          gyroData.g[1] = gY;
          gyroData.g[2] = gZ;
          gyroData.g[3] = abs_gyro;

          accData.a[0] = aX;
          accData.a[1] = aY;
          accData.a[2] = aZ;
          accData.a[2] = abs_acc;

          fslpData.fslp[0] = int(pressure);
          fslpData.fslp[1] = int(position);

          fsrData.fsr[0] = fsrReading_1;
          fsrData.fsr[1] = fsrReading_2;

          // get fsr readings
          fsrReading_1 = analogRead(fsrPin_1);  // this reading is from the toe
          fsrReading_2 = analogRead(fsrPin_2);  // this reading is from the heel

          /**
            The following two statements have the potential to cuase the most confusion. Please see the tutorial for
            more on this.
            What we are doing here is casting our union variables into a pointer of unsigned characters in
            order to allow us to pass the array of bytes to the setValue() function.
          */
          unsigned char *acc = (unsigned char *)&accData;
          unsigned char *gyro = (unsigned char *)&gyroData;
          unsigned char *fslp = (unsigned char *)&fslpData;
          unsigned char *fsr = (unsigned char *)&fsrData;

          /**
             Setting the values here will cause the notification mechanism on the moible app
             side to be enacted.
          */
          imuAccCharacteristic.setValue( acc, 16 );
          imuGyroCharacteristic.setValue( gyro, 16 );
          fslpCharacteristic.setValue( fslp, 8 );
          fsrCharacteristic.setValue( fsr, 8 );



          // insert the new data
          // we are only sending accel/gryo & abs accel/gyro in the TensorFlow input array
          // to determine the classification of the movement via Machine Learning
          // we are not including the pressure sensor values because OUR model does not work
          // the guy's model is ONLY for accelerometer and gyroscope values
          tflInputTensor->data.f[capturedSamples + 0] = (aX + 4.0) / 8.0;
          tflInputTensor->data.f[capturedSamples + 1] = (aY + 4.0) / 8.0;
          tflInputTensor->data.f[capturedSamples + 2] = (aZ + 4.0) / 8.0;
          tflInputTensor->data.f[capturedSamples + 3] = (gX + 2000.0) / 4000.0;
          tflInputTensor->data.f[capturedSamples + 4] = (gY + 2000.0) / 4000.0;
          tflInputTensor->data.f[capturedSamples + 5] = (gZ + 2000.0) / 4000.0;
          tflInputTensor->data.f[capturedSamples + 6] = (abs_acc + 4) / 8.0;
          tflInputTensor->data.f[capturedSamples + 7] = (abs_gyro + 2000.0) / 4000.0;


          capturedSamples += NUM_FEATURES_PER_SAMPLE;
        }
      }

      // Run inferencing
      TfLiteStatus invokeStatus = tflInterpreter->Invoke();
      if (invokeStatus != kTfLiteOk) {
        Serial.println("Invoke failed!");
        while (1);
        return;
      }

      // Loop through the output tensor values from the model
      // this will display the gesture/movement and the prediction percentage next to it
      for (int i = 0; i < NUM_GESTURES; i++) {
        Serial.print(GESTURES[i]);
        Serial.print(": ");
        Serial.println(tflOutputTensor->data.f[i], 3);
      }

      capturedSamples = 0;
      Serial.println();
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}


// Pressure sensor functions

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
