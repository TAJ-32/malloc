CFLAGS=-Wall -pedantic -g

all: test-malloc my-malloc.so

my-malloc.so: my-malloc.c
	gcc -g -Wall -pedantic -rdynamic -shared -fPIC -o my-malloc.so my-malloc.c

test-malloc: test-malloc.c
	gcc -g -Wall -pedantic -o test-malloc test-malloc.c

.PHONE: ex
ex:
	LD_PRELOAD=./my-malloc.so ls -l

.PHONE: debug
debug: 
	gdb --args env LD_PRELOAD=./my-malloc.so ls -l

.PHONE: clean
clean:
	rm -f my-malloc.so test-malloc
