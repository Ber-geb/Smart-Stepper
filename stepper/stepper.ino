
#include <Arduino_LSM9DS1.h>

#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

#include "model.h" // implement our model


namespace {
// global variables used for TensorFlow Lite (Micro)
tflite::ErrorReporter* tflErrorReporter = nullptr;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 20 * 1024;
uint8_t tensorArena[tensorArenaSize];
} // namespace


const float ACCELERATION_RMS_THRESHOLD = 2.0;  // RMS (root mean square) threshold of significant motion in G's -- might not need this..
const int NUM_CAPTURED_SAMPLES_PER_GESTURE = 119 * 2;
const int NUM_FEATURES_PER_SAMPLE = 12; // ax, ay, az, gx, gy, gz, abs_accel, abs_gyro, Pp, Px, P1, P2
const int TOTAL_SAMPLES = NUM_CAPTURED_SAMPLES_PER_GESTURE * NUM_FEATURES_PER_SAMPLE;

int capturedSamples = 0;

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
  "normal_walking",
  "toe_walking",
}; // might need to add in more gestures, ie. 'toe-jogging', 'toe-standing', etc...

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))



void setup() {
  Serial.begin(9600);
  while (!Serial);

  // initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
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

  // set up logging
  static tflite::MicroErrorReporter micro_error_reporter;
  tflErrorReporter = &micro_error_reporter;

  // get the TFL representation of the model byte array
  // if failed to obtain the model, report an error
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(tflErrorReporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         tflModel->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // pull in all the TFLM ops, you can remove this line and
  // only pull in the TFLM ops you need, if would like to reduce
  // the compiled size of the sketch.
  static tflite::AllOpsResolver tflOpsResolver;

  // Create an interpreter to run the model
  static tflite::MicroInterpreter static_interpreter(
    tflModel, tflOpsResolver, tensorArena, tensorArenaSize, tflErrorReporter);
  tflInterpreter = &static_interpreter;

  // Allocate memory for the model's input and output tensors
  // **if error occurs, might need to adjust tensor arena size
  TfLiteStatus allocate_status = tflInterpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(tflErrorReporter, "AllocateTensors() failed");
    return;
  }

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
}

void loop() {
  float aX, aY, aZ, gX, gY, gZ;
  float abs_acc, abs_gyro;

  char command = ' ';     // Might need to remove this command input. Instead, we want to collect samples every time.
  while (command != 's') {
    command = Serial.read();
  }

  if (command == 's') {
    // collect the remaining samples
    while (capturedSamples < TOTAL_SAMPLES) {
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

        // get fsr readings
        fsrReading_1 = analogRead(fsrPin_1);  // this reading is from the toe
        fsrReading_2 = analogRead(fsrPin_2);  // this reading is from the heel


        // insert the new data
        tflInputTensor->data.f[capturedSamples + 0] = (aX + 4.0) / 8.0;
        tflInputTensor->data.f[capturedSamples + 1] = (aY + 4.0) / 8.0;
        tflInputTensor->data.f[capturedSamples + 2] = (aZ + 4.0) / 8.0;
        tflInputTensor->data.f[capturedSamples + 3] = (gX + 2000.0) / 4000.0;
        tflInputTensor->data.f[capturedSamples + 4] = (gY + 2000.0) / 4000.0;
        tflInputTensor->data.f[capturedSamples + 5] = (gZ + 2000.0) / 4000.0;
        tflInputTensor->data.f[capturedSamples + 6] = (abs_acc + 4) / 8.0;
        tflInputTensor->data.f[capturedSamples + 7] = (abs_gyro + 2000.0) / 4000.0;

        // add in the pressure values to the input tensors here...

        
        // I just dont know why the dude added 4 or 2000 or divided by 8 or divided by 4000 for the accel and gyro values
        // whats the reasoning behind that???


        capturedSamples += NUM_FEATURES_PER_SAMPLE;
      }
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
  for (int i = 0; i < NUM_GESTURES; i++) {
    Serial.print(GESTURES[i]);
    Serial.print(": ");
    Serial.println(tflOutputTensor->data.f[i], 3);
  }
  command = ' ';
  capturedSamples = 0;
  Serial.println();
}


//// Pressure Sensor Functions

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
