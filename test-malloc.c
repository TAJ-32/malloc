#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char *a = malloc(1000);
	*a = 'h';

	char *b = malloc(2500);
	*b = 'g';

	free(a);

	char *c = malloc(1000);
	//c = "meh";



	//memcpy(a, "asdf", 4);	

	//free(ptr);
	//printf("This shouldn't work\n");
}
