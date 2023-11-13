my-malloc: my-malloc.c
	gcc -g -Wall -pedantic -rdynamic -shared -fPIC -o my-malloc.so my-malloc.c
	gdb --args env LD_PRELOAD=./my-malloc.so ./test-malloc

.PHONE: clean
clean:
	rm -f my-malloc
