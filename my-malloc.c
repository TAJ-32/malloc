#include <stdio.h>

static struct linked_list *heap_list = create_list();

void *my_malloc(size_t size);

void free(void *ptr);

struct allocation *create_chunk(size_t size);

struct allocation {
	int ptr;
	int user_size;
	int act_size; //actual size of whole chunk, sbrk_size + meta_size;
	int meta_size; //size of the struct in the chunk of memory
	int sbrk_size; //how much we will call sbrk for when they call malloc (will be more than they asked for)
	struct allocation *next_alloc; //pointer to next allocation in the linked list
};

struct linked_list {
	int size;
	struct *allocation head; //head of the Linked List
}

void *my_malloc(size_t size) {
	struct allocation *chunk = create_chunk(size, heap);
}

struct linked_list *create_list() {
	struct linked_list *heap_list;
	linked_list->size = 0;
	linked_list->head = NULL;

	return heap_list;
}

struct allocation *create_chunk(size_t size) {
	struct allocation *chunk; //how do I allocate memory to this
	
	chunk->user_size = size;
	chunk->meta_size = sizeof(chunk);
	chunk->sbrk_size = 1000;
	chunk->act_size = chunk->meta_size + chunk->sbrk_size;
	chunk->next_alloc = heap_list->head;
	heap_list->head = chunk;

	return chunk;	
}

int main(int argc, char *argv[])
{

}


/*
 *IDEAS:
 *	-Create a struct that represents a chunk of memory. It has properties
 *	of size, whether it is free/usable or not, what the range of the
 *	memory is, and a pointer to the beginning of it that will be returned
 *	when we make our malloc. So brk and sbrk will change the range of the
 *	memory I guess.
 *	-Have two separate variables in our struct, one for how much the user
 *	knows is allocated (how much they actually malloced) and how much
 *	we called brk or sbrk for actually. So kind of like our buf and
 *	hidden_buf. If the user calls malloc for the first time for malloc(12) we
 *	will sbrk(1000) or something and keep the other 988 bytes on standby. They are
 *	still free to be used (I think. they shouldn't be blocked off but on standby) and
 *	for every other malloc we call it will take from that chunk of 988 until it has been 
 *	exhausted, and then a new call to sbrk will be made.
 *	-It is important that the pointer variable in the struct specifically corresponds to
 *	a specific chunk of memory. So a struct cannot be for multiple chunks of memory. It is
 *	for ONE chunk and the pointer variable in it points to the beginning. So when a user
 *	calls free and they use the pointer, we need to retroactively see the pointer they give
 *	and if it is equal to the one in our struct to use the struct to then free the chunk that 
 *	the struct represents.
 *	- Important to remember that when free is called, it isn't just setting the chunk to be 
 *	usable but also memsetting all the data in there to be '\0'
 *	- We want to store the metadata with the actual allocated chunk. So we need each chunk of memory
 *	to actually allocate the size the user wants + the sizeof(struct we make)
 *	-How is the linked list going to work? What do you mean by us organizing the heap as
 *	struct, allocation, struct, allocation?
 *
 *	Will our mymalloc and myfree actually take the struct as an argument instead of just the ptr
 *
 *	-We need to make sure the pointer that the struct contains is aligned. Thus, the address
 *	of the pointer must be divisible by 16. Is there anything more complicated about that?
 */

