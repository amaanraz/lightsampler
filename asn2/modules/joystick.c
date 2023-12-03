#include <stdio.h>
#include <stdlib.h>
#include "joystick.h"
#include "misc.h"




float getXpos(){
    // change scale from [0,4000] --> [-1,1]
    int reading = readFromFileToScreen("/sys/bus/iio/devices/iio:device0/in_voltage3_raw");

    // convert scale
    float scaled = -1 * (((2.0*reading) / 4093) - 1);
    return scaled;
}

float getYpos(){
    // change scale from [0,4000] --> [-1,1]
    int reading = readFromFileToScreen("/sys/bus/iio/devices/iio:device0/in_voltage2_raw");

    // convert scale
    float scaled = -1 * (((2.0*reading) / 4093) - 1);
    return scaled;
}
