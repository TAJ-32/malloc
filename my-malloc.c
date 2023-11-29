#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

//is there gonna be a problem with all of these variables resetting everytime malloc is called? Or no because it's all in the heap?

static char* break_loc = 0;//sbrk(0); //should this be int or void* instead? not sure
static char* init_prgrm_brk = 0;

void *malloc(size_t size);

void free(void *ptr);

void *calloc(size_t nelem, size_t elsize);

void *realloc(void *ptr, size_t size);

size_t malloc_usable_size(void *ptr);

struct allocation *add_chunk(struct allocation *chunk, size_t size); 

struct allocation { //don't need this ptr field, struct pointer you're using will be the pointer you are using
	//char *ptr; //should this be an void pointer or char ptr because that will be size 1 byte which is what we want in the context of malloc
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

//search for free chunk and split as needed or call sbrk again

struct l_list *create_list(void);

struct allocation *create_chunk(size_t size, void *ptr);

//static struct l_list *linked_list = create_list(); //how can I allocate memory to this
						   //
static struct allocation *head = NULL;


/*
struct linked_list {
	//int size;
	//struct *allocation head; //head of the Linked List
}
*/

void *malloc(size_t size) {

	break_loc = sbrk(0);
	//struct allocation *chunk = create_chunk(size); can't do this until heap is initialized

	if (head == NULL) { //if malloc hasn't been called yet basically so nothing is in linked_list so the head is NULL
		//this int will give us the beginning of the program break (so where the heap begins)
		init_prgrm_brk = sbrk(0); //char *init_prgrm_brk = sbrk(0); //should this be char * or void * (will this change globally so I can use it in the head has been freed and I want to put something in that blank spot case?
		head = sbrk(5000); //arbitrary size I chose. the head will start as the heap itself. this is how we creat the linked list
		break_loc += 5000;
		head -> user_size = 5000 - sizeof(head); //amount of free space

		head -> next_alloc = NULL;
		head -> prev_alloc = NULL;

		//I think this will be where the first chunk starts. At the beginning of the heap. Which is where the linked list starts + the sizeof(chunk) because that is where the actual memory allocation begins, not including the metadata

		struct allocation *chunk = create_chunk(size, init_prgrm_brk + sizeof(chunk));

		chunk -> next_alloc = NULL;
		chunk -> prev_alloc = NULL;

		head = chunk;
		//linked_list -> size += 1;

		return chunk;

	}
	else { //linked list has stuff in it
		//char *bot_of_heap = head->ptr - sizeof(head); //bottom of the heap
		int amt_allocated = 0;
		struct allocation *temp = head;
		while (temp != NULL) {
			amt_allocated += temp -> user_size;
			temp = temp -> next_alloc;
		}


		if (init_prgrm_brk + amt_allocated + size > break_loc) {
			sbrk(5000);
		}

		/*
		//if the break is sufficiently greater than the amount allocated 
		if (init_prgrm_brk + amt_allocated + size < break_loc){ 
			struct allocation *chunk = add_chunk(chunk, size); //actually adds it to the linked_list and changes all the next_alloc, prev_alloc, and ptr
		}
		else { //need to move the break up
			sbrk(5000);
			struct allocation *chunk = add_chunk(chunk, size);
		}
		*/
		
		struct allocation *chunk = create_chunk(size, temp + sizeof(temp)); //this will temporarily set the chunk to some pointer that may or may not be accurate

		chunk = add_chunk(chunk, size); //this will put it in the right place

		return chunk;
	}
	return 0;
}

struct allocation *add_chunk(struct allocation *chunk, size_t size) {
		struct allocation *curr = head;
		if (((char *) head != init_prgrm_brk + sizeof(head)) && (size <= (char *) head-sizeof(head)-init_prgrm_brk)) { //if the head was freed and now the space at the beginning of the heap is open. We will put it right at the start of the heap
			head->prev_alloc = chunk;
			chunk->next_alloc = head;
			chunk->prev_alloc = NULL;

			//this will put it at beginning of prgrm brk so that every other time the if space between current and next alloc case will be used

			//how to deal with this ptr type mismatch
			chunk = (struct allocation *) init_prgrm_brk + sizeof(chunk);
			head = chunk;
			
			//chunk->ptr = chunk->next_alloc->ptr - sizeof(chunk->next_alloc) - size; //this is if you put it right before the last allocated chunk and there is null space before it so this will happen multiple times
			
			return chunk;
		}
		else {
			while (curr -> next_alloc != NULL) { //traverse through the linked list until you get to last non-NULL node
				if ((curr->next_alloc-sizeof(curr->next_alloc))-(curr+curr->user_size) >= size) { //if space between current and next alloc
					(curr -> next_alloc) -> prev_alloc = chunk;
					chunk -> next_alloc = curr -> next_alloc;
					curr -> next_alloc = chunk;
					chunk -> prev_alloc = curr;

					chunk = (chunk -> prev_alloc) + (chunk -> prev_alloc) -> user_size + sizeof(chunk);//is this necessary so that it doesn't stay null so that not every allocation equals NULL essentially

					return chunk; //return it before it can change it to being added to end of linked list
				}
				else {
					curr = curr -> next_alloc;
				}
			}
		}
		curr -> next_alloc = chunk;
		chunk -> prev_alloc = curr;
		struct allocation *prev = chunk -> prev_alloc;
		chunk = prev + prev -> user_size + sizeof(chunk); //the pointer to the new chunk should be the previous chunk's ptr plus however much it was allocated for
		chunk -> next_alloc = NULL; 

		return chunk;
}

void free(void *ptr) {
	struct allocation *chunk_to_free = head;

//	ptr = (char *) ptr;

	while (chunk_to_free != ptr) {
		chunk_to_free = chunk_to_free -> next_alloc;
	}
	
	//freeing the head case
	if (chunk_to_free == head) {
		head = head->next_alloc;
		head->prev_alloc = NULL;
		memset((char *) ptr - sizeof(chunk_to_free), '\0', chunk_to_free->act_size);

	}
	else {
		(chunk_to_free -> prev_alloc) -> next_alloc = chunk_to_free -> next_alloc;
		(chunk_to_free -> next_alloc) -> prev_alloc  = chunk_to_free -> prev_alloc;

		//are these next two lines even necessary?
		chunk_to_free->next_alloc = NULL; 
		chunk_to_free->prev_alloc = NULL;

		//should I cast the void *ptr to char *

		//would i start memsetting from the ptr or ptr - sizeof(chunk)
		memset((char *) ptr - sizeof(chunk_to_free), '\0', chunk_to_free -> act_size);
	}

	return;

}

void *calloc(size_t nelem, size_t elsize) {

	void *ptr = NULL;;
	return ptr;
}

void *realloc(void *ptr, size_t size) {

	return ptr;
}

size_t malloc_usable_size(void *ptr) {

	return 0;
}


/*
struct l_list *create_list() {
	struct l_list *linked_list;
	linked_list->size = 0;
	linked_list->head = NULL; //how do I set the head to be at the beginning of the heap
	return linked_list;
}
*/

//make the other 4 functions and then you can just start

//needs the address to actually do this
struct allocation *create_chunk(size_t size, void *ptr) {
	
	struct allocation *chunk = ptr; //how do I allocate memory to this. And should it be initialized to NULL;
	
	chunk->user_size = size;
	chunk->meta_size = sizeof(chunk);
	chunk->act_size = chunk->meta_size + size;



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
 *
 *
 *
 *(break_loc - ((chunk->prev_alloc->ptr + chunk->prev_alloc->user_size)
				+ chunk->act_size) > 0) 
 */

