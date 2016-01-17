//RASPBERRY PI GPIO Shift Register trst.
//For GPIO17 - Pin 11 on header
//May 27th 2013

//#include <stdlib>
//#include <stdio>
#include <string>
#include <iostream>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#include "RpiCore.h"

int main(void)
{

	//program flow:

	/*
	1) Setup pins.
	2) Get 8 bit data. uint8_t
	3) for each bit, ds to high (pin17) and set clock (pin22) to high. Wait an ms or slow (sleep(1))
	4) Clock to low, ds to low.

	**/
	RpiCore RCore();


	//True = output / false = input
	bool bInputSet = false;


	//GPIO 17 == pin 11 on the RPi header
	string consoleInput = "";
	volatile uintptr_t *pointerToGPIO;

	if ((pointerToGPIO = setupPins()) == NULL) cout << "Failure to setup pins\n";

	cout << "Raspberry Pi basic GPIO test program \n This program will test GPIO_17 (Pin 11 on header) \n";

	//Loop through GPIO's to set non special ones as input, ready for use:
	int i = 0;
	for (i = 0; i < 26;i++)
	{
		if (i == 17 || i == 22 || i == 23)
		{
			RCore.set_Input(i, pointerToGPIO);
		}

	}

	//0d235 = 0b11101011
	uint8_t testvalue = 235;

	//GPIO pin to test
	unsigned int pin = 17;
	char a;
	bool bQuit = false;

	//set 17,22,23 to be output pins
	RCore.set_Output(17, pointerToGPIO);
	RCore.set_Output(22, pointerToGPIO);
	RCore.set_Output(23, pointerToGPIO);



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
			cout << "Trying to set up serial output \n";
			unsigned int c = 0;
			for (c = 0;c < 8;c++)
			{
				//clear the pins
				RCore.reset_PinState(17, pointerToGPIO);
				RCore.reset_PinState(22, pointerToGPIO);

				//set the pins
				//
				unsigned int value = c % 2;
				if (value == 0)
				{
					RCore.set_PinState(17, pointerToGPIO, 0);
				}
				else
				{
					RCore.set_PinState(17, pointerToGPIO, 1);
				}
				cout << "Set pin 17 to " << value << " \n";

				RCore.set_PinState(22, pointerToGPIO, 1);
				sleep(1);
				RCore.set_PinState(22, pointerToGPIO, 0);

				cout << "For loop cycle + " << c << "\n";
			}
		}

		//bInputSet = true;
	}
	/*if (a == 'o')
	{
		// ** Needed to do once as you need to first set a pin to input before setting it to output
		if (bInputSet == false)
		{
			set_Input(pin, pointerToGPIO);
		}
	set_Output(pin, pointerToGPIO);
	}
	//end of while
	}

	*/

	//clear the pins.
	RCore.reset_PinState(17, pointerToGPIO);
	RCore.reset_PinState(22, pointerToGPIO);

	cout << "Program Finished.\n";

	return 0;
}