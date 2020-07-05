#ifndef SENIOR_DESIGN_LIBRARY_H
#define SENIOR_DESIGN_LIBRARY_H

extern const int fslpSenseLine;
extern const int fslpDriveLine1;
extern const int fslpDriveLine2;
extern const int fslpBotR0;

void analogReset();
int fslpGetPosition();
int fslpGetPressure();
void sendSensorData();

#endif
