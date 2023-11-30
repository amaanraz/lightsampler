#include <stdio.h>
#include "leds.h"

static int initI2cBus(char* bus, int address);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);
static void runCommand(char* command);

// Module Functions
void displayNum(int num); // Display number on lEd
void displayDec(double num);
void turnOffLeds();

// helper functions


// number object
typedef struct number {
	unsigned char row;
    int value;
} number;

number numbers[10][8] = {
    // zero
    {
        {REG_DIR1, 0x00},
        {REG_DIR2, 0x70},
        {REG_DIR3, 0x50},
        {REG_DIR4, 0x50},
        {REG_DIR5, 0x50},
        {REG_DIR6, 0x50},
        {REG_DIR7, 0x50},
        {REG_DIR8, 0x70},
    },
    // one
    {
        {REG_DIR1, 0x00},
        {REG_DIR2, 0x70},
        {REG_DIR3, 0x20},
        {REG_DIR4, 0x20},
        {REG_DIR5, 0x20},
        {REG_DIR6, 0x20},
        {REG_DIR7, 0x60},
        {REG_DIR8, 0x20},
    },
    // two 
	{
		{REG_DIR1, 0x00},
		{REG_DIR2, 0x70},
		{REG_DIR3, 0x40},
		{REG_DIR4, 0x20},
		{REG_DIR5, 0x10},
		{REG_DIR6, 0x10},
		{REG_DIR7, 0x50},
		{REG_DIR8, 0x20},
	},
	{//three
		{REG_DIR1, 0x00},
		{REG_DIR2, 0x70},
		{REG_DIR3, 0x10},
		{REG_DIR4, 0x10},
		{REG_DIR5, 0x70},
		{REG_DIR6, 0x10},
		{REG_DIR7, 0x10},
		{REG_DIR8, 0x70},
	},
	{//four
		{REG_DIR1, 0x00},
		{REG_DIR2, 0x10},
		{REG_DIR3, 0x10},
		{REG_DIR4, 0x10},
		{REG_DIR5, 0x70},
		{REG_DIR6, 0x50},
		{REG_DIR7, 0x50},
		{REG_DIR8, 0x50},
	},
	{//five
		{REG_DIR1, 0x00},
		{REG_DIR2, 0x70},
		{REG_DIR3, 0x50},
		{REG_DIR4, 0x10},
		{REG_DIR5, 0x70},
		{REG_DIR6, 0x40},
		{REG_DIR7, 0x50},
		{REG_DIR8, 0x70},
	},
	{//six
		{REG_DIR1, 0x00},
		{REG_DIR2, 0x70},
		{REG_DIR3, 0x50},
		{REG_DIR4, 0x50},
		{REG_DIR5, 0x70},
		{REG_DIR6, 0x40},
		{REG_DIR7, 0x40},
		{REG_DIR8, 0x70},
	},
	{//seven
		{REG_DIR1, 0x00},
		{REG_DIR2, 0x10},
		{REG_DIR3, 0x10},
		{REG_DIR4, 0x10},
		{REG_DIR5, 0x10},
		{REG_DIR6, 0x10},
		{REG_DIR7, 0x10},
		{REG_DIR8, 0x70},
	},
	{//eight
		{REG_DIR1, 0x00},
		{REG_DIR2, 0x70},
		{REG_DIR3, 0x50},
		{REG_DIR4, 0x50},
		{REG_DIR5, 0x70},
		{REG_DIR6, 0x50},
		{REG_DIR7, 0x50},
		{REG_DIR8, 0x70},
	},
	{//nine
		{REG_DIR1, 0x00},
		{REG_DIR2, 0x10},
		{REG_DIR3, 0x10},
		{REG_DIR4, 0x10},
		{REG_DIR5, 0x70},
		{REG_DIR6, 0x50},
		{REG_DIR7, 0x50},
		{REG_DIR8, 0x70},
	}
};

void ledinit(){
	runCommand("config-pin P9_18 i2c");
	runCommand("config-pin P9_17 i2c");

	runCommand("i2cset -y 1 0x70 0x21 0x00");
    runCommand("i2cset -y 1 0x70 0x81 0x00");
}

static int initI2cBus(char* bus, int address)
{
	int i2cFileDesc = open(bus, O_RDWR);
	if (i2cFileDesc < 0) {
		printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(-1);
	}

	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		perror("Unable to set I2C device to slave address.");
		exit(-1);
	}
	return i2cFileDesc;
}

static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) {
		perror("Unable to write i2c register");
		exit(-1);
	}
}

void displayNum(int num){
	// display int - two digits
	if(num > 100){
		printf("cannot display more than two digits\n");
		return;
	}

	// init bus
	int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

	// display
	int d1 = 0;
	int d2 = num;
    // divide the number into digits
    if (num > 9) {
        d1 = num / 10;
        d2 = num % 10;
    }


    for (int i = 0; i < 8; i++) {
        // write numbers
		int shiftedNum = numbers[d2][i].value >> 4;
		writeI2cReg(i2cFileDesc, numbers[d2][i].row, numbers[d1][i].value + shiftedNum);
    }
   
    close(i2cFileDesc);
}

void displayDec(double num){

	// init bus
	int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

	// convert double to int
	num = num*10;
	int n_num = (int)(num);

	// display
	int d1 = 0;
	int d2 = n_num;

    // divide the number into digits
    if (num > 9) {
        d1 = n_num / 10;
        d2 = n_num % 10;
    }


    for (int i = 0; i < 8; i++) {
        // write numbers
		int shiftedNum = numbers[d2][i].value >> 4;
		writeI2cReg(i2cFileDesc, numbers[d2][i].row, numbers[d1][i].value + shiftedNum);
    }

	// decimal point
	writeI2cReg(i2cFileDesc, REG_DIR1, 0x08);
   
    close(i2cFileDesc);
}

void turnOffLeds(){
	int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
	// turn off all rows
	writeI2cReg(i2cFileDesc, REG_DIR1, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIR2, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIR3, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIR4, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIR5, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIR6, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIR7, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIR8, 0x00);

	close(i2cFileDesc);
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

