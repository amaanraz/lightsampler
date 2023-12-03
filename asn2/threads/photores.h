#ifndef _PHOTORES_H_
#define _PHOTORES_H_

void photoresInit();
void photoresCancel();
void photoresShutDown();

// for joystick display
double getMaxVoltage();
double getMinVoltage();
long long getMaxInterval();
long long getMinInterval();
int getDip();

#endif