// RASPBERRY PI GPIO Control - Core Module - RpiCore
// Jan 2016
// Ian Buswell

// For GPIO17 - Pin 11 on header / GPIO 23

//#include <stdlib>
//#include <stdio>
#include <string>
#include <iostream>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#include "RpiCore.h"

using namespace std;

#define BASEMEM 0x20000000
#define BASEGPIO (BASEMEM + 0x200000)
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

bool setAsOutput = false;

char* RpiCore::binary(unsigned int v) {
	static char binstr[33];
	int i;

	binstr[32] = '\0';
	for (i = 0; i < 32; i++) {
		binstr[31 - i] = v & 1 ? '1' : '0';
		v = v / 2;
	}

	return binstr;
}

struct timeval GetTimeStamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv;
}

volatile uintptr_t *pointerToGPIO;

RpiCore::RpiCore()
{
	// Ctor!
	cout << "RPI Ctor - Initialised";

	volatile uintptr_t *pointerToGPIO;
	
	if ((pointerToGPIO = setupPins()) == NULL) cout << "Failure to setup pins\n";
	cout << "Raspberry Pi - Ultrasonic sensor test, GPIO 17 as trigger. 23 as Echo \n";

	//Loop through GPIO's to set non special ones as input, ready for use:
	int i = 0;
	for (i = 0; i < 26;i++)
	{
		if (i == 17 || i == 22 || i == 23)
		{
			set_Input(i, pointerToGPIO);
		}

	}
}

RpiCore::~RpiCore() {

	pointerToGPIO = null;

}


volatile uintptr_t* setupPins()
{
	//
	// Setting up the pins ready for GPIO use
	//
	// First setup memory mapping, then step through pins to initialise by setting them all to input.
	//

	//pGPIO = (uint32_t*) (BASEGPIO + 4);
	//pGPIO = (uint32_t*)0x20200000;
	//pGPIO = reinterpret_cast< void* >(0x20200000);
	//int addy = 0x20200000;
	//pGPIO = reinterpret_cast< void * >(addy);
	//pGPIO = (uint32_t *) BASEGPIO;

	//Technically you'd step through each pin and enable it to be an input
	//in order to set up the pins for use

	int fd;

	//open dev/mem
	if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
		cout << "Can't open /dev/mem \n";
		exit(-1);
	};

	cout << "Open done \n";
	//allocate memory block for mapping
	void *gpioMem;/*
	if ((gpioMem= malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
		cout << "allocation error \n";
		exit(-1);
	}*/
	if ((gpioMem = malloc(BLOCK_SIZE + (PAGE_SIZE - 1))) == NULL) {
		cout << "allocation error \n";
		exit(-1);
	};
	cout << "gpioMem = " << gpioMem << "\n&gpioMem = " << &gpioMem << "\n";
	cout << "allocate (malloc) done \n";

	/*if ((unsigned long)gpioMem % PAGE_SIZE)
		gpioMem += PAGE_SIZE - ((unsigned long)gpioMem % PAGE_SIZE);*/

	volatile uintptr_t *gpio_map;
	// 1(uint32_t *)mmap(gpioMem, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, BASEGPIO);
	// (partially works, wrong mem addy)
	// 2 gpio_map = mmap((void*)0x0200000, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, BASEGPIO);
	//3  gpio_map = mmap((void*)0x00000000, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, BASEGPIO);

	gpio_map = static_cast<volatile uintptr_t*>(mmap(gpioMem, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BASEGPIO));

	close(fd);

	if ((long)gpio_map < 0) {
		cout << "mmap error\n" << (int)gpio_map << "\n";
		perror("Cannot map due to \n");
		exit(-1);
	};

	cout << "mmap section done \n";

	//pGPIO = (uint32_t*) gpio_map;
	volatile uintptr_t *pGPIO;
	pGPIO = gpio_map;

	//debug testing
	cout << "BaseGPIO (as decimal)= " << BASEGPIO << "\n";
	cout << "pGPIO is pointing to = " << pGPIO << "\n";
	cout << "&pGPIO addressof value (where pointer is stored) = " << &pGPIO << "\n";
	cout << "*pGPIO derefence value = " << *pGPIO << "\n";

	cout << "*Finish Pin Setup* \n";

	return pGPIO;
}


