#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char *a = malloc(1000);

	memcpy(a, "asdf", 4);	

	//free(ptr);
	//printf("This shouldn't work\n");
}
