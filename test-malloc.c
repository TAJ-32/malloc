#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

int main(int argc, char *argv[])
{
	char *a = malloc(12);

	*a = 'a';
	*(a + 1) = 'b';
	*(a + 3) = 'c';

	size_t b = malloc_usable_size(a);

	printf("%ld\n", b);

	return 0;
}
