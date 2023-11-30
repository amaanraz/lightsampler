#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdio.h>
#include "shutdown.h"
#include "../modules/joystick.h"
#include "../modules/leds.h"
#include "photores.h"
#include "input.h"

static pthread_t threadPID;


static int readFromFileToScreen(char *fileName);
static void runCommand(char* command);

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

// given from guides
static void runCommand(char* command)
{
 // Execute the shell command (output into pipe)
 FILE *pipe = popen(command, "r");
 // Ignore output of the command; but consume it
 // so we don't get an error when closing the pipe.
 char buffer[1024];
 while (!feof(pipe) && !ferror(pipe)) {
    if (fgets(buffer, sizeof(buffer), pipe) == NULL)
        break;
        // printf("--> %s", buffer); // Uncomment for debugging
    }
    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        perror("Unable to execute command:");
        printf(" command: %s\n", command);
        printf(" exit code: %d\n", exitCode);
    }
}