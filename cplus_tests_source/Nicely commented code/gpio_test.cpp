//RASPBERRY PI GPIO PIN TEST
//For GPIO17 - Pin 11 on header
//November 2012

//#include <stdlib>
//#include <stdio>
#include <string>
#include <iostream>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
using namespace std;

#define BASEMEM 0x20000000
#define BASEGPIO (BASEMEM + 0x200000)
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

volatile uintptr_t* setupPins();
void set_Output(unsigned int pin, volatile uintptr_t* pGPIO);
void set_Input(unsigned int pin, volatile uintptr_t* pGPIO);
void set_PinState(unsigned int pin, volatile uintptr_t* pGPIO);
void reset_PinState(unsigned int pin, volatile uintptr_t* pGPIO);
bool setAsOutput = false;


int main(void)
{
//True = output / false = input
bool bInputSet = false;

//GPIO 17 == pin 11 on the RPi header
int testPin = 17;
string consoleInput = "";
bool bQuit = false;

volatile uintptr_t *pointerToGPIO;

if( (pointerToGPIO = setupPins()) == NULL) cout << "Failure to setup pins\n";

cout << "Raspberry Pi basic GPIO test program \n This program will test GPIO_17 (Pin 11 on header) \n";


//GPIO pin to test
unsigned int g = 17;
char a;

while (bQuit == false)
{

cout << "Press i for input, o for output or q to quit\n";
cin >> a;

if (a == 'q')
{
bQuit = true;
}
if (a == 'i')
{ 
set_Input(g, pointerToGPIO);
bInputSet = true;
}
if (a == 'o')
{
	// ** Needed to do once as you need to first set a pin to input before setting it to output
	if (bInputSet == false)
	{
		set_Input(g, pointerToGPIO);
	}
set_Output(g, pointerToGPIO);
}
//end of while
}

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

//Technically you'd step through each pin and enable it to be an input
//in order to set up the pins for use

int fd;

//open dev/mem  -- LINUX Specific code which is handy due to how Linux's file system treats system parts.
//dev/mem allows read/write access directly to the memory.
if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) <0){
	cout << "Can't open /dev/mem \n";
	exit(-1);
	};
	
cout << "Open done \n";
	
void *gpioMem;

//allocate memory block for mapping -- Needed to get C++ to be able to write directly to memory.
if ((gpioMem= malloc(BLOCK_SIZE+(PAGE_SIZE-1))) == NULL) {
	cout << "allocation error \n";
	exit(-1);
};
cout << "gpioMem = " << gpioMem << "\n&gpioMem = "<< &gpioMem << "\n";
cout << "allocate (malloc) done \n";

volatile uintptr_t *gpio_map;
//map the memory allocated in the previous malloc. mmap returns a void pointer, so casting to a uintptr_t so it can be dereferenced later.
gpio_map = static_cast<volatile uintptr_t*>(mmap(gpioMem, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, BASEGPIO));

//close the file descriptor.
close(fd);
 
 //Handling if there's been a mapping problem.
  if ((long)gpio_map < 0) {
      cout<<"mmap error\n" << (int)gpio_map << "\n";
      perror("Cannot map due to \n");
	exit (-1);
   };

cout << "mmap section done \n";

//Possibly not needed. 
volatile uintptr_t *pGPIO;
pGPIO = gpio_map;

//debug testing
cout << "BaseGPIO (as decimal)= " << BASEGPIO << "\n";
cout <<"pGPIO is pointing to = " << pGPIO << "\n";
cout <<"&pGPIO addressof value (where pointer is stored) = " << &pGPIO << "\n";
cout <<"*pGPIO derefence value = " << *pGPIO << "\n";

cout << "*Finish Pin Setup* \n";

return pGPIO;
}


void set_Input(unsigned int pin, volatile uintptr_t* pGPIO)
{
//This function sets the selected GPIO Pin (the pin according to the GPIO controller, not what's printed on the RPi board) to be an input
//Note: The function select register requires a value of 000 to acknowledge the pin is set to be an input

//At this point, pGPIO should be pointing to 0x2020 0000

//This code works out which register to affect.
//Eg, GPIO 17 / 10 = 1.7, as C++ truncates values, it's actually 1. GPIO 23 would be 2, GPIO 9 would be 0 etc...
unsigned int a = pin / 10;

//'b' works out which set of three bits in the register we're going to change
//Eg (GPIO 17 mod 10) = 7 
// 7 *3  = 21  -- 21 is the value we're doing a left shift by later
unsigned int b = (pin % 10) *3;

//'c' is the actual location we're editing. 
//Of note is the fact 'a' is an unsigned int with a value of 1. A unsigned int has a size of 32 bits (4 bytes).
//We're doing pointer arithmetic, so it's not adding decimal 1 to the memory location, but adding the a multiple of the size of a.
//as 'a' = 1 if you put in PGIO 17 earlier, it's incrementing by 'a'*4 bytes (1*4 = 4)
//If we'ed used GPIO 23 earlier, it would be 2*4 (8 bytes) being added.
unsigned int c = *(pGPIO + a);
//'c' should be pointing to 0x2020 0004 at this point.

// << is a left shift operation. I'm cheating here by using a decimal value of 7. 7 has a binary value of 111. So we're inserting 111 at postion b (21?)
// (7 << b) results in 00 000 000 111 000 000 000 000 000 000 000
//However, we're also doing some really funky stuff along side that.
// ~ is a bitwise operation for "1's complement", which essentially inverts it to 11 111 111 000 111 111 111 111 111 111
// Bitwise &= is an AND operation and assignment.
// What this does is take the existing register, performs and logic on it to preserve everything but the 000 at location 21 which forces 000 into the register
*(pGPIO+a) = (c &= ~(7 << b ));


//make sure output reg is cleared.
reset_PinState(pin, pGPIO);


cout <<"End set as input \n";
}



//set pin as output
void set_Output(unsigned int pin, volatile uintptr_t* pGPIO)
{
cout << "trying to set pin" << pin <<" as OUTPUT \n";


//set mode
unsigned int a = pin / 10;
unsigned int b = (pin % 10) *3;
unsigned int c = *(pGPIO + a);
//all functions the same as input, but instead we're only putting a 1 into the code to make it: 00 000 000 001 000 000 000 000 000 000
//Not doing an invert here, and just using a logical OR
*(pGPIO+a) = (c |= (1 << b ));

//turns the pin on.
set_PinState(pin, pGPIO);

cout <<"End set as output \n";

}

void set_PinState(unsigned int pin, volatile uintptr_t* pGPIO)
{
//set pin state
cout <<"\nGPSET0 dereference value =" << *(pGPIO + 7)<< "\n";

//GPSET 0
//pGPIO+7 = 0x2020 001C

*(pGPIO+7) = 1 << pin;
cout <<"GPSET0 dereference value (after setting output to 1) =" << *(pGPIO + 7)<< "\n";
}

void reset_PinState(unsigned int pin, volatile uintptr_t* pGPIO)
{
//clear pin state
//GPCLR0 - 2020 0028
*(pGPIO+10) = 1 << pin;
}