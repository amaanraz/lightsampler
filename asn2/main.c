#include <stdio.h>
#include <pthread.h> 
#include <math.h>
#include "modules/joystick.h"
#include "modules/leds.h"
#include "threads/photores.h"
#include "threads/input.h"
#include "threads/shutdown.h"

int main(){

    shutdownInit();
    photoresInit();
    inputInit();
    

    photoresShutDown();
    inputShutDown();
    shutdownShutDown();
    // turnOffLeds();

    return 0;
}