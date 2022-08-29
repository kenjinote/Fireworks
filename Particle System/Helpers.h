/*
Some helper functions, mostly for random number generation.
*/

#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>		// Included for the random number generator routines.
#include <Windows.h>	// For DWORD
#include <time.h>       // For time

// seeds the random number generator with the system time
static void seedRandomNumberGenerator()
{
	srand(static_cast<unsigned int>(time(NULL)));
}

// returns a random integer number
static unsigned int random_number()
{
	return rand();
}

// returns a random number in the range specified by a and b
static unsigned int random_number(unsigned int a, unsigned int b)	
{
	// prevent division by zero as part of modulus calculation below
	if(b == a)
		return 0;

	return a + (rand() % (b - a));
}

// converts a float into a DWORD.
static inline DWORD FtoDW(float f) 
{
	return *((DWORD*)&f); 
}

#endif