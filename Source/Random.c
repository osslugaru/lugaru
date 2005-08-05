
#include "Random.h"
#include "stdlib.h"


int randSeed = 1;

short Random()
{
	/*unsigned int value = (unsigned int)randSeed;
	unsigned int out=0;
	int i=31;

	while (i > 0)
	{
	value ^= 0x81010000;
	value ^= out;
	if (value & 1)
	{
	value >>= 1;
	value |= 0x80000000;
	}
	else
	value >>= 1;
	out <<= 1;
	out |= (value&0x1);
	i--;
	}
	if (value&0x80000000)
	{
	out <<= 1;
	out |= 1;
	}
	else
	out <<= 1;

	randSeed += out;
	*/
	return rand();
}
