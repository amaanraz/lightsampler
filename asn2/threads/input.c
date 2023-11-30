#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdio.h>
#include "input.h"
#include "../modules/joystick.h"
#include "../modules/leds.h"
#include "photores.h"

static pthread_t threadPID;

static void* runMod(void* unused){
    while(1){
        float x = getXpos();
        float y = getYpos();

        // proritize left/right over up down
        if(x > 0.5){
            // right
            displayDec((double)getMaxInterval() / 1000000);
        } else if (x < -0.5){
            // left
            displayDec((double)getMinInterval() / 1000000);
        } else if(y > 0.5){
            // up
            displayDec(getMaxVoltage());
        } else if (y < -0.5){
            // down
            displayDec(getMinVoltage());
        } else {
            // center
            displayNum(0);
        }
    }

    return NULL;
}

void inputInit(){
    // create thread
    int retVal = pthread_create(&threadPID, NULL, runMod, NULL);

    // error checking
    if (retVal != 0){
        // error code
        perror("input thread error");
        exit(1);
    }
}


void inputCancel(){
    pthread_cancel(threadPID);
}

void inputShutDown(){
    // REMEBER TO REMOVE CACNEL
    // inputCancel();

    pthread_join(threadPID, NULL);
}