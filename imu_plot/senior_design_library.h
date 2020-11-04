#ifndef SENIOR_DESIGN_LIBRARY_H
#define SENIOR_DESIGN_LIBRARY_H

extern const int fslpSenseLine;
extern const int fslpDriveLine1;
extern const int fslpDriveLine2;
extern const int fslpBotR0;

extern int pressure, position;
extern int fsrReading_1;
extern int fsrReading_2;


void analogReset();
int fslpGetPosition();
int fslpGetPressure();
void sendSensorData();

#endif
