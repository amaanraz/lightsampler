#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdio.h>
#include "shutdown.h"
#include "../modules/joystick.h"
#include "../modules/leds.h"
#include "photores.h"
#include "input.h"
#include "../modules/misc.h"

static pthread_t threadPID;

static void* runMod(void* unused){
    while(1){
        // check if user button pressed
        int userButton = readFromFileToScreen("/sys/class/gpio/gpio72/value");

        if(userButton == 0){
            // button is pressed
            // cancel threads
            printf("Shutting Down\n");
            photoresCancel();
            inputCancel();
            shutdownCancel();
        }
    }

    return NULL;
}

void shutdownInit(){
    // create thread
    int retVal = pthread_create(&threadPID, NULL, runMod, NULL);

    // export pin
    runCommand("config-pin p8.43 gpio");

    // error checking
    if (retVal != 0){
        // error code
        perror("shutdown thread error");
        exit(1);
    }
}


void shutdownCancel(){
    pthread_cancel(threadPID);
}

void shutdownShutDown(){
    // REMEBER TO REMOVE CACNEL
    // shutdownCancel();

    pthread_join(threadPID, NULL);
}
