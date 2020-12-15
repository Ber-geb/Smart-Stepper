I uploaded a new version of the stepper.ino file. It is using the guy's model and it only reads 
accelerometer and gyroscope values to send for ML. However, we can still send both accel/gryo and 
pressure sensor values via BLE so we can see the visualization on the app. Make sure you have the
Arduino_TensorFlowLite library installed and make sure it is version 1.15 ALPHA (not version 2.1).
You also have to change the orientation of the board so that the usb is facing the heel instead of
the toe. This is because this was the board's positioning when the dude collected his data.