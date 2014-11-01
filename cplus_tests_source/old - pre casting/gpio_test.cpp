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

int setupPins();
void set_Output(int pin);
void set_Input(int pin);
bool setAsOutput = false;
//static uint32_t *pGPIO;
void *pGPIO;


char *binary (unsigned int v) {
static char binstr[17] ;
int i ;

binstr[16] = '\0' ;
for (i=0; i<16; i++) {
binstr[15-i] = v & 1 ? '1' : '0' ;
v = v / 2 ;
}

return binstr ;
}



int main(void)
{
//True = output / false = input
bool setAsOutput = false;

//GPIO 17 == pin 11 on the RPi header
int testPin = 17;
string consoleInput = "";
bool loopContinue = true;

if(setupPins() != 0) cout << "Failure to setup pins\n";

cout << "Raspberry Pi basic GPIO test program \n This program will test GPIO_17 (Pin 11 on header) \n";


//LOOP
/*
while(loopContinue)
{
	consoleInput = "";
	cout << "\n Pin 11 set to: " << setAsOutput << ". Change? y/n/q\n";
	cin >> consoleInput;

	if (consoleInput == "y")
	{
		//swap
		setAsOutput = !setAsOutput; 
		cout << " Yes clause";
	} 
	else if (consoleInput == "n")
	{
		//No change, do nothing
		
		cout << " No clause";
	}
	cout << "\nPin 11 now = " << setAsOutput;
	if (consoleInput == "q")
	{
		loopContinue = false;
		cout << " Quit clause \n";
	}
	//set pin as either in or out
	if (setAsOutput)
	{
	set_Output(11);
	}
	else set_Input(11);
};
*/
cout << "Program Finished.\n";

return 0;
}


int setupPins()
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
/*pGPIO = (uint32_t *) BASEGPIO;

cout << "BaseGPIO (as decimal)= " << BASEGPIO << "\n";
cout <<"pGPIO is pointing to = " << pGPIO << "\n";
cout <<"pGPIO derefence value =" << *pGPIO << "\n";
*/



//Technically you'd step through each pin and enable it to be an input
//in order to set up the pins for use

int setupPin = 17;
set_Input(setupPin);

int fd;

//open dev/mem
if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) <0){
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
if ((gpioMem= malloc(BLOCK_SIZE)) == NULL) {
	cout << "allocation error \n";
	exit(-1);
};
cout << "allocate (malloc) done \n";

 /*if ((unsigned long)gpioMem % PAGE_SIZE)
     gpioMem += PAGE_SIZE - ((unsigned long)gpioMem % PAGE_SIZE);*/

void *gpio_map;
// 1(uint32_t *)mmap(gpioMem, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, BASEGPIO);
// (partially works, wrong mem addy)
// 2 gpio_map = mmap((void*)0x0200000, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, BASEGPIO);
//3  gpio_map = mmap((void*)0x00000000, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, BASEGPIO);

gpio_map = mmap(gpioMem, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, BASEGPIO);

 close(fd);
 
  if ((long)gpio_map < 0) {
      cout<<"mmap error\n" << (int)gpio_map << "\n";
      perror("Cannot map due to \n");
	exit (-1);
   };

cout << "mmap section done \n";

//pGPIO = (uint32_t*) gpio_map;
pGPIO = gpio_map;

//debug testing
cout << "BaseGPIO (as decimal)= " << BASEGPIO << "\n";
cout <<"pGPIO is pointing to = " << pGPIO << "\n";
cout <<"pGPIO addressof value (where pointer is stored) = " << &pGPIO << "\n";
cout <<"pGPIO derefence value =" << ((int*)pGPIO) << "\n";

cout << "*Finish Pin Setup* \n";

/*
//set as input
*pGPIO = (*(pGPIO+((pin)/10)) &= ~(7<<(((pin)%10)*3)));
//set as output
*pGPIO = (*(pGPIO+((pin)/10)) |= ~(1<<(((pin)%10)*3)));
*/
int g = 17;
set_Input(g);

return 0;
}

//Set pin as input
/*
void set_Input(int pin)
{

cout << "trying to set pin" << pin <<" as input \n";
//set as input
*pGPIO = ((pGPIO+((pin)/10)) &= ~(7<<(((pin)%10)*3)));

}
*/
void set_Input(int pin)
{

cout << "trying to set pin" << pin <<" as input \n";
//set as input
unsigned int m;
//*pGPIO = ((pGPIO+((pin)/10)) &= ~(7<<(((pin)%10)*3)));

//change pGPIO to point to correct location for pin
pGPIO = ((unsigned int*)pGPIO +((pin/10)*4));

//perform bitwise operation calculations
m = ~(0x7 << ((pin%10)*3));

//dereference pGPIO to make the bitwise operations take effect
*pGPIO = *((reinterpret_cast<unsigned int*>(*pGPIO) & m));


}



//set pin as output
void set_Output(int pin)
{
/*
cout << "trying to set pin "<<"pin"<< " as output.\n";
//set as output
pGPIO = (*(pGPIO+((pin)/10)) |= ~(1<<(((pin)%10)*3)));
*/
}
