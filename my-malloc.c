#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>


static void* break_loc = sbrk(0); //should this be int instead? not sure

void *malloc(size_t size);

void free(void *ptr);

void *calloc(size_t nelem, size_t elsize);

void *realloc(void *ptr, size_t size);

size_t malloc_usable_size(void *ptr);

struct allocation *add_chunk(struct allocation *chunk, int size) 

struct allocation {
	void *ptr; //should this be an void pointer or just int
	int user_size;
	int act_size; //actual size of whole chunk, sbrk_size + meta_size;
	int meta_size; //size of the struct in the chunk of memory
	struct allocation *next_alloc; //pointer to next allocation in the linked list
	struct allocation *prev_alloc;

};

struct l_list {
	int size;
	struct allocation *head; //will represent the beginning of the linkedlist that starts at beginning of the heap.
				 //Before any malloc is called, this is just a pointer to the beginning and end of the heap which is just that starting line
};


struct l_list *create_list(void);

struct allocation *create_chunk(size_t size);

struct l_list *linked_list = create_list(); //how can I allocate memory to this

/*
struct linked_list {
	//int size;
	//struct *allocation head; //head of the Linked List
}
*/

void *malloc(size_t size) {
	
	struct allocation *chunk = create_chunk(size);

	if (linked_list->size == 0) { //if malloc hasn't been called yet basically so nothing is in linked_list

		//this int will give us the beginning of the program break (so where the heap begins.
		// Will be the pointer to the head of the linked_list (well actually we will need to add sizeof(struct)
		// to get the actual pointer to the first chunk)
		void *init_prgrm_brk = sbrk(0);
		sbrk(5000); //arbitrary size I chose.
		break_loc += 5000;
		chunk -> next_alloc = NULL;
		chunk -> prev_alloc = NULL;
		chunk -> ptr = init_prgrm_brk + sizeof(chunk) //I think this will be where the first chunk starts. At the beginning of the heap. Which is where the linked list starts + the sizeof(chunk) because that is where the actual memory allocation begins, not including the metadata
		linked_list -> head = chunk;
		linked_list -> size += 1;

	}
	else { //linked list has stuff in it
	       //if the break is far enough above the newest allocation. This actually won't work because there might be a gap in the heap that allows for an allocation without moving the break and this would still work but we don't want it to
		if (break_loc - ((chunk->prev_alloc->ptr + chunk->prev_alloc->user_size)
				+ chunk->act_size) > 0) { //if the break is far enough above the newest allocation 

			chunk = add_chunk(chunk, size); //actually adds it to the linked_list and changes all the next_alloc, prev_alloc, and ptr
		}
		else { //need to move the break up
			sbrk(5000);
			chunk = add_chunk(chunk, size);
		}
	}
	return chunk -> ptr;
}

struct allocation *add_chunk(struct allocation *chunk, size_t size) {
		struct allocation *curr = linked_list -> head;
		while (curr -> next_alloc != NULL) { //traverse through the linked list until you get to last non-NULL node
			if ((curr -> next_alloc) -> ptr - (curr -> ptr + curr -> user_size) >= size) { //if space between current and next alloc
				(curr -> next_alloc) -> prev_alloc = chunk;
				chunk -> next_alloc = curr -> next_alloc;
				curr -> next_alloc = chunk;
				chunk -> prev_alloc = curr;

				chunk -> ptr = (chunk -> prev_alloc) -> ptr + (chunk -> prev_alloc) -> user_size + sizeof(chunk);

				return chunk; //return it before it can change it to being added to end of linked list
			}
			else {
				curr = curr -> next_alloc;
			}
		}
		curr -> next_alloc = chunk;
		chunk -> prev_alloc = curr;
		struct allocation *prev = chunk -> prev_alloc;
		chunk -> ptr = prev -> ptr + prev -> user_size + sizeof(chunk); //the pointer to the new chunk should be the previous chunk's ptr plus however much it was allocated for
		chunk -> next_alloc = NULL; 

		return chunk;
}

void free(void *ptr) {
	struct allocation *chunk_to_free = linked_list->head;
	while (chunk_to_free -> ptr != ptr) {
		chunk_to_free = chunk_to_free -> next_alloc;
	}
	(chunk_to_free -> prev_alloc) -> next_alloc = chunk_to_free -> next_alloc;
	(chunk_to_free -> next_alloc) -> prev_alloc  = chunk_to_free -> prev_alloc;

	//would i start memsetting from the ptr or ptr - sizeof(chunk)
	memset(ptr - sizeof(chunk_to_free), '\0', chunk_to_free -> act_size);
}

void *calloc(size_t nelem, size_t elsize) {

}

void *realloc(void *ptr, size_t size) {

}

size_t malloc_usable_size(void *ptr) {

}


struct l_list *create_list() {
	struct l_list *linked_list;
	linked_list->size = 0;
	linked_list->head = NULL; //how do I set the head to be at the beginning of the heap
	return linked_list;
}

//make the other 4 functions and then you can just start

struct allocation *create_chunk(size_t size) {
	
	struct allocation *chunk; //how do I allocate memory to this
	
	chunk->user_size = size;
	chunk->meta_size = sizeof(chunk);
	chunk->act_size = chunk->meta_size + size;
	struct allocation *temp = linked_list -> head;


	//chunk->next_alloc = heap_list->head;
	//heap_list->head = chunk;

	return chunk;
	
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

