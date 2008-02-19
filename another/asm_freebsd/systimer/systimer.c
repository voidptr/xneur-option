#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

extern void getsystimer(unsigned int *low, unsigned int *hight);

int main()
{
	unsigned int low, hight;

	getsystimer(&low, &hight);

	uint64_t ticks = ((uint64_t)hight << 32) + low;

        printf("Getting system timer: %llu\n", ticks);

	return EXIT_SUCCESS;
}
