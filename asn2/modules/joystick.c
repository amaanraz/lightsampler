#include <stdio.h>
#include <stdlib.h>
#include "joystick.h"


// prototypes
static int readFromFileToScreen(char *fileName);

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

// file info
static int readFromFileToScreen(char *fileName)
{
    FILE *pFile = fopen(fileName, "r");
    if (pFile == NULL) {
        printf("ERROR: Unable to open file (%s) for read\n", fileName);
        exit(-1);
    }
    // Read string (line)
    const int MAX_LENGTH = 1024;
    char buff[MAX_LENGTH];
    fgets(buff, MAX_LENGTH, pFile);
    // Close
    fclose(pFile);
    
    return atoi(buff);
    
}