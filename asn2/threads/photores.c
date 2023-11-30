#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "photores.h"
#include "../modules/leds.h"

// light objects
typedef struct {
    double voltage;
    long long timestamp;
} lightSample;

static pthread_t threadPID;
pthread_t sampling_thread;
static lightSample buff[1000]; // buffer for sampling
int buffindex = 0;
pthread_mutex_t buffmutex = PTHREAD_MUTEX_INITIALIZER;

// State variables for analysis
double total_voltage = 0.0;
double prev_avg_voltage = 0.0;
double gmin_voltage = 5.1;
double gmax_voltage = 0.0;
int dip_count = 0;
long long last_second_timestamp = 0;
long long gmin_interval = 100000000000;
long long gmax_interval = 0;
long long total_interval = 0;
int interval_count = 0;


// read light values
static int readFromFileToScreen(char *fileName);
long long getTimeInMs(void);
long long getTimeInNs(void);
void sleepForMs(long long delayInMs);
static void *sample();
static void analyzeSamples();

static void* runMod(void* unused){
    
    ledinit();

    // Create the thread to read photoresistor samples
    if (pthread_create(&sampling_thread, NULL, sample, NULL) != 0) {
        fprintf(stderr, "Error creating sampling thread\n");
        return NULL;
    }

    sleep(1);
    printf("Starting Sampling...\n");

    while (1)
    {
        analyzeSamples();

        sleep(1);
    }
    return NULL;
}

void photoresInit(){
    // create thread
    int retVal = pthread_create(&threadPID, NULL, runMod, NULL);

    // error checking
    if (retVal != 0){
        // error code
        perror("photo thread error");
        exit(1);
    }
}


void photoresCancel(){
    pthread_cancel(threadPID);
    pthread_cancel(sampling_thread);
}

void photoresShutDown(){
    // REMEBER TO REMOVE CACNEL
    // photoresCancel();

    pthread_join(threadPID, NULL);
    pthread_join(sampling_thread, NULL);

    turnOffLeds();
}

static void *sample(){
    while (1) {
        // Read A2D
        double reading = readFromFileToScreen("/sys/bus/iio/devices/iio:device0/in_voltage1_raw");
        double voltage = ((double)reading / 4095) * 1.8;
        
        // Store the value in the buffer
        pthread_mutex_lock(&buffmutex);
        if (buffindex < 1000) {
            buff[buffindex].voltage = voltage;
            buff[buffindex].timestamp = getTimeInNs();
            buffindex++;
        }
        pthread_mutex_unlock(&buffmutex);
        
        // Sleep for 1ms
        sleepForMs(1);
    }

    return NULL;
}

static void analyzeSamples() {
    pthread_mutex_lock(&buffmutex);

    long long current_timestamp = getTimeInNs();
    long long elapsed_time = current_timestamp - last_second_timestamp;

    int current_second_samples = 0;
    double current_second_total_voltage = 0.0;
    double min_voltage = 1.8;
    double max_voltage = 0.0;
    int current_second_dip_count = 0;
    long long current_second_min_interval = 1000000.1;
    long long current_second_max_interval = 0;
    long long current_second_total_interval = 0;
    int current_second_interval_count = 0;
    double avg_voltage = 0;
    bool dip = true;

    // Exponential smoothing parameters
    // double alpha = 0.999;

    for (int i = 0; i < buffindex; ++i) {
        if (current_timestamp - buff[i].timestamp <= elapsed_time) {
            current_second_samples++;
            current_second_total_voltage += buff[i].voltage;

            if (buff[i].voltage < min_voltage) {
                min_voltage = buff[i].voltage;
                gmin_voltage = min_voltage;
            }
            if (buff[i].voltage > max_voltage) {
                max_voltage = buff[i].voltage;
                gmax_voltage = max_voltage;
            }
            
            
            if ((buff[i].voltage <= (prev_avg_voltage + 0.1)) && i != 0){
                if(!dip){
                    current_second_dip_count++;
                    dip = true;
                }
            }
            if (buff[i].voltage > (prev_avg_voltage + 0.13)){
                dip = false;
            }

            avg_voltage += buff[i].voltage;
            prev_avg_voltage = avg_voltage / buffindex;

            

            

            // Check for dips
            // if ((buff[i].voltage) >= 0.03) {
            //     // If the voltage is 0.1V or more away from the current average light level
            //     // and not within the hysteresis range, count it as a dip
            //     current_second_dip_count++;
            // }

            if (i > 0) {
                long long interval = buff[i].timestamp - buff[i - 1].timestamp;
                current_second_total_interval += interval;

                if (interval < current_second_min_interval) {
                    current_second_min_interval = interval;
                    gmin_interval = current_second_min_interval;
                }
                if (interval > current_second_max_interval) {
                    current_second_max_interval = interval;
                    gmax_interval = current_second_max_interval;
                }

                current_second_interval_count++;
            }
        }
    }

    total_voltage += current_second_total_voltage;
    // prev_avg_voltage = (alpha * prev_avg_voltage) + ((1 - alpha) * (current_second_total_voltage / current_second_samples));
    // prev_avg_voltage = avg_voltage / buffindex;
    // dip_count += current_second_dip_count;

    if (current_second_interval_count > 0) {
        gmin_interval = current_second_min_interval;
        gmax_interval = current_second_max_interval;
        total_interval += current_second_total_interval;
        interval_count += current_second_interval_count;
    }

    buffindex = 0;
    last_second_timestamp = current_timestamp;

    pthread_mutex_unlock(&buffmutex);

    // Print the analysis results
    printf("Interval ms (%.3f, %.3f) avg=%.3f   ", (double)gmin_interval / 1000000, (double)gmax_interval / 1000000, (double)total_interval / interval_count / 1000000);
    printf("Samples V (%.3f, %.3f) avg=%.3f   ", min_voltage, max_voltage, prev_avg_voltage);
    printf("# Dips: %d   # Samples: %d\n", current_second_dip_count, current_second_samples);
}

// helper functions
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

long long getTimeInMs(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000 + nanoSeconds / 1000000;
    return milliSeconds;
}

long long getTimeInNs(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long nanoSecondsTime = seconds * 1000000000 + nanoSeconds;
    return nanoSecondsTime;
}

void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}

// for joystick
double getMaxVoltage(){
    return gmax_voltage;
}

double getMinVoltage(){
    return gmin_voltage;
}

long long getMaxInterval(){
    return gmax_interval;
}

long long getMinInterval(){
    return gmin_interval;
}