#include <Arduino_LSM9DS1.h>
//#include <MadgwickAHRS.h>  -- uncomment to use this filter
#include "./senior_design_library.h"
#include <ArduinoBLE.h>

// Madgwick
//Madgwick filter;
// sensor's sample rate is fixed at 119 Hz:
//const float sensorRate = 119;

//float roll, pitch, heading;

//long previousMillis = 0;  // last timechecked, in ms
//unsigned long micros_per_reading, micros_previous;
//String oldimuReading = "0";

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

  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  // start the filter to run at the sample rate:
  //filter.begin(119);

  //delay(10000);

  Serial.print("IMU sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();

  // Setup bluetooth
  //  BLE.setLocalName("ArduinoIMU");
  //  BLE.setAdvertisedService(imuService);
  //  imuService.addCharacteristic(imuCharacteristic);
  //  BLE.addService(imuService);
  ////  imuCharacteristic.writeValue(oldimuReading); // set initial value for this characteristic

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

  // start advertising
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");


  //  micros_per_reading = 1000000 / 119;
  //  micros_previous = micros();
  delay(250);
}

void loop() {
  pressure = fslpGetPressure();

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

      //      sendSensorData();
      float ax, ay, az; // Acceleration
      float gx, gy, gz; // Gyroscope
      float abs_acc, abs_gyro; // Absolute accelerometer and gyroscope

      if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
        // read orientation x, y and z eulers
        IMU.readAcceleration(ax, ay, az);
        IMU.readGyroscope(gx, gy, gz);
        //IMU.readMagneticField(mx, my, mz);
        //filter.update(gx, gy, gz, ax, ay, az, mx, my, mz); //for all 3'
        //filter.updateIMU(gx, gy, gz, ax, ay, az);
        //roll = filter.getRoll();
        //pitch = filter.getPitch();
        //heading = filter.getYaw();

        //  Serial.print("Orientation: ");
        //  Serial.print(heading);
        //  Serial.print(" ");
        //  Serial.print(pitch);
        //  Serial.print(" ");
        //  Serial.println(roll);

        // Calculate absolute values
        abs_acc = sqrt(ax * ax + ay * ay + az * az);
        abs_gyro = sqrt(gx * gx + gy * gy + gz * gz);

        /**
          convert the raw gyro & accelerometer data to degrees/second and assign them to the elements of
          the float array in the union representing the gyroscope & accelerometer data.
        */

        gyroData.g[0] = gx;
        gyroData.g[1] = gy;
        gyroData.g[2] = gz;
        gyroData.g[3] = abs_gyro;

        accData.a[0] = ax;
        accData.a[1] = ay;
        accData.a[2] = az;
        accData.a[2] = abs_acc;

        fslpData.fslp[0] = int(pressure);
        fslpData.fslp[1] = int(position);

        fsrData.fsr[0] = fsrReading_1;
        fsrData.fsr[1] = fsrReading_2;

        // Send 3x eulers over bluetooth as 1x byte array
        String imuData = String(ax) + "," +  String(ay) + "," + String(az) + "," + String(gx) + "," + String(gy) + "," + String(gz)
                         + "," + String(abs_acc) + "," + String(abs_gyro)
                         + "," + String(pressure) + "," + String(position) + "," + String(fsrReading_1) + "," + String(fsrReading_2);
        Serial.println(imuData);

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



      }

      delay(200);
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }

}