void RpiCore::set_Input(unsigned int pin, volatile uintptr_t* pGPIO)
{

	cout << "trying to set pin" << pin << " as INPUT \n";

	//pGPIO = *(pGPIO + ((pin)/10)) &= ~(7 << (((pin)%10)*3) );

	//debug testing
	/*
	cout <<"\nValues before changes\n\n";
	cout <<"pGPIO = " << pGPIO << "\n";
	cout <<"&pGPIO addressof value (where pointer is stored) = " << &pGPIO << "\n";
	cout <<"*pGPIO derefence value =" << *pGPIO << "\n";
	*/

	unsigned int a = pin / 10;
	unsigned int b = (pin % 10) * 3;
	unsigned int c = *(pGPIO + a);

	*(pGPIO + a) = (c &= ~(7 << b));
	//make sure output reg is cleared.
	reset_PinState(pin, pGPIO);

	//debug testing
	/*
	cout <<"\nValues after changes\n\n";
	cout <<"pGPIO + a  = " << (pGPIO +a )<< "\n";
	cout <<"&pGPIO addressof value (where pointer is stored) = " << &pGPIO << "\n";
	cout <<"*pGPIO + a derefence value =" << *(pGPIO + a) << "\n";

	cout <<"End set as input \n";*/
}



//set pin as output
void RpiCore::set_Output(unsigned int pin, volatile uintptr_t* pGPIO)
{
	cout << "trying to set pin" << pin << " as OUTPUT \n";

	//debug testing
	/*
	cout <<"\nValues before changes \n\n";
	cout <<"pGPIO = " << pGPIO << "\n";
	cout <<"&pGPIO addressof value (where pointer is stored) = " << &pGPIO << "\n";
	cout <<"*pGPIO derefence value =" << *pGPIO << "\n";
	*/
	//set mode
	unsigned int a = pin / 10;
	unsigned int b = (pin % 10) * 3;
	unsigned int c = *(pGPIO + a);
	*(pGPIO + a) = (c |= (1 << b));
	/*
	//debug testing
	cout <<"Values after change but before setting output state \n";
	cout <<"pGPIO + a = " << (pGPIO + a) << "\n";
	cout <<"&pGPIO addressof value (where pointer is stored) = " << &pGPIO << "\n";
	cout <<"*pGPIO + a derefence value =" << *(pGPIO + a)<< "\n";

	set_PinState(pin, pGPIO);

	cout <<"End set as output \n";
	*/
}

void RpiCore::set_PinState(unsigned int pin, volatile uintptr_t* pGPIO)
{
	//set pin state
	cout << "\nGPSET0 dereference value =" << *(pGPIO + 7) << "\n";
	*(pGPIO + 7) = 1 << pin;
	cout << "GPSET0 dereference value (after setting output to 1) =" << *(pGPIO + 7) << "\n";
}

void RpiCore::set_PinState(unsigned int pin, volatile uintptr_t* pGPIO, unsigned int value)
{
	cout << "set_Pinstate value = " << value << "\n";
	//if (value != 1 or value != 0)
	//{
	//return;
	//} 
	//else{

	//set pin state
	cout << "\nGPSET0 dereference value =" << *(pGPIO + 7) << "\n";
	*(pGPIO + 7) = value << pin;
	cout << "GPSET0 dereference value (after setting output to " << value << ") =" << *(pGPIO + 7) << "\n";
}
//}

void RpiCore::reset_PinState(unsigned int pin, volatile uintptr_t* pGPIO)
{
	//clear pin state
	//GPCLR0 for most pins. GPCLR1 is only for pins 30+, which aren't GPIO anyway on a rev1 board.
	*(pGPIO + 10) = 1 << pin;
}

int RpiCore::get_PinState(unsigned int pin, volatile uintptr_t* pGPIO)
{
	int val = 0;
	unsigned int a = 13; //register offset. 7 = GPSET0. 10 = GPCLR0. 13 = GPLEV0
	//Access GPLEV0 register for the correct pin
	if ((*(pGPIO + a) & (1 << (pin & 31))) != 0)
		val = 1;
	else
		val = 0;

	//cout <<"pGPIO + a  = " << (pGPIO +a )<< "\n";
	//cout <<"&pGPIO addressof value (where pointer is stored) = " << &pGPIO << "\n";
	//cout <<"*pGPIO + a derefence value =" << *(pGPIO + a) << "\n";

	return val;
}