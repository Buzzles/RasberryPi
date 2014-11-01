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


int main(void)
{

int test = 1;
int *pInt = &test;

unsigned int test1 = 1;
unsigned int *pUnsignedInt  = &test1;

uintptr_t test2 = 1;
uintptr_t *pUnsignedInt_Pointer = &test2;

cout << "\npointer pInt points to: " << pInt;
pInt++;
cout << "\nincremented pointer pInt, now points to: " << pInt;

cout << "\n\nPointer pUnsignedInt points to: " << pUnsignedInt;
pUnsignedInt++;
cout << "\nincremented pUnsignedInt points to: " << pUnsignedInt;

cout << "\n\nPointer pUnsignedInt_Pointer points to: " << pUnsignedInt_Pointer;
pUnsignedInt_Pointer++;
cout << "\nIncremented Pointer pUnsignedInt_Pointer points to: " << pUnsignedInt_Pointer << "\n *Finished* \n";

return 0;
}