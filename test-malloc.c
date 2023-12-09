#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char *ptr = malloc(8);

	*ptr = 'a';
	*(ptr + 1) = 'b';

	char *ptr2 = realloc(ptr, 12);
	*(ptr2 + 2) = 'c';
	for (int i = 0; i < 3; i++) {
		printf("%c ", *(ptr2 + i));
	}

	getchar();
	return 0;
}
