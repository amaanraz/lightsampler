#ifndef _MISC_H_
#define _MISC_H_

long long getTimeInMs(void);
long long getTimeInNs(void);
void sleepForMs(long long delayInMs);
int readFromFileToScreen(char *fileName);
void runCommand(char* command);

#endif