//RASPBERRY PI GPIO Shift Register trst.
//For GPIO17 - Pin 11 on header / GPIO 23
//Oct 2014

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
using namespace std;

#define BASEMEM 0x20000000
#define BASEGPIO (BASEMEM + 0x200000)
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

volatile uintptr_t* setupPins();
void set_Output(unsigned int pin, volatile uintptr_t* pGPIO);
void set_Input(unsigned int pin, volatile uintptr_t* pGPIO);
void set_PinState(unsigned int pin, volatile uintptr_t* pGPIO);
void set_PinState(unsigned int pin, volatile uintptr_t* pGPIO, unsigned int value);
void reset_PinState(unsigned int pin, volatile uintptr_t* pGPIO);
int get_PinState(unsigned int pin, volatile uintptr_t* pGPIO);
bool setAsOutput = false;
//static uint32_t *pGPIO;



char *binary(unsigned int v) {
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



int main(void)
{
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

	unsigned int trig = 17;
	unsigned int echo = 23;

	//Actual program
	char a;
	bool bQuit = false;
	//set 17 as output, leave 23 as input 
	set_Output(trig, pointerToGPIO);
	//set_Output(23, pointerToGPIO);
	while (bQuit == false)
	{
		cout << "Press a to start, or q to quit\n";
		cin >> a;

		if (a == 'q')
		{
			bQuit = true;
		}
		if (a == 'a')
		{
			//clear the pins
			cout << "Starting: Clearing Trigger Pin \n";
			reset_PinState(trig, pointerToGPIO); //make sure trigger pin is 0
			usleep(100); //Make sure sensor is settled.

			cout << "Start Trigger \n";
			//trigger!
			set_PinState(trig, pointerToGPIO, 1);
			usleep(100);
			//set_PinState(trig, pointerToGPIO, 0);
			reset_PinState(trig, pointerToGPIO);
			cout << "...Trigger send complete \n";

			cout << "Wait for echo \n";
			//get times and wait for responses.
			int start_time = 0;
			while (get_PinState(echo, pointerToGPIO) == 0)
			{
				start_time = GetTimeStamp().tv_usec; //?
			}
			int end_time = 0;
			while (get_PinState(echo, pointerToGPIO) == 1)
			{
				end_time = GetTimeStamp().tv_usec;
			}

			//work out distances.
			int duration = end_time - start_time;
			int distance1 = 0;
			distance1 = duration * 17150; //343 m/s = speed of sound -> 34300 cm/s. Our duration value includes the time taken for the pulse to go there and back, so half it = 17150 (NB. May be bollocks). 

			int distance2 = duration / 58; // us / 58 to get cm. Based on duration of pulse.

			int distance3 = (duration * 343) / 58;

			cout << "Duration = " << duration << "\n";
			cout << "Distance1 (\?\?): duration * 17150 = " << distance1 << "\n";
			cout << "Distance2 (cm): duration /58 = " << distance2 << "\n";
			cout << "Distance3 (cm): (duration*343)/58 = " << distance3 << "\n";


		}
	}

	//clear the pins.
	reset_PinState(trig, pointerToGPIO);
	reset_PinState(echo, pointerToGPIO);

	cout << "Program Finished.\n";

	return 0;
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


void set_Input(unsigned int pin, volatile uintptr_t* pGPIO)
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
void set_Output(unsigned int pin, volatile uintptr_t* pGPIO)
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

void set_PinState(unsigned int pin, volatile uintptr_t* pGPIO)
{
	//set pin state
	cout << "\nGPSET0 dereference value =" << *(pGPIO + 7) << "\n";
	*(pGPIO + 7) = 1 << pin;
	cout << "GPSET0 dereference value (after setting output to 1) =" << *(pGPIO + 7) << "\n";
}

void set_PinState(unsigned int pin, volatile uintptr_t* pGPIO, unsigned int value)
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

void reset_PinState(unsigned int pin, volatile uintptr_t* pGPIO)
{
	//clear pin state
	//GPCLR0 for most pins. GPCLR1 is only for pins 30+, which aren't GPIO anyway on a rev1 board.
	*(pGPIO + 10) = 1 << pin;
}

int get_PinState(unsigned int pin, volatile uintptr_t* pGPIO)
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