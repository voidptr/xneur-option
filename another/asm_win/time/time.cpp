#include <stdio.h>

extern "C" int getsystimer(unsigned int *low, unsigned int *hight);

int main()
{
	unsigned int low, hight;
	getsystimer(&low, &hight);

	__int64 ticks = ((unsigned __int64)hight << 32) + low;

	printf("Getting system time: %llo\n", ticks);
	
	return 0;
}
//---------------------------------------------------------------------------
