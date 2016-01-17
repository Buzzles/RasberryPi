// RASPBERRY PI GPIO Control - Core Module - RpiCore
// Jan 2016
// Ian Buswell

#include <string>
#include <iostream>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

#define BASEMEM 0x20000000
#define BASEGPIO (BASEMEM + 0x200000)
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

class RpiCore
{
private:
	char *binary(unsigned int v);
	struct timeval GetTimeStamp();
	volatile uintptr_t *pointerToGPIO;
	volatile uintptr_t* setupPins();

public:

	RpiCore();
	~RpiCore();

	void set_Output(unsigned int pin, volatile uintptr_t* pGPIO);
	void set_Input(unsigned int pin, volatile uintptr_t* pGPIO);
	void set_PinState(unsigned int pin, volatile uintptr_t* pGPIO);
	void set_PinState(unsigned int pin, volatile uintptr_t* pGPIO, unsigned int value);
	void reset_PinState(unsigned int pin, volatile uintptr_t* pGPIO);
	int get_PinState(unsigned int pin, volatile uintptr_t* pGPIO);
};